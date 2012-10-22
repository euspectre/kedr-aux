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

   common_script.sh [OPTIONS] <scenario_program> [params...]

Run
   <scenario_program> [params...]
and store coverage trace into file 'trace.info' and time(in seconds) into
file 'time_elapsed'

Return 0 if preparations for running scenario and after-scenario actions
for storing coverage trace are performed successfully. Otherwise
return 1, and futher executiong this program are meaningless: it failed
to prepare to the test.

Coverage trace include filesystem(s) mouning and module loading before test
and filesystem unmount and module unloading after test.
If failed to unmount filesystem or module unloading, coverage trace will
be stored as possible, but error indicator will be returned.


	common_script.sh -h|--help

Print given help and return.

Options:
	-d|--directory <dir>
		Directory where to store resulted files: coverage trace and file.
		If not specified, current directory ('.') is used.
	
	-f|--filesystem <fs>
		File system to be mount during scenario. There should be a line
		in '/etc/fstab', which describe mounting of that system.
		Currently only device path is supported as filesystem: /dev/sdb.
		
		Before mounting, filesystem is formatted, so every scenario sees
		empty filesystem.
		
		Several options may be specified, in that case all given
		filesystems will be mounted.
		
		If not specified, /dev/hdb filesystem is used.
EOF
}

# Inside script 'time' is interpreted as builtin. Use full filename
# for use 'time' as utility.
time_utility=`which time`
if test -z "${time_utility}"; then
	printf_error "'time' utility is not found.\n"
	exit 1
fi

TEMP=`getopt -o "+d:f:h" -l "directory:filesystem:,help" -n "$0" -- "$@"`

if test $? != 0; then
    print_usage
    exit 1
fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

# Default directory for store results.
result_dir=.
# Array of filesystems, delimited by ' '.
# NOTE: Do not use real arrays, because them are not supported by some
# shells (like dash).
test_fs=

while true ; do
    case "$1" in
    -d|--directory)
        result_dir="$2"
        shift 2;;
	-f|--filesystem)
		# Push back value into 'test_fs' array.
		if test -z "${test_fs}"; then
			test_fs="$2"
		else
			test_fs="${test_fs} $2"
		fi
        shift 2;;
    -h|--help)
        print_usage
        exit 0;;
    --) shift ; break ;;
    *) printf_error "Internal error!" ; exit 1 ;;
    esac
done

if test -z "${test_fs}"; then
	# Default fs (currently hardcoded)
	test_fs="/dev/hdb"
fi

# Name of the tested module(currently hardcoded)
module_name=xfs
# Deduce filesystem type from fs driver name.
fs_type=${module_name}
# Deduce source directory for module
module_source_dir="fs/${module_name}"

# Check whether given filesystem is mounted.
# Return 0 if it is, 1 if it is not.
#
# Usage: fs_is_mount fs
fs_is_mount()
{
	fs=$1
	mount | grep "${fs}" > /dev/null
}

# Mount given filesystem
# Return 0 on success, otherwise print error message and return non-zero.
#
# Usage: fs_mount fs

fs_mount()
{
	fs=$1
	if ! mount "${fs}"; then
		printf_error "Failed to mount filesystem '%s'\n" "${fs}"
		return 1
	fi
	
	return 0
}


# Unmount given filesystem
# Return 0 on success, otherwise print error message and return non-zero.
#
# Usage: fs_unmount fs

fs_unmount()
{
	fs=$1
	if ! umount $fs; then
		printf_error "Failed to unmount filesystem '%s'\n" "${fs}"
		return 1
	fi
	
	return 0
}

# (Re)Load module and (re)mount filesystems
# Return 0 on success and non-zero code on fail.
prepare_all()
{
	for fs in ${test_fs}; do
		if fs_is_mount "${fs}"; then
			printf_status "Unmount already mounted filesystem '%s'...\n" "${fs}"
			if ! fs_unmount $fs; then
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
	for fs in ${test_fs}; do
		if ! mkfs -t ${fs_type} -q -f ${fs}; then
			printf_error "Failed to create filesystem on device '%s' for test.\n" "${fs}"
			return 1
		fi
	done

	printf_status "Load module %s.\n" "${module_name}"
	if ! modprobe "${module_name}"; then
		printf_error "Failed to load module %s\n" "${module_name}"
		return 1
	fi

	printf_status "Mount filesystem(s) for test.\n"
	for fs in ${test_fs}; do
		if ! fs_mount "${fs}"; then
			return 1
		fi
	done

	printf_status "Module %s is prepared for coverage measurement.\n" "${module_name}"
}

# Unmount filesystems and unload module.
# Return 0 on success and non-zero code on fail.
finalize_all()
{
	printf_status "Unmount filesystem(s) used in tests.\n"
	for fs in ${test_fs}; do
		if fs_is_mount "${fs}"; then
			if ! fs_unmount "${fs}"; then
				return 1
			fi
		fi
	done

	printf_status "Unload module %s.\n" "${module_name}"
	if ! modprobe -r "${module_name}"; then
		printf_error "Failed to unload module.\n"
		printf_status "Attempt to store trace, while it is not full...\n"
		if ${get_coverage_script} "${trace_file}"; then
			printf_status "Partial trace has been stored into '%s'.\n" "${trace_file}"
		fi
		exit 1
	fi

}

# Script for collect module coverage
get_coverage_script="/home/tester/kedr/aux-sources/coverage/scripts/get_coverage.sh -d ${module_source_dir}"


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
	if ${get_coverage_script} "${trace_file}"; then
		printf_status "Partial trace has been stored into '%s'.\n" "${trace_file}"
	fi
	
	exit 1
fi

# Store trace
printf_status "Store coverage trace...\n"
if ! ${get_coverage_script} "${trace_file}"; then
	exit 1
fi
printf_status "Trace has been stored into '%s'.\n" "${trace_file}"

