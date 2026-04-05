/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_del_message_db.h"

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <cstdint>
#include <exception>
#include <memory>

#include "data/data_peer_id.h"

#include "sqlite_orm.h"

namespace AhiGram::DelMessage {

Database &Database::Instance() {
    static Database instance;
    return instance;
}

void Database::init() {
    const auto basePath = cWorkingDir() + u"tdata/"_q;
    if (!QDir().exists(basePath)) {
        QDir().mkpath(basePath);
    }

    _dbPath = basePath + u"ahigram_messages.sqlite"_q;
    const auto dbPath = _dbPath.toStdString();

    _storage = std::make_unique<decltype(detail::makeSavedMessagesStorage(""))>(
        detail::makeSavedMessagesStorage(dbPath)
    );
    _storage->sync_schema();
    _initialized = true;

    LOG(("AhiGram DB: initialized at %1").arg(QString::fromStdString(dbPath)));
}

void Database::upsertMessage(const SavedMessage &msg) {
    if (!_initialized) return;
    try {
        _storage->replace(msg);
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (upsert): %1").arg(e.what()));
    }
}

std::pair<int64_t, int64_t> Database::cleanupInfo() {
    if (!_initialized) {
        return { 0, 0 };
    }

    if (_dbPath.isEmpty()) {
        return { 0, 0 };
    }

    const auto fileSizeBytes = int64_t(QFileInfo(_dbPath).size());
    return { fileSizeBytes, fileSizeBytes };
}

void Database::clearDeletedMessages() {
    if (!_initialized) return;
    try {
        using namespace sqlite_orm;
        _storage->transaction([&] {
            _storage->remove_all<SavedMessage>();
            return true;
        });
        _storage->vacuum();
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (clearDeletedMessages): %1").arg(e.what()));
    }
}

void Database::markDeleted(int64_t peerId, int64_t msgId) { 
    if (!_initialized) return;
    try {
        using namespace sqlite_orm;
        _storage->update_all(
            set(c(&SavedMessage::is_deleted) = 1),
            where(
                c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        );
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (markDeleted): %1").arg(e.what()));
    }
}

std::vector<SavedMessage> Database::getAllDeletedUserMessages() const {
    if (!_initialized) return {};
    try {
        using namespace sqlite_orm;
        auto results = _storage->get_all<SavedMessage>(
            where(c(&SavedMessage::is_deleted) == 1),
            order_by(&SavedMessage::date).asc()
        );
        std::vector<SavedMessage> userMessages;
        userMessages.reserve(results.size());
        for (auto &msg : results) {
            if (peerIsUser(PeerId(msg.peer_id))) {
                userMessages.push_back(std::move(msg));
            }
        }
        return userMessages;
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (getAllDeletedUserMessages): %1").arg(e.what()));
    }
    return {};
}

bool Database::hasMessage(int64_t peerId, int64_t msgId) const {
    if (!_initialized) return false;
    try {
        using namespace sqlite_orm;
        return _storage->count<SavedMessage>(
            where(
                c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        ) > 0;
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (hasMessage): %1").arg(e.what()));
    }
    return false;
}

} // namespace AhiGram::DelMessage
