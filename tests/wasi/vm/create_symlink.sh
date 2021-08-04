#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -f ./data/dummy.txt ./data/test_link

data="Hello World\n"
echo -n "$data" > ./data/dummy.txt

if [[ "$(./create_symlink_vm)" != "$data" ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
