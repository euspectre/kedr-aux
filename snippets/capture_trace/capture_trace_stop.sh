#!/bin/sh

############################################################################
# Usage:
#		capture_trace_stop.sh
# 
# Stop listening to the trace pipe and capture the messages output there.
############################################################################

if test $# -ne 0 ; then
    printf "Usage: $0\n"
	exit 0
fi

PID_FILE=/tmp/kedr_capture_trace.pid

if test ! -f "${PID_FILE}"; then
	printf "Nothing to stop: no process is listening to the trace pipe now.\n"
	exit 1
fi

LISTENER_PID=$(cat "${PID_FILE}")

# Check if $PID_FILE indeed contains a pid (a weak check).
echo ${LISTENER_PID} | grep '^[0-9][0-9]*$' > /dev/null
if test $? -ne 0; then
	printf "Invalid pid of a listener process: \"${LISTENER_PID}\"\n"

# Remove the pid file as it is probably corrupted somehow.
	rm -f "${PID_FILE}"
	exit 1
fi

# Check if this pid is actually the id of a listener process.
ALL_PS_OUT=$(ps -ef) 
LISTENER_PRESENT=$(echo "${ALL_PS_OUT}" | grep "${LISTENER_PID}.*cat.*/trace_pipe" | wc -l)

if test ${LISTENER_PRESENT} -eq 1; then
	kill ${LISTENER_PID}
fi
# If there are no listeners, do nothing, just remove .pid file.

rm -f ${PID_FILE}

exit 0
