/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_region_api.h"
#include "ahigram/networking/ahi_networking_common.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace AhiGram::Api {

struct RegionChecker::Private {
	std::unique_ptr<QNetworkAccessManager> manager;
	QPointer<QNetworkReply> reply;
};

RegionChecker::RegionChecker()
: _private(std::make_unique<Private>()) {
}

RegionChecker::~RegionChecker() {
	cancel();
}

void RegionChecker::cancel() {
	if (const auto reply = _private->reply) {
		reply->disconnect();
		reply->abort();
		reply->deleteLater();
	}
	_private->reply = nullptr;
	_private->manager.reset();
}

void RegionChecker::check(
		Fn<void(QString countryCode)> onDone,
		Fn<void()> onFail) {
	cancel();
	_private->manager = std::make_unique<QNetworkAccessManager>();
	auto request = QNetworkRequest(QUrl(Networking::Constants::kApiDomain + u"/countryCode"_q));
	request.setAttribute(
		QNetworkRequest::RedirectPolicyAttribute,
		QNetworkRequest::NoLessSafeRedirectPolicy);
	request.setTransferTimeout(Networking::Constants::kIpCheckTimeout);
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
		const auto doc = QJsonDocument::fromJson(data);
		if (!doc.isObject()) {
			onFail();
			return;
		}
		const auto obj = doc.object();
		const auto code = obj.value(u"country_code"_q).toString().trimmed().toUpper();
		if (code.isEmpty()) {
			onFail();
			return;
		}
		onDone(code);
	});
}

} // namespace AhiGram::Api
