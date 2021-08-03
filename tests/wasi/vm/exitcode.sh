#!/bin/bash
./exitcode_vm
if (($? != 120)); then
	echo "Fail!"
	exit 1
else
	exit 0
fi
