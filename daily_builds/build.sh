#!/bin/bash
########################################################################
# This script launches the build system for each machine specified in.
# 'machines.list' (each line has the form "NAME IP_ADDRESS").
#
# Parameters are loaded from 'build.conf'. See the comments in that file for
# a description of each parameter.
#
# If "email_logs" parameter is set, the results will be sent to the recipients 
# (subscribers) whose addresses are listed in 'subscribers.list'.
########################################################################

if test $# -ne 0 ; then
    printf "Usage: $0\n"
    exit 0
fi

########################################################################
# Globals
########################################################################
WORK_DIR=${PWD}

CONF_FILE="${WORK_DIR}/build.conf"
MACHINES_FILE="${WORK_DIR}/machines.list"
SUBSCRIBERS_FILE="${WORK_DIR}/subscribers.list"

# Name of the file with the names of the tests that may fail
# (the file should be located in ${WORK_DIR})
MAY_FAIL_FILE="may_fail.list"

RESULT_DIR="${WORK_DIR}/results"

# Main log file on the host machine
MAIN_LOG="${RESULT_DIR}/host_main.log"

# Summary
SUMMARY_FILE="${RESULT_DIR}/summary.txt"

# Archive containing the results
RESULT_ARCHIVE=""

# Main directory on the target machine to operate in
# (will be set by preparePackage())
TARGET_DIR=""

# Name of the main log file on the target machine
# Should be consistent with what the script on the target machine uses.
TARGET_LOG="build.log"

# Name of the archive (without extension) with sources
# (will be set by preparePackage())
ARCHIVE_NAME=""

# Full name of the archive with sources
# (will be set by preparePackage())
ARCHIVE_FILE=""

# Full name of the script to be executed on target machines
# (will be set by getScriptForTarget())
SCRIPT_FILE=""

########################################################################
# Configuration parameters (see also build.conf file)
########################################################################

# Mercurial repository containing the source code of the system
REPO_MAIN=""

# Mercurial repository containing auxiliary files like the build script, etc.
REPO_AUX=""

# Path to the script (in 'repo_aux' repository) to be executed on each of 
# the target machines. 
# The script will be called with name of the source archive as a parameter
# It is that script that actually builds and tests the system on the target 
# machine, etc.
EXEC_SCRIPT=""

# Name of the package
PACKAGE_NAME=""

# Version of the package
PACKAGE_VERSION=""

# If this is not empty, the revision number will be appended to the name of
# the source package. 
#
# The exact value of this parameter does not matter, it is only checked 
# whether it is empty or not.
APPEND_REVISION=""

# If this is not empty, the build system will try to start required virtual
# machines automatically, shut them down when done testing and restore 
# the latest snapshot of each one after that.
# Otherwise, it is assumed that the machine is managed manually.
#
# The exact value of this parameter does not matter, it is only checked 
# whether it is empty or not.
MANAGE_VM=""

# If both this parameter and 'manage_vm' have non-empty values, the build 
# system will attempt to restore the latest snapshot of each virtual machine
# before starting the machine.
# If 'manage_vm' is empty, the parameter is ignored (as if it were empty 
# too).
#
# The exact value of this parameter does not matter, it is only checked 
# whether it is empty or not.
RESTORE_CURRENT_SNAPSHOT=""

# If this is not empty, the build system will send an archive containing 
# the logs to the subscribers. The addresses of the subscribers will be read
# from 'subscribers.list' file.
#
# The exact value of this parameter does not matter, it is only checked 
# whether it is empty or not.
EMAIL_LOGS=""

# How long (in seconds) to wait for a virtual machine to start up or
# shut down.
VM_TIMEOUT=""

# How long (in seconds) to wait for a virtual machine to execute a command.
VM_MAX_TIME=3600

# User name on the target virtual machine (not related to the user on the 
# host machine that launches the build system).
VM_USER=""

# Password for the user on the target virtual machine. The target machines 
# are assumed to be stored and used locally (probably on the host machine 
# itself). So the lack of security should not be a big problem.
VM_PASS=""

########################################################################
# Utility functions
########################################################################

# The function outputs the message specified as its arguments 
# both to the summary log and to stdout
printMessage()
{
    printf "$*" >> "${MAIN_LOG}"
    printf "$*"
}

