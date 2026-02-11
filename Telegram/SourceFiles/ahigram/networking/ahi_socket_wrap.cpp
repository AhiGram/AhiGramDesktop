/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_socket_wrap.h"
#include "stdafx.h"

#include <QtCore/QDateTime>
#include <algorithm>
#include <random>

namespace AhiGram::Networking {

AhiSocket::AhiSocket(not_null<QThread *> thread,
                     std::unique_ptr<MTP::details::AbstractSocket> &&wrapped,
                     bool protocolForFiles)
    : AbstractSocket(thread), _wrapped(std::move(wrapped)),
      _protocolForFiles(protocolForFiles) {
  LOG(("AhiSocket: Wrapper created for %1 connection.")
          .arg(_protocolForFiles ? u"file"_q : u"data"_q));
  if (_wrapped) {
    _wrapped->connected() | rpl::start_to_stream(_connected, _lifetime);
    _wrapped->disconnected() | rpl::start_to_stream(_disconnected, _lifetime);
    _wrapped->readyRead() | rpl::start_to_stream(_readyRead, _lifetime);
    _wrapped->error() | rpl::start_to_stream(_error, _lifetime);
    _wrapped->syncTimeRequests() |
        rpl::start_to_stream(_syncTimeRequests, _lifetime);
  }
}

void AhiSocket::connectToHost(const QString &address, int port) {
  _wrapped->connectToHost(address, port);
}

bool AhiSocket::isGoodStartNonce(bytes::const_span nonce) {
  return _wrapped->isGoodStartNonce(nonce);
}

void AhiSocket::timedOut() { _wrapped->timedOut(); }

bool AhiSocket::isConnected() { return _wrapped->isConnected(); }

bool AhiSocket::hasBytesAvailable() { return _wrapped->hasBytesAvailable(); }

int64 AhiSocket::read(bytes::span buffer) { return _wrapped->read(buffer); }

void AhiSocket::write(bytes::const_span prefix, bytes::const_span buffer) {
  if (!_wrapped)
    return;

  static thread_local std::default_random_engine generator(
      std::random_device{}());

  _totalBytesWritten += prefix.size() + buffer.size();

  if (!_firstWriteDone && !buffer.empty()) {
    _firstWriteDone = true;

    if (!prefix.empty()) {
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

  if (_protocolForFiles && _totalBytesWritten < 16 * 1024 && !buffer.empty()) {
    const auto chunkSize = std::size_t(512);
    auto offset = std::size_t(0);
    while (offset < buffer.size()) {
      const auto remaining = buffer.size() - offset;
      const auto current = std::min(remaining, chunkSize);
      _wrapped->write(offset == 0 ? prefix : bytes::const_span(),
                      buffer.subspan(offset, current));
      offset += current;
    }
    return;
  }

  if (!buffer.empty() && buffer.size() >= 64 && buffer.size() < 1024) {
    const auto maxSplit = std::min(buffer.size() - 1, std::size_t(128));
    const auto minSplit = std::min(maxSplit, std::size_t(64));
    std::uniform_int_distribution<std::size_t> dist(minSplit, maxSplit);
    const auto split = dist(generator);

    _wrapped->write(prefix, buffer.subspan(0, split));
    _wrapped->write({}, buffer.subspan(split));
  } else {
    _wrapped->write(prefix, buffer);
  }
}

int32 AhiSocket::debugState() { return _wrapped->debugState(); }

QString AhiSocket::debugPostfix() const {
  return _wrapped->debugPostfix() + u"_ahi"_q;
}

void AhiSocket::setDebugId(const QString &id) {
  _debugId = id;
  if (_wrapped) {
    _wrapped->setDebugId(id);
  }
}

std::unique_ptr<MTP::details::AbstractSocket>
Wrap(std::unique_ptr<MTP::details::AbstractSocket> &&socket,
     not_null<QThread *> thread, bool protocolForFiles) {
  if (!socket) {
    return nullptr;
  }
  return std::make_unique<AhiSocket>(thread, std::move(socket),
                                     protocolForFiles);
}

} // namespace AhiGram::Networking
