/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <QtCore/QString>

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

inline const QString kRemoteProxyListUrl = u"https://ahigram.online/proxy/list"_q;
inline const QString kIpCheckUrl = u"http://ip-api.com/line/?fields=countryCode"_q;

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
} // namespace AhiGram::Networking