printSeparator()
{
    printMessage "========================================================\n"
}

# Check if a parameter has a non-empty value (name - $1, value - $2)
checkParam()
{
    if test -z "$2"; then
        printMessage "Parameter $1 should have a non-empty value\n"
        exit 1
    fi    
}

########################################################################
# Load the configuration parameters
########################################################################
loadConfiguration()
{
    if test ! -f "$1"; then
        printMessage "Failed to open configuration file: $1\n"
        exit 1
    fi
    printMessage "Loading configuration from $1\n"
    
    OLD_IFS=${IFS}
    IFS=$'\n'

    for LINE in $(cat $1); do
        IFS=${OLD_IFS}

# Ignore the line if is it blank or is a comment
        echo "${LINE}" | grep -E '(^[\t ]*#)|(^[\t ]*$)' > /dev/null
        if test $? -eq 1; then
            echo "${LINE}" | grep -E '^[\t ]*[A-Za-z0-9_][A-Za-z0-9_]*[\t ]*=.*' > /dev/null
            if test $? -ne 0; then
                printf "$1: expected definition in NAME = VALUE format\n"
                exit 1
            fi
            
# Begin processing of the line
# Get everything before the first '=' character, strip spaces.
            PAR_NAME=$(echo "${LINE}" | sed -e 's/^[\t ]*\([A-Za-z0-9_][A-Za-z0-9_]*\)[\t ]*=.*/\1/')
            
# Get everything starting from the first non-space character after 
# the first '='
            PAR_VALUE=$(echo "${LINE}" | sed -e 's/^[^=]*=[\t ]*\(.*\)/\1/')
            #printMessage "Name: \"${PAR_NAME}\", VALUE: \"${PAR_VALUE}\"\n" 

# Set the values of the internal variables
            case "${PAR_NAME}" in
            "repo_main")
                REPO_MAIN=${PAR_VALUE}
                ;;
            "repo_aux")
                REPO_AUX=${PAR_VALUE}
                ;;
            "exec_script")
                EXEC_SCRIPT=${PAR_VALUE}
                ;;
            "package_name")
                PACKAGE_NAME=${PAR_VALUE}
                ;;
            "package_version")
                PACKAGE_VERSION=${PAR_VALUE}
                ;;
            "append_revision")
                APPEND_REVISION=${PAR_VALUE}
                ;;
            "manage_vm")
                MANAGE_VM=${PAR_VALUE}
                ;;
            "restore_current_snapshot")
                RESTORE_CURRENT_SNAPSHOT=${PAR_VALUE}
                ;;
            "email_logs")
                EMAIL_LOGS=${PAR_VALUE}
                ;;
            "vm_timeout")
                VM_TIMEOUT=${PAR_VALUE}
                ;;
            "vm_max_time")
                VM_MAX_TIME=${PAR_VALUE}
                ;;
            "vm_user")
                VM_USER=${PAR_VALUE}
                ;;
            "vm_pass")
                VM_PASS=${PAR_VALUE}
                ;;
            *)
                printMessage "$1: unknown parameter \"${PAR_NAME}\"\n"
                exit 1
                ;;
            esac
# End processing of the line    
        fi
        IFS=$'\n'
    done
    IFS=${OLD_IFS}
    
# Check if all required parameters are defined.
# append_revision, manage_vm and email_logs are optional. Default value: "".
    checkParam "repo_main"  "${REPO_MAIN}"
    checkParam "repo_aux"   "${REPO_AUX}"
    checkParam "exec_script"  "${EXEC_SCRIPT}"
    checkParam "package_name" "${PACKAGE_NAME}"
    checkParam "package_version" "${PACKAGE_VERSION}"
    checkParam "vm_timeout" "${VM_TIMEOUT}"
    checkParam "vm_max_time" "${VM_MAX_TIME}"
    checkParam "vm_user"    "${VM_USER}"
    checkParam "vm_pass"    "${VM_PASS}"
}

