#!/bin/sh
DEVICE="/dev/cfake"

# Repeatedly send requests to the device to initiate a series of calls to 
# the target functions from within it.
# This can be used to test hot patching (both instrumentation and 
# uninstrumentation) of a live and running target module.

while true; do
	echo "123456789asfdasd" > ${DEVICE} || exit 1
	dd if=${DEVICE} of=/dev/null ibs=10 count=3 || exit 1
done
