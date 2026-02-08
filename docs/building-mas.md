## Build instructions for Mac App Store

> [!IMPORTANT]
> These instructions are intended for creating a Mac App Store (MAS) compatible build. Follow the standard [Build for macOS][mac] instructions first to set up your environment.

### Prepare Breakpad

Breakpad is used for crash reporting within the AhiGram Desktop client. Navigate to your ***BuildPath*** and run the following commands to clone and prepare the necessary components:

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

AhiGram Desktop builds for the Mac App Store must comply with Apple's sandboxing requirements. This means certain features (like direct file system access or the **ByeDPI** proxy integration) may require additional `entitlements` to function within the sandbox.

- Ensure your `api_id` and `api_hash` are correctly configured in the environment.
- Use the `Telegram.xcodeproj` generated in the `out` folder.
- Select the **Release** configuration and the appropriate target for distribution.

---

### Legal Note on MAS Distribution
As AhiGram is a fork of a **GPL v3** project, distributing it via the Mac App Store involves strict compliance with both the GPL and Apple's Terms of Service. Ensure you have provided a link to your source code within the app's "About" section as required by the license.

[mac]: building-mac.md
[xcode]: building-xcode.md
