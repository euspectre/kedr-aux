#!/bin/bash
#######################################################################
# This script runs on a target machine. 
# It unpacks the source archive the name of which is passed as a parameter, 
# builds and tests the system. The output is dumped to build.log file
# in the same directory where this script is located.
# Other .log files can be created, they will also be retrieved by the
# host machine.
#######################################################################
WORK_DIR=$(cd `dirname $0` && pwd)
chmod +w "${WORK_DIR}" || exit 1
cd "${WORK_DIR}" || exit 1 

# Main log file. If any errors are detected, the file should have 
# words "Errors occured" somewhere in its last two lines. 
# exitFailure() takes care of this, so use it instead of plain 'exit 1'.
LOG_FILE="${WORK_DIR}/build.log"

# The directory to build the system in (out-of-tree build is to be done).
BUILD_DIR="${WORK_DIR}/build"

# The directories to install the system into.
INSTALL_DIR="${WORK_DIR}/install"

# System information
LOCAL_MACHINE=$(uname -n)
LOCAL_ARCH=$(uname -m)
LOCAL_KERNEL=$(uname -r)

########################################################################
# Utility functions
########################################################################

# The function outputs the message specified as its arguments 
# both to the summary log and to stdout
printMessage()
{
    printf "$*" >> "${LOG_FILE}"
    printf "$*"
}

printSeparator()
{
    printMessage "========================================================\n"
}

# Writes an error marker at the end of the log and aborts execution of this
# script.
exitFailure()
{
    printf "[Build System] Errors occured\n" >> "${LOG_FILE}"
    exit 1
}

# Writes an success marker at the end of the log and stops execution of this
# script.
exitSuccess()
{
    printf "[Build System] Building and testing completed successfully\n" >> "${LOG_FILE}"
    exit 0
}

########################################################################
# main()
########################################################################
# Note: source_archive_name is just the name of the file, without ".tar.bz2"
if test $# -ne 1 ; then
    printMessage "Usage: $0 <source_archive_name>\n"
	exitFailure
fi

ARCHIVE_DIR="$1"
ARCHIVE_FILE="${ARCHIVE_DIR}.tar.bz2"

rm -rf "${LOG_FILE}" 
rm -rf "${ARCHIVE_DIR}" "${BUILD_DIR}" "${INSTALL_DIR}" 

printMessage "Machine: ${LOCAL_MACHINE} (${LOCAL_ARCH}), kernel: ${LOCAL_KERNEL}\n"
printMessage "Source package: ${ARCHIVE_FILE}\n"
printSeparator

########################################################################
# Check if the necessary tools and data are available.
which cmake > /dev/null 2>&1
if test $? -ne 0; then
    printMessage "'cmake' tool is not found.\n" 
    printMessage "Please check if CMake is installed.\n"
    exitFailure
fi

if test ! -f "${ARCHIVE_FILE}"; then
    printMessage "Not found: ${ARCHIVE_FILE}\n"
    exitFailure
fi

########################################################################
{
    mkdir -p "${BUILD_DIR}" && \
    mkdir -p "${INSTALL_DIR}"
} >> "${LOG_FILE}" 2>&1

# Unpack the archive
tar xfj "${ARCHIVE_FILE}" 
if test $? -ne 0; then
    printMessage "Failed to unpack ${ARCHIVE_FILE}\n"
    exitFailure
fi
printSeparator

########################################################################
# Configure, build and install the system
cd "${BUILD_DIR}" >> "${LOG_FILE}" 2>&1
if test $? -ne 0; then
    printMessage "Failed to change directory to ${BUILD_DIR}\n"
    exitFailure
fi

cmake \
    -DCMAKE_VERBOSE_MAKEFILE="ON" \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    "${WORK_DIR}/${ARCHIVE_DIR}" >> "${LOG_FILE}" 2>&1
if test $? -ne 0; then
    printMessage "Failed to configure the system\n"
    exitFailure
fi

KBUILD_VERBOSE=1 make >> "${LOG_FILE}" 2>&1
if test $? -ne 0; then
    printMessage "Failed to build the system\n"
    exitFailure
fi

make install >> "${LOG_FILE}" 2>&1
if test $? -ne 0; then
    printMessage "Failed to install the system to ${INSTALL_DIR}\n"
    exitFailure
fi
printSeparator

########################################################################
cd "${WORK_DIR}" || exitFailure

# TODO: add more instructions to check if the system operates correctly.
# Use 'exitFailure' to abort this script if it does not.
# Use 'printMessage' to output messages.

########################################################################
# Uninstall
cd "${BUILD_DIR}" || exitFailure
make uninstall >> "${LOG_FILE}" 2>&1
if test $? -ne 0; then
    printMessage "Failed to uninstall the system\n"
    exitFailure
fi
printSeparator

#######################################################################
exitSuccess
