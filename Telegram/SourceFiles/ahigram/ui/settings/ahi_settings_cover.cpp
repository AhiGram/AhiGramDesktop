/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_settings_cover.h"
#include "ahigram/ahi_lang.h"
#include "ahigram/utils/ahi_version.h"

#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/basic_click_handlers.h"
#include "ui/text/text_utilities.h"
#include "ui/boxes/peer_qr_box.h"
#include "ui/layers/generic_box.h"
#include "window/window_session_controller.h"
#include "styles/style_info.h"
#include "styles/style_settings.h"

#include <QPainter>

namespace AhiGram {

AhiCover::AhiCover(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: FixedHeightWidget(
	parent,
	st::settingsPhotoTop
		+ st::infoProfileCover.photo.size.height()
		+ st::settingsPhotoBottom)
, _controller(controller)
, _logo(this)
, _name(this, st::infoProfileCover.name)
, _version(this, st::defaultFlatLabel)
, _channel(this, st::infoProfileMegagroupCover.status) {

	_name->setText(AhiGram::tr(u"ahigram_profile_name"_q));
	_name->setSelectable(true);

	_version->setText(AhiGram::GetAppVersionString());
	_version->setSelectable(true);

	_channel->setMarkedText(Ui::Text::Link(AhiGram::tr(u"ahigram_profile_channel"_q)));
	_channel->overrideLinkClickHandler([=] {
		UrlClickHandler::Open(u"https://t.me/AhiGram"_q);
	});

	_logo->resize(st::infoProfileCover.photo.size);
	_logo->paintRequest(
	) | rpl::on_next([=](QRect clip) {
		QPainter p(_logo);
		p.setRenderHint(QPainter::Antialiasing);
		p.setRenderHint(QPainter::SmoothPixmapTransform);

		const auto size = st::infoProfileCover.photo.size.height();
		const auto rect = QRect(0, 0, size, size);

		const auto iconPath = u":/gui/art/logo_256.png"_q;
		const auto icon = QImage(iconPath);
		if (!icon.isNull()) {
			const auto scaled = icon.scaled(
				size,
				size,
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation);
			const auto x = (size - scaled.width()) / 2;
			const auto y = (size - scaled.height()) / 2;
			p.drawImage(x, y, scaled);
		}
	}, _logo->lifetime());

	setupChildGeometry();
}

void AhiCover::setupChildGeometry() {
	widthValue(
	) | rpl::on_next([=](int newWidth) {
		_logo->moveToLeft(
			st::settingsPhotoLeft,
			st::settingsPhotoTop,
			newWidth);
		refreshNameGeometry(newWidth);
		refreshVersionGeometry(newWidth);
		refreshChannelGeometry(newWidth);
	}, lifetime());
}

void AhiCover::refreshNameGeometry(int newWidth) {
	const auto nameLeft = st::settingsNameLeft;
	const auto nameTop = st::settingsNameTop;
	const auto nameWidth = newWidth - nameLeft - st::settingsButtonRightSkip;
	_name->resizeToWidth(nameWidth);
	_name->moveToLeft(nameLeft, nameTop, newWidth);
}

void AhiCover::refreshVersionGeometry(int newWidth) {
	const auto left = st::settingsPhoneLeft;
	const auto top = st::settingsPhoneTop;
	_version->moveToLeft(left, top, newWidth);
}

void AhiCover::refreshChannelGeometry(int newWidth) {
	const auto left = st::settingsUsernameLeft;
	const auto top = st::settingsUsernameTop;
	_channel->moveToLeft(left, top, newWidth);
}

} // namespace AhiGram
