#!/bin/sh
#######################################################################
# This script runs on a target machine. 
# It unpacks the source archive the name of which is passed as a parameter, 
# builds and tests the system. The output is dumped to build.log file
# in the same directory where this script is located.
#######################################################################
WORK_DIR=$(cd `dirname $0` && pwd)
chmod +w "${WORK_DIR}" || exit 1
cd "${WORK_DIR}" || exit 1 

#######################################################################
if test $# -ne 1 ; then
    printf "Usage: $0 <source_archive>\n"
	exit 1
fi

#######################################################################
LOG_FILE="${WORK_DIR}/build.log"

# TODO: unpack the archive, build and test the system, dump the output
# to build.log

#<>
KVER=$(uname -r)
printf "Test: ${KVER}\n" > ${LOG_FILE}
printf "Source package: $1\n" >> ${LOG_FILE}
#<>

#######################################################################
exit 0
