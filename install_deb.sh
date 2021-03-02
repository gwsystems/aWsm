#!/bin/bash
# Installs dependencies, builds Silverfish, and places it into your path
# Be sure to validate the Rust and LLVM install scripts we invoke below!

# Exit if not executing script from project root because we use relative paths
if [[ $(git remote get-url origin) != *"/aWsm.git" || $(git rev-parse --show-toplevel) != $(pwd) ]]; then
  echo "Install script must be run from project root."
  exit 1
fi

# Check to see if ./wasmception directory exists and is not empty.
# If not assume user didn't initialize submodules
if [[ ! -d "./wasmception" ]] || [[ -z "$(ls -A wasmception)" ]]; then
  git submodule update --init --recursive
fi

# Wasmception
# Install Subversion
sudo apt install subversion --yes

# Build
# This is super slow. Does something need to get modified in the Makefile?
cd wasmception || exit
make
cd .. || exit

## Silverfish
sudo apt install build-essential --yes

if [[ -x "$(command -v rustup)" ]]; then
  rustup update
else
  curl https://sh.rustup.rs -sSf | bash -s -- -y
fi

source "$HOME/.cargo/env"
export PATH="$HOME/.cargo/bin:$PATH"
sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
LLVM_VERSION=9
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-$LLVM_VERSION 100
sudo apt install libc++-dev libc++abi-dev --yes
cargo build --release
sudo cp -t /usr/bin ./target/release/silverfish
