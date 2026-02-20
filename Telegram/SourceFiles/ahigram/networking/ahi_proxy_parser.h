/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahigram/networking/ahi_networking_common.h"
#include <vector>
#include <QString>

namespace AhiGram::Networking {

class ProxyParser final {
public:
    [[nodiscard]] static std::vector<ProxyCandidate> Parse(const QString &text);
    [[nodiscard]] static ProxyCandidate FromLink(const QString &link);
};

} // namespace AhiGram::Networking
