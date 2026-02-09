/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"

namespace Main {
class Session;
} // namespace Main

namespace AhiGram {
void Initialize(not_null<Main::Session*> session);
} // AhiGram

