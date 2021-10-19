#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

if FOO=BAR BAR=BAZ BAZ=FOO ./environ_vm; then
	exit 0
else
	exit 1
fi
