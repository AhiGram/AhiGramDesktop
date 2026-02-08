## Build instructions for Linux using Docker (AhiGram Desktop)

This document describes how to build **AhiGram Desktop** (a fork of Telegram Desktop) using Docker. This method ensures a consistent build environment and avoids dependency conflicts on your host system.

### Prepare folder

Choose a folder for the future build, for example **/home/user/AhiBuild**. It will be named ***BuildPath*** in the rest of this document. All commands should be launched from your terminal.

### Obtain your API credentials

To interact with Telegram servers, you will require an **api_id** and **api_hash**.
- Instructions on how to obtain them: [click here][api_credentials].
- **Note:** For AhiGram, it is highly recommended to use your own production keys to avoid rate limits.

### Clone source code and prepare libraries

Install [poetry](https://python-poetry.org), navigate to your ***BuildPath*** and run:

    git clone --recursive https://github.com/AhiGram/AhiGramDesktop.git
    cd AhiGramDesktop
    ./Telegram/build/prepare/linux.sh

### Building the project

Run the Docker container to build the project. Replace `YOUR_API_ID` and `YOUR_API_HASH` with your actual credentials.

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/ahigram" \
        tdesktop:centos_env \
        /usr/src/ahigram/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

#### Creating a Debug Build
If you need to debug your changes (e.g., testing the **ByeDPI** integration), use the following command:

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/ahigram" \
        -e CONFIG=Debug \
        tdesktop:centos_env \
        /usr/src/ahigram/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

The compiled binaries will be located in the `out` directory.

### Visual Studio Code integration

If you prefer developing in VS Code, you can use **Dev Containers**:

1. Open the **AhiGramDesktop** folder in VS Code.
2. Install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.
3. Add your API credentials to `.vscode/settings.json`:

    {
        "cmake.configureSettings": {
            "TDESKTOP_API_ID": "YOUR_API_ID",
            "TDESKTOP_API_HASH": "YOUR_API_HASH"
        }
    }

4. Click the green button in the bottom left corner and select **"Reopen in Container"**.

[api_credentials]: api_credentials.md
