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
