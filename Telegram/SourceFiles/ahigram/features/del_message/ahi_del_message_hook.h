/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"

namespace Data {
class Session;
} // namespace Data

class HistoryItem;

namespace AhiGram::DelMessage {

[[nodiscard]] bool shouldSaveDeletedMessages();
[[nodiscard]] bool shouldLoadDeletedMessages();

void saveSnapshotFromItem(not_null<HistoryItem*> item);
MTPMessage deserializeMessage(const std::vector<char> &raw_mtp);

void restoreDeletedPrivateMessages(not_null<Data::Session*> session);

} // namespace AhiGram::DelMessage
