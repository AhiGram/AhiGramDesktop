/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_socket_wrap.h"
#include "ahigram/core/ahi_storage.h"
#include "stdafx.h"

#include <QtCore/QDateTime>
#include <algorithm>
#include <random>

namespace AhiGram::Networking {

AhiSocket::AhiSocket(
    not_null<QThread *> thread,
    std::unique_ptr<MTP::details::AbstractSocket> &&wrapped,
    bool protocolForFiles)
: AbstractSocket(thread)
, _wrapped(std::move(wrapped))
, _protocolForFiles(protocolForFiles)
, _firstWriteDone(false)
, _totalBytesWritten(0) {
    
    AhiGram::Storage::Settings::Instance().data().ahiBypass.value(
    ) | rpl::on_next([this](bool enabled) {
        _bypassEnabled = enabled;
    }, _lifetime);

    if (_wrapped) {
        _wrapped->connected() | rpl::start_to_stream(_connected, _lifetime);
        _wrapped->disconnected() | rpl::start_to_stream(_disconnected, _lifetime);
        _wrapped->readyRead() | rpl::start_to_stream(_readyRead, _lifetime);
        _wrapped->error() | rpl::start_to_stream(_error, _lifetime);
        _wrapped->syncTimeRequests() | rpl::start_to_stream(_syncTimeRequests, _lifetime);
    } else {
        LOG(("AhiSocket Error: Wrapped socket is null!"));
    }
}

void AhiSocket::connectToHost(const QString &address, int port) {
    if (_wrapped) {
#ifdef _DEBUG
        LOG(("AhiSocket: Connecting to %1:%2").arg(address).arg(port));
#endif
        _wrapped->connectToHost(address, port);
    }
}

bool AhiSocket::isGoodStartNonce(bytes::const_span nonce) {
    return _wrapped ? _wrapped->isGoodStartNonce(nonce) : false;
}

void AhiSocket::timedOut() { 
    if (_wrapped) _wrapped->timedOut(); 
}

bool AhiSocket::isConnected() { 
    return _wrapped ? _wrapped->isConnected() : false; 
}

bool AhiSocket::hasBytesAvailable() { 
    return _wrapped ? _wrapped->hasBytesAvailable() : false; 
}

int64 AhiSocket::read(bytes::span buffer) { 
    return _wrapped ? _wrapped->read(buffer) : -1; 
}

void AhiSocket::write(bytes::const_span prefix, bytes::const_span buffer) {
    if (!_wrapped) return;

    if (!_bypassEnabled) {
        _wrapped->write(prefix, buffer);
        return;
    }

    static thread_local std::mt19937 generator(std::random_device{}());
    
    _totalBytesWritten += prefix.size() + buffer.size();

    if (!_firstWriteDone && !buffer.empty()) {
        _firstWriteDone = true;
        if (!prefix.empty()) {
#ifdef _DEBUG
            LOG(("AhiSocket: Initial handshake write (prefix: %1, buffer: %2)").arg(prefix.size()).arg(buffer.size()));
#endif
            if (debugPostfix().contains(u"_ee"_q)) {
                const auto split = std::min(buffer.size(), std::size_t(3));
                _wrapped->write(prefix, buffer.subspan(0, split));
                _wrapped->write({}, buffer.subspan(split));
            } else if (prefix.size() >= 64) {
                _wrapped->write(prefix.subspan(0, 1), prefix.subspan(1, 63));
                _wrapped->write({}, buffer);
            } else {
                _wrapped->write(prefix, buffer);
            }
            return;
        }
    }

    if (!buffer.empty() && (_protocolForFiles || buffer.size() > 128)) {
        std::uniform_int_distribution<std::size_t> dist(512, 1024);
        
        auto offset = std::size_t(0);
        int chunks = 0;

        while (offset < buffer.size()) {
            const auto remaining = buffer.size() - offset;
            const auto currentLimit = dist(generator);
            
            if (remaining < 100 && offset > 0) {
                 _wrapped->write({}, buffer.subspan(offset));
                 chunks++;
                 break;
            }

            const auto current = std::min(remaining, currentLimit);
            _wrapped->write(offset == 0 ? prefix : bytes::const_span(), buffer.subspan(offset, current));
            offset += current;
            chunks++;
        }
        
#ifdef _DEBUG
        if (chunks > 1) {
            LOG(("AhiSocket: Sharded %1 bytes into %2 chunks").arg(buffer.size()).arg(chunks));
        }
#else
        if (chunks > 1 && buffer.size() > 1000000) { 
            LOG(("AhiSocket: Large chunk sharded (%1 bytes)").arg(buffer.size()));
        }
#endif
    } else {
        _wrapped->write(prefix, buffer);
    }
}

int32 AhiSocket::debugState() { 
    return _wrapped ? _wrapped->debugState() : 0; 
}

QString AhiSocket::debugPostfix() const {
    return (_wrapped ? _wrapped->debugPostfix() : QString()) + u"_ahi"_q;
}

void AhiSocket::setDebugId(const QString &id) {
    _debugId = id;
    if (_wrapped) _wrapped->setDebugId(id);
}

std::unique_ptr<MTP::details::AbstractSocket>
Wrap(
    std::unique_ptr<MTP::details::AbstractSocket> &&socket,
    not_null<QThread *> thread, bool protocolForFiles) {
    if (!socket) {
        LOG(("AhiSocket Error: Cannot wrap null socket!"));
        return nullptr;
    }
    return std::make_unique<AhiSocket>(thread, std::move(socket), protocolForFiles);
}

} // namespace AhiGram::Networking
