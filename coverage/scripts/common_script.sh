#!/bin/sh

# Execute given test scenario and measure its time and
# coverage of FS driver, triggered by the test.

# Usage: see 'print_usage' function (or call with -h option)

printf_error()
{
    printf "$@" >&2
}

printf_status()
{
    printf "$@" >&2
}


print_usage()
{
cat <<EOF
Usage:

   common_script.sh OPTIONS <scenario_program> [params...]

Run
   <scenario_program> [params...]
and store coverage trace into file 'trace.info' and time(in seconds) into
file 'time_elapsed'

Return 0 if preparations for running scenario and after-scenario actions
for storing coverage trace are performed successfully. Otherwise
return 1, and futher executiong this program are meaningless: it failed
to prepare to the test.

Coverage trace include device(s) mouning and module loading before test
and filesystem unmount and module unloading after test.
If failed to unmount device or unload module, coverage trace will
be stored anyway (if possible), but error indicator will be returned.


	common_script.sh -h|--help

Print given help and return.

Options:
	-f|filesystem <fs>
		Filesystem to be tested (as it is given in mkfs).
		This option is required.
	-d|--directory <dir>
		Directory where to store resulted files: coverage trace and file.
		If not specified, current directory ('.') is used.

	--device <dev>
		Block device to be mount during scenario. There should be a line
		in '/etc/fstab', which describe mounting of that device.
		Currently only device path is supported: /dev/sdb.
		
		Before mounting, device is formatted to the testing filesystem,
		so every scenario sees empty filesystem.
		
		Several options may be specified, in that case all given
		devices will be mounted.
		
		If not specified, /dev/hdb filesystem is used.
	--format-command <command>
		Command format device(s) under test to tested filesystem.
		Note, that currently arguments are not supported for command(TODO).
		By default,
		
			mkfs -t <fs> <dev>
		
		is used for every device under test.
	-m|--module <module>
		Module which supports filesystem under test.
		This module is (re)loading before test, unloaded after test,
		then coverage for it is stored.
		Default is	"<fs>"
EOF
}

# Inside script 'time' is interpreted as builtin. Use full filename
# for use 'time' as utility.
time_utility=`which time`
if test -z "${time_utility}"; then
	printf_error "'time' utility is not found.\n"
	exit 1
fi

# Script for collect module coverage
get_coverage_script="`dirname $0`/get_coverage.sh"



TEMP=`getopt -o "+d:f:m:h" \
-l "directory:,filesystem:,module:,device:,format-command:,help" \
-n "$0" -- "$@"`

if test $? != 0; then
    print_usage
    exit 1
fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

# Default directory for store results.
result_dir=.
# Array of devices, delimited by ' '.
# NOTE: Do not use real arrays, because them are not supported by some
# shells (like dash).
test_dev=
# Filesystem under test.
test_fs=
# Module which supports given filesystem.
module_name=
# Device format command.
format_command=


while true ; do
    case "$1" in
    -d|--directory)
        result_dir="$2"
        shift 2;;
	-f|--filesystem)
		test_fs="$2"
		shift 2;;
	--device)
		# Push back value into 'test_fs' array.
		if test -z "${test_dev}"; then
			test_dev="$2"
		else
			test_dev="${test_dev} $2"
		fi
        shift 2;;
    -m|--module)
		module_name="$2"
		shift 2;;
	--format-command)
		format_command="$2"
		shift 2;;
    -h|--help)
        print_usage
        exit 0;;
    --) shift ; break ;;
    *) printf_error "Internal error!" ; exit 1 ;;
    esac
done

if test -z "${test_fs}"; then
	printf_error "Filesystem for test should be specified\n"
	exit 1
fi

if test -z "${module_name}"; then
	module_name=${test_fs}
fi

if test -z "${test_dev}"; then
	# Default device
	test_dev="/dev/hdb"
fi

# Deduce source directory for module
module_source_dir="fs/${module_name}"

