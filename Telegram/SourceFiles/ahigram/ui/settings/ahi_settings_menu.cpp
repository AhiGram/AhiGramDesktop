/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_menu.h"
#include "ahigram/core/ahi_storage.h"
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
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"

#include "styles/style_ahi_settings.h"
#include "styles/style_settings.h"

#include "base/timer.h"

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
    auto &settings = ::AhiGram::Storage::Settings::Instance();

    const auto bypassInitial = settings.data().ahiBypass.current();
    Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_bypass_title"_q));
    Ui::AddSkip(content);

    const auto bypassButton = content->add(
        object_ptr<Ui::SettingsButton>(
            content,
            AhiGram::trReactive(u"ahigram_bypass_slowdown_title"_q),
            st::ahiSettingsButtonNoIcon
        )
    );

    bypassButton->toggleOn(rpl::single(bypassInitial));

    bypassButton->toggledChanges(
    ) | rpl::on_next([=, &settings](bool toggled) {
        settings.data().ahiBypass.force_assign(toggled);
    }, content->lifetime());

    Ui::AddSkip(content);
    Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_delmsg_section"_q));
    Ui::AddSkip(content);

    const auto saveInitial = settings.data().saveDelMessage.current();
    const auto loadInitial = settings.data().loadDelMessage.current();

    const auto saveButton = content->add(
        object_ptr<Ui::SettingsButton>(
            content,
            AhiGram::trReactive(u"ahigram_save_del_message"_q),
            st::ahiSettingsButtonNoIcon
        )
    );
    saveButton->toggleOn(rpl::single(saveInitial));

    const auto loadButton = content->add(
        object_ptr<Ui::SettingsButton>(
            content,
            AhiGram::trReactive(u"ahigram_load_del_message"_q),
            st::ahiSettingsButtonNoIcon
        )
    );
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
        return toggled != settings.data().saveDelMessage.current();
    }) | rpl::on_next([=, &settings](bool toggled) {
        settings.data().saveDelMessage.force_assign(toggled);
    }, content->lifetime());

    loadButton->toggledChanges(
    ) | rpl::filter([&settings](bool toggled) {
        return toggled != settings.data().loadDelMessage.current();
    }) | rpl::on_next([=, &settings](bool toggled) {
        settings.data().loadDelMessage.force_assign(toggled);
        scheduleRestartPrompt();
    }, content->lifetime());

    Ui::ResizeFitChild(this, content);
}

} // namespace Settings
