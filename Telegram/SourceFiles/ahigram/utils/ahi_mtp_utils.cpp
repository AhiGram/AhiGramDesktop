/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_mtp_utils.h"

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

} // namespace AhiGram::Utils
