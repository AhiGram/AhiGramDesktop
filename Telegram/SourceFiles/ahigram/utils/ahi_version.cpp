/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/utils/ahi_version.h"

#include "core/version.h"

namespace AhiGram {

QString GetAppVersion() {
	return QString::number(AppVersion);
}

QString GetAppVersionString() {
	return u"v"_q + QString::fromLatin1(AppVersionStr);
}

} // namespace AhiGram
