########################################################################
# Add the tests here and set their properties if necessary
########################################################################
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/test_something1.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/test_something1.sh"
)

kedr_test_add_script (hello.somethingA.1 test_something.sh)
kedr_test_add_script (hello.somethingB.1 test_something.sh 1 2 "RTFM, Hacker")
kedr_test_add_script (hello.somethingC.1 test_something1.sh)
kedr_test_add_script (hello.somethingD.1 test_something2.sh)
kedr_test_add_script (hello.somethingD.2 test_something.sh 112)
