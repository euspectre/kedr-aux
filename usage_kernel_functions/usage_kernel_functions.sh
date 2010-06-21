#! /bin/sh

# Output list of symbols, used by that modules,
# which are currently running on the machine.
# Number corresponding to each function
# represents number of modules which use this function.

perl imported_functions_for_modules.pl `perl modules_list.pl`
