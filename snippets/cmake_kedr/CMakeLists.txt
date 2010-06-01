cmake_minimum_required(VERSION 2.6)

enable_language(C)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
find_package(Kbuild)
# See conventions
set(KEDR_INSTALL_PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/lib")
set(KEDR_INSTALL_PREFIX_INCLUDE "${CMAKE_INSTALL_PREFIX}/include/kedr")
set(KEDR_INSTALL_PREFIX_KINCLUDE "${CMAKE_INSTALL_PREFIX}/include/kedr")
set(KEDR_INSTALL_PREFIX_KMODULE "${CMAKE_INSTALL_PREFIX}/lib/modules/${KBUILD_VERSION_STRING}/misc")
set(KEDR_INSTALL_PREFIX_KSYMVERS "${CMAKE_INSTALL_PREFIX}/lib/modules/${KBUILD_VERSION_STRING}/symvers")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


add_subdirectory(controller)