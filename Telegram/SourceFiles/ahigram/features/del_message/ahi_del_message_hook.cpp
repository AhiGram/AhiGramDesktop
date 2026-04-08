/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_del_message_hook.h"
#include "ahi_del_message_db.h"
#include "ahigram/core/ahi_storage.h"

#include "api/api_text_entities.h"
#include "data/data_peer_id.h"
#include "data/data_session.h"
#include "data/data_user.h"

#include "history/history.h"
#include "history/history_item.h"
#include "main/main_session.h"

#include <crl/crl.h>

#include <algorithm>
#include <memory>
#include <tuple>

namespace AhiGram::DelMessage {

namespace {

std::vector<char> serializeMessage(const MTPMessage &msg) {
    mtpBuffer buffer;
    msg.write(buffer);
    const auto bytes = reinterpret_cast<const char*>(buffer.constData());
    return std::vector<char>(bytes, bytes + buffer.size() * sizeof(mtpPrime));
}

std::string extractMediaType(const MTPMessageMedia &media) {
    switch (media.type()) {
    case mtpc_messageMediaPhoto:        return "photo";
    case mtpc_messageMediaDocument: {
        const auto &doc = media.c_messageMediaDocument();
        if (!doc.vdocument()) return "doc";
        const auto &d = doc.vdocument()->c_document();
        for (const auto &attr : d.vattributes().v) {
            if (attr.type() == mtpc_documentAttributeVideo) {
                const auto &v = attr.c_documentAttributeVideo();
                if (v.is_round_message()) return "round";
                return "video";
            } else if (attr.type() == mtpc_documentAttributeAudio) {
                const auto &a = attr.c_documentAttributeAudio();
                if (a.is_voice()) return "audio_voice";
                return "audio";
            } else if (attr.type() == mtpc_documentAttributeSticker) {
                return "sticker";
            } else if (attr.type() == mtpc_documentAttributeAnimated) {
                return "gif";
            }
        }
        return "doc";
    }
    case mtpc_messageMediaGeo:
    case mtpc_messageMediaGeoLive:      return "geo";
    case mtpc_messageMediaContact:      return "contact";
    case mtpc_messageMediaPoll:         return "poll";
    case mtpc_messageMediaVenue:        return "venue";
    default:                            return "";
    }
}

SavedMessage parseMessage(const MTPDmessage &d) {
    SavedMessage msg;

    msg.peer_id = peerFromMTP(d.vpeer_id()).value;
    msg.msg_id  = d.vid().v;
    msg.date    = d.vdate().v;
    msg.text    = d.vmessage().v.toStdString();

    if (const auto from = d.vfrom_id()) {
        msg.from_id = peerFromMTP(*from).value;
    }

    if (const auto rank = d.vfrom_rank()) {
        msg.from_rank = rank->v.toStdString();
    }

    if (const auto reply = d.vreply_to()) {
        if (reply->type() == mtpc_messageReplyHeader) {
            const auto &r = reply->c_messageReplyHeader();
            if (const auto id = r.vreply_to_msg_id()) {
                msg.reply_to_id = id->v;
            }
        }
    }

    if (const auto fwd = d.vfwd_from()) {
        const auto &f = fwd->c_messageFwdHeader();
        if (const auto fromId = f.vfrom_id()) {
            msg.fwd_from_id = peerFromMTP(*fromId).value;
        }
        if (const auto name = f.vfrom_name()) {
            msg.fwd_from_name = name->v.toStdString();
        }
    }

    if (const auto gid = d.vgrouped_id()) {
        msg.grouped_id = gid->v;
    }

    if (const auto media = d.vmedia()) {
        msg.media_type = extractMediaType(*media);
    }

    const auto flags = d.vflags().v;
    msg.is_out        = (flags & MTPDmessage::Flag::f_out) ? 1 : 0;
    msg.is_post       = (flags & MTPDmessage::Flag::f_post) ? 1 : 0;
    msg.is_noforwards = (flags & MTPDmessage::Flag::f_noforwards) ? 1 : 0;

    msg.is_deleted = 0;

    return msg;
}

[[nodiscard]] MTPMessage BuildMtpMessageFromItem(not_null<HistoryItem*> item) {
	if (!item->isRegular()) {
		return MTPMessage();
	}
	const auto history = item->history();
	const auto peer = history->peer;
	auto &main = history->session();

	auto flags = MTPDmessage::Flags(MTPDmessage::Flag::f_from_id);
	if (item->out()) {
		flags |= MTPDmessage::Flag::f_out;
	}

	const auto &text = item->originalText();
	const auto entities = Api::EntitiesToMTP(
		&main,
		text.entities,
		Api::ConvertOption::WithLocal);

	return MTP_message(
		MTP_flags(flags),
		MTP_int(item->id.bare),
		(item->out()
			? peerToMTP(main.userPeerId())
			: peerToMTP(item->from()->id)),
		MTPint(),
		MTPstring(),
		peerToMTP(peer->id),
		MTPPeer(),
		MTPMessageFwdHeader(),
		MTP_long(0),
		MTPlong(),
		MTPMessageReplyHeader(),
		MTP_int(item->date()),
		MTP_string(text.text),
		MTP_messageMediaEmpty(),
		MTPReplyMarkup(),
		entities,
		MTPint(),
		MTPint(),
		MTPMessageReplies(),
		MTPint(),
		MTPstring(),
		MTPlong(),
		MTPMessageReactions(),
		MTPVector<MTPRestrictionReason>(),
		MTP_int(0),
		MTPint(),
		MTPlong(),
		MTPFactCheck(),
		MTPint(),
		MTPlong(),
		MTPSuggestedPost(),
		MTPint(),
		MTPstring());
}

} // namespace 

bool shouldSaveDeletedMessages() {
	return AhiGram::Storage::Settings::Instance().data().saveDelMessage.current();
}

bool shouldLoadDeletedMessages() {
	return AhiGram::Storage::Settings::Instance().data().loadDelMessage.current();
}

void saveSnapshotFromItem(not_null<HistoryItem*> item) {
	Database::Instance().init();
	if (!shouldSaveDeletedMessages()) {
		return;
	}
	if (!item->isRegular() || item->out()) {
		return;
	}
	const auto peerId = item->history()->peer->id;
	if (!peerIsUser(peerId) || item->history()->peer->isBot()) {
		return;
	}
	const auto mtp = BuildMtpMessageFromItem(item);
	if (mtp.type() != mtpc_message) {
		return;
	}
	const auto &d = mtp.c_message();
	auto saved = parseMessage(d);
	saved.owner_id = item->history()->session().userPeerId().value;
	saved.raw_mtp = serializeMessage(mtp);
	saved.is_deleted = 1;
	Database::Instance().upsertMessage(saved);
}

} // namespace AhiGram::DelMessage

