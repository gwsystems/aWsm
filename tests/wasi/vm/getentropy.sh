#!/bin/bash

if [[ $(./getentropy_vm) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
