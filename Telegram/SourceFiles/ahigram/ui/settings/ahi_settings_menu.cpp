/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_menu.h"
#include "ahi_settings_cover.h"
#include "ahi_settings_network.h"
#include "ahi_settings_chat.h"
#include "ahi_settings_privacy.h"
#include "ahi_settings_other.h"
#include "ahigram/ahi_lang.h"

#include "settings/settings_common.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "ui/basic_click_handlers.h"
#include "window/window_session_controller.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"
#include "styles/style_menu_icons.h"

namespace Settings {

AhiMainSettings::AhiMainSettings(
		QWidget *parent,
		not_null<Window::SessionController*> controller)
: Section<AhiMainSettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiMainSettings::title() {
	return AhiGram::trReactive(u"ahigram_settings_title"_q);
}

void AhiMainSettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	Ui::AddSkip(content, st::settingsPhotoTop);

	content->add(
		object_ptr<AhiGram::AhiCover>(content, _controller));

	Ui::AddDivider(content);
	Ui::AddSkip(content);

	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_menu_settings"_q));

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_bypass_title"_q),
		st::settingsButton,
		{ &st::ahiIconNetwork }
	)->setClickedCallback([=] {
		_controller->showSettings(AhiNetworkSettings::Id());
	});

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_delmsg_section"_q),
		st::settingsButton,
		{ &st::ahiIconChat }
	)->setClickedCallback([=] {
		_controller->showSettings(AhiChatSettings::Id());
	});

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_privacy_section"_q),
		st::settingsButton,
		{ &st::ahiIconPrivacy }
	)->setClickedCallback([=] {
		_controller->showSettings(AhiPrivacySettings::Id());
	});

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_other_section"_q),
		st::settingsButton,
		{ &st::ahiIconOther }
	)->setClickedCallback([=] {
		_controller->showSettings(AhiOtherSettings::Id());
	});

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_about_project"_q));

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_github"_q),
		st::settingsButton,
		{ &st::menuIconLink }
	)->setClickedCallback([=] {
		UrlClickHandler::Open(u"https://github.com/AhiGram/AhiGramDesktop"_q);
	});

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_telegram_release"_q),
		st::settingsButton,
		{ &st::menuIconChannel }
	)->setClickedCallback([=] {
		UrlClickHandler::Open(u"https://t.me/AhiGramReleases"_q);
	});

	Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_telegram_info"_q),
		st::settingsButton,
		{ &st::menuIconChannel }
	)->setClickedCallback([=] {
		UrlClickHandler::Open(u"https://t.me/AhiGram"_q);
	});

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