namespace {

[[nodiscard]] MTPMessage DeserializeSavedMtp(const std::vector<char> &raw_mtp) {
	const auto primes = reinterpret_cast<const mtpPrime*>(raw_mtp.data());
	const auto count = raw_mtp.size() / sizeof(mtpPrime);
	auto from = primes;
	const auto end = primes + count;
	MTPMessage msg;
	if (!msg.read(from, end)) {
		return MTPMessage();
	}
	return msg;
}

struct RestoreState final {
	std::vector<AhiGram::DelMessage::SavedMessage> messages;
	int index = 0;
};

void ProcessRestoreBatch(
		not_null<Data::Session*> session,
		const std::shared_ptr<RestoreState> &state) {
	constexpr auto kBatch = 32;
	const auto &messages = state->messages;
	const auto n = int(messages.size());
	const auto end = std::min(state->index + kBatch, n);
	for (; state->index < end; ++state->index) {
		const auto &saved = messages[state->index];
		if (saved.raw_mtp.empty()) {
			continue;
		}
		const auto peerId = PeerId(saved.peer_id);
		if (!peerIsUser(peerId)) {
			continue;
		}
		const auto mtp = DeserializeSavedMtp(saved.raw_mtp);
		if (mtp.type() != mtpc_message) {
			continue;
		}
		session->history(peerId)->ahiRestoreDeletedMessage(mtp);
	}
	if (state->index < n) {
		crl::on_main(&session->session(), [=] {
			ProcessRestoreBatch(session, state);
		});
	} else {
		session->sendHistoryChangeNotifications();
	}
}

} // namespace

namespace AhiGram::DelMessage {

void restoreDeletedPrivateMessages(not_null<Data::Session*> session) {
	Database::Instance().init();
	if (!shouldLoadDeletedMessages()) {
		return;
	}
	const auto ownerId = session->session().userPeerId().value;
	auto all = Database::Instance().getAllDeletedUserMessages(ownerId);
	if (all.empty()) {
		return;
	}
	std::sort(all.begin(), all.end(), [](const SavedMessage &a, const SavedMessage &b) {
		return std::tie(a.peer_id, a.msg_id) < std::tie(b.peer_id, b.msg_id);
	});
	auto state = std::make_shared<RestoreState>();
	state->messages = std::move(all);
	crl::on_main(&session->session(), [=] {
		ProcessRestoreBatch(session, state);
	});
}

void restoreDeletedPrivateMessagesForPeer(
		not_null<Data::Session*> session,
		PeerId peerId) {
	Database::Instance().init();
	if (!shouldLoadDeletedMessages()) {
		return;
	}
	if (!peerIsUser(peerId)) {
		return;
	}
	const auto ownerId = session->session().userPeerId().value;
	const auto peerKey = static_cast<int64_t>(peerId.value);
	auto rows = Database::Instance().getDeletedUserMessagesForPeer(ownerId, peerKey);
	if (rows.empty()) {
		return;
	}
	for (const auto &saved : rows) {
		if (saved.raw_mtp.empty()) {
			continue;
		}
		const auto mtp = DeserializeSavedMtp(saved.raw_mtp);
		if (mtp.type() != mtpc_message) {
			continue;
		}
		session->history(peerId)->ahiRestoreDeletedMessage(mtp);
	}
	session->sendHistoryChangeNotifications();
}

} // namespace AhiGram::DelMessage
