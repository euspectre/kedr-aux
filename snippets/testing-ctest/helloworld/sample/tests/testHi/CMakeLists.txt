set (testHi_SOURCES test.cpp)

# This application is only needed for testing.
# We create the executable as usual, kedr_test_add_target () will take care
# of the rest.
add_executable (testHi ${testHi_SOURCES})

# Make sure the application is built before the tests are run. 
# The application will not be built when make (make all) is called.
kedr_test_add_target(testHi)
