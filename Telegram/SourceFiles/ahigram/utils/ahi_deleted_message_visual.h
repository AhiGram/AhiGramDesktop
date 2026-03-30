/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"
#include "rpl/lifetime.h"

class Painter;

namespace AhiGram {

void SubscribeDeletedOpacityRepaint(rpl::lifetime &lifetime, Fn<void()> repaint);

class DeletedMessagePainterOpacityGuard {
public:
	DeletedMessagePainterOpacityGuard(Painter &p, bool itemIsAhiDeleted);
	~DeletedMessagePainterOpacityGuard();

	DeletedMessagePainterOpacityGuard(const DeletedMessagePainterOpacityGuard&) = delete;
	DeletedMessagePainterOpacityGuard &operator=(
		const DeletedMessagePainterOpacityGuard&) = delete;

private:
	Painter *_p = nullptr;
	bool _restore = false;
};

} // namespace AhiGram
