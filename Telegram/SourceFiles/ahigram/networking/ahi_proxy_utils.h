/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "ahi_networking_common.h"

namespace Main {
class Session;
} // namespace Main

namespace AhiGram::Networking {

void ApplyProxyCandidate(const ProxyCandidate &best, Main::Session *session = nullptr);

} // namespace AhiGram::Networking
