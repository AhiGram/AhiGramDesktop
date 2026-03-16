<div align="center">
<img src="./docs/assets/icon.png" width="150" align="center">

# [AhiGram](https://github.com/AhiGram/AhiGramDesktop)

[![Telegram Channel](https://img.shields.io/badge/release-blue?logo=telegram&labelColor=gray)](https://t.me/AhiGramReleases)
[![Telegram Channel](https://img.shields.io/badge/channel-blue?logo=telegram&labelColor=gray)](https://t.me/AhiGram)
[![GitHub Downloads](https://img.shields.io/github/downloads/AhiGram/AhiGramDesktop/total?logo=github)](https://github.com/AhiGram/AhiGramDesktop/releases/latest)

A modified version of [**Telegram Desktop**](https://github.com/telegramdesktop/tdesktop) that combines useful functionality from various community forks and introduces additional improvements.  
It uses the official [Telegram API][telegram_api] and the secure [MTProto][telegram_proto] protocol.
</div>

## Privacy Features
- **Ghost Mode** — view messages, stories, and chats without updating your online status or activity indicators.
- **Deleted Messages Access** — allows viewing messages that were removed from chats. Media files are cached in the background, and removed messages may reappear after restarting the client.
- **Disable Sponsored Messages** — optional setting to hide Telegram’s sponsored content inside the client.

## Proxy and Network Tools
- **Smart MTProto Proxy Discovery** — automatically searches for available public proxies (including SOCKS5 and external sources). Proxy parsing logic is handled server-side and discovered proxies are stored locally to speed up future connections.
- **Auto-Switching** — automatically connects to the fastest available proxy node.
- **Slowdown Bypass** — traffic can be routed through a WebSocket tunnel without requiring manual configuration (thanks [tg-ws-proxy](https://github.com/Flowseal/tg-ws-proxy)).
- **Cleaner Chat List** — proxy promotion channels are hidden from the chat list.

## Developer Utilities
- Copy the ID of premium emojis directly from the emoji selection menu.
- Add premium emoji packs without needing an active Telegram Premium subscription.
- Display `callback_data` for inline keyboard buttons.

## Other Improvements
- Based on the latest Telegram Desktop updates.
- Minor interface adjustments.
- Various small bug fixes and quality-of-life improvements.

[//]: # (LINKS)
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[win64]: docs/building-win-x64.md
[mac]: docs/building-mac.md
[linux]: docs/building-linux.md
