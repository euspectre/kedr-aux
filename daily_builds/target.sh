#!/bin/bash
#######################################################################
# This script runs on a target machine. 
# It unpacks the source archive the name of which is passed as a parameter, 
# builds and tests the system. The output is dumped to build.log file
# in the same directory where this script is located.
# Other .log files can be created, they will also be retrieved by the
# host machine.
#
# [!!!] DO NOT place this script to a path that contains spaces.
#######################################################################
WORK_DIR=$(cd `dirname $0` && pwd)
chmod +w "${WORK_DIR}" || exit 1
cd "${WORK_DIR}" || exit 1 

# Main log file. If any errors are detected, the file should have 
# words "Errors occured" somewhere in its last two lines. 
# exitFailure() takes care of this, so use it instead of plain 'exit 1'.
LOG_FILE="${WORK_DIR}/build.log"

# The directories to build the system in (out-of-tree build is to be done).
BUILD_DIR="${WORK_DIR}/build"
BUILD_DIR_USER="${WORK_DIR}/build.user"
BUILD_DIR_KERNEL="${WORK_DIR}/build.kernel"

# The directory to install the system into.
INSTALL_DIR=""

# The directory to copy the installed examples into (for building).
EXAMPLES_DIR="${WORK_DIR}/examples"

# The directory for various temporary stuff.
TEMP_DIR="${WORK_DIR}/temp"

# The file with the names of the tests that may fail.
MAY_FAIL_FILE="${WORK_DIR}/may_fail.list"

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
# "Hide" the build tree of KEDR and build the examples
########################################################################
buildExamples()
{
	printMessage "===== Checking if the examples can be built w/o KEDR build tree =====\n"
	# Move the build trees to a temporary location to hide them from the 
	# installed examples.
	cd "${WORK_DIR}" || exitFailure
	mv "${BUILD_DIR}" "${BUILD_DIR_USER}" "${BUILD_DIR_KERNEL}" "${TEMP_DIR}" || exitFailure

	# Copy the installed examples to another directory (the default one 
	# will probably be read only) and try to build each one of them.
	cp -r "${INSTALL_DIR}/share/kedr/examples" "${EXAMPLES_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to copy the examples from ${INSTALL_DIR}/share/kedr/examples to ${EXAMPLES_DIR}\n"
		exitFailure
	fi

	# just in case
	export PATH=$PATH:${INSTALL_DIR}/bin
	
	# Build each example (but there is no need test them here as 
	# checkScenarioLocal() does this)
	for dd in ${EXAMPLES_DIR}/examples/* ${EXAMPLES_DIR}/examples/*/*; do
		if test -f "${dd}/makefile" || test -f "${dd}/Makefile"; then
			printf "Building example in ${dd}\n" >> "${LOG_FILE}"
			${MAKE_CMD} -C "${dd}" >> "${LOG_FILE}" 2>&1
			if test $? -ne 0; then
				printMessage "Failed to build the example in ${dd}\n"
				exitFailure
			else
				printf "Successfully built the example in ${dd}\n" >> "${LOG_FILE}"
			fi
		fi
	done
	
	cd "${WORK_DIR}" || exitFailure

	# Restore the build trees
	mv "${TEMP_DIR}/build" "${TEMP_DIR}/build.user" "${TEMP_DIR}/build.kernel" . || exitFailure
	printSeparator
}

