########################################################################
# Add the tests here and set their properties if necessary
########################################################################

kedr_test_add_script (sample.groupA.1 test_sample.sh "AAA")
kedr_test_add_script (sample.groupA.2 test_sample.sh "BBB")
kedr_test_add_script (sample.groupB.1 test_sample.sh) 
kedr_test_add_script (sample.groupC.1 test_sample.sh)
kedr_test_add_script (sample.groupB.2 test_sample.sh "CCC" "EEE")
kedr_test_add_script (sample.groupD.1 test_hi.sh)
kedr_test_add_script (sample.groupD.2 test_hi.sh "CCC")
kedr_test_add_script (sample.groupD.3 test_hi.sh "CCC" "EEE" "PPP")
kedr_test_add_script (sample.groupD.4 test_hi.sh "CCC" "EEE" "CCC" "EEE")
kedr_test_add_script (sample.groupD.5 test_hi.sh "CCC" "EEE" "CCC" "EEE" "NNN")

# This subdirectory contains the sources of an application needed for some 
# of the tests.
add_subdirectory (testHi)
