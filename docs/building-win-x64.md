# Build instructions for Windows 64-bit (AhiGram Desktop)

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)
- [Qt Visual Studio Tools](#qt-visual-studio-tools)

## Prepare folder

Build environment: **Visual Studio 2022**, SDK **10.0.26100.0**.

Create ***BuildPath*** (e.g., `D:\AhiBuild`) with subfolders `ThirdParty` and `Libraries`.

> [!CAUTION]
> Use **x64 Native Tools Command Prompt for VS 2022.bat** for all commands.

### Obtain your API credentials

You need **api_id** and **api_hash**: [click here][api_credentials].

## Install third party software

* **Python 3.10**: Add to PATH during install.
* **Git**: Latest version.

## Clone source code and prepare libraries

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    Telegram\build\prepare\win.bat

## Build the project

    configure.bat x64 -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

1. Open `out\Telegram.sln` in VS 2022.
2. Build the **Telegram** project (Debug/Release).
3. Result is in `out\Debug` or `out\Release`.

[api_credentials]: api_credentials.md
