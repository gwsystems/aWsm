#!/bin/bash

git clone https://github.com/WebAssembly/wasi-libc.git

# Resize Wasm pages from 64k to 4k
mv wasi-libc/libc-bottom-half/headers/public/__macro_PAGESIZE.h wasi-libc/libc-bottom-half/headers/public/__macro_PAGESIZE.h.old
mv wasi-libc/expected/wasm32-wasi/predefined-macros.txt wasi-libc/expected/wasm32-wasi/predefined-macros.txt.old

sed "s/#define PAGESIZE (0x10000)/#define PAGESIZE (0x1000)/" \
    wasi-libc/libc-bottom-half/headers/public/__macro_PAGESIZE.h.old \
    >wasi-libc/libc-bottom-half/headers/public/__macro_PAGESIZE.h

sed "s/#define PAGESIZE (0x10000)/#define PAGESIZE (0x1000)/" \
    wasi-libc/expected/wasm32-wasi/predefined-macros.txt.old \
    >wasi-libc/expected/wasm32-wasi/predefined-macros.txt

rm wasi-libc/libc-bottom-half/headers/public/__macro_PAGESIZE.h.old
rm wasi-libc/expected/wasm32-wasi/predefined-macros.txt.old

make -C wasi-libc CC=/usr/bin/clang-13 \
    AR=/usr/bin/llvm-ar-13 \
    NM=/usr/bin/llvm-nm-13

make CC=/usr/bin/clang-13 \
    AR=/usr/bin/llvm-ar-13 \
    NM=/usr/bin/llvm-nm-13
