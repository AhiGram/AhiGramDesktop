/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/api/ahi_proxy_remote_api.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace AhiGram::Api {
namespace {

[[nodiscard]] std::vector<Networking::ProxyCandidate> ParseProxyListJsonImpl(
		const QByteArray &json) {
	const auto doc = QJsonDocument::fromJson(json);
	if (!doc.isObject()) {
		return {};
	}
	const auto root = doc.object();
	const auto proxies = root.value(u"proxies"_q).toArray();
	auto result = std::vector<Networking::ProxyCandidate>();
	result.reserve(proxies.size());
	for (const auto &item : proxies) {
		if (!item.isObject()) {
			continue;
		}
		const auto o = item.toObject();
		auto c = Networking::ProxyCandidate();
		c.host = o.value(u"host"_q).toString();
		c.port = o.value(u"port"_q).toInt();
		c.secret = o.value(u"secret"_q).toString();
		c.isFakeTls = o.value(u"is_fake_tls"_q).toBool();
		if (c.isValid()) {
			result.push_back(std::move(c));
		}
	}
	return result;
}

} // namespace

std::vector<Networking::ProxyCandidate> ParseProxyListJson(const QByteArray &json) {
	return ParseProxyListJsonImpl(json);
}

struct RemoteProxyListLoader::Private {
	std::unique_ptr<QNetworkAccessManager> manager;
	QPointer<QNetworkReply> reply;
};

RemoteProxyListLoader::RemoteProxyListLoader()
: _private(std::make_unique<Private>()) {
}

RemoteProxyListLoader::~RemoteProxyListLoader() {
	cancel();
}

void RemoteProxyListLoader::cancel() {
	if (const auto reply = _private->reply) {
		reply->disconnect();
		reply->abort();
		reply->deleteLater();
	}
	_private->reply = nullptr;
	_private->manager.reset();
}

void RemoteProxyListLoader::load(
		const QString &url,
		Fn<void(std::vector<Networking::ProxyCandidate>)> onDone,
		Fn<void()> onFail) {
	cancel();
	_private->manager = std::make_unique<QNetworkAccessManager>();
	auto request = QNetworkRequest(QUrl(url));
	request.setAttribute(
		QNetworkRequest::RedirectPolicyAttribute,
		QNetworkRequest::NoLessSafeRedirectPolicy);
	const auto reply = _private->manager->get(request);
	_private->reply = reply;
	const QPointer<QNetworkReply> guard(reply);
	QObject::connect(reply, &QNetworkReply::finished, [=] {
		if (!guard) {
			return;
		}
		if (_private->reply != reply) {
			return;
		}
		_private->reply = nullptr;
		const auto data = reply->readAll();
		const auto err = reply->error();
		reply->deleteLater();
		_private->manager.reset();
		if (err != QNetworkReply::NoError) {
			onFail();
			return;
		}
		onDone(ParseProxyListJsonImpl(data));
	});
}

} // namespace AhiGram::Api
