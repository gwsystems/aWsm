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
brew install wabt

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

# Install Wasmception
if [[ ! -d "./wasmception" ]] || [[ -z "$(ls -A wasmception)" ]]; then
	git submodule update --init --recursive
fi
cd wasmception || exit
make
cd .. || exit

# Install WASI-SDK if WASI_SDK_PATH not already set
if [[ -n "${WASI_SDK_PATH}" ]] && [[ -x "${WASI_SDK_PATH}/bin/clang" ]];; then
	echo "wasi-sdk detected"
else
	# Install WASI-SDK 12 to /usr/local/opt/wasi-sdk
	# This uses the same Mac OS idioms as the LLVM brew installation above
	sudo mkdir /usr/local/opt/wasi-sdk
	pushd /usr/local/opt/wasi-sdk || exit
	sudo wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-macos.tar.gz
	sudo tar -xvf wasi-sdk-12.0-macos.tar.gz
	sudo mv wasi-sdk-12.0/* .
	sudo rm -rf wasi-sdk-12.0
	sudo rm wasi-sdk-12.0-macos.tar.gz
	popd || exit
	echo 'export WASI_SDK_PATH="/usr/local/opt/wasi-sdk/"' >> ~/.zshrc
	source ~/.zshrc
fi

# Install libuv and uvwasi
make -C ./runtime/thirdparty install
