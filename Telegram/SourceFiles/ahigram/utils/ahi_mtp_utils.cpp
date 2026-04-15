/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_mtp_utils.h"

#include "apiwrap.h"
#include "data/data_peer.h"
#include "main/main_session.h"

namespace AhiGram::Utils {

std::vector<char> SerializeMtpMessage(const MTPMessage &msg) {
    mtpBuffer buffer;
    msg.write(buffer);
    const auto bytes = reinterpret_cast<const char*>(buffer.constData());
    return std::vector<char>(bytes, bytes + buffer.size() * sizeof(mtpPrime));
}

MTPMessage DeserializeMtpMessage(const std::vector<char> &data) {
    if (data.empty()) {
        return MTPMessage();
    }
    const auto primes = reinterpret_cast<const mtpPrime*>(data.data());
    const auto count = data.size() / sizeof(mtpPrime);
    auto from = primes;
    const auto end = primes + count;
    MTPMessage msg;
    if (!msg.read(from, end)) {
        return MTPMessage();
    }
    return msg;
}

void AcceptAllChatJoinRequests(
        not_null<PeerData*> peer,
        Fn<void()> done,
        Fn<void()> fail) {
    using Flag = MTPmessages_HideAllChatJoinRequests::Flag;
    peer->session().api().request(MTPmessages_HideAllChatJoinRequests(
        MTP_flags(Flag::f_approved),
        peer->input(),
        MTPstring()
    )).done([=](const MTPUpdates &result) {
        peer->session().api().applyUpdates(result);
        if (done) {
            done();
        }
    }).fail([=] {
        if (fail) {
            fail();
        }
    }).send();
}

} // namespace AhiGram::Utils
