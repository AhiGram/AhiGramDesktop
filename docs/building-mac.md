## Build instructions for macOS (AhiGram Desktop)

### Prepare folder

Choose a folder for the build, for example **/Users/user/AhiBuild**. It will be named ***BuildPath***.

**Note about disk space:**
- **~35 GB** for libraries.
- **~20 GB** for the `out` folder.

### Obtain your API credentials

You will require **api_id** and **api_hash**. [Click here][api_credentials] for details.

### Clone source code and prepare libraries

Go to ***BuildPath*** and run:

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    brew install git automake cmake wget pkg-config gnu-tar ninja nasm meson

    sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    ./Telegram/build/prepare/mac.sh

### Building the project

Go to ***BuildPath*/AhiGramDesktop/Telegram** and run:

    ./configure.sh -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

Launch Xcode, open ***BuildPath*/AhiGramDesktop/out/Telegram.xcodeproj** and build for Debug or Release.

[api_credentials]: api_credentials.md
