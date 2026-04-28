/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_other.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/ahi_lang.h"

#include "base/options.h"
#include "core/application.h"
#include "ui/controls/compose_ai_button_factory.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "window/notifications_manager.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"

namespace Settings {

AhiOtherSettings::AhiOtherSettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<AhiOtherSettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiOtherSettings::title() {
	return AhiGram::trReactive(u"ahigram_other_section"_q);
}

void AhiOtherSettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AhiGram::Storage::Settings::Instance().data();

	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_ads_section"_q));

	const auto disableAdsButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_disable_telegram_ads"_q),
			st::ahiSettingsButtonNoIcon));
	disableAdsButton->toggleOn(settings.disableTelegramAds.value());

	disableAdsButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.disableTelegramAds.current();
	}) | rpl::on_next([=, &settings](bool toggled) {
		settings.disableTelegramAds.force_assign(toggled);
	}, content->lifetime());

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_other_section"_q));

	const auto button = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_disable_colored_buttons"_q),
			st::ahiSettingsButtonNoIcon));
	
	button->toggleOn(settings.disableColoredButtons.value());

	button->toggledChanges() | rpl::on_next([&settings](bool toggled) {
		settings.disableColoredButtons.force_assign(toggled);
		Core::App().notifications().updateAll();
	}, content->lifetime());

	const auto storiesButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_disable_stories"_q),
			st::ahiSettingsButtonNoIcon));

	storiesButton->toggleOn(settings.disableStories.value());

	storiesButton->toggledChanges() | rpl::on_next([&settings](bool toggled) {
		settings.disableStories.force_assign(toggled);
		Core::App().notifications().updateAll();
	}, content->lifetime());

	const auto hideAiButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_hide_ai_button"_q),
			st::ahiSettingsButtonNoIcon));

	auto &aiOption = base::options::lookup<bool>(Ui::kOptionHideAiButton);
	hideAiButton->toggleOn(rpl::single(aiOption.value()));

	hideAiButton->toggledChanges() | rpl::on_next([&aiOption](bool toggled) {
		aiOption.set(toggled);
	}, content->lifetime());

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
