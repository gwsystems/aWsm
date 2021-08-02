#!/bin/bash

rm -rf ./data/notadir
touch ./data/notadir

if [[ $(./notdir_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
