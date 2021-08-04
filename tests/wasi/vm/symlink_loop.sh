#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -f ./data/loop2 ./data/loop1
touch ./data/loop2
ln -s ./data/loop2 ./data/loop1
rm ./data/loop2
ln -s ./data/loop1 ./data/loop2

if [[ $(./symlink_loop_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
