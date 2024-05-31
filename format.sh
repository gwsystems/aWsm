#!/bin/bash

LLVM_VERSION=13

validate() {
	utility="clang-format"
	utility_version="$("$utility" --version 2> /dev/null)" || {
		echo "$utility not found in path!"
		exit 1
	}

	regex="version ([0-9]+).([0-9]+).([0-9]+)"
	declare -i major=0
	declare -i minor=0
	declare -i patch=0
	declare -i required_major=$LLVM_VERSION
	declare -i required_minor=0
	declare -i required_patch=0

	if [[ "$utility_version" =~ $regex ]]; then
		major="${BASH_REMATCH[1]}"
		minor="${BASH_REMATCH[2]}"
		patch="${BASH_REMATCH[3]}"
	fi

	if ((major >= required_major)) && ((minor >= required_minor)) && ((patch >= required_patch)); then
		echo "$utility $required_major.$required_minor.$required_patch required. Satisfied by $major.$minor.$patch"
	else
		echo "$utility $required_major.$required_minor.$required_patch required. Not satisfied by $major.$minor.$patch"
		exit 1
	fi
}

help() {
	echo "Usage: $0 [OPTION]..."
	echo "Formats source code in the repository, applying formatting in place by default"
	echo ""
	echo "-h, --help        Print Help"
	echo "-d, --dry-run     Dry run formatters, returning 0 when code passes, 1 otherwise"
	echo ""
	echo "Exit status:"
	echo "0 if code required no formatting"
	echo "non-zero exit status otherwise"
}

dry_run_clang() {
	find runtime example_code \
		\( -path "runtime/thirdparty" \) -prune -false -o \
		-type f \( -iname \*.h -o -iname \*.c -o -iname \*.s -o -iname \*.ld \) -print0 \
		| xargs --null clang-format -Werror -n -ferror-limit=1
}

# Format all *.c, *.h, *.ld, and *.s files in runtime and example_code
format() {
	find runtime example_code \
		\( -path "runtime/thirdparty" \) -prune -false -o \
		-type f \( -iname \*.h -o -iname \*.c -o -iname \*.s -o -iname \*.ld \) -print0 \
		| xargs --null clang-format -i
	cargo fmt
}

case $1 in
	"-h" | "--help") help ;;
	"-d" | "--dry-run") validate && dry_run_clang ;;
	"") validate && format ;;
esac
