/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <QtCore/QLocale>

namespace AhiGram::Utils {

[[nodiscard]] inline bool IsRestrictedRegion() {
    const auto country = QLocale::system().country();
    return (country == QLocale::Russia)
        || (country == QLocale::Iran);
}

[[nodiscard]] inline bool IsRestrictedRegionCode(const QString &code) {
    const auto upper = code.trimmed().toUpper();
    return (upper == u"RU"_q)
        || (upper == u"IR"_q);
}

} // namespace AhiGram::Utils
