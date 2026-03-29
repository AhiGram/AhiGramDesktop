/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahi_networking_common.h"

#include <rpl/lifetime.h>
#include <vector>
#include <memory>

namespace base {
class Timer;
} // namespace base

namespace Main {
class Session;
} // namespace Main

namespace MTP {
class Error;
struct ProxyData;
} // namespace MTP

namespace AhiGram::Networking {

class ProxyAutobot final {
public:
    explicit ProxyAutobot(not_null<Main::Session*> session);
    ~ProxyAutobot();

    void refresh();

private:
    void fetchFromChannels();
    void startTesting(std::vector<ProxyCandidate> &&candidates);
    void applyBest(const ProxyCandidate &best);
    
    void parseSingleMessage(const MTPMessage &message, std::vector<ProxyCandidate> &candidates);
    void switchToNext();
    
    [[nodiscard]] bool isCurrentProxyOurs() const;
    void checkFetchFinished();

    const not_null<Main::Session*> _session;
    std::vector<ProxyCandidate> _workingCandidates;
    int _currentProxyIndex = -1;
    
    bool _isRefreshing = false;
    int _pendingRequests = 0;
    std::vector<ProxyCandidate> _fetchedCandidates;
    crl::time _lastApplyTime = 0;

    std::unique_ptr<base::Timer> _failoverTimer;
    rpl::lifetime _lifetime;
};

} // namespace AhiGram::Networking
