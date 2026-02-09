/*
This file is part of AhiGram,
a fork of Telegram Desktop for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/AhiGram/AhiGramDesktop/blob/master/LEGAL
*/

#include "ahi_main.h"

#include "main/main_session.h"

namespace AhiGram {
    void Initialize(not_null<Main::Session*> session) {
        LOG(("AhiGram: Initializing for session %1...")
        .arg(QString::number(session->userId().bare)));
    }
}
