#!/bin/bash

if [[ $(./main_args_vm foo -bar --baz=value) -ne 0 ]]; then
	echo "Fail!"
	exit 1
else
	exit 0
fi
