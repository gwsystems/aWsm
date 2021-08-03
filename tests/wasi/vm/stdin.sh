#!/bin/bash

input="wakka wakka"
output="$(echo "$input" | ./stdin_vm)"
if [[ $input != "$output" ]]; then
	echo "Fail"
	exit 1
else
	exit 0
fi
