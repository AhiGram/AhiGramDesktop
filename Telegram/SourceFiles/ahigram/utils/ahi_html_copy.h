/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <memory>

class QMimeData;
struct TextForMimeData;

namespace AhiGram {

[[nodiscard]] std::unique_ptr<QMimeData> CopyAsHtml(
	const TextForMimeData &text);

} // namespace AhiGram
