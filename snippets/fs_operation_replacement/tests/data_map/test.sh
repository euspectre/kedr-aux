test_module="module/test_module.ko"

insmod "${test_module}"
if test $? -ne 0; then
    printf "Test failed.\n"
    exit 1
fi

rmmod "${test_module}"

printf "Test passed.\n"
