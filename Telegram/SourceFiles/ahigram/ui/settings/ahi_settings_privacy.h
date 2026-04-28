/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "settings/settings_common_session.h"

namespace Window {
class SessionController;
} // namespace Window

namespace Settings {

class AhiPrivacySettings : public Section<AhiPrivacySettings> {
public:
	AhiPrivacySettings(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

	const not_null<Window::SessionController*> _controller;
};

} // namespace Settings
