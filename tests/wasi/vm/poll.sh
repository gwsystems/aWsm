#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

echo "Note: Don't touch the keyboard as this test checks STDIN"
./poll_vm