########################################################################
# Get the script to be executed on the target machines from the 
# Mercurial repository.
########################################################################
getScriptForTarget()
{
    AUX_DIR=temp_aux
    SCRIPT_FILE=`basename ${EXEC_SCRIPT}`
        
    cd "${WORK_DIR}"
    printMessage "Retrieving the script to be executed on target machines from\n"
    printMessage "${REPO_AUX}\n\n"

    rm -f "${SCRIPT_FILE}"
    rm -rf "${AUX_DIR}"
    
    hg --version >> "${MAIN_LOG}"
    hg clone "${REPO_AUX}" "${AUX_DIR}" >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "Unable to retrieve the script from the following repository:\n"
        printMessage "${REPO_AUX}\n\n"
        exit 1
    fi
    
    cp "${AUX_DIR}/${EXEC_SCRIPT}" . >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "${EXEC_SCRIPT} is missing from the repository.\n\n"
        rm -rf "${AUX_DIR}"
        exit 1
    fi
    
    chmod 755 "${SCRIPT_FILE}" >> "${MAIN_LOG}" 2>&1
   
    rm -rf "${AUX_DIR}"
}

########################################################################
# Prepare the source package
########################################################################
preparePackage()
{
    SRC_DIR=temp_src
    ARCHIVE_NAME="${PACKAGE_NAME}-${PACKAGE_VERSION}"
    TARGET_DIR="/tmp/${ARCHIVE_NAME}.build"
           
    cd "${WORK_DIR}"
    printMessage "Retrieving the source code of \"${PACKAGE_NAME}\" system from\n"
    printMessage "${REPO_MAIN}\n\n"
    
    rm -rf ${SRC_DIR}
    rm -rf ${ARCHIVE_NAME}*
    
    hg --version >> "${MAIN_LOG}"
    hg clone "${REPO_MAIN}" "${SRC_DIR}" >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "Unable to retrieve the sources from the following repository:\n"
        printMessage "${REPO_MAIN}\n\n"
        exit 1
    fi
    
    # Append revision to the name of the package if requested.
    if test -n "${APPEND_REVISION}"; then
        REVISION=$(hg id -i ${SRC_DIR})
        ARCHIVE_NAME="${ARCHIVE_NAME}-${REVISION}"
    fi

    ARCHIVE_DIR="${ARCHIVE_NAME}"
    mv "${SRC_DIR}/sources" "${ARCHIVE_DIR}" >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        cd "${WORK_DIR}"
        printMessage "Failed to arrange files in the source package\n"
        
        rm -rf "${SRC_DIR}"
        rm -rf "${ARCHIVE_DIR}"
        exit 1
    fi
    
    cd "${WORK_DIR}"
    ARCHIVE_FILE=${ARCHIVE_NAME}.tar.bz2
    tar cjf "${ARCHIVE_FILE}" "${ARCHIVE_DIR}" >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "Failed to prepare the source package\n"
        
        rm -rf "${SRC_DIR}"
        rm -rf "${ARCHIVE_DIR}"
        exit 1
    fi
    
    rm -rf "${SRC_DIR}"
    rm -rf "${ARCHIVE_DIR}"
}

########################################################################
# Execute a command on the specified target machine.
# 
# Usage:
#   execCommandOnTarget <machine_ip> <command> [timeout]
# If <timeout> is specified, the function waits as many seconds at most for
# the command to complete. If <timeout> is not specified, ${VM_TIMEOUT} is
# used.
#
# The function also uses global configuration variables (VM_USER, etc.)
########################################################################
execCommandOnTarget()
{
    if test -z "$1$2"; then
        printMessage "Error: IP address of the target machine and the command must not be empty\n"
        exit 1
    fi
    
    # [NB] The names are lowercase because they are used only locally
    exec_ip=$1
    exec_command="$2"
    exec_timeout="$3"
    if test -z "${exec_timeout}"; then
        exec_timeout="${VM_TIMEOUT}"
    fi
    
    {
    printf "Executing: ${exec_command}\n"
    expect  -c "set timeout ${exec_timeout}" \
        -c "spawn ssh ${VM_USER}@${exec_ip} \"${exec_command}\"" \
        -c "expect -ex \":\"" \
        -c "send \"${VM_PASS}\n\"" \
        -c "expect eof"
    } >> "${MAIN_LOG}" 2>&1
}

