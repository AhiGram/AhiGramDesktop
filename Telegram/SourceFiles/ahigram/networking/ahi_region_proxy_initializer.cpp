/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_region_proxy_initializer.h"
#include "ahi_proxy_tester.h"
#include "ahi_proxy_utils.h"
#include "ahigram/api/ahi_proxy_remote_api.h"
#include "ahigram/api/ahi_region_api.h"
#include "ahigram/utils/ahi_region_utils.h"
#include "ahigram/core/ahi_storage.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "main/main_account.h"
#include "main/main_domain.h"

namespace AhiGram::Networking {

RegionProxyInitializer::RegionProxyInitializer()
: _regionChecker(std::make_unique<AhiGram::Api::RegionChecker>())
, _remoteLoader(std::make_unique<AhiGram::Api::RemoteProxyListLoader>()) {
}

RegionProxyInitializer::~RegionProxyInitializer() = default;

void RegionProxyInitializer::start() {
	const auto &ahiSettings = AhiGram::Storage::Settings::Instance().data();
	const bool bypass = ahiSettings.ahiBypass.current();
	if (!bypass) {
		return;
	}

	_regionChecker->check(
		[=](QString countryCode) {
			const bool needProxy = AhiGram::Utils::IsRestrictedRegionCode(countryCode);
			if (needProxy) {
				const auto &proxySettings = Core::App().settings().proxy();
				if (proxySettings.isEnabled() && proxySettings.selected().valid()) {
					return;
				}
				fetchRemoteList();
			} else {
				auto &settings = AhiGram::Storage::Settings::Instance();
				if (settings.data().ahiBypass.current()) {
					settings.data().ahiBypass.force_assign(false);
					settings.save();
				}
			}
		},
		[=] {
			LOG(("AhiGram Error: Region check failed, falling back to locale."));
			const bool needProxy = AhiGram::Utils::IsRestrictedRegion();
			if (needProxy) {
				const auto &proxySettings = Core::App().settings().proxy();
				if (proxySettings.isEnabled() && proxySettings.selected().valid()) {
					return;
				}
				fetchRemoteList();
			}
		});
}

void RegionProxyInitializer::fetchRemoteList() {
    _remoteLoader->load(
        Constants::kApiDomain + u"/proxy/list"_q,
        [=](std::vector<ProxyCandidate> list) {
            std::vector<ProxyCandidate> valid;
            for (auto &c : list) {
                if (c.isValid()) {
                    valid.push_back(std::move(c));
                }
            }
            if (!valid.empty()) {
                startTesting(std::move(valid));
            }
        },
        [] {
            LOG(("AhiGram Error: Failed to fetch remote proxy list."));
        });
}

void RegionProxyInitializer::startTesting(std::vector<ProxyCandidate> &&candidates) {
    auto loader = std::make_shared<bool>(true);
    
    const auto &account = Core::App().activeAccount();
    
    ProxyTester::Test(&account.mtp(), std::move(candidates))
    | rpl::on_next([=](const ProxyCandidate &res) {
        if (res.ping >= 0 && res.ping < Constants::kFailedPing) {
            if (*loader) {
                *loader = false;
                ApplyProxyCandidate(res);
            }
        }
    }, _lifetime);
}

} // namespace AhiGram::Networking
