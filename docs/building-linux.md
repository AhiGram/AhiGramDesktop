## Build instructions for Linux using Docker (AhiGram Desktop)

This document describes how to build **AhiGram Desktop** using Docker. This method ensures a consistent build environment and avoids dependency conflicts.

### Prepare folder

Choose a folder for the build, for example **/home/user/AhiBuild**. It will be named ***BuildPath***.

### Obtain your API credentials

You will require an **api_id** and **api_hash**.
- Instructions on how to obtain them: [click here][api_credentials].

### Clone source code and prepare libraries

Install [poetry](https://python-poetry.org), navigate to your ***BuildPath*** and run:

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    ./Telegram/build/prepare/linux.sh

### Building the project

Run the Docker container. **Note:** We use `/usr/src/tdesktop` as the internal path to maintain compatibility with Telegram's build scripts.

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/tdesktop" \
        tdesktop:centos_env \
        /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

#### Creating a Debug Build
If you need to debug your changes:

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/tdesktop" \
        -e CONFIG=Debug \
        tdesktop:centos_env \
        /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

The compiled binaries will be located in the `out` directory.

### Visual Studio Code integration

If you use **Dev Containers**:
1. Open the **AhiGramDesktop** folder in VS Code.
2. Ensure you have the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
3. Add credentials to `.vscode/settings.json`:

    {
        "cmake.configureSettings": {
            "TDESKTOP_API_ID": "YOUR_API_ID",
            "TDESKTOP_API_HASH": "YOUR_API_HASH"
        }
    }

[api_credentials]: api_credentials.md
