/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_menu.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/features/del_message/ahi_del_message_db.h"
#include "ahigram/ahi_lang.h"

#include "core/application.h"
#include "dialogs/ui/dialogs_top_bar_suggestion_content.h"
#include "lang/lang_keys.h"
#include "rpl/producer.h"
#include "rpl/range.h"
#include "rpl/filter.h"
#include "rpl/variable.h"
#include "settings/settings_common_session.h"
#include "settings/settings_common.h"
#include "ui/boxes/confirm_box.h"
#include "ui/qt_object_factory.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "window/notifications_manager.h"

#include "styles/style_ahi_settings.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

#include "ui/widgets/continuous_sliders.h"

#include "base/basic_types.h"
#include "base/timer.h"

namespace Settings {
namespace {

void SetupAhiBypass(
		not_null<Ui::VerticalLayout*> container,
		AhiGram::SettingsData &settings) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, AhiGram::trReactive(u"ahigram_bypass_title"_q));

	const auto bypassButton = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			AhiGram::trReactive(u"ahigram_bypass_slowdown_title"_q),
			st::ahiSettingsButtonNoIcon));

	bypassButton->toggleOn(settings.ahiBypass.value());

	bypassButton->toggledChanges(
	) | rpl::on_next([=, &settings](bool toggled) {
		settings.ahiBypass.force_assign(toggled);
	}, container->lifetime());
}

void SetupDisableTelegramAds(
		not_null<Ui::VerticalLayout*> container,
		AhiGram::SettingsData &settings) {
	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(
		container,
		AhiGram::trReactive(u"ahigram_ads_section"_q));

	const auto disableAdsButton = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			AhiGram::trReactive(u"ahigram_disable_telegram_ads"_q),
			st::ahiSettingsButtonNoIcon));
	disableAdsButton->toggleOn(settings.disableTelegramAds.value());

	disableAdsButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.disableTelegramAds.current();
	}) | rpl::on_next([=, &settings](bool toggled) {
		settings.disableTelegramAds.force_assign(toggled);
	}, container->lifetime());
}

void SetupDelMessageOptions(
		not_null<Ui::VerticalLayout*> container,
		not_null<Window::SessionController*> controller,
		AhiGram::SettingsData &settings) {
	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, AhiGram::trReactive(u"ahigram_delmsg_section"_q));

	const auto saveInitial = settings.saveDelMessage.current();
	const auto loadInitial = settings.loadDelMessage.current();

	const auto saveButton = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			AhiGram::trReactive(u"ahigram_save_del_message"_q),
			st::ahiSettingsButtonNoIcon));
	saveButton->toggleOn(rpl::single(saveInitial));

	const auto loadButton = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			AhiGram::trReactive(u"ahigram_load_del_message"_q),
			st::ahiSettingsButtonNoIcon));
	loadButton->toggleOn(rpl::single(loadInitial));

	const auto restartTimer = container->lifetime().make_state<base::Timer>();
	restartTimer->setCallback([=] {
		controller->show(Ui::MakeConfirmBox({
			.text = tr::lng_settings_need_restart(),
			.confirmed = [] { Core::Restart(); },
			.confirmText = tr::lng_settings_restart_now(),
			.cancelText = tr::lng_settings_restart_later(),
		}));
	});
	const auto scheduleRestartPrompt = [=] {
		restartTimer->callOnce(st::settingsButtonNoIcon.toggle.duration);
	};

	saveButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.saveDelMessage.current();
	}) | rpl::on_next([=, &settings](bool toggled) {
		settings.saveDelMessage.force_assign(toggled);
	}, container->lifetime());

	loadButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.loadDelMessage.current();
	}) | rpl::on_next([=, &settings](bool toggled) {
		settings.loadDelMessage.force_assign(toggled);
		scheduleRestartPrompt();
	}, container->lifetime());
}

