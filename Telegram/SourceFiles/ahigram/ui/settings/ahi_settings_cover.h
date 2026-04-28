/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ui/widgets/buttons.h"
#include "ui/wrap/padding_wrap.h"

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {
class FlatLabel;
class IconButton;
} // namespace Ui

namespace AhiGram {

class AhiCover final : public Ui::FixedHeightWidget {
public:
	AhiCover(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

private:
	void setupChildGeometry();
	void refreshNameGeometry(int newWidth);
	void refreshVersionGeometry(int newWidth);
	void refreshChannelGeometry(int newWidth);

	const not_null<Window::SessionController*> _controller;

	object_ptr<Ui::RpWidget> _logo;
	object_ptr<Ui::FlatLabel> _name = { nullptr };
	object_ptr<Ui::FlatLabel> _version = { nullptr };
	object_ptr<Ui::FlatLabel> _channel = { nullptr };
};

} // namespace AhiGram