########################################################################
# Run the self-tests.
# ${TEST_DIR} must be the path to the directory containing run_tests.sh
# on entry (the top directory of the test tree).
#
# Does nothing if CHECK_KEDR_SELFTEST is not set to "yes".
########################################################################
runTests()
{
	if test "t${CHECK_KEDR_SELFTEST}" = "tyes"; then
		# Run the tests. 
		printMessage "===== Running the tests =====\n"
		
		TEST_RESULT_DIR="${TEST_DIR}/$(uname -r)/Testing"

		# We may have used '${MAKE_CMD} test' instead of '${MAKE_CMD} check'
		# here as the targets required for the tests should have already 
		# been built by '${MAKE_CMD} build_tests'.
		# Still, using '${MAKE_CMD} check' should make no harm.
		TESTS_FAILED=0
		sh "${TEST_DIR}/run_tests.sh" >> "${LOG_FILE}" 2>&1
		TESTS_FAILED=$?

		# Copy the test logs to ${WORK_DIR} from where they could be automatically
		# retrieved by the host machine
		find "${TEST_RESULT_DIR}" -name '*.log' -exec cp {} "${WORK_DIR}" \;

		# Process the test failures, if any, check if they are known
		if test ${TESTS_FAILED} -ne 0; then
			TEST_FAIL_LIST="${WORK_DIR}/LastTestsFailed.log"
			if test -f "${TEST_FAIL_LIST}" -a -f "${MAY_FAIL_FILE}" ; then
		# For each line in ${TEST_FAIL_LIST} file check if it is matched by some 
		# record from ${MAY_FAIL_FILE}. If so, the failure is known, so do not 
		# take it into account.
				UNKNOWN_FAILURES=""
				for fail_record in $(cat "${TEST_FAIL_LIST}"); do
					echo ${fail_record} | grep -q -F -f "${MAY_FAIL_FILE}"
					if test $? -ne 0; then
						UNKNOWN_FAILURES="yes"
						printMessage "Unknown test failure: ${fail_record}\n"
					fi
				done

				if test -n "${UNKNOWN_FAILURES}"; then
					printMessage "There are unknown failures in the tests.\n"
					exitFailure    
				fi

				printMessage "All the test failures are known.\n"
		# If all the failures are known, go on as if no failure occured.

			else
		# No ${MAY_FAIL_FILE} or something else bad happened.
				printMessage "The tests report failures, see the logs for details.\n"
				exitFailure
			fi
		fi
		printSeparator
	fi

}

########################################################################
# Scenario: ("local" installation and self-tests)
# - configure KEDR
# - build
# - (if ${CHECK_KEDR_SELFTEST} is "yes") build and execute tests
# - install 
# - uninstall
########################################################################
checkScenarioLocal()
{
	printMessage "\n"
	printMessage "Checking scenario: \"local\" installation\n" 
	printMessage "CHECK_KEDR_SELFTEST is \"${CHECK_KEDR_SELFTEST}\"\n"
	printMessage "ENABLE_STD_PAYLOADS is \"${ENABLE_STD_PAYLOADS}\"\n\n"

	INSTALL_DIR="${WORK_DIR}/install"

	cd "${WORK_DIR}" || exitFailure
	if test -d "${ARCHIVE_DIR}"; then
		chmod -R a+w "${ARCHIVE_DIR}"  || exitFailure
	fi

	rm -rf "${ARCHIVE_DIR}" "${BUILD_DIR}" "${INSTALL_DIR}" 
	rm -rf "${EXAMPLES_DIR}" "${TEMP_DIR}"

	####################################################################
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
	
	chmod -R a-w "${WORK_DIR}/${ARCHIVE_DIR}"
	if test $? -ne 0; then
		printMessage "Failed to make the source tree read only (${WORK_DIR}/${ARCHIVE_DIR})\n"
		exitFailure
	fi
	printSeparator

	####################################################################
	# Configure, build, test and install the system
	cd "${BUILD_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to change directory to ${BUILD_DIR}\n"
		exitFailure
	fi

	add_cmake_options=
	if test "t${ENABLE_STD_PAYLOADS}" = "tnone"; then
		add_cmake_options="-DKEDR_STANDARD_CALLM_PAYLOADS=OFF -DKEDR_STANDARD_FSIM_PAYLOADS=OFF -DKEDR_LEAK_CHECK=OFF"
	elif test "t${ENABLE_STD_PAYLOADS}" = "tall"; then
		add_cmake_options="-DKEDR_STANDARD_CALLM_PAYLOADS=ON -DKEDR_STANDARD_FSIM_PAYLOADS=ON -DKEDR_LEAK_CHECK=ON"
	fi
	
	if test "t${CHECK_KEDR_SELFTEST}" = "tyes"; then
		add_cmake_options="${add_cmake_options} -DWITH_TESTING=yes"
	fi

	# Configure the build
	printMessage "===== Configuring the system =====\n"
	cmake ${add_cmake_options} \
		-DCMAKE_VERBOSE_MAKEFILE="OFF" \
		-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
		"${WORK_DIR}/${ARCHIVE_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to configure the system\n"
		exitFailure
	fi

	if test "t${ENABLE_STD_PAYLOADS}" = "tnone"; then
		if test -d "${BUILD_DIR}/payloads_callm"; then
			printMessage "Standard payloads (callm) are enabled but they should not be.\n"
			exitFailure
		fi
		if test -d "${BUILD_DIR}/payloads_fsim"; then
			printMessage "Standard payloads (fsim) are enabled but they should not be.\n"
			exitFailure
		fi
	fi

	# Build the system
	printMessage "===== Building the system =====\n"
	KBUILD_VERBOSE=1 ${MAKE_CMD} >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to build the system\n"
		exitFailure
	fi
	printSeparator
	
	# Install the system
	printMessage "===== Installing the system =====\n"
	${MAKE_CMD} install >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to install the system to ${INSTALL_DIR}\n"
		exitFailure
	fi
	printSeparator

	TEST_DIR="${INSTALL_DIR}/var/kedr/tests"
	runTests

	####################################################################
	cd "${WORK_DIR}" || exitFailure

	####################################################################
	# Uninstall
	printMessage "===== Uninstalling the system =====\n"
	cd "${BUILD_DIR}" || exitFailure
	${MAKE_CMD} uninstall >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to uninstall the system\n"
		exitFailure
	fi
	printSeparator
}