void SetupDeletedMessageOpacity(
		not_null<Ui::VerticalLayout*> container,
		AhiGram::SettingsData &settings) {
	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);

	const auto button = Settings::AddButtonWithIcon(
		container,
		AhiGram::trReactive(u"ahigram_deleted_opacity_title"_q),
		st::settingsButton,
		{ &st::menuIconShowInChat });
	button->toggleOn(rpl::single(settings.deletedMessageOpacityEnabled.current()));
	button->toggledChanges(
	) | rpl::on_next([&settings](bool enabled) {
		settings.deletedMessageOpacityEnabled.force_assign(enabled);
	}, container->lifetime());

	constexpr auto kMin = 30;
	constexpr auto kMax = 100;
	const auto valuesCount = kMax - kMin + 1;

	auto sliderWithLabel = Settings::MakeSliderWithLabel(
		container,
		st::settingsScale,
		st::settingsScaleLabel,
		st::normalFont->spacew * 2,
		st::settingsScaleLabel.style.font->width("100%"),
		true);
	const auto opacityRow = sliderWithLabel.widget.data();
	container->add(
		std::move(sliderWithLabel.widget),
		st::settingsScalePadding);
	const auto slider = sliderWithLabel.slider;
	const auto label = sliderWithLabel.label;
	slider->setAccessibleName(AhiGram::tr(u"ahigram_deleted_opacity_title"_q));

	const auto updateLabel = [=](int percent) {
		label->setText(QString::number(percent) + u"%"_q);
	};
	updateLabel(settings.deletedMessageOpacityPercent.current());

	slider->setPseudoDiscrete(
		valuesCount,
		[=](int index) { return kMin + index; },
		settings.deletedMessageOpacityPercent.current(),
		[=, &settings](int percent) {
			updateLabel(percent);
			settings.deletedMessageOpacityPercent.force_assign(percent);
		});

	opacityRow->setVisible(true);
}

void SetupClearDeletedMessages(
		not_null<Ui::VerticalLayout*> container,
		not_null<Window::SessionController*> controller) {
	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);

	const auto cleanupSizeBytes = [] {
		const auto info = AhiGram::DelMessage::Database::Instance().cleanupInfo();
		return info.second;
	};

	const auto formatMb = [](int64_t bytes) {
		if (bytes <= 0) {
			return u"0MB"_q;
		}
		const auto mb = double(bytes) / (1024.0 * 1024.0);
		const auto mbStr = (mb < 10.)
			? (QString::number(mb, 'f', 1) + u"MB"_q)
			: (QString::number(static_cast<qint64>(mb + 0.5)) + u"MB"_q);
		return mbStr;
	};

	const auto computeTitle = [&] {
		return AhiGram::tr(u"ahigram_clear_deleted_messages"_q)
			+ u" ( "_q
			+ formatMb(cleanupSizeBytes())
			+ u" )"_q;
	};

	const auto clearTitle = computeTitle();
	const auto clearTitleValue = container->lifetime().make_state<rpl::variable<QString>>(clearTitle);

	const auto clearButton = Settings::AddButtonWithIcon(
		container,
		clearTitleValue->value(),
		st::ahiSettingsButton,
		{ &st::menuIconDelete });
	clearButton->setClickedCallback([=] {
		controller->show(Ui::MakeConfirmBox({
			.text = AhiGram::tr(u"ahigram_clear_deleted_messages_confirm"_q),
			.confirmed = [=](Fn<void()> close) {
				AhiGram::DelMessage::Database::Instance().clearDeletedMessages();
				*clearTitleValue = computeTitle();
				close();
			},
			.confirmText = tr::lng_box_ok(),
			.cancelText = tr::lng_box_no(),
		}));
	});
}

void SetupOther(
		not_null<Ui::VerticalLayout*> container,
		AhiGram::SettingsData &settings) {
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, AhiGram::trReactive(u"ahigram_other_section"_q));

	const auto button = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			AhiGram::trReactive(u"ahigram_disable_colored_buttons"_q),
			st::ahiSettingsButtonNoIcon));
	
	button->toggleOn(settings.disableColoredButtons.value());

	button->toggledChanges() | rpl::on_next([&settings](bool toggled) {
		settings.disableColoredButtons.force_assign(toggled);
		Core::App().notifications().updateAll();
	}, container->lifetime());
}

} // namespace

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
	auto &settings = ::AhiGram::Storage::Settings::Instance().data();

	SetupAhiBypass(not_null(content), settings);
	SetupDisableTelegramAds(not_null(content), settings);
	SetupDelMessageOptions(not_null(content), _controller, settings);
	SetupDeletedMessageOpacity(not_null(content), settings);
	SetupClearDeletedMessages(not_null(content), _controller);
	SetupOther(not_null(content), settings);

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
