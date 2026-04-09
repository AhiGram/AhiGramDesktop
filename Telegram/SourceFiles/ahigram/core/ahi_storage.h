/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "rpl/variable.h"

#include <QtCore/QString>
#include <QtCore/QVariantMap>

namespace AhiGram {

class SettingsData {
public:
    SettingsData();

    rpl::variable<bool> ahiBypass;
    rpl::variable<bool> disableTelegramAds = { true };
    rpl::variable<bool> saveDelMessage = { false };
    rpl::variable<bool> loadDelMessage = { false };
    rpl::variable<bool> deletedMessageOpacityEnabled = { true };
    rpl::variable<int> deletedMessageOpacityPercent = { 50 };
    rpl::variable<bool> disableColoredButtons = { false };

    [[nodiscard]] QVariantMap toMap() const;
    void fillFromMap(const QVariantMap &map);
};

namespace Storage {

class Settings {
public:
    static Settings& Instance();

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    SettingsData& data() { return _data; }
    const SettingsData& data() const { return _data; }

    void save();
    void load();
    void reset();

    void setAutoSave(bool enabled);

private:
    Settings();
    ~Settings();

    void loadFromDisk();
    void saveToDisk();
    [[nodiscard]] QString settingsPath() const;

    SettingsData _data;
    rpl::lifetime _lifetime;
    bool _dirty = false;
    bool _autosave = true;
};

} // namespace Storage
} // namespace AhiGram
