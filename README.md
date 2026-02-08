# [AhiGram Desktop](https://t.me/AhiGram)

AhiGram Desktop is a custom fork of the official [Telegram Desktop][telegram_desktop] client, built using the [Telegram API][telegram_api] and the [MTProto][telegram_proto] secure protocol.

The project focuses on introducing **new features**, enhancing **privacy**, and providing a **refined UI design** while maintaining the performance and security of the original client. A key priority is the integration of advanced tools for **anti-blocking** and ensuring stable connectivity.

The source code is published under GPLv3 with an OpenSSL exception. The license is available [here][license].

## Community & Support

Stay updated and get help through our official Telegram channels:

* **News & Updates:** [@AhiGram](https://t.me/AhiGram)
* **Releases & Downloads:** [@AhiGramReleases](https://t.me/AhiGramReleases)
* **Q&A and FAQ:** [@AhiGramFAQ](https://t.me/AhiGramFAQ)

## Supported Systems

Currently, AhiGram Desktop is maintained for the following platforms:

* **Windows 7 and above** (64-bit & 32-bit)
* **GNU/Linux** (64-bit static build)

## Build Instructions

To build AhiGram Desktop from source, follow the platform-specific guides:

* **Windows:** [(32-bit)][win32] | [(64-bit)][win64]
* **GNU/Linux:** [Building with Docker][linux]

---

## License & Third-party

AhiGram Desktop is a derivative work and inherits all third-party licenses from the original Telegram Desktop project (including Qt, OpenSSL, FFmpeg, etc.).

[//]: # (LINKS)
[telegram]: https://telegram.org
[telegram_desktop]: https://desktop.telegram.org
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[win32]: docs/building-win.md
[win64]: docs/building-win-x64.md
[linux]: docs/building-linux.md
