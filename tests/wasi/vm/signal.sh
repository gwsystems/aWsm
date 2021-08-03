#!/bin/bash

./signal_vm
if (($? != 130)); then
	exit 1
fi
exit 0
