/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_network.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/ahi_lang.h"

#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"

namespace Settings {

AhiNetworkSettings::AhiNetworkSettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<AhiNetworkSettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiNetworkSettings::title() {
	return AhiGram::trReactive(u"ahigram_bypass_title"_q);
}

void AhiNetworkSettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AhiGram::Storage::Settings::Instance().data();

	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_auto_proxy"_q));

	const auto bypassButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_bypass_slowdown_title"_q),
			st::ahiSettingsButtonNoIcon));

	bypassButton->toggleOn(settings.ahiBypass.value());

	bypassButton->toggledChanges(
	) | rpl::on_next([=, &settings](bool toggled) {
		settings.ahiBypass.force_assign(toggled);
	}, content->lifetime());

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
