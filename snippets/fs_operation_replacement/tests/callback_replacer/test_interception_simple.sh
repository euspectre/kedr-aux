#!/bin/sh
replacer_module="./usp_replacer/usp_replacer.ko"
process_user_string_module="./process_user_string/process_user_string.ko"
payload_module="./payload/payload.ko"
user_module="./user/char_counter.ko"

was_called_file="/sys/module/char_counter/parameters/was_called"
was_intercepted_file="/sys/module/payload/parameters/was_intercepted"

control_file="/proc/write_anything_here"

unload_all()
{
    rmmod "$user_module"
    rmmod "$payload_module"
    rmmod "$process_user_string_module"
    rmmod "$replacer_module"
}

if ! insmod "$replacer_module"; then
    printf "Cannot load replacer module for user string processor's callback.\n"
    exit 1
fi

if ! insmod "$process_user_string_module"; then
    printf "Cannot module implemented function with callback.\n"
    rmmod "$replacer_module"
    exit 1
fi

if ! insmod "$payload_module"; then
    printf "Cannot load payload module.\n"
    rmmod "$process_user_string_module"
    rmmod "$replacer_module"
    exit 1
fi

if ! insmod "$user_module"; then
    printf "Cannot module which uses function with callback.\n"
    rmmod "$payload_module"
    rmmod "$process_user_string_module"
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

if ! rmmod "$user_module"; then
    printf "Fail to unload module which uses function with callback.\n"
    exit 1
fi

if ! rmmod "$payload_module"; then
    printf "Fail to unload payload module.\n"
    exit 1
fi

if ! rmmod "$process_user_string_module"; then
    printf "Fail to unload module which implement function with callback.\n"
    exit 1
fi

if ! rmmod "$replacer_module"; then
    printf "Fail to unload replacer module for user string processor's callback.\n"
    exit 1
fi