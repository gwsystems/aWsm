#!/bin/bash

rm -rf ./data/readdir
mkdir ./data/readdir
touch ./data/readdir/input.txt
touch ./data/readdir/input2.txt
touch ./data/readdir/notadir
mkdir ./data/readdir/subdir

if [[ $(./readdir_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
