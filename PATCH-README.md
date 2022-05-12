

To run gocr with 4k pages

1. ./patch-wasi-libc.sh
2. make -C applications dist/gocr.awsm
3. ./applications/dist/gocr.awsm <applications/wasm_apps/gocr/examples/5x8.pnm

