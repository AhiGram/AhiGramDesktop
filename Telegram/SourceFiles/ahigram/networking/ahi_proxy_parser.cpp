/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_proxy_parser.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

namespace AhiGram::Networking {

std::vector<ProxyCandidate> ProxyParser::Parse(const QString &text) {
    if (text.isEmpty()) {
        return {};
    }

    std::vector<ProxyCandidate> results;
    static const QRegularExpression re(u"(tg://proxy\\?|https://t\\.me/proxy\\?)[^\\s\"'<>]+"_q);
    
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        auto candidate = FromLink(match.captured(0));
        
        if (candidate.isValid()) {
            results.push_back(std::move(candidate));
        }
    }
    
    return results;
}

ProxyCandidate ProxyParser::FromLink(const QString &link) {
    ProxyCandidate result;
    
    auto cleanLinkView = QStringView(link);
    while (!cleanLinkView.isEmpty() && (cleanLinkView.endsWith('.') || cleanLinkView.endsWith(')') || cleanLinkView.endsWith(']'))) {
        cleanLinkView = cleanLinkView.left(cleanLinkView.size() - 1);
    }

    const auto url = QUrl(cleanLinkView.toString());
    const auto query = QUrlQuery(url);
    
    auto server = query.queryItemValue(u"server"_q);
    if (server.endsWith('.')) {
        server.chop(1);
    }
    
    result.host = std::move(server);
    if (result.host.isEmpty()) {
        return result;
    }

    bool ok = false;
    result.port = query.queryItemValue(u"port"_q).toInt(&ok);
    if (!ok) {
        result.port = 0;
    }
    
    result.secret = query.queryItemValue(u"secret"_q);
    
    const auto &s = result.secret;
    const auto isHexFakeTls = s.startsWith(u"ee"_q, Qt::CaseInsensitive);
    const auto isBase64FakeTls = (s.length() >= 22) 
        && s.startsWith(u"7"_q) 
        && (s.size() > 1 && s[1] >= 'g' && s[1] <= 'v');

    result.isFakeTls = isHexFakeTls || isBase64FakeTls;
    
    return result;
}

} // namespace AhiGram::Networking
