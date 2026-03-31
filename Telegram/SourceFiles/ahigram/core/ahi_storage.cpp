/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_storage.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <algorithm>

namespace AhiGram {

QVariantMap SettingsData::toMap() const {
    return {
        { u"ahiBypass"_q, ahiBypass.current() },
        { u"disableTelegramAds"_q, disableTelegramAds.current() },
        { u"saveDelMessage"_q, saveDelMessage.current() },
        { u"loadDelMessage"_q, loadDelMessage.current() },
        { u"deletedMessageOpacityEnabled"_q, deletedMessageOpacityEnabled.current() },
        { u"deletedMessageOpacityPercent"_q, deletedMessageOpacityPercent.current() },
    };
}

void SettingsData::fillFromMap(const QVariantMap &map) {
    ahiBypass.force_assign(map.value(u"ahiBypass"_q, ahiBypass.current()).toBool());
    disableTelegramAds.force_assign(map.value(
        u"disableTelegramAds"_q,
        disableTelegramAds.current()).toBool());
    saveDelMessage.force_assign(
        map.value(u"saveDelMessage"_q, saveDelMessage.current()).toBool());
    loadDelMessage.force_assign(
        map.value(u"loadDelMessage"_q, loadDelMessage.current()).toBool());
    deletedMessageOpacityEnabled.force_assign(map.value(
        u"deletedMessageOpacityEnabled"_q,
        deletedMessageOpacityEnabled.current()).toBool());
    const auto rawPercent = map.value(
        u"deletedMessageOpacityPercent"_q,
        deletedMessageOpacityPercent.current()).toInt();
    deletedMessageOpacityPercent.force_assign(std::clamp(rawPercent, 30, 100));
}

namespace Storage {
namespace {

const auto kSettingsFileName = u"ahigram_settings.json"_q;

QString MakeSettingsPath() {
    const auto basePath = cWorkingDir() + u"tdata/"_q;
    if (!QDir().exists(basePath)) {
        QDir().mkpath(basePath);
    }
    return basePath + kSettingsFileName;
}

} // namespace

Settings& Settings::Instance() {
    static Settings instance;
    return instance;
}

Settings::Settings() {
    loadFromDisk();

    auto markDirty = [this] {
        _dirty = true;
        if (_autosave) saveToDisk();
    };

    _data.ahiBypass.changes() | rpl::on_next(markDirty, _lifetime);
    _data.disableTelegramAds.changes() | rpl::on_next(markDirty, _lifetime);
    _data.saveDelMessage.changes() | rpl::on_next(markDirty, _lifetime);
    _data.loadDelMessage.changes() | rpl::on_next(markDirty, _lifetime);
    _data.deletedMessageOpacityEnabled.changes() | rpl::on_next(markDirty, _lifetime);
    _data.deletedMessageOpacityPercent.changes() | rpl::on_next(markDirty, _lifetime);
}

Settings::~Settings() {
    if (_dirty && _autosave) {
        saveToDisk();
    }
}

void Settings::loadFromDisk() {
    const auto path = settingsPath();
    QFile file(path);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        _dirty = false;
        return;
    }

    const auto json = QJsonDocument::fromJson(file.readAll());
    if (json.isObject()) {
        _data.fillFromMap(json.object().toVariantMap());
    }
    _dirty = false;
}

void Settings::saveToDisk() {
    if (!_dirty && QFile::exists(settingsPath())) return;

    QFile file(settingsPath());
    if (file.open(QIODevice::WriteOnly)) {
        const auto json = QJsonDocument::fromVariant(_data.toMap());
        file.write(json.toJson(QJsonDocument::Indented));
        _dirty = false;
    }
}

QString Settings::settingsPath() const {
    return MakeSettingsPath();
}

void Settings::save() {
    saveToDisk();
}

void Settings::load() {
    loadFromDisk();
}

void Settings::reset() {
    _data.fillFromMap({});
    _dirty = true;
    if (_autosave) saveToDisk();
}

void Settings::setAutoSave(bool enabled) {
    _autosave = enabled;
}

} // namespace Storage
} // namespace AhiGram
