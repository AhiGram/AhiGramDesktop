/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/networking/ahi_proxy_utils.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "main/main_session.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "mtproto/mtproto_proxy_data.h"
#include "apiwrap.h"

namespace AhiGram::Networking {

void ApplyProxyCandidate(const ProxyCandidate &best, Main::Session *session) {
    auto &proxySettings = Core::App().settings().proxy();
    const auto &current = proxySettings.selected();

    if (proxySettings.isEnabled() && current.host == best.host && current.port == best.port) {
        return;
    }

    MTP::ProxyData proxy;
    proxy.host = best.host;
    proxy.port = best.port;
    proxy.password = best.secret;
    proxy.type = MTP::ProxyData::Type::Mtproto;
    if (!proxy.valid()) return;

    auto &list = proxySettings.list();
    auto it = std::find_if(list.begin(), list.end(), [&](const MTP::ProxyData &p) {
        return p.host == proxy.host && p.port == proxy.port;
    });

    if (it == list.end()) {
        if (list.size() >= Constants::kMaxProxyCount) {
            int removed = 0;
            list.erase(std::remove_if(list.begin(), list.end(), [&](const MTP::ProxyData &p) {
                if (removed >= Constants::kProxyCleanupBatch) return false;
                if (p.host == current.host && p.port == current.port) return false;
                removed++;
                return true;
            }), list.end());
        }
        list.push_back(proxy);
    }

    proxySettings.setSelected(proxy);
    proxySettings.setSettings(MTP::ProxyData::Settings::Enabled);
    proxySettings.connectionTypeChangesNotify();

    Core::App().saveSettingsDelayed();
    
    if (session) {
        session->account().mtp().restart();
    } else {
        const auto &account = Core::App().activeAccount();
        account.mtp().restart();
    }
}

} // namespace AhiGram::Networking
