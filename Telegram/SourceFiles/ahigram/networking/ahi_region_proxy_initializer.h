/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahigram/networking/ahi_networking_common.h"
#include <rpl/lifetime.h>

namespace AhiGram::Api {
class RemoteProxyListLoader;
} // namespace AhiGram::Api

namespace AhiGram::Networking {

class RegionProxyInitializer final {
public:
    RegionProxyInitializer();
    ~RegionProxyInitializer();

    void start();

private:
    void fetchRemoteList();
    void startTesting(std::vector<ProxyCandidate> &&candidates);

    std::unique_ptr<AhiGram::Api::RemoteProxyListLoader> _remoteLoader;
    rpl::lifetime _lifetime;
};

} // namespace AhiGram::Networking
