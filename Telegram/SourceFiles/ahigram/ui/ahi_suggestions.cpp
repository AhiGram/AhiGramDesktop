/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/ui/ahi_suggestions.h"
#include "ahigram/core/ahi_storage.h"

#include "main/main_session.h"
#include "ui/basic_click_handlers.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"

#include "styles/style_ahi_base.h"
#include "data/data_session.h"
#include "data/data_changes.h"

namespace AhiGram {

AhiTopBarSuggestion::AhiTopBarSuggestion(
	not_null<Ui::RpWidget*> parent,
	not_null<Main::Session*> session)
: TopBarSuggestionContent(parent)
, _session(session) {
	setupContent();
}

void AhiTopBarSuggestion::setupContent() {
	setRightIcon(RightIcon::Close);

	auto title = Ui::Text::Bold(u"✨ AhiGram"_q);
	auto description = TextWithEntities{ u"Welcome to your fork! Click here to join our channel."_q };

	setContent(std::move(title), std::move(description));

	setClickedCallback([=] {
		UrlClickHandler::Open(u"https://t.me/AhiGram"_q);
	});
}

bool ShowWelcomeIfNeeded(
	not_null<Main::Session*> session,
	not_null<Ui::RpWidget*> parent,
	Dialogs::TopBarSuggestionContent **contentPtr,
	Fn<void()> repeat) {

	auto &storage = AhiGram::Storage::Settings::Instance();
	if (storage.data().welcomeShown.current()) {
		return false;
	}

	storage.data().welcomeShown = true;

	auto &content = *contentPtr;
	if (!content || !dynamic_cast<AhiTopBarSuggestion*>(content)) {
		content = Ui::CreateChild<AhiTopBarSuggestion>(parent.get(), session);

		content->setHideCallback([=] {
			repeat();
		});

		parent->widthValue(
		) | rpl::on_next([=](int width) {
			content->resizeToWidth(width);
		}, content->lifetime());
	}

	content->setLeftPadding(rpl::single(st::ahiDialogsTopBarLeftPadding));
	
	content->resizeToWidth(parent->width());
	content->show();

	return true;
}

} // namespace AhiGram
