/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"

#include <QtCore/QString>

#include <memory>

namespace AhiGram::Api {

class RegionChecker final {
public:
	RegionChecker();
	~RegionChecker();

	RegionChecker(const RegionChecker &) = delete;
	RegionChecker &operator=(const RegionChecker &) = delete;

	void check(
		Fn<void(QString countryCode)> onDone,
		Fn<void()> onFail);
	void cancel();

private:
	struct Private;
	const std::unique_ptr<Private> _private;
};

} // namespace AhiGram::Api
