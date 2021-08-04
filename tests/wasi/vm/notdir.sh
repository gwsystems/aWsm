#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit
mkdir -p data

rm -rf ./data/notadir
touch ./data/notadir

if [[ $(./notdir_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
