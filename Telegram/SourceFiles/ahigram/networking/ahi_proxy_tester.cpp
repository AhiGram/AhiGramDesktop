/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_proxy_tester.h"

#include "mtproto/connection_abstract.h"
#include "mtproto/mtp_instance.h"

#include <QtCore/QTimer>
#include <QtCore/QThread>

#ifdef _DEBUG
#define AHI_LOG(x) LOG(x)
#else
#define AHI_LOG(x)
#endif

namespace AhiGram::Networking {

struct ProxyTester::Instance final : public std::enable_shared_from_this<ProxyTester::Instance> {
    Instance(not_null<MTP::Instance*> mtp, std::vector<ProxyCandidate> &&list)
        : mtp(mtp), queue(std::move(list)) {
    }

    const not_null<MTP::Instance*> mtp;
    std::vector<ProxyCandidate> queue;
    int nextIndex = 0;
    int activeCount = 0;
    
    struct ActiveTest {
        int index;
        int retry;
        qint64 firstPing;
        MTP::details::ConnectionPointer connection;
    };
    base::flat_map<MTP::details::AbstractConnection*, std::unique_ptr<ActiveTest>> activeTests;

    rpl::event_stream<ProxyCandidate> results;
    rpl::lifetime lifetime;

    void start() {
        while (activeCount < Constants::kMaxConcurrentTests && startNext()) {
        }
    }

    bool startNext() {
        if (nextIndex >= static_cast<int>(queue.size())) return false;

        const int index = nextIndex++;
        activeCount++;
        
        auto test = std::make_unique<ActiveTest>();
        test->index = index;
        test->retry = 0;
        test->firstPing = 0;
        
        startSubTest(std::move(test));
        return true;
    }

    void startSubTest(std::unique_ptr<ActiveTest> test) {
        const auto &candidate = queue[test->index];
        AHI_LOG(("AhiGram: Testing proxy %1/%2 (Attempt %3): %4:%5")
            .arg(test->index + 1)
            .arg(queue.size())
            .arg(test->retry + 1)
            .arg(candidate.host)
            .arg(candidate.port));

        MTP::ProxyData proxy;
        proxy.host = candidate.host;
        proxy.port = candidate.port;
        proxy.password = candidate.secret;
        proxy.type = MTP::ProxyData::Type::Mtproto;

        const auto secret = proxy.secretFromMtprotoPassword();
        if (!secret.empty() 
            && secret.size() != 16 
            && !(secret.size() == 17 && secret[0] == bytes::type(0xDD))
            && !(secret.size() >= 21 && secret[0] == bytes::type(0xEE))) {
            AHI_LOG(("AhiGram: Proxy %1 has invalid secret format, skipping").arg(test->index + 1));
            auto &candidate = queue[test->index];
            candidate.ping = Constants::kFailedPing;
            results.fire_copy(candidate);
            finishOne();
            return;
        }

        const auto protocol = MTP::DcOptions::Variants::Tcp;
        const auto dcId = mtp->mainDcId();

        test->connection = MTP::details::AbstractConnection::Create(
            mtp, protocol, QThread::currentThread(), secret, proxy);

        auto self = shared_from_this();
        auto pointer = test->connection.get();
        activeTests[pointer] = std::move(test);

        QObject::connect(pointer, &MTP::details::AbstractConnection::connected, [this, self, pointer] {
            handleConnected(pointer);
        });

        auto failed = [this, self, pointer](auto &&...args) {
            handleFailed(pointer);
        };
        QObject::connect(pointer, &MTP::details::AbstractConnection::disconnected, failed);
        QObject::connect(pointer, &MTP::details::AbstractConnection::error, failed);

        pointer->connectToServer(candidate.host, candidate.port, secret, dcId, false);

        auto currentRetry = activeTests[pointer]->retry;
        QTimer::singleShot(Constants::kPingTimeoutMs, [this, self, pointer, currentRetry] {
            if (activeTests.contains(pointer) && activeTests[pointer]->retry == currentRetry) {
                AHI_LOG(("AhiGram: Proxy %1 timeout").arg(activeTests[pointer]->index + 1));
                handleFailed(pointer);
            }
        });
    }

    void handleConnected(MTP::details::AbstractConnection* pointer) {
        if (!activeTests.contains(pointer)) return;
        auto test = std::move(activeTests[pointer]);
        activeTests.erase(pointer);

        const auto p = pointer->pingTime();
        if (test->retry == 0) {
            test->firstPing = p;
            test->retry++;
            test->connection = nullptr;
            auto self = shared_from_this();
            QTimer::singleShot(Constants::kRetryDelayMs, [this, self, test = std::move(test)]() mutable {
                startSubTest(std::move(test));
            });
        } else {
            auto &candidate = queue[test->index];
            candidate.ping = (test->firstPing + p) / 2;
            AHI_LOG(("AhiGram: Proxy %1 stable, avg ping: %2")
                .arg(test->index + 1)
                .arg(candidate.ping));
            results.fire_copy(candidate);
            finishOne();
        }
    }

    void handleFailed(MTP::details::AbstractConnection* pointer) {
        if (!activeTests.contains(pointer)) return;
        auto test = std::move(activeTests[pointer]);
        activeTests.erase(pointer);

        auto &candidate = queue[test->index];
        candidate.ping = Constants::kFailedPing;
        AHI_LOG(("AhiGram: Proxy %1 failed").arg(test->index + 1));
        results.fire_copy(candidate);
        finishOne();
    }

    void finishOne() {
        activeCount--;
        if (!startNext() && activeCount == 0) {
            AHI_LOG(("AhiGram: All proxies tested."));
            results.fire_done();
        }
    }
};

rpl::producer<ProxyCandidate> ProxyTester::Test(
    not_null<MTP::Instance*> mtp,
    std::vector<ProxyCandidate> &&candidates) {
    if (candidates.empty()) return rpl::never<ProxyCandidate>();
    
    return rpl::make_producer<ProxyCandidate>([mtp, list = std::move(candidates)](auto consumer) mutable {
        auto instance = std::make_shared<Instance>(mtp, std::move(list));

        instance->results.events()
        | rpl::on_next_done([consumer](ProxyCandidate &&res) {
            consumer.put_next(std::move(res));
        }, [consumer] {
            consumer.put_done();
        }, instance->lifetime);

        instance->start();
        
        return rpl::lifetime([instance] {
            for (auto &pair : instance->activeTests) {
                pair.second->connection = nullptr;
            }
            instance->activeTests.clear();
            instance->lifetime.destroy();
        });
    });
}

} // namespace AhiGram::Networking
