#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

./signal_vm
if (($? != 130)); then
	exit 1
fi
exit 0
