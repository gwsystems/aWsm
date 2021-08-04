#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -rf ./data/testdir
./stat_vm
