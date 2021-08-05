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
  echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >>~/.zshrc
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

# Install libuv
pushd runtime || exit
wget https://github.com/libuv/libuv/archive/refs/tags/v1.42.0.tar.gz
tar -xvf v1.42.0.tar.gz
mv libuv-1.42.0/ libuv
pushd libuv || exit
mkdir -p build
pushd build || exit
cmake ..
cmake --build .
popd || exit
popd || exit
popd || exit

# Install uvwasi
pushd runtime || exit
wget https://github.com/nodejs/uvwasi/archive/refs/tags/v0.0.11.tar.gz
tar -xvf v0.0.11.tar.gz
mv uvwasi-0.0.11 uvwasi
rm v0.0.11.tar.gz
pushd uvwasi || exit
mkdir -p out/cmake
pushd out/cmake || exit
cmake ../.. -DBUILD_TESTING=ON
cmake --build .
ctest -C Debug --output-on-failure
popd || exit
popd || exit
popd || exit
