#!/bin/bash
# Installs dependencies, builds aWsm, and places it into your path
# Be sure to validate the Rust and LLVM install scripts we invoke below!

sudo="sudo"
if [[ $(whoami) == "root" ]]; then
	sudo=""
fi

# Exit if not executing script from project root because we use relative paths
# We want to match without case because the URL of the git repo is case insensitive, but the resulting
# directory is case sensitive. Thus, we can end up with either "aWsm" or "awsm."
shopt -s nocasematch
if [[ $(git remote get-url origin) != *"/awsm"* || $(git rev-parse --show-toplevel) != $(pwd) ]]; then
	echo "Install script must be run from project root."
	exit 1
fi
shopt -u nocasematch

# Install Build Dependencies
$sudo apt install build-essential --yes
$sudo apt install cmake --yes
$sudo apt install lsb-release wget software-properties-common --yes

# Initialize Wasmception submodule
# Check to see if ./wasmception directory exists and is not empty.
# If not assume user didn't initialize submodules
if [[ ! -d "./wasmception" ]] || [[ -z "$(ls -A wasmception)" ]]; then
	git submodule update --init --recursive
fi

# Build Wasmception from Source
cd wasmception || exit
make
cd .. || exit

# Install Rust
if [[ -x "$(command -v rustup)" ]]; then
	rustup update
else
	curl https://sh.rustup.rs -sSf | bash -s -- -y
fi
source "$HOME/.cargo/env"
export PATH="$HOME/.cargo/bin:$PATH"

# Install LLVM build dependencies
LLVM_VERSION=11
$sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" bash $LLVM_VERSION
$sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100
$sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$LLVM_VERSION 100
$sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-$LLVM_VERSION 100
sudo apt install libc++-11-dev libc++abi-11-dev --yes

# Build and install aWsm
cargo build --release
$sudo cp -t /usr/bin ./target/release/awsm

# Install WASI-SDK 12 to /opt/wasi-sdk, the path wasi-sdk assumed by default
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk_12.0_amd64.deb
dpkg -i wasi-sdk_12.0_amd64.deb
rm -f wasi-sdk_12.0_amd64.deb

# Install libuv and uvwasi
make -C ./runtime/thirdparty install
