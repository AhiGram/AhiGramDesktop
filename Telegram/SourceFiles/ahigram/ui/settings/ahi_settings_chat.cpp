/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_chat.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/features/del_message/ahi_del_message_db.h"
#include "ahigram/ahi_lang.h"

#include "core/application.h"
#include "lang/lang_keys.h"
#include "ui/boxes/confirm_box.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "settings/settings_common.h"
#include "base/timer.h"
#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"
#include "styles/style_menu_icons.h"

namespace Settings {

AhiChatSettings::AhiChatSettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<AhiChatSettings>(parent, controller)
, _controller(controller) {
	setupContent();
}

rpl::producer<QString> AhiChatSettings::title() {
	return AhiGram::trReactive(u"ahigram_delmsg_section"_q);
}

void AhiChatSettings::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AhiGram::Storage::Settings::Instance().data();

	Ui::AddSkip(content);
	Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_deleted_messages_subsection"_q));

	const auto saveInitial = settings.saveDelMessage.current();
	const auto loadInitial = settings.loadDelMessage.current();

	const auto saveButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_save_del_message"_q),
			st::ahiSettingsButtonNoIcon));
	saveButton->toggleOn(rpl::single(saveInitial));

	const auto loadButton = content->add(
		object_ptr<Ui::SettingsButton>(
			content,
			AhiGram::trReactive(u"ahigram_load_del_message"_q),
			st::ahiSettingsButtonNoIcon));
	loadButton->toggleOn(rpl::single(loadInitial));

	const auto restartTimer = content->lifetime().make_state<base::Timer>();
	restartTimer->setCallback([=] {
		_controller->show(Ui::MakeConfirmBox({
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
	}, content->lifetime());

	loadButton->toggledChanges(
	) | rpl::filter([&settings](bool toggled) {
		return toggled != settings.loadDelMessage.current();
	}) | rpl::on_next([=, &settings](bool toggled) {
		settings.loadDelMessage.force_assign(toggled);
		scheduleRestartPrompt();
	}, content->lifetime());

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	const auto button = Settings::AddButtonWithIcon(
		content,
		AhiGram::trReactive(u"ahigram_deleted_opacity_title"_q),
		st::settingsButton,
		{ &st::menuIconShowInChat });
	button->toggleOn(rpl::single(settings.deletedMessageOpacityEnabled.current()));
	button->toggledChanges(
	) | rpl::on_next([&settings](bool enabled) {
		settings.deletedMessageOpacityEnabled.force_assign(enabled);
	}, content->lifetime());

	constexpr auto kMin = 30;
	constexpr auto kMax = 100;
	const auto valuesCount = kMax - kMin + 1;

	auto sliderWithLabel = Settings::MakeSliderWithLabel(
		content,
		st::settingsScale,
		st::settingsScaleLabel,
		st::normalFont->spacew * 2,
		st::settingsScaleLabel.style.font->width("100%"),
		true);
	const auto opacityRow = sliderWithLabel.widget.data();
	content->add(
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

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

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
	const auto clearTitleValue = content->lifetime().make_state<rpl::variable<QString>>(clearTitle);

	const auto clearButton = Settings::AddButtonWithIcon(
		content,
		clearTitleValue->value(),
		st::ahiSettingsButton,
		{ &st::menuIconDelete });
	clearButton->setClickedCallback([=] {
		_controller->show(Ui::MakeConfirmBox({
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

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
