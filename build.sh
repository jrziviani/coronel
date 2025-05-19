#!/bin/bash

set -e

readonly BINUTILS_VERSION='2.44'
readonly GCC_VERSION='15.1.0'
readonly BUILD_PATH="build"

die() {
    printf "$1\n" >&2
    exit $2
}

print_header() {
    printf '*************************************************************\n'
    printf '\t%s\n' "$1"
    printf '*************************************************************\n'
}

print_message() {
    printf " --> %s\n" "$1"
}

assert_target() {
    local target="$1"

    [[ "$target" != "release" && "$target" != "debug" ]] &&
       die "Unexpected target $target" 2
}

config_cross_compiler() {
    print_header "Setup the cross compiler"

    local contrib="$PWD/contrib"

    if [[ -d $BUILD_PATH/compiler/bin && -f $BUILD_PATH/compiler/.done ]]
    then
        export PATH=$PATH:$BUILD_PATH/compiler/bin
        print_message "Compiler looks fine, return"
        return
    fi

    mkdir -p $BUILD_PATH/compiler/tmp
    pushd $BUILD_PATH/compiler
        local cpath=$PWD

        pushd tmp
            if [[ ! -f .binutils ]]
            then
                if [[ ! -f binutils-$BINUTILS_VERSION.tar.xz ]]
                then
                    print_message "Downloading binutils"
                    curl -O https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz
                fi

                print_message "Building binutils"
                tar xf binutils-$BINUTILS_VERSION.tar.xz
                mkdir -p build-binutils
                pushd build-binutils
                    ../binutils-$BINUTILS_VERSION/configure \
                        --target=x86_64-elf \
                        --prefix="$cpath" \
                        --with-sysroot \
                        --disable-nls \
                        --disable-werror

                    make -j $(nproc)
                    make install
                popd # build-binutils
                touch .binutils
                rm binutils-$BINUTILS_VERSION.tar.xz
            fi

            if [[ ! -f .gcc ]]
            then
                if [[ ! -f gcc-$GCC_VERSION.tar.xz ]]
                then
                    print_message "Downloading gcc"
                    curl -O https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
                fi

                print_message "Building gcc"
                tar xf gcc-$GCC_VERSION.tar.xz
                # cp "$contrib/t-x86_64-elf" gcc-$GCC_VERSION/gcc/config/i386/
                # cp "$contrib/config.gcc" gcc-GCC_VERSION/gcc/

                export "LD_LIBRARY_PATH=/usr/lib64:$LD_LIBRARY_PATH"

                mkdir -p build-gcc
                pushd build-gcc
                    export PATH="$PATH:$cpath/bin"
                    export TARGET=x86_64-elf
                    ../gcc-$GCC_VERSION/configure \
                        --target=x86_64-elf \
                        --prefix="$cpath" \
                        --disable-nls \
                        --enable-languages=c,c++ \
                        --without-headers

                    make -j $(nproc) all-gcc
                    make -j $(nproc) all-target-libgcc CFLAGS_FOR_TARGET='-O2 -mcmodel=large -mno-red-zone'
                    make install-gcc
                    make install-target-libgcc
                popd # build-gcc
                touch .gcc
                rm gcc-$GCC_VERSION.tar.xz
            fi
        popd # tmp

        rm -fr tmp
        touch .done
        print_message "Done"
    popd # build/compiler

    export PATH=$PATH:$PWD/$BUILD_PATH/compiler/bin
}



config_cross_compiler

pushd $BUILD_PATH
cmake -H.. -Bdebug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build debug
popd

ln -fs $BUILD_PATH/debug/compile_commands.json .