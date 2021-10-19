#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

if ./main_args_vm foo -bar --baz=value; then
	exit 0
else
	exit 1
fi
