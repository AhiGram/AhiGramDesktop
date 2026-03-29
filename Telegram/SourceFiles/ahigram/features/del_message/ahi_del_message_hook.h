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

namespace AhiGram::DelMessage {

[[nodiscard]] bool shouldSaveDeletedMessages();
[[nodiscard]] bool shouldLoadDeletedMessages();

void onNewMessage(const MTPMessage &msg);
void onDeleteMessages(const QVector<MTPint> &ids);
void onDeleteChannelMessages(PeerId peerId, const QVector<MTPint> &ids);
MTPMessage deserializeMessage(const std::vector<char> &raw_mtp);

void restoreDeletedPrivateMessages(not_null<Data::Session*> session);

} // namespace AhiGram::DelMessage
