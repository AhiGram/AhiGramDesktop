/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "main/main_session.h"
#include "settings/settings_common_session.h"
#include <qwidget.h>

namespace Settings {

class AhiMainSettings : public Section<AhiMainSettings> {
public:
	AhiMainSettings(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override;
private:
	void setupContent();

    const not_null<Window::SessionController*> _controller;
};

} // namespace Settings
