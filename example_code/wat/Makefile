%.wasm: %.wat
	wat2wasm $< -o $@

%.bc: %.wasm
	../../target/release/awsm $< -o $@

%.ll: %.bc
	llvm-dis-12 $< -o $@

clean:
	rm -f *.ll
	rm -f *.bc
	rm -f *.wasm
