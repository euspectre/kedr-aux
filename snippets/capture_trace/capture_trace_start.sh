#!/bin/sh

############################################################################
# Usage:
#		capture_trace_start.sh <path>
# 
# Start listening to the trace pipe, capture the messages output there by
# the payload modules and store them in the file specified by <path>.
#
# If the main trace directory is not /sys/kernel/debug/tracing (for example,
# if debugfs is mounted to a directory other than /sys/kernel/debug), you 
# should specify the path to the main trace directory in $BASE_TRACE_DIR.
# 
# A message occupies a single line in the trace.
# Currently, the messages have the following format (the number of spaces
# may vary, of course):
# TASK-PID    [CPU#]    TIMESTAMP:  EVENT: DATA
# 
# Example:
# rmmod-2274  [000] 16770.039434: called_kfree: arguments: (dd5eb000)
############################################################################

if test $# -ne 1 ; then
    printf "Usage: $0 <path_where_to_store_trace>\n"
	exit 0
fi

BASE_TRACE_DIR=${BASE_TRACE_DIR:-/sys/kernel/debug/tracing}
PID_FILE=/tmp/kedr_capture_trace.pid
OUT_FILE="$1"

if test -f "${PID_FILE}"; then
	# temporary file exists
	printf "It seems that some other process is now listening to the trace "
	printf "or the previous attempt to listen to it aborted unexpectedly.\n" 
	printf "Please execute\n    $(dirname $0)/capture_trace_stop.sh\nbefore "
	printf "trying to run\n    $0\n"
	exit 1
fi

printf "" > "${OUT_FILE}" || exit 1

# Start listening to the trace pipe and capturing the events
cat "${BASE_TRACE_DIR}/trace_pipe" >> "${OUT_FILE}" &
LISTENER_PID=$!
# $! is the pid of the last backgroud process launched from this shell.

# Check if the listener process is actually running.
ps -e | grep $! > /dev/null
if test $? -ne 0; then
	printf "Failed to start listening to the trace pipe\n"
	exit 1
fi

echo ${LISTENER_PID} > "${PID_FILE}"

exit 0
