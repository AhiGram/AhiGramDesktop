# Build instructions for Windows 64-bit (AhiGram Desktop)

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)
- [Qt Visual Studio Tools](#qt-visual-studio-tools)

## Prepare folder

The build is performed using **Visual Studio 2022** with the **10.0.26100.0** SDK version.

Choose an empty folder for the build, for example **D:\AhiBuild**. It will be referred to as ***BuildPath***. Create two subfolders: ***BuildPath*\ThirdParty** and ***BuildPath*\Libraries**.

All commands must be executed from the **x64 Native Tools Command Prompt for VS 2022.bat** (found in **Start Menu > Visual Studio 2022**). Do not use a standard Command Prompt or PowerShell.

### Obtain your API credentials

You will require an **api_id** and **api_hash** to access the Telegram API.
- Instructions on how to obtain them: [click here][api_credentials].

## Install third party software

* **Python 3.10**: Download from [python.org](https://www.python.org/downloads/) and ensure "Add to PATH" is checked during installation.
* **Git**: Download and install from [git-scm.com](https://git-scm.com/download/win).

## Clone source code and prepare libraries

Open the **x64 Native Tools Command Prompt**, navigate to your ***BuildPath*** and run:

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    Telegram\build\prepare\win.bat

## Build the project

Navigate to ***BuildPath*\AhiGramDesktop\Telegram** and run the configuration script with your credentials:

    configure.bat x64 -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

1. Open ***BuildPath*\AhiGramDesktop\out\Telegram.sln** in Visual Studio 2022.
2. Select the **Telegram** project in the Solution Explorer.
3. Go to **Build > Build Telegram** (available for both Debug and Release configurations).
4. The resulting `AhiGram.exe` (or `Telegram.exe`) will be located in ***BuildPath*\AhiGramDesktop\out\Debug** (or **Release**).

### Qt Visual Studio Tools

For an improved debugging experience, it is recommended to install the Qt extensions:
* **Extensions** -> **Manage Extensions**
* Search for **Qt** in the **Online** tab.
* Install **Qt Visual Studio Tools**.

---

### License
AhiGram Desktop is a fork of Telegram Desktop and is licensed under the **GNU GPL v3**.

[api_credentials]: api_credentials.md
