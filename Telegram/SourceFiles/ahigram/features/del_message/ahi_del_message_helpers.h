/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahi_del_message_db.h"
#include "scheme.h"

namespace AhiGram::DelMessage {

[[nodiscard]] SavedMessage MtpToSavedMessage(
	const MTPDmessage &d,
	PeerId ownerId);

[[nodiscard]] MTPMessage BuildMtpFromHistoryItem(not_null<HistoryItem*> item);

} // namespace AhiGram::DelMessage
