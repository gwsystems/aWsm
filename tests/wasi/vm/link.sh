#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -rf ./data/input.txt ./data/hard_link.txt

echo "DUMMY" > ./data/input.txt

if [[ $(./link_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
