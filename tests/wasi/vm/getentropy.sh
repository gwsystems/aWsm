#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

if [[ $(./getentropy_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
