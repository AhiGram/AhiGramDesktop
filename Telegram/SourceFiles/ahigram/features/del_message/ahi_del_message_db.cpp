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
#include "core/launcher.h"

#include "data/data_peer_id.h"

#include "sqlite_orm.h"

namespace AhiGram::DelMessage {

Database &Database::Instance() {
    static Database instance;
    return instance;
}

namespace {
const int kCurrentDbVersion = 2;
} // namespace

void Database::init() {
	if (_initialized) {
		return;
	}

	const auto tdata = cWorkingDir() + "tdata/";
	if (!QDir(tdata).exists()) {
		QDir().mkpath(tdata);
	}

	_dbPath = tdata + "ahigram_messages.sqlite";
	_storage = std::make_unique<Storage>(detail::makeSavedMessagesStorage(_dbPath.toStdString()));

	static constexpr int kCurrentDbVersion = 2;
	try {
		const auto version = _storage->pragma.user_version();
		if (version < kCurrentDbVersion) {
			_storage->drop_table_if_exists("saved_messages");
			_storage->pragma.user_version(kCurrentDbVersion);
		}
	} catch (const std::exception &e) {
		LOG(("AhiGram DB Error: pre-init failed: %1").arg(e.what()));
	}

	_storage->sync_schema();
	_initialized = true;

	LOG(("AhiGram DB: initialized at %1 (v%2)").arg(
		_dbPath,
		QString::number(kCurrentDbVersion)));
}

void Database::upsertMessage(const SavedMessage &msg) {
    init();
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
    init();
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

void Database::deleteMessage(int64_t ownerId, int64_t peerId, int64_t msgId) { 
    init();
    try {
        using namespace sqlite_orm;
        _storage->remove_all<SavedMessage>(
            where(
                c(&SavedMessage::owner_id) == ownerId 
                and c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        );
    } catch (const std::exception &e) {
        LOG(("AhiGram Error: Database::deleteMessage fail: %1").arg(e.what()));
    }
}

void Database::markDeleted(int64_t ownerId, int64_t peerId, int64_t msgId) { 
    init();
    try {
        using namespace sqlite_orm;
        _storage->update_all(
            set(c(&SavedMessage::is_deleted) = 1),
            where(
                c(&SavedMessage::owner_id) == ownerId
                and c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        );
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (markDeleted): %1").arg(e.what()));
    }
}

std::vector<SavedMessage> Database::getAllDeletedUserMessages(
        int64_t ownerId) const {
    const_cast<Database*>(this)->init();
    try {
        using namespace sqlite_orm;
        auto results = _storage->get_all<SavedMessage>(
            where(
                c(&SavedMessage::owner_id) == ownerId
                and c(&SavedMessage::is_deleted) == 1
            ),
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

std::vector<SavedMessage> Database::getDeletedUserMessagesForPeer(
        int64_t ownerId,
        int64_t peerId) const {
    const_cast<Database*>(this)->init();
    try {
        using namespace sqlite_orm;
        return _storage->get_all<SavedMessage>(
            where(
                c(&SavedMessage::owner_id) == ownerId
                and c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::is_deleted) == 1
            ),
            order_by(&SavedMessage::date).asc()
        );
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (getDeletedUserMessagesForPeer): %1").arg(e.what()));
    }
    return {};
}

bool Database::hasMessage(int64_t ownerId, int64_t peerId, int64_t msgId) const {
    const_cast<Database*>(this)->init();
    try {
        using namespace sqlite_orm;
        return _storage->count<SavedMessage>(
            where(
                c(&SavedMessage::owner_id) == ownerId
                and c(&SavedMessage::peer_id) == peerId
                and c(&SavedMessage::msg_id) == msgId
            )
        ) > 0;
    } catch (const std::exception &e) {
        LOG(("AhiGram DB error (hasMessage): %1").arg(e.what()));
    }
    return false;
}

} // namespace AhiGram::DelMessage
