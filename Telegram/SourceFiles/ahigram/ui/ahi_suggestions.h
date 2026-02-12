/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "dialogs/ui/dialogs_top_bar_suggestion_content.h"

namespace Main { 
class Session; 
} // namespace Main

namespace Ui { 
class RpWidget; 
} // namespace Ui

namespace AhiGram {

class AhiTopBarSuggestion final : public Dialogs::TopBarSuggestionContent {
public:
	AhiTopBarSuggestion(
		not_null<Ui::RpWidget *> parent,
		not_null<Main::Session *> session);

private:
	void setupContent();

	const not_null<Main::Session *> _session;
};

bool ShowWelcomeIfNeeded(
	not_null<Main::Session*> session,
	not_null<Ui::RpWidget*> parent,
	Dialogs::TopBarSuggestionContent **content,
	Fn<void()> repeat);

} // namespace AhiGram
