/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#pragma once

#include "base/basic_types.h"
#include "base/bytes.h"
#include "mtproto/details/mtproto_abstract_socket.h"

#include <memory>

#include <QtCore/QString>
#include <QtCore/QThread>
#include <rpl/lifetime.h>

namespace AhiGram::Networking {

class AhiSocket final : public ::MTP::details::AbstractSocket {
public:
  AhiSocket(not_null<QThread *> thread,
            std::unique_ptr<MTP::details::AbstractSocket> &&wrapped,
            bool protocolForFiles);

  void connectToHost(const QString &address, int port) override;
  [[nodiscard]] bool isGoodStartNonce(bytes::const_span nonce) override;
  void timedOut() override;
  [[nodiscard]] bool isConnected() override;
  [[nodiscard]] bool hasBytesAvailable() override;
  [[nodiscard]] int64 read(bytes::span buffer) override;
  void write(bytes::const_span prefix, bytes::const_span buffer) override;

  int32 debugState() override;
  [[nodiscard]] QString debugPostfix() const override;

  void setDebugId(const QString &id);

private:
  void handleConnected();

  std::unique_ptr<MTP::details::AbstractSocket> _wrapped;
  rpl::lifetime _lifetime;
  bool _protocolForFiles = false;
  bool _firstWriteDone = false;
  int64 _totalBytesWritten = 0;
};

std::unique_ptr<MTP::details::AbstractSocket>
Wrap(std::unique_ptr<MTP::details::AbstractSocket> &&socket,
     not_null<QThread *> thread, bool protocolForFiles);

} // namespace AhiGram::Networking
