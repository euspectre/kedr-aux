#!/bin/sh
# Usage: test.sh [indent]

current_test_number=0
tests_failed=0

if test $# -eq 0; then
	indent=""
else
	indent="$1"
fi

printf "${indent}Test operation replacer of type 'replace_pointer'\n"

# execute_test test_name test_script
execute_test()
{
	test_name="$1"
	test_script="$2"
	current_test_number=$(($current_test_number + 1))
	printf "${indent}$current_test_number. $test_name ... "
	if ! sh $test_script; then
		tests_failed=$(($tests_failed + 1))
		printf "Failed\n"
	else
		printf "Passed\n"
	fi
}

## Tests
execute_test intercepction_simple ./test_interception_simple.sh
execute_test intercepction_special ./test_interception_special.sh
execute_test intercepction_mixed ./test_interception_mixed.sh
execute_test replacement_update ./test_replacement_update.sh

if test "$tests_failed" = "0"; then
	printf "${indent}All $current_test_number tests passed.\n"
	exit 0
else
	printf "${indent}$tests_failed tests failed from $current_test_number.\n"
	exit 1
fi
