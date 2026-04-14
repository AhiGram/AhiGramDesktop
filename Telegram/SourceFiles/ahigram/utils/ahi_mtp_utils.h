/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <vector>
#include "scheme.h"

namespace AhiGram::Utils {

[[nodiscard]] std::vector<char> SerializeMtpMessage(const MTPMessage &msg);
[[nodiscard]] MTPMessage DeserializeMtpMessage(const std::vector<char> &data);

} // namespace AhiGram::Utils
