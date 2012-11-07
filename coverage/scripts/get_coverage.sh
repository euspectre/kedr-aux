#!/bin/sh

# 
# Store current kernel coverage information into trace file.
#
# Trace file is generated using lcov utility, so it has corresponded
# format.
#
# Also may perform additional coverage-related requests:
# checking for coverage support for current kernel and reset coverage
# counters.
# 

# Usage:
# 
#    get_coverage.sh [options] trace_file
#
#        Collect coverage information and store it into 'trace_file'.
#
#    get_coverage.sh [options] -c|--check
#    
#        Check that coverage is supported by the kernel(coverage
#        information is present).
#
#    get_coverage.sh [options] -r|--reset
#
#        Reset coverage counters.
#    
#    get_coverage.sh -h|--help
#
#        Print given help and exit.
#   
# In all cases 'options' may be:
#
#    -d|--directory <dir>
#        
#        If given, coverage information will be limited to this source
#      directory.
#       
#        'dir' should be relative directory, such as "fs/xfs".
#

print_usage()
{
cat <<EOF
Usage:

   get_coverage.sh [options] trace_file

       Collect coverage information and store it into 'trace_file'.

   get_coverage.sh [options] -c|--check
   
       Check that coverage is supported by the kernel(coverage
       information is present).

   get_coverage.sh [options] -r|--reset

       Reset coverage counters.

   get_coverage.sh -h|--help

       Print given help and exit.


In all cases 'options' may be:

   -d|--directory <dir>
       
       If given, coverage information will be limited to this source
     directory.
      
       'dir' should be relative directory, such as "fs/xfs".
EOF
}



printf_error()
{
    printf "$@" >&2
}



# Kernel build directory with all links resolved.
#
# Last fact is crusial, because this directory is used as subdirectory
# in gcov-hierarchy in debugfs.
BUILD_DIR=$(readlink -f -n /lib/modules/`uname -r`/build)

if ! test -d ${BUILD_DIR}; then
    printf_error "Failed to find sources from which kernel has built:\n"
    printf_error "Directory '%s' is not exist.\n" ${BUILD_DIR}
    exit 
fi

# Debugfs is expected to be mounted here.
debug_fs="/sys/kernel/debug"

# Check whether coverage is supported by kernel.
#
# If called with argument, interpret it as source (sub)directory
# and check whether coverage information is exits for it.
check_coverage_support()
{
    gcov_dir="${debug_fs}/gcov"
    if ! test -d "${gcov_dir}"; then
        printf_error "Coverage is not supported by the kernel(or debugfs is not mounted into '%s')\n" $1
        return 1
    fi
    
    if test "$1"; then
        coverage_dir="${gcov_dir}/${BUILD_DIR}/$1"
        if ! test -d "${coverage_dir}"; then
            printf_error "There is no coverage information for directory '%s'\n" $1
        fi
    fi
    
    return 0
}

# '--directory' argument, if exist. Otherwise empty.
source_dir=

# Request type. "default" - write trace, "reset" - reset, "check" - check.
mode="default"

TEMP=`getopt -o "d:hcr" -l "directory:,check,reset,help" -n "$0" -- "$@"`

if test $? != 0; then
    print_usage
    exit 1
fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

module_name=

while true ; do
    case "$1" in
    -d|--directory)
        if test "${source_dir}"; then
            printf_error "Several '--directory' options are currently not supported.\n"
            exit 1
        fi
        source_dir=$2
        shift 2;;
    -c|--check)
        case "$mode" in
        default)
            mode="check";;
        reset)
            print_error "Error: Both '$1' and 'reset' options are given.\n"
            exit 1;;
        check)
            print_error "Error: Double '$1' option.\n"
            exit 1;;
        esac
        shift;;
    -r|--reset)
        case "$mode" in
        default)
            mode="reset";;
        reset)
            print_error "Error: Double '$1' option.\n"
            exit 1;;
        check)
            print_error "Error: Both '$1' and 'check' options are given.\n"
            exit 1;;
        esac
        shift;;
    -h|--help)
        print_usage
        exit 0;;
    --) shift ; break ;;
    *) printf_error "Internal error!" ; exit 1 ;;
    esac
done

case "$mode" in
default)
    # Temporary directory for copy coverage counters from debugfs
    tmp_counters_dir="/var/tmp/script_get_coverage/counters"
    
    if ! test "$1"; then
        printf_error "Trace file should be specified.\n"
        exit 1
    fi
    
    trace_file="$1"

    if test -d "$tmp_counters_dir"; then
        rm -rf $tmp_counters_dir
    fi
    if ! mkdir -p "$tmp_counters_dir"; then
        printf_error "Failed to create temporary directory for counters.\n"
        exit 1
    fi

    if ! check_coverage_support "${source_dir}"; then
        exit 1
    fi
    
    coverage_dir="${debug_fs}/gcov/${BUILD_DIR}"
    if test "${source_dir}"; then
        coverage_dir=${coverage_dir}/${source_dir}
    fi

    #TODO: make files expansion safe in case of empty dir(if that case is possible).
    if ! cp -L -r -t "$tmp_counters_dir" $coverage_dir/*; then
        printf_error "Error while copiing counters into temporary directory.\n"
        exit 1
    fi
    
    if ! lcov -q -c -d "$tmp_counters_dir" -b "$BUILD_DIR" -o "$trace_file"; then
        printf_error "Lcov failed to collect coverage statistics.\n"
        exit 1
    fi
    ;;
check)
    if ! check_coverage_support "${source_dir}"; then
        exit 1
    fi
    ;;
reset)
    if test "${source_dir}"; then
        if ! check_coverage_support "${source_dir}"; then
            exit 1
        fi
        coverage_dir="${debug_fs}/gcov/${BUILD_DIR}/${source_dir}"
        for file in `find "${coverage_dir}" -name "*.gcda"`; do
            if ! echo 0 > "$file"; then
                printf_error "Failed to reset counter '$file'.\n"
                exit 1
            fi
        done
    else
        gcov_reset_file="${debug_fs}/gcov/reset"
        if ! echo 0 > "$gcov_reset_file"; then
            printf_error "Failed to reset coverage counters.\n"
            exit 1
        fi
    fi
    ;;
esac
