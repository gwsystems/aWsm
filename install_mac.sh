#!/bin/zsh
git submodule update --init --recursive

# Uncomment to install brew
if ! command -v brew &> /dev/null
then 
  curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh | bash -s -- -y
else
  echo "Brew install detected"
fi

# Install required tools
brew install svn
brew install wget

# Install LLVM and clang stuff
xcode-select install
if ! command -v llvm-config &> /dev/null
then 
    brew install llvm
    echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
else
    echo "LLVM detected"
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