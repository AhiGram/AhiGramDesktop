# Build instructions for Windows (32-bit)

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)

## Prepare folder

Build environment: **Visual Studio 2022**, SDK **10.0.26100.0**.

> [!IMPORTANT]
> Use **x86 Native Tools Command Prompt for VS 2022.bat**.

### Obtain your API credentials

[Click here][api_credentials] for instructions.

## Install third party software

* Python 3.10, Git.

## Clone source code and prepare libraries

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    Telegram\build\prepare\win.bat

## Build the project

    configure.bat -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

1. Open `out\Telegram.sln` in VS 2022.
2. Build the **Telegram** project.

[api_credentials]: api_credentials.md
