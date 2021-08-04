#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit
mkdir -p data

rm -f ./data/outside
ln -s /bin/bash ./data/outside

if [[ $(./symlink_escape_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