########################################################################
# Upload a file to the specified target machine.
# 
# Usage:
#   uploadToTarget <machine_ip> <what> <where>
# 
# <what>  - a local path to the file to upload
# <where> - a path on the target machine to upload the file to (must not 
#   contain spaces)
#
# The function also uses global configuration variables (VM_USER, etc.)
########################################################################
uploadToTarget()
{
    if test -z "$1$2$3"; then
        printMessage "Error: uploadToTarget(): arguments must not be empty\n"
        exit 1
    fi
    
    # [NB] The names are lowercase because they are used only locally
    upload_ip=$1
    upload_what="$2"
    upload_where="$3"
    
    {
    expect  -c "set timeout ${VM_TIMEOUT}" \
        -c "spawn scp \"${upload_what}\" ${VM_USER}@${upload_ip}:${upload_where}" \
        -c "expect -ex \":\"" \
        -c "send \"${VM_PASS}\n\"" \
        -c "expect eof"
    } >> "${MAIN_LOG}" 2>&1
}

########################################################################
# Download a file from the specified target machine.
# 
# Usage:
#   downloadFromTarget <machine_ip> <what> <where>
# 
# <what>  - a path on the target machine to the file to be downloaded 
#   (must not contain spaces)
# <where> - a local path to place the downloaded file to.
#   
#
# The function also uses global configuration variables (VM_USER, etc.)
########################################################################
downloadFromTarget()
{
    if test -z "$1$2$3"; then
        printMessage "Error: downloadFromTarget(): arguments must not be empty\n"
        exit 1
    fi
    
    # [NB] The names are lowercase because they are used only locally
    download_ip=$1
    download_what="$2"
    download_where="$3"
    
    {
    expect  -c "set timeout ${VM_TIMEOUT}" \
        -c "spawn scp ${VM_USER}@${download_ip}:${download_what} \"${download_where}\"" \
        -c "expect -ex \":\"" \
        -c "send \"${VM_PASS}\n\"" \
        -c "expect eof"
    } >> "${MAIN_LOG}" 2>&1
}

########################################################################
# Do the building and testing on the specified target machine.
# 
# Usage:
#   doTarget <machine_name> <machine_ip>
# The function also uses global configuration variables (VM_USER, etc.)
########################################################################
doTarget()
{
    if test -z "$1$2"; then
        printMessage "Error: name and IP address of a target machine must not be empty\n"
        exit 1
    fi
    
    if test -z "${TARGET_DIR}"; then
        printMessage "Error: main directory on the target machine is not specified\n"
    fi
    
    # [NB] The names are lowercase because they are used only locally
    vm_name=$1
    vm_ip=$2
    
    printMessage "Processing target machine \"${vm_name}\" (${vm_ip})\n"
    MACHINE_RESULT_DIR="${RESULT_DIR}/${vm_name}.result"
    
    # Create directory for the results
    mkdir -p "${MACHINE_RESULT_DIR}" >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "Failed to create directory ${MACHINE_RESULT_DIR}\n"
        exit 1
    fi
    
    if test -n "${MANAGE_VM}"; then
    # Revert to the current snapshot, start the machine, etc.
    {
        if test -n "${RESTORE_CURRENT_SNAPSHOT}"; then
            VBoxManage snapshot ${vm_name} restorecurrent
            if test $? -ne 0; then
                printf "Warning: failed to restore current snapshot for machine \"${vm_name}\"\n"
            fi
        fi
        VBoxHeadless --startvm ${vm_name} --vrdp=off &
        VBOX_HEADLESS_PID=$!
        # $! is the pid of the last backgroud process launched 
        # from this shell.
        
        # Wait a little...
        sleep 3
        # ... and check if 'VboxHeadless' is actually running (in case
        # of an error, it will stop immediately).
        ps -e | grep ${VBOX_HEADLESS_PID} > /dev/null
        if test $? -ne 0; then
            printf "Failed to launch VBoxHeadless tool\n"
        else
            # Wait some more and hope the machine will start before 
            # we finish sleeping
            sleep ${VM_TIMEOUT}
            
            # If the machine has not started up yet, give it some more time
            ping -c 1 ${vm_ip} > /dev/null
            if test $? -ne 0; then
                sleep ${VM_TIMEOUT}
            fi
        fi
    } >> "${MAIN_LOG}" 2>&1
    fi
    
    # Check if the machine is accessible
    ping -c 1 ${vm_ip} > /dev/null
    if test $? -ne 0; then
        printMessage "Machine \"${vm_name}\" (${vm_ip}) is not accessible\n"
        
        # Just in case
        if test -n "${MANAGE_VM}"; then
            printMessage "Turning off \"${vm_name}\" - just in case\n"
            VBoxManage controlvm ${vm_name} poweroff >> "${MAIN_LOG}" 2>&1
        fi
    else
        # The target machine seems to be working, so send the data there,
        # launch the build script there and collect the results.
        # 
        # For now, it is not checked here whether the operations fail or not.
        # It should be found out manually using the collected logs, etc.
        cd "${WORK_DIR}"

        # Create a build directory and upload the sources and the target 
        # script there
        execCommandOnTarget ${vm_ip} "rm -rf ${TARGET_DIR}"
        execCommandOnTarget ${vm_ip} "mkdir -p ${TARGET_DIR}"
        uploadToTarget ${vm_ip} "${ARCHIVE_FILE}" "${TARGET_DIR}"
        uploadToTarget ${vm_ip} "${SCRIPT_FILE}" "${TARGET_DIR}"
        uploadToTarget ${vm_ip} "${MAY_FAIL_FILE}" "${TARGET_DIR}"
        
        # Run the build, etc., on the target system
        execCommandOnTarget \
            ${vm_ip} \
            "${TARGET_DIR}/${SCRIPT_FILE} ${ARCHIVE_NAME}" \
            "${VM_MAX_TIME}"
        
        # Collect the results
        downloadFromTarget ${vm_ip} "${TARGET_DIR}/*.log" "${MACHINE_RESULT_DIR}/"
        
        if test -n "${MANAGE_VM}"; then
            printMessage "Shutting down the target machine\n"
            execCommandOnTarget ${vm_ip} "/sbin/shutdown -h now"
            sleep ${VM_TIMEOUT}
        fi  
    fi
}

