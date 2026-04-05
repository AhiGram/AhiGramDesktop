/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahigram/networking/ahi_networking_common.h"

#include "base/basic_types.h"

#include <QtCore/QByteArray>

#include <memory>

namespace AhiGram::Api {

[[nodiscard]] std::vector<Networking::ProxyCandidate> ParseProxyListJson(
	const QByteArray &json);

class RemoteProxyListLoader final {
public:
	RemoteProxyListLoader();
	~RemoteProxyListLoader();

	RemoteProxyListLoader(const RemoteProxyListLoader &) = delete;
	RemoteProxyListLoader &operator=(const RemoteProxyListLoader &) = delete;

	void load(
		const QString &url,
		Fn<void(std::vector<Networking::ProxyCandidate>)> onDone,
		Fn<void()> onFail);
	void cancel();

private:
	struct Private;
	const std::unique_ptr<Private> _private;
};

} // namespace AhiGram::Api
