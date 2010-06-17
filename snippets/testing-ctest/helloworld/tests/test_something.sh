#!/bin/bash

WORK_DIR=${PWD}

# A fake test that passes if called with no arguments and fails otherwise
printf "Say \"Hi\"!\n"

if test $# -ne 0; then
    # A fake failure
    exit 1
fi

exit 0