########################################################################
# Scenario: ("global" installation, copying and building examples, etc.)
# - configure KEDR to install to a default location
# - build
# - install 
# - (if CHECK_EXAMPLES is "yes") copy the installed examples to a particular
#   directory move the build tree to a temporary dir, try to build the 
#   examples and then restore the build tree
# - (if CHECK_KEDR_SELFTEST is "yes") run the tests 
# - uninstall
########################################################################
checkScenarioGlobal()
{
	printMessage "\n"
	printMessage "Checking scenario: \"global\" installation\n" 
	printMessage "CHECK_KEDR_SELFTEST is \"${CHECK_KEDR_SELFTEST}\"\n"
	printMessage "CHECK_EXAMPLES is \"${CHECK_EXAMPLES}\"\n"
	printMessage "ENABLE_STD_PAYLOADS is \"${ENABLE_STD_PAYLOADS}\"\n"
	printMessage "\n"

	# CMAKE_INSTALL_PREFIX defaults to "/usr/local" on Linux
	INSTALL_DIR="/usr/local" 

	cd "${WORK_DIR}" || exitFailure
	rm -rf "${ARCHIVE_DIR}" "${BUILD_DIR}"
	rm -rf "${EXAMPLES_DIR}" "${TEMP_DIR}"

	####################################################################
	{
		mkdir -p "${BUILD_DIR}" && \
		mkdir -p "${EXAMPLES_DIR}" && \
		mkdir -p "${TEMP_DIR}"
	} >> "${LOG_FILE}" 2>&1

	# Unpack the archive
	tar xfj "${ARCHIVE_FILE}" 
	if test $? -ne 0; then
		printMessage "Failed to unpack ${ARCHIVE_FILE}\n"
		exitFailure
	fi
	printSeparator

	####################################################################
	# Configure, build and install the system
	cd "${BUILD_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to change directory to ${BUILD_DIR}\n"
		exitFailure
	fi

	add_cmake_options=
	if test "t${ENABLE_STD_PAYLOADS}" = "tnone"; then
		add_cmake_options="-DKEDR_STANDARD_CALLM_PAYLOADS=OFF -DKEDR_STANDARD_FSIM_PAYLOADS=OFF -DKEDR_LEAK_CHECK=OFF"
	elif test "t${ENABLE_STD_PAYLOADS}" = "tall"; then
		add_cmake_options="-DKEDR_STANDARD_CALLM_PAYLOADS=ON -DKEDR_STANDARD_FSIM_PAYLOADS=ON -DKEDR_LEAK_CHECK=ON"
	fi
	
	if test "t${CHECK_KEDR_SELFTEST}" = "tyes"; then
		add_cmake_options="${add_cmake_options} -DWITH_TESTING=yes"
	fi

	# Configure the build
	printMessage "===== Configuring the system =====\n"
	cmake ${add_cmake_options} \
		"${WORK_DIR}/${ARCHIVE_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to configure the system\n"
		exitFailure
	fi

	# Build the system
	printMessage "===== Building the system =====\n"
	${MAKE_CMD} >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to build the system\n"
		exitFailure
	fi
	printSeparator
	
	# Install the system
	printMessage "===== Installing the system =====\n"
	${MAKE_CMD} install >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to install the system to ${INSTALL_DIR}\n"
		exitFailure
	fi
	printSeparator

	if test "t${CHECK_EXAMPLES}" = "tyes"; then
		buildExamples
	fi
	####################################################################
	
	TEST_DIR="/var/opt/kedr/tests"
	runTests
	####################################################################

	# Uninstall
	printMessage "===== Uninstalling the system =====\n"
	cd "${BUILD_DIR}" || exitFailure
	${MAKE_CMD} uninstall >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to uninstall the system\n"
		exitFailure
	fi
	printSeparator
}