########################################################################
# Process the list of target machines.
# Usage:
#   processTargetMachines <machine_list_file>
# For each machine, the list file must contain its name and IP address at 
# the same line.
# #-comments and blank lines are ignored.
#
# The names of machines must contain only the following characters: 
# [A-Za-z0-9_-]
########################################################################
processTargetMachines()
{
    if test ! -f "$1"; then
        printMessage "Failed to open machine list: $1\n"
        exit 1
    fi
    
    printMessage "Machine list file: $1\n"
    
    OLD_IFS=${IFS}
    IFS=$'\n'

    for LINE in $(cat $1); do
        IFS=${OLD_IFS}

# Ignore the line if is it blank or is a comment
        echo "${LINE}" | grep -E '(^[\t ]*#)|(^[\t ]*$)' > /dev/null
        if test $? -eq 1; then
            echo "${LINE}" | grep -E '^[\t ]*[A-Za-z0-9_-]{1,}[\t ]{1,}([0-9]{1,3})(\.[0-9]{1,3}){3}' > /dev/null
            if test $? -ne 0; then
                printf "$1: syntax error in \"${LINE}\"\n"
                printf "Expected the following format: NAME IP_ADDRESS\n"
                exit 1
            fi
# Begin processing of the line

# Get the name of the machine
            MACHINE_NAME=$(echo "${LINE}" | sed -e 's/^[\t ]*\([A-Za-z0-9_-][A-Za-z0-9_-]*\)[\t ][\t ]*.*/\1/')
# Get the IP address
            MACHINE_IP=$(echo "${LINE}" | sed -e 's/^.*[^0-9]\([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\).*/\1/')

# Process the target machine
            printSeparator
            doTarget "${MACHINE_NAME}" "${MACHINE_IP}" 

# End processing of the line    
        fi
        IFS=$'\n'
    done
    IFS=${OLD_IFS}
}

