#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit

rm -rf ./data/original.txt ./data/replacement.txt
original="Original"
replacement="Replacement"
echo "$original" > ./data/original.txt
echo "$replacement" > ./data/replacement.txt

result="$(./freopen_vm)"
if [[ "$result" != "$replacement" ]]; then
	echo "Fail!: was '$result', expected '$replacement'"
	exit 1
else
	exit 0
fi