########################################################################
# Configure, build and install the given part of KEDR: user-mode or 
# kernel-mode.
#
# Usage: processPart <build_dir> <mode>
# <build_dir> - the absolute path to the build directory.
# <mode> - "user-mode" or "kernel-mode".
########################################################################
processPart()
{
	build_dir=$1
	mode=$2

	if test -z "${build_dir}"; then
		printMessage "Usage: processPart <build_dir> <mode>\n"
		exitFailure
	fi

	cd "${build_dir}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to change directory to ${build_dir}\n"
		exitFailure
	fi

	if test "t${mode}" = "tuser-mode"; then
		mode_option="-DUSER_PART_ONLY=yes"
	elif test "t${mode}" = "tkernel-mode"; then
		mode_option="-DKERNEL_PART_ONLY=yes"
	else
		printMessage "Unsupported mode: ${mode}\n"
		exitFailure
	fi

	printMessage "===== Configuring the ${mode} part of KEDR =====\n"
	cmake \
		${mode_option} \
		-DKEDR_STANDARD_CALLM_PAYLOADS=ON \
		-DKEDR_STANDARD_FSIM_PAYLOADS=ON \
		-DKEDR_LEAK_CHECK=ON \
		-DWITH_TESTING=yes \
		"${WORK_DIR}/${ARCHIVE_DIR}" >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to configure the system\n"
		exitFailure
	fi

	# Build
	printMessage "===== Building the ${mode} part of KEDR =====\n"
	${MAKE_CMD} >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to build the system\n"
		exitFailure
	fi
	printSeparator
	
	# Install
	printMessage "===== Installing the ${mode} part of KEDR =====\n"
	${MAKE_CMD} install >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to install the system\n"
		exitFailure
	fi
	printSeparator

	cd "${WORK_DIR}" || exitFailure
}

