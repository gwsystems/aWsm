#!/bin/bash

output="$(./atof_vm 3.14)"

if [[ $output != "3.14" ]]; then
	exit 1
fi
