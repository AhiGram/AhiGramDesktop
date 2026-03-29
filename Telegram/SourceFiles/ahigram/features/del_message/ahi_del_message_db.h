/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <memory>

#include "sqlite_orm.h"

namespace AhiGram::DelMessage {

struct SavedMessage {
    int64_t     peer_id         = 0;
    int64_t     msg_id          = 0;
    int64_t     from_id         = 0;
    std::string from_rank;
    int64_t     date            = 0;
    std::string text;
    std::string media_type;
    std::string media_path;
    int64_t     reply_to_id     = 0;
    std::string fwd_from_name;
    int64_t     fwd_from_id     = 0;
    int64_t     grouped_id      = 0;
    int         is_out          = 0;
    int         is_post         = 0;
    int         is_noforwards   = 0;
    int         is_deleted      = 0;
    std::vector<char> raw_mtp;
};

namespace detail {

inline auto makeSavedMessagesStorage(const std::string &path) {
    using namespace sqlite_orm;
    return make_storage(
        path,
        make_table("saved_messages",
            make_column("peer_id",       &SavedMessage::peer_id),
            make_column("msg_id",        &SavedMessage::msg_id),
            make_column("from_id",       &SavedMessage::from_id),
            make_column("from_rank",     &SavedMessage::from_rank),
            make_column("date",          &SavedMessage::date),
            make_column("text",          &SavedMessage::text),
            make_column("media_type",    &SavedMessage::media_type),
            make_column("media_path",    &SavedMessage::media_path),
            make_column("reply_to_id",   &SavedMessage::reply_to_id),
            make_column("fwd_from_name", &SavedMessage::fwd_from_name),
            make_column("fwd_from_id",   &SavedMessage::fwd_from_id),
            make_column("grouped_id",    &SavedMessage::grouped_id),
            make_column("is_out",        &SavedMessage::is_out),
            make_column("is_post",       &SavedMessage::is_post),
            make_column("is_noforwards", &SavedMessage::is_noforwards),
            make_column("is_deleted",    &SavedMessage::is_deleted),
            make_column("raw_mtp",       &SavedMessage::raw_mtp),
            primary_key(&SavedMessage::peer_id, &SavedMessage::msg_id)
        )
    );
}

} // namespace detail

class Database {
public:
    static Database &Instance();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    void init();

    void upsertMessage(const SavedMessage &msg);
    void markDeleted(int64_t peerId, int64_t msgId);
    void markDeletedBatch(int64_t peerId, const std::vector<int64_t> &msgIds);
    void markDeletedNonChannel(const std::vector<int64_t> &msgIds);

    [[nodiscard]] std::optional<SavedMessage> getMessage(int64_t peerId, int64_t msgId) const;
    [[nodiscard]] std::vector<SavedMessage> getDeletedMessages(int64_t peerId) const;
    [[nodiscard]] std::vector<SavedMessage> getAllDeletedUserMessages() const;
    [[nodiscard]] bool hasMessage(int64_t peerId, int64_t msgId) const;

private:
    Database() = default;
    ~Database() = default;

    std::unique_ptr<decltype(detail::makeSavedMessagesStorage(""))> _storage;
    bool _initialized = false;
};

} // namespace AhiGram::DelMessage
