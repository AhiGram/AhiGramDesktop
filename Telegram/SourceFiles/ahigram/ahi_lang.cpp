/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahigram/ahi_lang.h"

#include "base/basic_types.h"
#include "lang/lang_instance.h"

#include <QFile>
#include <QTextStream>
#include <unordered_map>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace AhiGram {
namespace {

std::unordered_map<Language, QMap<QString, QString>> g_translations;
rpl::variable<Language> g_currentLanguage = Language::English;
rpl::lifetime g_languageLifetime;

bool LoadTranslations(Language lang, const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QMap<QString, QString> translations;
    QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    while (!stream.atEnd()) {
        const auto line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(u"//"_q)) continue;

        const auto eqPos = line.indexOf(u'=');
        if (eqPos <= 0) continue;

        auto key = line.mid(0, eqPos).trimmed();
        auto value = line.mid(eqPos + 1).trimmed();

        if (key.startsWith('"') && key.endsWith('"')) {
            key = key.mid(1, key.length() - 2);
        }
        
        value = value.trimmed();
        if (value.endsWith(';')) {
            value = value.left(value.length() - 1).trimmed();
        }
        if (value.startsWith('"') && value.endsWith('"')) {
            value = value.mid(1, value.length() - 2);
        }

        value.replace(u"\\n"_q, u"\n"_q);

        translations[key] = value;
    }

    g_translations[lang] = translations;
    return true;
}

void InitializeLanguage() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        
        auto determineLanguage = [] {
            const auto langId = Lang::GetInstance().id();
            const auto baseId = Lang::GetInstance().baseId();
            const auto currentLangId = baseId.isEmpty() ? langId : baseId;
            const auto langCode = currentLangId.left(2).toLower();
            return (langCode == u"ru") ? Language::Russian : Language::English;
        };

        g_currentLanguage = determineLanguage();

        Lang::GetInstance().idChanges(
        ) | rpl::on_next([=] {
            const auto newLang = determineLanguage();
            if (g_currentLanguage.current() != newLang) {
                g_currentLanguage = newLang;
            }
        }, g_languageLifetime);
    }
}

} // namespace

void InitializeLang() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    const auto enResPath = u":/ahigram/en.strings"_q;
    const auto ruResPath = u":/ahigram/ru.strings"_q;

    LoadTranslations(Language::English, enResPath);
    LoadTranslations(Language::Russian, ruResPath);

    if (g_translations[Language::Russian].empty()) {
        g_translations[Language::Russian] = g_translations[Language::English];
    }
}

Language GetCurrentLanguage() {
    InitializeLanguage();
    return g_currentLanguage.current();
}

QString GetLanguageId() {
    return GetCurrentLanguage() == Language::Russian ? u"ru"_q : u"en"_q;
}

QString tr(const QString &key) {
    InitializeLanguage();
    InitializeLang();

    const auto lang = g_currentLanguage.current();
    const auto &translations = g_translations[lang];

    const auto it = translations.find(key);
    if (it != translations.end()) {
        return it.value();
    }

    const auto &enTranslations = g_translations[Language::English];
    const auto enIt = enTranslations.find(key);
    if (enIt != enTranslations.end()) {
        return enIt.value();
    }

    return u"<missing: "_q + key + u">"_q;
}

rpl::producer<QString> trReactive(const QString &key) {
    InitializeLanguage();
    InitializeLang();

    return g_currentLanguage.value(
    ) | rpl::map([=](Language lang) {
        return tr(key);
    }) | rpl::distinct_until_changed();
}

rpl::producer<QString> languageChanges() {
    InitializeLanguage();
    return g_currentLanguage.value(
    ) | rpl::map([](Language lang) {
        return (lang == Language::Russian) ? u"ru"_q : u"en"_q;
    }) | rpl::distinct_until_changed();
}

} // namespace AhiGram
