/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"
#include "data/data_peer_id.h"

namespace Data {
class Session;
} // namespace Data

class HistoryItem;

namespace AhiGram::DelMessage {

[[nodiscard]] bool shouldSaveDeletedMessages();
[[nodiscard]] bool shouldLoadDeletedMessages();

void saveSnapshotFromItem(not_null<HistoryItem*> item);

void restoreDeletedPrivateMessages(not_null<Data::Session*> session);
void restoreDeletedPrivateMessagesForPeer(
	not_null<Data::Session*> session,
	PeerId peerId);

} // namespace AhiGram::DelMessage
