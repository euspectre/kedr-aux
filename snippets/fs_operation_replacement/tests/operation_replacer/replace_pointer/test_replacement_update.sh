#!/bin/sh
replacer_module="./delegate_operation_replacer/delegate_operation_replacer.ko"
base_update_replacement_module="./base_update_replacement/delegate_base_update_replacement.ko"
payload_module="./payload/payload.ko"
delegate_impl_update_operations="./user_update_operations/delegate_impl_update_operations.ko"

was_called_file="/sys/module/delegate_impl_update_operations/parameters/was_called"
was_intercepted_file="/sys/module/payload/parameters/was_intercepted"

control_file="/proc/delegate"
status_file="/sys/module/delegate_base_update_replacement/parameters/status"

unload_all()
{
    rmmod "$delegate_impl_update_operations"
    rmmod "$payload_module"
    rmmod "$base_update_replacement_module"
    rmmod "$replacer_module"
}

if ! insmod "$replacer_module"; then
    printf "Cannot load replacer module for delegate operations.\n"
    exit 1
fi

if ! insmod "$base_update_replacement_module"; then
    printf "Cannot load delegate base module.\n"
    rmmod "$replacer_module"
    exit 1
fi

if ! insmod "$payload_module"; then
    printf "Cannot load payload module.\n"
    rmmod "$base_update_replacement_module"
    rmmod "$replacer_module"
    exit 1
fi

if ! insmod "$delegate_impl_update_operations"; then
    printf "Cannot load delegate implementation.\n"
    rmmod "$payload_module"
    rmmod "$base_update_replacement_module"
    rmmod "$replacer_module"
    exit 1
fi

if ! echo 1 > "$control_file"; then
    printf "Cannot write to control file.\n"
    unload_all
    exit 1
fi

was_called=`cat $was_called_file`
if test "$was_called" = "0"; then
    printf "Original operation wasn't called.\n"
    unload_all
    exit 1
fi

was_intercepted=`cat $was_intercepted_file`
if test "$was_intercepted" = "0"; then
    printf "Operation wasn't intercepted.\n"
    unload_all
    exit 1
fi

# Call delegate again
echo 0 > "$was_called_file"
echo 0 > "$was_intercepted_file"

if ! echo 1 > "$control_file"; then
    printf "Cannot write to control file.\n"
    unload_all
    exit 1
fi

was_called=`cat $was_called_file`
if test "$was_called" = "0"; then
    printf "Original operation wasn't called.\n"
    unload_all
    exit 1
fi

was_intercepted=`cat $was_intercepted_file`
if test "$was_intercepted" = "0"; then
    printf "Operation wasn't intercepted.\n"
    unload_all
    exit 1
fi



if ! rmmod "$delegate_impl_update_operations"; then
    printf "Fail to unload delegate implementation module.\n"
    exit 1
fi

status=`cat $status_file`
if test "$status" != "0"; then
    printf "Delegate implementation module was unloaded with errors.\n"
    rmmod "$payload_module"
    rmmod "$base_update_replacement_module"
    rmmod "$delegate_operation_replacer"
    exit 1
fi

if ! rmmod "$payload_module"; then
    printf "Fail to unload payload module.\n"
    exit 1
fi

if ! rmmod "$base_update_replacement_module"; then
    printf "Fail to unload delegate base module.\n"
    exit 1
fi

if ! rmmod "$replacer_module"; then
    printf "Fail to unload replacer for delegate operations.\n"
    exit 1
fi