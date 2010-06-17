#!/bin/bash

WORK_DIR=${PWD}

# A fake test that passes if called with one argument and fails otherwise
printf "Say \"Hi\" to component \"Sample\" !\n"

if test $# -ne 1; then
    # A fake failure
    exit 1
fi

exit 0
