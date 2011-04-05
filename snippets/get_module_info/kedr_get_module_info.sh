#!/bin/sh
########################################################################
# kedr_get_module_info.sh - print information about a loaded kernel
# module (usually, a target module for KEDR).
# This script prints the start addresses of the code sections as well as 
# the parameters, etc.
#
# Usage:
#       kedr_get_module_info.sh <module_name>
########################################################################
if test $# -ne 1; then
    printf "Usage:\n\tkedr_get_module_info.sh <module_name>\n" > /dev/stderr
    exit 1
fi

MODULE=$1

lsmod | grep -E "^${MODULE}[[:blank:]]+" > /dev/null
if test $? -ne 0; then
    printf "Module \"${MODULE}\" is not found.\n"
    exit 1
fi

# Addresses of code ("text") sections
printf "Code sections:\n"
for ss in /sys/module/${MODULE}/sections/.*text*; do 
    printf "`basename ${ss}`\t`cat ${ss}`\n"; 
done

# Parameters
if test -d /sys/module/${MODULE}/parameters; then
    printf "\nParameters:\n"
    for pp in /sys/module/${MODULE}/parameters/[a-zA-Z_0-9]*; do
        printf "`basename ${pp}`\t`cat ${pp}`\n"; 
    done
fi

exit 0
