/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_menu.h"
#include "ahigram/core/ahi_storage.h"
#include "ahigram/ahi_lang.h"

#include "dialogs/ui/dialogs_top_bar_suggestion_content.h"
#include "rpl/producer.h"
#include "rpl/range.h"
#include "rpl/variable.h"
#include "settings/settings_common_session.h"
#include "settings/settings_common.h"
#include "ui/qt_object_factory.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"

#include "styles/style_ahi_settings.h"

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
    
    auto initialValue = settings.data().ahiBypass.current();

    Ui::AddSubsectionTitle(content, AhiGram::trReactive(u"ahigram_bypass_title"_q));
    Ui::AddSkip(content);

    const auto bypassButton = content->add(
        object_ptr<Ui::SettingsButton>(
            content,
            AhiGram::trReactive(u"ahigram_bypass_slowdown_title"_q),
            st::ahiSettingsButtonNoIcon
        )
    );

    bypassButton->toggleOn(rpl::single(initialValue));

    bypassButton->toggledChanges(
    ) | rpl::on_next([=, &settings](bool toggled) {
        settings.data().ahiBypass.force_assign(toggled);
    }, content->lifetime());

    Ui::ResizeFitChild(this, content);
}

} // namespace Settings