########################################################################
# Analyse the results collected from the target machines, prepare a short
# summary as well as the archive with these results and the summary.
########################################################################
processResults()
{
    cd "${RESULT_DIR}" || exit 1
    for res_dir in *.result; do
        if test -d "${res_dir}"; then
            res_machine=$(echo "${res_dir}" | sed -e 's/\..*//')
            
            # Check if build.log file is present and find out whether it
            # contains error verdict or not
            if test -f "${res_dir}/${TARGET_LOG}"; then
                tail -3 "${res_dir}/${TARGET_LOG}" | grep "Building and testing completed successfully" > /dev/null
                if test $? -ne 0; then
                    printf "${res_machine}: Errors occured, see the logs for details.\n" >> "${SUMMARY_FILE}"
                else
                    printf "${res_machine}: Process completed successfully.\n" >> "${SUMMARY_FILE}"
                fi
            else
                printf "${res_machine}: Failed to complete the process.\n" >> "${SUMMARY_FILE}"
            fi
        fi
    done
    
    cd "${WORK_DIR}" || exit 1
    CURRENT_DATE=$(date)
    printf "\nBuild system finished at ${CURRENT_DATE}\n" >> "${SUMMARY_FILE}"
    
    # Pack the results
    TIMESTR=`date +%Y-%m-%d_%H-%M-%S`
    RESULT_ARCHIVE="results-${TIMESTR}.tar.bz2"
    rm -f *results*.bz2
    
    tar cjf ${RESULT_ARCHIVE} results/ || exit 1
}

########################################################################
# Send the resuls by email to the subscribers
########################################################################
emailResults()
{
    cd "${WORK_DIR}" || exit 1
    bsMailerConf="bs_mailer.conf"
    rm -rf ${bsMailerConf}
    
    which bs_mailer > /dev/null 2>&1
    if test $? -ne 0; then
        printMessage "bs_mailer application is not found.\n"
        printMessage "Its sources should be distributed with the build system\n"
        printMessage "Please make sure it is installed and 'bs_mailer' executable is in \$PATH.\n"
        exit 1
    fi
    
    if test ! -f "${bsMailerConf}.in"; then
        printMessage "${bsMailerConf}.in is missing\n"
        exit 1
    fi
    
    cat "${bsMailerConf}.in" | sed -e "s#@RESULT_ARCHIVE@#${RESULT_ARCHIVE}#" > "${bsMailerConf}"
    if test $? -ne 0; then
        printMessage "Failed to generate ${bsMailerConf} from ${bsMailerConf}.in\n"
        exit 1
    fi
    
    bs_mailer ${bsMailerConf} >> "${MAIN_LOG}" 2>&1
    if test $? -ne 0; then
        printMessage "Failed to email the results to the subscribers\n"
        exit 1
    fi
}

########################################################################
# main
########################################################################
rm -rf "${RESULT_DIR}"
mkdir -p "${RESULT_DIR}" || exit 1

rm -rf temp*

# Write a header to the summary file.
{
printf "<< Spectre's automatic distributed build and run system >>\n"

HOST_MACHINE=`uname -n`
HOST_ARCH=`uname -m`
printf "Host: ${HOST_MACHINE} (${HOST_ARCH})\n"

CURRENT_DATE=$(date)
printf "Build system started at: ${CURRENT_DATE}\n\n"

} >> "${SUMMARY_FILE}"

# Check if Mercurial is available.
which hg > /dev/null 2>&1
if test $? -ne 0; then
    printMessage "Mercurial version control system is not found.\n"
    printMessage "Please make sure it is installed and 'hg' executable is in \$PATH.\n"
    exit 1
fi

# Check if VirtualBox tools are available if virtual machine management has
# been requested.
if test -n "${MANAGE_VM}"; then
    which VBoxManage > /dev/null 2>&1
    if test $? -ne 0; then
        printMessage "VBoxManage executable is not found.\n"
        printMessage "Please make sure VirtualBox 3.1.8 or newer is installed.\n"
        exit 1
    fi
    
    which VBoxHeadless > /dev/null 2>&1
    if test $? -ne 0; then
        printMessage "VBoxHeadless executable is not found.\n"
        printMessage "Please make sure VirtualBox 3.1.8 or newer is installed.\n"
        exit 1
    fi
fi

# Do the work
loadConfiguration "${CONF_FILE}"
printSeparator

getScriptForTarget
printSeparator

preparePackage
printSeparator

processTargetMachines "${MACHINES_FILE}"
printSeparator

processResults
printSeparator

# Send the results by email to the subscribers if requested.
if test -n "${EMAIL_LOGS}"; then
    emailResults
    printSeparator
fi

# Just in case
rm -rf temp*

printf "Completed.\n"
exit 0
