/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_invisible_mode.h"
#include "ahigram/core/ahi_storage.h"

namespace AhiGram::InvisibleMode {

bool IsEnabled() {
	return AhiGram::Storage::Settings::Instance().data().invisibleModeEnabled.current();
}

} // namespace AhiGram::InvisibleMode
