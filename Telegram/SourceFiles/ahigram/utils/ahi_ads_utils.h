/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahigram/core/ahi_storage.h"

namespace AhiGram {

[[nodiscard]] inline bool AdsDisabled() {
	return Storage::Settings::Instance().data().disableTelegramAds.current();
}

} // namespace AhiGram
