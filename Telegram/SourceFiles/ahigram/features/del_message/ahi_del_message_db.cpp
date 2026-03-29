/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_del_message_db.h"

#include <QtCore/QString>
#include <QtCore/QDir>
#include <cstdint>
#include <exception>
#include <memory>
#include <qdir.h>

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

    const auto dbPath = (basePath + u"ahigram_messages.sqlite"_q).toStdString();

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

void Database::markDeletedBatch(int64_t peerId, const std::vector<int64_t> &msgIds) {
    if (!_initialized || msgIds.empty()) return;
    try {
        _storage->transaction([&] {
            using namespace sqlite_orm;
            for (const auto id : msgIds) {
                _storage->update_all(
                    set(c(&SavedMessage::is_deleted) = 1),
                    where(
                        c(&SavedMessage::peer_id) == peerId
                        and c(&SavedMessage::msg_id) == id
                    )
                );
            }
            return true;
        });
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (markDeletedBatch): %1").arg(e.what()));
    }
}

std::optional<SavedMessage> Database::getMessage(int64_t peerId, int64_t msgId) const {
    if (!_initialized) return std::nullopt;
    try {
        using namespace sqlite_orm;
        auto results = _storage->get_all<SavedMessage>(
            where(
                c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        );
        if (!results.empty()) return results.front();
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (getMessage): %1").arg(e.what()));
    }
    return std::nullopt;
}

std::vector<SavedMessage> Database::getDeletedMessages(int64_t peerId) const {
    if (!_initialized) return {};
    try {
        using namespace sqlite_orm;
        return _storage->get_all<SavedMessage>(
            where(
                c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::is_deleted) == 1
            ),
            order_by(&SavedMessage::date).asc()
        );
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (getDeletedMessages): %1").arg(e.what()));
    }
    return {};
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

void Database::markDeletedNonChannel(const std::vector<int64_t> &msgIds) {
    if (!_initialized || msgIds.empty()) return;
    try {
        _storage->transaction([&] {
            using namespace sqlite_orm;
            for (const auto id : msgIds) {
                _storage->update_all(
                    set(c(&SavedMessage::is_deleted) = 1),
                    where(c(&SavedMessage::msg_id) == id)
                );
            }
            return true;
        });
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (markDeletedNonChannel): %1").arg(e.what()));
    }
}


} // namespace AhiGram::DelMessage
