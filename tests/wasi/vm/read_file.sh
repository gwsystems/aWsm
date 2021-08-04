#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -f ./data/input.txt
echo "Hello Once" > ./data/input.txt
if [[ $(./read_file_vm | wc -m) -ne 11 ]]; then
	echo "fail"
	exit 1
else
	exit 0
fi
