/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_del_message_helpers.h"

#include "api/api_text_entities.h"
#include "data/data_document.h"
#include "data/data_media_types.h"
#include "data/data_peer_id.h"
#include "data/data_session.h"
#include "history/history.h"
#include "history/history_item.h"
#include "main/main_session.h"

namespace AhiGram::DelMessage {
namespace {

std::string ExtractMediaType(const MTPMessageMedia &media) {
	return media.match([&](const MTPDmessageMediaPhoto &data) {
		return "photo";
	}, [&](const MTPDmessageMediaDocument &data) {
		const auto &doc = data.vdocument();
		if (!doc || doc->type() != mtpc_document) return "doc";
		const auto &d = doc->c_document();
		for (const auto &attr : d.vattributes().v) {
			if (attr.type() == mtpc_documentAttributeVideo) {
				return attr.c_documentAttributeVideo().is_round_message() ? "round" : "video";
			} else if (attr.type() == mtpc_documentAttributeAudio) {
				return attr.c_documentAttributeAudio().is_voice() ? "audio_voice" : "audio";
			} else if (attr.type() == mtpc_documentAttributeSticker) {
				return "sticker";
			} else if (attr.type() == mtpc_documentAttributeAnimated) {
				return "gif";
			}
		}
		return "doc";
	}, [&](const MTPDmessageMediaGeo &data) {
		return "geo";
	}, [&](const MTPDmessageMediaGeoLive &data) {
		return "geo";
	}, [&](const MTPDmessageMediaContact &data) {
		return "contact";
	}, [&](const MTPDmessageMediaPoll &data) {
		return "poll";
	}, [&](const MTPDmessageMediaVenue &data) {
		return "venue";
	}, [](auto &&) {
		return "";
	});
}

QVector<MTPDocumentAttribute> BuildDocumentAttributes(
		not_null<DocumentData*> document) {
	auto attributes = QVector<MTPDocumentAttribute>();
	const auto &dims = document->dimensions;
	const auto hasDims = (dims.width() > 0 && dims.height() > 0);
	
	if (document->isAnimation()) {
		attributes.push_back(MTP_documentAttributeAnimated());
		if (hasDims && document->hasDuration()) {
			attributes.push_back(MTP_documentAttributeVideo(
				MTP_flags(0),
				MTP_double(document->duration() / 1000.),
				MTP_int(dims.width()),
				MTP_int(dims.height()),
				MTPint(),
				MTPdouble(),
				MTPstring()));
		}
	} else if (const auto sticker = document->sticker()) {
		attributes.push_back(MTP_documentAttributeSticker(
			MTP_flags(0),
			MTP_string(sticker->alt),
			MTP_inputStickerSetEmpty(),
			MTPMaskCoords()));
		if (hasDims) {
			attributes.push_back(MTP_documentAttributeImageSize(
				MTP_int(dims.width()),
				MTP_int(dims.height())));
		}
	}
	
	const auto filename = document->filename();
	if (!filename.isEmpty()) {
		attributes.push_back(MTP_documentAttributeFilename(
			MTP_string(filename)));
	}
	
	return attributes;
}

} // namespace

SavedMessage MtpToSavedMessage(const MTPDmessage &d, PeerId ownerId) {
	SavedMessage msg;
	msg.owner_id = ownerId.value;
	msg.peer_id = peerFromMTP(d.vpeer_id()).value;
	msg.msg_id = d.vid().v;
	msg.date = d.vdate().v;
	msg.text = d.vmessage().v.toStdString();

	if (const auto from = d.vfrom_id()) {
		msg.from_id = peerFromMTP(*from).value;
	}
	if (const auto rank = d.vfrom_rank()) {
		msg.from_rank = rank->v.toStdString();
	}
	if (const auto reply = d.vreply_to()) {
		reply->match([&](const MTPDmessageReplyHeader &data) {
			if (const auto id = data.vreply_to_msg_id()) {
				msg.reply_to_id = id->v;
			}
		}, [](auto &&) {});
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
		msg.media_type = ExtractMediaType(*media);
	}

	const auto flags = d.vflags().v;
	msg.is_out = (flags & MTPDmessage::Flag::f_out) ? 1 : 0;
	msg.is_post = (flags & MTPDmessage::Flag::f_post) ? 1 : 0;
	msg.is_noforwards = (flags & MTPDmessage::Flag::f_noforwards) ? 1 : 0;
	msg.is_deleted = 0;

	return msg;
}

MTPMessage BuildMtpFromHistoryItem(not_null<HistoryItem*> item) {
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

	auto media = MTP_messageMediaEmpty();
	if (const auto itemMedia = item->media()) {
		if (const auto document = itemMedia->document()) {
			if (document->isAnimation() || document->sticker()) {
				flags |= MTPDmessage::Flag::f_media;
				
				const auto attributes = BuildDocumentAttributes(document);
				
				using Flag = MTPDmessageMediaDocument::Flag;
				media = MTP_messageMediaDocument(
					MTP_flags(Flag::f_document),
					MTP_document(
						MTP_flags(0),
						MTP_long(document->id),
						MTP_long(document->_access),
						MTP_bytes(document->_fileReference),
						MTP_int(document->date),
						MTP_string(document->mimeString()),
						MTP_long(document->size),
						MTP_vector<MTPPhotoSize>(),
						MTPVector<MTPVideoSize>(),
						MTP_int(document->_dc),
						MTP_vector<MTPDocumentAttribute>(attributes)),
					MTPVector<MTPDocument>(),
					MTPPhoto(),
					MTPint(),
					MTPint());
			}
		}
	}

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
		MTPPeer(),
		MTPMessageReplyHeader(),
		MTP_int(item->date()),
		MTP_string(text.text),
		media,
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

} // namespace AhiGram::DelMessage
