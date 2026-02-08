## Build instructions for Mac App Store (AhiGram Desktop)

> [!IMPORTANT]
> These instructions are intended for creating a Mac App Store (MAS) compatible build. Follow the standard [Build for macOS][mac] instructions first to set up your environment.

### Prepare Breakpad

Breakpad is used for crash reporting. Navigate to your ***BuildPath*** and run:

    MACOSX_DEPLOYMENT_TARGET=10.13

    cd Libraries

    # Clone Breakpad and specific LSS version
    git clone https://chromium.googlesource.com/breakpad/breakpad
    cd breakpad
    git checkout bc8fb886
    git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
    cd src/third_party/lss
    git checkout a91633d1
    cd ../../..

    # Apply Telegram-specific patches
    git apply ../patches/breakpad.diff

    # Build Breakpad targets
    cd src/client/mac
    xcodebuild -project Breakpad.xcodeproj -target Breakpad -configuration Debug build
    xcodebuild -project Breakpad.xcodeproj -target Breakpad -configuration Release build

    # Build dump_syms tool
    cd ../../tools/mac/dump_syms
    xcodebuild -project dump_syms.xcodeproj -target dump_syms -configuration Release build
    cd ../../../../..

### Sandboxing & App Store Requirements

AhiGram Desktop MAS builds must comply with Apple's sandboxing.

- Ensure your `api_id` and `api_hash` are correctly configured.
- Use the `Telegram.xcodeproj` generated in the `out` folder.
- Select the **Release** configuration and the appropriate target.

[mac]: building-mac.md
[xcode]: building-xcode.md
