/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahigram/networking/ahi_networking_common.h"
#include <vector>

namespace MTP {
class Instance;
} // namespace MTP

namespace AhiGram::Networking {

class ProxyTester final {
public:
    [[nodiscard]] static rpl::producer<ProxyCandidate> Test(
        not_null<MTP::Instance*> mtp,
        std::vector<ProxyCandidate> &&candidates);

private:
    struct Instance;
};

} // namespace AhiGram::Networking
