/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_proxy_autobot.h"
#include "ahi_proxy_parser.h"
#include "ahi_proxy_tester.h"
#include "ahigram/networking/ahi_proxy_utils.h"
#include "ahigram/core/ahi_storage.h"

#include "base/timer.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "main/main_account.h"
#include "apiwrap.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "mtproto/mtproto_proxy_data.h"

#ifdef _DEBUG
#define AHI_LOG(x) LOG(x)
#else
#define AHI_LOG(x)
#endif

namespace AhiGram::Networking {

ProxyAutobot::ProxyAutobot(not_null<Main::Session*> session)
    : _remoteLoader(std::make_unique<AhiGram::Api::RemoteProxyListLoader>())
    , _session(session) {

    _session->account().mtp().restartsByTimeout()
    | rpl::on_next([=] {
        if (AhiGram::Storage::Settings::Instance().data().ahiBypass.current()) {
            AHI_LOG(("AhiGram: MTP restart by timeout detected."));
            switchToNext();
        }
    }, _lifetime);

    AhiGram::Storage::Settings::Instance().data().ahiBypass.changes()
    | rpl::on_next([=](bool enabled) {
        if (enabled) {
            AHI_LOG(("AhiGram: ahiBypass enabled, triggering immediate refresh."));
            refresh();
        }
    }, _lifetime);

    _failoverTimer = std::make_unique<base::Timer>([this] {
        if (!AhiGram::Storage::Settings::Instance().data().ahiBypass.current()) {
            return;
        }

        const auto now = crl::now();
        if (_lastApplyTime > 0 && now - _lastApplyTime < 10000) {
            return;
        }

        const auto state = _session->account().mtp().dcstate(0);
        if (state == MTP::DisconnectedState || state == MTP::ConnectingState) {
            AHI_LOG(("AhiGram: Connection state %1 is not Ready, checking failover...").arg(state));
            switchToNext();
        }
    });

    _failoverTimer->callEach(Constants::kFailoverCheckIntervalMs);
}

ProxyAutobot::~ProxyAutobot() = default;

void ProxyAutobot::refresh() {
    if (_isRefreshing) return;
    if (!AhiGram::Storage::Settings::Instance().data().ahiBypass.current()) {
        return;
    }

    AHI_LOG(("AhiGram: ProxyAutobot refresh started..."));
    _isRefreshing = true;
    _fetchedCandidates.clear();

    fetchRemoteList();
}

void ProxyAutobot::fetchRemoteList() {
    _remoteLoader->cancel();
    _remoteLoader->load(
        Constants::kRemoteProxyListUrl,
        [=](std::vector<ProxyCandidate> list) {
            for (auto &c : list) {
                if (MTP::ProxyData::ValidMtprotoPassword(c.secret)) {
                    _fetchedCandidates.push_back(std::move(c));
                }
            }
            afterRemoteFetchFinished();
        },
        [=] {
            AHI_LOG(("AhiGram: Remote proxy list request failed"));
            afterRemoteFetchFinished();
        });
}

void ProxyAutobot::afterRemoteFetchFinished() {
    const auto &channelUsernames = AhiGram::Storage::Settings::Instance()
        .data().proxyChannels.current();
    const auto channelCount = channelUsernames.size();
    if (shouldFetchChannels() && channelCount > 0) {
        _pendingRequests = channelCount;
        fetchFromChannels();
    } else {
        finalizeFetchAndTest();
    }
}

bool ProxyAutobot::shouldFetchChannels() const {
    if (_session->account().mtp().dcstate(0) != MTP::ConnectedState) {
        return false;
    }
    const auto &proxySettings = Core::App().settings().proxy();
    if (!proxySettings.isEnabled()) {
        return false;
    }
    const auto &current = proxySettings.selected();
    return current.type == MTP::ProxyData::Type::Mtproto;
}

void ProxyAutobot::finalizeFetchAndTest() {
    if (!_fetchedCandidates.empty()) {
        std::sort(_fetchedCandidates.begin(), _fetchedCandidates.end(), [](const ProxyCandidate &a, const ProxyCandidate &b) {
            if (a.host != b.host) return a.host < b.host;
            return a.port < b.port;
        });
        _fetchedCandidates.erase(std::unique(_fetchedCandidates.begin(), _fetchedCandidates.end(), [](const ProxyCandidate &a, const ProxyCandidate &b) {
            return a.host == b.host && a.port == b.port;
        }), _fetchedCandidates.end());

        startTesting(std::move(_fetchedCandidates));
    } else {
        _isRefreshing = false;
    }
}

void ProxyAutobot::fetchFromChannels() {
	const auto &channelUsernames = AhiGram::Storage::Settings::Instance()
		.data().proxyChannels.current();
	for (const auto &channelUsername : channelUsernames) {
		_session->api().request(MTPcontacts_ResolveUsername(
			MTP_flags(0),
			MTP_string(channelUsername),
			MTP_string()
		)).done([=](const MTPcontacts_ResolvedPeer &result) {
			const auto &data = result.c_contacts_resolvedPeer();
			if (data.vchats().v.empty()) {
				checkFetchFinished();
				return;
			}

			data.vchats().v.front().match([&](const MTPDchannel &channel) {
				if (!channel.vaccess_hash()) {
					checkFetchFinished();
					return;
				}
				const auto inputPeer = MTP_inputPeerChannel(channel.vid(), *channel.vaccess_hash());
				_session->api().request(MTPmessages_GetHistory(
					inputPeer,
					MTP_int(0), MTP_int(0), MTP_int(0),
					MTP_int(Constants::kHistoryLimit),
					MTP_int(0), MTP_int(0), MTP_long(0)
				)).done([=](const MTPmessages_Messages &res) {
					res.match([&](const MTPDmessages_messagesNotModified &) {
					}, [&](const auto &data) {
						for (const auto &message : data.vmessages().v) {
							parseSingleMessage(message, _fetchedCandidates);
						}
					});
					checkFetchFinished();
				}).fail([=](const MTP::Error &error) {
					checkFetchFinished();
				}).send();
			}, [=](const auto &) {
				checkFetchFinished();
			});
		}).fail([=](const MTP::Error &error) {
			checkFetchFinished();
		}).send();
	}
}

void ProxyAutobot::checkFetchFinished() {
    _pendingRequests--;
    if (_pendingRequests <= 0) {
        finalizeFetchAndTest();
    }
}

void ProxyAutobot::parseSingleMessage(const MTPMessage &message, std::vector<ProxyCandidate> &candidates) {
    message.match([&](const MTPDmessage &m) {
        auto fromText = ProxyParser::Parse(qs(m.vmessage()));
        for (auto &c : fromText) {
            if (MTP::ProxyData::ValidMtprotoPassword(c.secret)) candidates.push_back(std::move(c));
        }
        if (const auto markup = m.vreply_markup()) {
            markup->match([&](const MTPDreplyInlineMarkup &im) {
                for (const auto &row : im.vrows().v) {
                    row.match([&](const MTPDkeyboardButtonRow &r) {
                        for (const auto &btn : r.vbuttons().v) {
                            btn.match([&](const MTPDkeyboardButtonUrl &u) {
                                auto c = ProxyParser::FromLink(qs(u.vurl()));
                                if (c.isValid() && MTP::ProxyData::ValidMtprotoPassword(c.secret)) {
                                    candidates.push_back(std::move(c));
                                }
                            }, [](const auto&) {});
                        }
                    });
                }
            }, [](const auto&) {});
        }
    }, [](const auto&) {});
}

void ProxyAutobot::startTesting(std::vector<ProxyCandidate> &&candidates) {
    AHI_LOG(("AhiGram: Starting parallel testing for %1 unique proxies...").arg(candidates.size()));

    struct LocalResults {
        std::vector<ProxyCandidate> list;
        bool appliedFirst = false;
    };
    auto local = std::make_shared<LocalResults>();

    ProxyTester::Test(&_session->account().mtp(), std::move(candidates))
    | rpl::on_next_done([=](const ProxyCandidate &res) {
        if (res.ping >= 0 && res.ping < Constants::kFailedPing) {
            local->list.push_back(res);

            const auto state = _session->account().mtp().dcstate(0);
            const bool currentBroken = (state != MTP::ConnectedState);

            if ((!local->appliedFirst || currentBroken) && res.ping < 1000) {
                AHI_LOG(("AhiGram: Fast apply proxy: %1 (ping: %2)").arg(res.host).arg(res.ping));
                local->appliedFirst = true;
                applyBest(res);
            }
        }
    }, [=] {
        _isRefreshing = false;
        AHI_LOG(("AhiGram: Unified testing finished. Found %1 working proxies.").arg(local->list.size()));

        if (!local->list.empty()) {
            std::sort(local->list.begin(), local->list.end(), [=](const ProxyCandidate &a, const ProxyCandidate &b) {
                auto weightA = static_cast<double>(a.ping);
                auto weightB = static_cast<double>(b.ping);
                if (a.isFakeTls) weightA *= Constants::kFakeTlsWeightBonus;
                if (b.isFakeTls) weightB *= Constants::kFakeTlsWeightBonus;
                return weightA < weightB;
            });

            _workingCandidates = std::move(local->list);
            _currentProxyIndex = 0;

            const auto &best = _workingCandidates[0];
            const auto &current = Core::App().settings().proxy().selected();

            if (current.host != best.host || current.port != best.port) {
                applyBest(best);
            }
        }
    }, _lifetime);
}

void ProxyAutobot::applyBest(const ProxyCandidate &best) {
    _lastApplyTime = crl::now();
    ApplyProxyCandidate(best, _session);
}

void ProxyAutobot::switchToNext() {
    if (_workingCandidates.empty()) {
        if (AhiGram::Storage::Settings::Instance().data().ahiBypass.current()) {
            refresh();
        }
        return;
    }
    if (!isCurrentProxyOurs()) return;

    _currentProxyIndex++;
    if (_currentProxyIndex >= static_cast<int>(_workingCandidates.size())) {
        refresh();
        return;
    }

    AHI_LOG(("AhiGram: Failover switching to candidate %1/%2").arg(_currentProxyIndex + 1).arg(_workingCandidates.size()));
    applyBest(_workingCandidates[_currentProxyIndex]);
}

bool ProxyAutobot::isCurrentProxyOurs() const {
    const auto &proxySettings = Core::App().settings().proxy();
    if (!proxySettings.isEnabled()) return false;
    const auto &current = proxySettings.selected();
    for (const auto &c : _workingCandidates) {
        if (c.host == current.host && c.port == current.port) return true;
    }
    return false;
}

} // namespace AhiGram::Networking
