#!/bin/bash

# exit when any command fails
set -e

# The -f option force the rebuild of IREE runtime frameworks.
while getopts hf flag; do
    case "${flag}" in
    f) FORCE_REBUILD="YES" ;;
    *)
        echo "$0 -h|-f"
        echo "  -h : display this message"
        echo "  -f : force rebuild runtime frameworks"
        exit 0
        ;;
    esac
done

SCRIPT_DIR=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")
SRC_DIR=$SCRIPT_DIR
BUILD_DIR=$SCRIPT_DIR/build
XCFRAMEWORK="$BUILD_DIR"/bpe.xcframework


# Build the tokenizer into a framework for each target.
if [[ $FORCE_REBUILD == "YES" ]]; then
    echo "┌------------------------------------------------------------------------------┐"
    echo "  Deleting existig IREE runtime frameworks ..."
    echo "└------------------------------------------------------------------------------┘"
    rm -rf "$IREE_BUILD_RUNTIME_DIR"
fi

function build_for_ios() {
    case $1 in
    sim) sdk=iphonesimulator ;;
    dev) sdk=iphoneos ;;
    *)
        echo "Unknown target $1 when calling build_for_ios"
        exit 5
        ;;
    esac

    arch=$2
    label=ios-$1-"$arch"
    build_dir="$BUILD_DIR"/"$label"

    test_file="$build_dir"/bpe.framework/bpe
    if test -f "$test_file" && lipo -info "$test_file"; then
        echo "Skip building iree.framework for $label."
    else
        echo "┌------------------------------------------------------------------------------┐"
        echo "  Building for $label ..."
        echo "   src: $SRC_DIR "
        echo "   build: $build_dir "
        echo "   build log: $build_dir/build.log"
        echo "└------------------------------------------------------------------------------┘"
        mkdir -p "$build_dir" # So to create the build.log file.
        cmake -S . \
            -B "$build_dir" \
            -GNinja \
            -DCMAKE_SYSTEM_NAME=iOS \
            -DCMAKE_OSX_SYSROOT="$(xcodebuild -version -sdk $sdk Path)" \
            -DCMAKE_OSX_ARCHITECTURES="$arch" \
            -DCMAKE_SYSTEM_PROCESSOR="$arch" \
            -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
            -DCMAKE_IOS_INSTALL_COMBINED=YES \
            -DCMAKE_INSTALL_PREFIX="$build_dir"/install \
            >"$build_dir"/build.log 2>&1
        cmake --build "$build_dir" >>"$build_dir"/build.log 2>&1
    fi
}

function build_for_macos() {
    arch=$1
    label=macos-"$arch"
    build_dir="$BUILD_DIR"/"$label"

    test_file="$build_dir"/bpe.framework/bpe
    if test -f "$test_file" && lipo -info "$test_file"; then
        echo "Skip building iree.framework for $label."
    else
        echo "┌------------------------------------------------------------------------------┐"
        echo "  Building for $label ..."
        echo "   src: $SRC_DIR "
        echo "   build: $build_dir "
        echo "   build log: $build_dir/build.log"
        echo "└------------------------------------------------------------------------------┘"
        mkdir -p "$build_dir" # So to create the build.log file.
        cmake -S . \
            -B "$build_dir" \
            -GNinja \
            -DCMAKE_OSX_ARCHITECTURES="$arch" >"$build_dir"/build.log 2>&1
        cmake --build "$build_dir" >"$build_dir"/build.log 2>&1
    fi
}

function merge_fat_static_library() {
    src_label=$2
    dst_label=$1

    src="$BUILD_DIR"/$src_label/bpe.framework/bpe
    dst="$BUILD_DIR"/$dst_label/bpe.framework/bpe

    if lipo -info "$dst" | grep 'Non-fat' >/dev/null; then
        echo "┌------------------------------------------------------------------------------┐"
        echo "  Building FAT static library ..."
        echo "   merge: $src"
        echo "    into: $dst"
        echo "└------------------------------------------------------------------------------┘"
        merged=/tmp/libmerged-"$src_label"-"$dst_label".a
        lipo "$src" "$dst" -create -output "$merged"
        mv "$merged" "$dst"
    fi
}

# Step 1. Build the following frameworks
#
# Note: We cannot build for dev-x86_64 because Apple does not offer
# SDK for it. If we do so, CMake will prompt us about missing required
# architecture x86_64 in file
# /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS16.2.sdk/usr/lib/libc++.tbd
# build_for_ios dev x86_64
#
# This step also merge dependent static libraries into the target library.
build_for_ios sim arm64
build_for_ios sim x86_64
build_for_ios dev arm64
build_for_macos x86_64
build_for_macos arm64

# Step 2. Merge the frameworks of the same OS platform
#  ios-simulator-arm64+x86_64
#  macos-arm64+x86_64
merge_fat_static_library ios-sim-arm64 ios-sim-x86_64
merge_fat_static_library macos-arm64 macos-x86_64

# Step 3. Merge the above frameworks into an XCFramework
echo "┌------------------------------------------------------------------------------┐"
echo "  Aggregating frameworks into an xcframework ..."
echo "└------------------------------------------------------------------------------┘"
rm -rf "$XCFRAMEWORK"
xcodebuild -create-xcframework \
    -framework "$BUILD_DIR"/macos-arm64/bpe.framework \
    -framework "$BUILD_DIR"/ios-sim-arm64/bpe.framework \
    -framework "$BUILD_DIR"/ios-dev-arm64/bpe.framework \
    -output "$XCFRAMEWORK"
tree -L 1 -d "$XCFRAMEWORK"
