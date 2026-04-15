/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_del_message_hook.h"
#include "ahi_del_message_db.h"
#include "ahi_del_message_helpers.h"
#include "ahigram/utils/ahi_mtp_utils.h"
#include "ahigram/core/ahi_storage.h"

#include "data/data_peer_id.h"
#include "data/data_session.h"
#include "history/history.h"
#include "history/history_item.h"
#include "main/main_session.h"

#include <crl/crl.h>
#include <algorithm>

namespace AhiGram::DelMessage {
namespace {

struct RestoreState final {
	std::vector<SavedMessage> messages;
	int index = 0;
};

void QueueMessage(not_null<Data::Session*> session, const SavedMessage &saved) {
	if (saved.raw_mtp.empty()) {
		return;
	}
	const auto peerId = PeerId(saved.peer_id);
	if (!peerIsUser(peerId)) {
		return;
	}
	const auto mtp = Utils::DeserializeMtpMessage(saved.raw_mtp);
	if (mtp.type() != mtpc_message) {
		return;
	}
	session->history(peerId)->ahiQueueDeletedMessage(mtp);
}

void ProcessRestoreBatch(
		not_null<Data::Session*> session,
		const std::shared_ptr<RestoreState> &state) {
	constexpr auto kBatch = 32;
	const auto &messages = state->messages;
	const auto n = int(messages.size());
	const auto end = std::min(state->index + kBatch, n);

	for (; state->index < end; ++state->index) {
		QueueMessage(session, messages[state->index]);
	}

	if (state->index < n) {
		crl::on_main(&session->session(), [=] {
			ProcessRestoreBatch(session, state);
		});
	} else {
		session->sendHistoryChangeNotifications();
	}
}

bool IsSaveable(not_null<HistoryItem*> item) {
	if (!shouldSaveDeletedMessages()) {
		return false;
	}
	if (!item->isRegular() || item->out()) {
		return false;
	}
	const auto peer = item->history()->peer;
	if (peer->isSelf()) {
		return false;
	}
	if (item->from()->id == item->history()->session().userPeerId()) {
		return false;
	}
	return peerIsUser(peer->id) && !peer->isBot();
}

} // namespace

bool shouldSaveDeletedMessages() {
	return AhiGram::Storage::Settings::Instance().data().saveDelMessage.current();
}

bool shouldLoadDeletedMessages() {
	return AhiGram::Storage::Settings::Instance().data().loadDelMessage.current();
}

void saveSnapshotFromItem(not_null<HistoryItem*> item) {
	if (!IsSaveable(item)) {
		return;
	}
	const auto mtp = BuildMtpFromHistoryItem(item);
	if (mtp.type() != mtpc_message) {
		return;
	}
	auto saved = MtpToSavedMessage(
		mtp.c_message(),
		item->history()->session().userPeerId());
	saved.raw_mtp = Utils::SerializeMtpMessage(mtp);
	saved.is_deleted = 1;
	Database::Instance().upsertMessage(saved);
}

void restoreDeletedPrivateMessages(not_null<Data::Session*> session) {
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
	if (!shouldLoadDeletedMessages() || !peerIsUser(peerId)) {
		return;
	}
	const auto ownerId = session->session().userPeerId().value;
	const auto peerKey = static_cast<int64_t>(peerId.value);
	const auto rows = Database::Instance().getDeletedUserMessagesForPeer(ownerId, peerKey);
	if (rows.empty()) {
		return;
	}
	for (const auto &saved : rows) {
		QueueMessage(session, saved);
	}
	session->sendHistoryChangeNotifications();
}

} // namespace AhiGram::DelMessage
