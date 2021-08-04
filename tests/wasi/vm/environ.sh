#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

SHOULD_PASS=1 ./environ_vm || exit 1
SHOULD_PASS=0 ./environ_vm && exit 1
exit 0
