#!/bin/zsh
git submodule update --init --recursive

# Install brew
if [[ -x "$(command -v brew)" ]]; then
	echo "Brew install detected"
else
	curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh | bash -s -- -y
fi

# Install required tools
brew install svn
brew install wget
brew install cmake

# Install LLVM and clang stuff
xcode-select install
if [[ -x "$(command -v rustup)" ]]; then
	echo "LLVM detected"
else
	brew install llvm
	echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
fi

# Install Rust
if [[ -x "$(command -v rustup)" ]]; then
	rustup update
else
	curl https://sh.rustup.rs -sSf | bash -s -- -y
fi
source "$HOME/.cargo/env"
export PATH="$HOME/.cargo/bin:$PATH"

# Build project
cargo build --release

# Install WASI-SDK 1 2to /opt/wasi-sdk, the path wasi-sdk assumed by default
mkdir /opt/wasi-sdk
pushd /opt/wasi-sdk || exit
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-macos.tar.gz
tar -xvf wasi-sdk-12.0-macos.tar.gz
mv wasi-sdk-12.0/* .
rm -rf wasi-sdk-12.0
rm wasi-sdk-12.0-macos.tar.gz
popd || exit

# Install libuv and uvwasi
make -C ./runtime/thirdparty install