# Check whether given device is mounted.
# Return 0 if it is, 1 if it is not.
#
# Usage: dev_is_mount dev
dev_is_mount()
{
	dev=$1
	mount | grep "^${dev} " > /dev/null
}

# Mount given device
# Return 0 on success, otherwise print error message and return non-zero.
#
# Usage: dev_mount dev

dev_mount()
{
	dev=$1
	if ! mount "${dev}"; then
		printf_error "Failed to mount devices '%s'\n" "${dev}"
		return 1
	fi
	
	return 0
}


# Unmount given device
# Return 0 on success, otherwise print error message and return non-zero.
#
# Usage: dev_unmount dev

dev_unmount()
{
	dev=$1
	if ! umount $dev; then
		printf_error "Failed to unmount device '%s'\n" "${dev}"
		return 1
	fi
	
	return 0
}

# (Re)Load module and (re)mount devices
# Return 0 on success and non-zero code on fail.
prepare_all()
{
	for dev in ${test_dev}; do
		if dev_is_mount "${dev}"; then
			printf_status "Unmount already mounted device '%s'...\n" "${dev}"
			if ! dev_unmount $dev; then
				return 1
			fi
		fi
	done

	if lsmod | grep "${module_name}" > /dev/null; then
		printf_status "Unload already loaded module %s.\n" "${module_name}"
		if ! modprobe -r "${module_name}"; then
			return 1
		fi
	fi

	printf_status "Create/clean filesystem(s) for test...\n"
	if test -n "${format_command}"; then
		if ! $format_command; then
			printf_error "Failed to format device(s) for test.\n"
			return 1
		fi
	else
		for dev in ${test_dev}; do
			if ! mkfs -t ${test_fs} ${dev}; then
				printf_error "Failed to create filesystem on device '%s' for test.\n" "${dev}"
				return 1
			fi
		done
	fi
	printf_status "Load module %s.\n" "${module_name}"
	if ! modprobe "${module_name}"; then
		printf_error "Failed to load module %s\n" "${module_name}"
		return 1
	fi

	printf_status "Mount device(s) for test.\n"
	for dev in ${test_dev}; do
		if ! dev_mount "${dev}"; then
			return 1
		fi
	done

	printf_status "Module %s is prepared for coverage measurement.\n" "${module_name}"
}

# Unmount filesystems and unload module.
# Return 0 on success and non-zero code on fail.
finalize_all()
{
	printf_status "Unmount device(s) used in tests.\n"
	for dev in ${test_dev}; do
		if dev_is_mount "${dev}"; then
			if ! dev_unmount "${dev}"; then
				return 1
			fi
		fi
	done

	printf_status "Unload module %s.\n" "${module_name}"
	if ! modprobe -r "${module_name}"; then
		printf_error "Failed to unload module.\n"
		return 1
	fi

}

if test "$#" -eq "0"; then
	printf_error "Command for execute test scenario should be specified.\n"
	exit 1
fi
# Create directory for results if it doesn't exist.
if ! test -d "${result_dir}"; then
	if ! mkdir -p "${result_dir}"; then
		printf_error "Failed to create directory for results.\n"
		exit 1
	fi
fi

trace_file="${result_dir}/trace.info"
time_file="${result_dir}/time_elapsed"

# Prepare for testing
if ! prepare_all; then
	printf_error "Failed to prepare for testing.\n"
	exit 1
fi

# Testing
printf_status "Running tests...\n"
${time_utility} -f "%e" -o ${time_file} --quiet "$@"

# Finalize
if ! finalize_all; then
	printf_status "Attempt to store coverage trace, while it is not full..."
	if ${get_coverage_script} -d "${module_source_dir}" "${trace_file}"; then
		printf_status "Partial trace has been stored into '%s'.\n" "${trace_file}"
	fi
	
	exit 1
fi

# Store trace
printf_status "Store coverage trace...\n"
if ! ${get_coverage_script} -d "${module_source_dir}" "${trace_file}"; then
	exit 1
fi
printf_status "Trace has been stored into '%s'.\n" "${trace_file}"

