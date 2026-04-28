/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_privacy.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/ahi_lang.h"

#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"

namespace Settings {

AhiPrivacySettings::AhiPrivacySettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<AhiPrivacySettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiPrivacySettings::title() {
	return AhiGram::trReactive(u"ahigram_privacy_section"_q);
}

void AhiPrivacySettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AhiGram::Storage::Settings::Instance().data();

	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_invisible_mode_sections"_q));

	const auto invisibleModeButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_invisible_mode_title"_q),
			st::ahiSettingsButtonNoIcon));

	invisibleModeButton->toggleOn(settings.invisibleModeEnabled.value());

	invisibleModeButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.invisibleModeEnabled.current();
	}) | rpl::on_next([&settings](bool toggled) {
		settings.invisibleModeEnabled.force_assign(toggled);
	}, content->lifetime());

	const auto showInMenuButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_invisible_mode_show_in_menu"_q),
			st::ahiSettingsButtonNoIcon));

	showInMenuButton->toggleOn(settings.invisibleModeShowInMenu.value());

	showInMenuButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.invisibleModeShowInMenu.current();
	}) | rpl::on_next([&settings](bool toggled) {
		settings.invisibleModeShowInMenu.force_assign(toggled);
	}, content->lifetime());

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
