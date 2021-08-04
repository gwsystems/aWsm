#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit
mkdir -p data

rm -rf ./data/testdir
./stat_vm
