#!/bin/bash

rm -f ./data/input.txt
echo "Hello Twice" > ./data/input.txt
if [[ $(./read_file_twice_vm | wc -m) -ne 24 ]]; then
	echo "fail"
	exit 1
else
	exit 0
fi