########################################################################
# Same as checkScenarioGlobal but with support for multi-kernel builds.
# All standard plugins are enabled. Tests are also enabled.
########################################################################
checkScenarioGlobalMultiKernel()
{
	printMessage "\n"
	printMessage "Checking scenario: multi-kernel build.\n"
	printMessage "\n"

	# CMAKE_INSTALL_PREFIX defaults to "/usr/local" on Linux
	INSTALL_DIR="/usr/local" 

	cd "${WORK_DIR}" || exitFailure
	rm -rf "${ARCHIVE_DIR}" "${BUILD_DIR_USER}" "${BUILD_DIR_KERNEL}"
	rm -rf "${EXAMPLES_DIR}" "${TEMP_DIR}"

	####################################################################
	{
		mkdir -p "${BUILD_DIR_USER}" && \
		mkdir -p "${BUILD_DIR_KERNEL}" && \
		mkdir -p "${EXAMPLES_DIR}" && \
		mkdir -p "${TEMP_DIR}"
	} >> "${LOG_FILE}" 2>&1

	# Unpack the archive
	tar xfj "${ARCHIVE_FILE}" 
	if test $? -ne 0; then
		printMessage "Failed to unpack ${ARCHIVE_FILE}\n"
		exitFailure
	fi
	printSeparator

	####################################################################
	# Configure, build and install the system
	processPart "${BUILD_DIR_USER}"   "user-mode"
	processPart "${BUILD_DIR_KERNEL}" "kernel-mode"
	####################################################################
	
	CHECK_KEDR_SELFTEST=yes
	TEST_DIR="/var/opt/kedr/tests"
	runTests
	####################################################################

	# Uninstall
	printMessage "===== Uninstalling the system =====\n"
	cd "${BUILD_DIR_KERNEL}" || exitFailure
	${MAKE_CMD} uninstall >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to uninstall the kernel-mode part.\n"
		exitFailure
	fi

	cd "${BUILD_DIR_USER}" || exitFailure
	${MAKE_CMD} uninstall >> "${LOG_FILE}" 2>&1
	if test $? -ne 0; then
		printMessage "Failed to uninstall the user-mode part.\n"
		exitFailure
	fi
	printSeparator
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

INSTALL_DIR="${WORK_DIR}/install"

# make will use the following number of "jobs" to perform the build, etc.
# [NB] 'make' should always be invoked in "parallel mode" (make -j N) here.
# This may slow the things down a bit on uniprocessor systems but it is OK
# as we need to check parallel builds anyway.
# This also makes separate checkParallelBuild() scenarios obsolete.
#
# [!!!] Make sure ${MAKE_CMD} is always used here rather than plain 'make'.
KEDR_NJOBS=4
MAKE_CMD="make -j ${KEDR_NJOBS}"

rm -rf "${LOG_FILE}" 
rm -rf "${ARCHIVE_DIR}" "${BUILD_DIR}" "${INSTALL_DIR}" 
rm -rf "${EXAMPLES_DIR}" "${TEMP_DIR}"

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

CHECK_EXAMPLES=no

# Check multi-kernel build. All standard plugins are enabled.
checkScenarioGlobalMultiKernel

# Check "global" installation. Also check if the installed examples
# can be built without the binary tree of KEDR available.
ENABLE_STD_PAYLOADS=default
CHECK_KEDR_SELFTEST=yes
CHECK_EXAMPLES=yes
checkScenarioGlobal

CHECK_EXAMPLES=no

# Build and test KEDR with all standard plugins (payload modules) disabled.
ENABLE_STD_PAYLOADS=none
CHECK_KEDR_SELFTEST=yes
checkScenarioGlobal

# Now check everything without running the tests.
# This is to make sure installation does not depend on anything intended
# for tests only.
ENABLE_STD_PAYLOADS=default
CHECK_KEDR_SELFTEST=no
checkScenarioGlobal

# "Local" installation.
# Build and test KEDR with the default set of standard plugins (payload 
# modules) enabled.
ENABLE_STD_PAYLOADS=default
CHECK_KEDR_SELFTEST=yes
checkScenarioLocal

# NOTE: 
# If you would like to add more scenarios to check if the system operates
# correctly, add them here.
# Use 'exitFailure' to abort this script if a failure has been detected.
# Use 'printMessage' to output messages.
# Use ${MAKE_CMD} instead of 'make'.

#######################################################################
exitSuccess
