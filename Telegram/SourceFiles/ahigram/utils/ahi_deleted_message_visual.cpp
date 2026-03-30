/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/utils/ahi_deleted_message_visual.h"
#include "ahigram/core/ahi_storage.h"

#include "ui/painter.h"

#include "rpl/map.h"
#include "rpl/merge.h"

namespace AhiGram {

void SubscribeDeletedOpacityRepaint(rpl::lifetime &lifetime, Fn<void()> repaint) {
	auto &d = Storage::Settings::Instance().data();
	rpl::merge(
		d.deletedMessageOpacityEnabled.changes() | rpl::to_empty,
		d.deletedMessageOpacityPercent.changes() | rpl::to_empty
	) | rpl::on_next(std::move(repaint), lifetime);
}

DeletedMessagePainterOpacityGuard::DeletedMessagePainterOpacityGuard(
		Painter &p,
		bool itemIsAhiDeleted) {
	if (!itemIsAhiDeleted) {
		return;
	}
	auto &s = Storage::Settings::Instance().data();
	if (!s.deletedMessageOpacityEnabled.current()) {
		return;
	}
	const auto percent = s.deletedMessageOpacityPercent.current();
	if (percent >= 100) {
		return;
	}
	p.save();
	p.setOpacity((percent / 100.) * p.opacity());
	_p = &p;
	_restore = true;
}

DeletedMessagePainterOpacityGuard::~DeletedMessagePainterOpacityGuard() {
	if (_restore) {
		_p->restore();
	}
}

} // namespace AhiGram
