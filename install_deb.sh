#!/bin/bash
# Installs dependencies, builds Silverfish, and places it into your path
# Be sure to validate the Rust and LLVM install scripts we invoke below!
sudo apt install build-essential --yes
curl https://sh.rustup.rs -sSf | sh
source $HOME/.cargo/env
sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
LLVM_VERSION=9
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION 100
sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-$LLVM_VERSION 100
sudo apt install libc++-dev libc++abi-dev --yes
git clone https://github.com/gwsystems/silverfish.git
cd silverfish
cargo build --release
mkdir -p $HOME/bin
cp -t $HOME/bin target/release/silverfish
source $HOME/bin