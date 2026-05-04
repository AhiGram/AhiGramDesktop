/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <QtCore/QString>
#include <set>

namespace AhiGram::Networking {

struct ProxyCandidate {
    QString host;
    int port = 0;
    QString secret;
    qint64 ping = -1;
    bool isFakeTls = false;

    [[nodiscard]] bool isValid() const {
        return !host.isEmpty() && port > 0;
    }
};

namespace Constants {

inline const QString kApiDomain = u"https://ahigram.online"_q;

constexpr auto kIpCheckTimeout = 5000;
constexpr auto kHistoryLimit = 20;
constexpr auto kFailedPing = 9999;
constexpr auto kPingTimeoutMs = 4000;
constexpr auto kFailoverCheckIntervalMs = 30000;
constexpr auto kRetryDelayMs = 50;
constexpr auto kMaxConcurrentTests = 10;

constexpr auto kFakeTlsWeightBonus = 0.7;

constexpr auto kMaxProxyCount = 20;
constexpr auto kProxyCleanupBatch = 8;

} // namespace Constants

namespace Badges {

inline const std::set<uint64> kAhiVerifiedUserIds = {
    8718631251, 8202660494
};

inline const std::set<uint64> kAhiVerifiedChannelIds = {
    3297989627, 3291032260, 3570237114, 3948544020, 
    3792934639, 3139108773, 3218775843, 3309132628,
    3122145798
};

} // namespace Badges

} // namespace AhiGram::Networking
