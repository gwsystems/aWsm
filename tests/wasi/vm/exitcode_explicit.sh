#!/bin/bash
./exitcode_explicit_vm
if (($? != 120)); then
	echo "Fail!"
	exit 1
else
	exit 0
fi
