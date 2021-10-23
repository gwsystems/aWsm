#!/bin/bash

echo "****************************************************************************"
echo "git commit:"
git log -n 1
echo "****************************************************************************"
echo -n "wasm-opt: "
wasm-opt --version
echo "****************************************************************************"
echo "WASI_SDK_PATH: $WASI_SDK_PATH"
echo "****************************************************************************"
echo "WASI clang:"
"${WASI_SDK_PATH}/bin/clang" -v
echo "****************************************************************************"
echo "System clang:"
clang -v
echo "****************************************************************************"
echo "Local clang:"
../../wasi-sdk/bin/clang -v
echo "****************************************************************************"
echo "Makefile:"
cat Makefile
echo "****************************************************************************"
echo "Try Opt level 0:"

cat Makefile | grep -q "wasm-opt -O0" || {
	echo "Makefile expected to find wasm-opt -O2, not found"
	exit 1
}

sed -i.bak 's/wasm-opt -O0/wasm-opt -O0 --dae-optimizing --const-hoisting/' Makefile
make clean vm/atof_vm vm/atof_original_vm debug
mkdir atof_logs
mv wasm/atof_original_metrics.txt atof_logs
mv wasm/atof_func_metrics.txt atof_logs
mv wasm/atof_original_func_metrics.txt atof_logs
mv wasm/atof_call_graph.dot atof_logs
mv wasm/atof_call_graph.svg atof_logs
mv wasm/atof_original_call_graph.dot atof_logs
mv wasm/atof_original_call_graph.svg atof_logs
mv wasm/atof.wat atof_logs
mv wasm/atof_original.wat atof_logs

./vm/atof_vm 3.14
if [[ $? -eq 3 ]]; then
	echo "aWsm Optimized Success"
else
	echo "aWsm Optimized Failure"
fi

./vm/atof_original_vm 3.14
if [[ $? -eq 3 ]]; then
	echo "aWsm Original Success"
else
	echo "aWsm Original Failure"
fi

wasmtime ./wasm/atof_original.wasm 3.14
if [[ $? -eq 3 ]]; then
	echo "wasmtime Original Success"
else
	echo "wasmtime Original Failure"
fi

wasmtime ./wasm/atof.wasm 3.14
if [[ $? -eq 3 ]]; then
	echo "wasmtime Optimized Success"
else
	echo "wasmtime Optimized Failure"
fi

mv Makefile.bak Makefile
