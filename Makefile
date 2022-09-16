SHELL:=/bin/bash

.PHONY: all
all: awsm applications

.PHONY: clean
clean: awsm.clean applications.clean

.PHONY: submodules
submodules:
	git submodule update --init --recursive

# aWsm: the WebAssembly to LLVM bitcode compiler
.PHONY: awsm
awsm: 
	cargo build --release

.PHONY: awsm.clean
awsm.clean:
	cargo clean

# aWsm Applications
.PHONY: applications
applications:
	make -C applications all.awsm

.PHONY: applications.clean
applications.clean:
	make -C applications clean
