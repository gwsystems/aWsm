#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit
mkdir -p data

rm -rf ./data/dummy.txt ./data/input_link.txt
data="Hello World\n"
echo -n "$data" > ./data/dummy.txt
ln -s /sandbox/data/dummy.txt ./data/input_link.txt

if [[ "$(./follow_symlink_vm)" != "$data" ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
