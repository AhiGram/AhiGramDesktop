/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "rpl/producer.h"

#include <QString>
#include <QMap>

namespace AhiGram {

enum class Language {
    English,
    Russian,
};

[[nodiscard]] Language GetCurrentLanguage();
[[nodiscard]] QString GetLanguageId();

void InitializeLang();

[[nodiscard]] QString tr(const QString &key);

[[nodiscard]] rpl::producer<QString> trReactive(const QString &key);
[[nodiscard]] rpl::producer<QString> languageChanges();



} // namespace AhiGram
