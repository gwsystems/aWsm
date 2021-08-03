#!/bin/bash

SHOULD_PASS=1 ./environ_vm || exit 1
SHOULD_PASS=0 ./environ_vm && exit 1
exit 0
