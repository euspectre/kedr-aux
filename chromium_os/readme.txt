build_system_for_chromiumos.patch - patch for KEDR configuration and build system that allows to build KEDR for Chromium OS.

These changes to configuration and build system should be generalized. This may allow configuring and building KEDR on one system for another one (cross-build). The profiles (platform/*.cmake) could then be used to build KEDR for another kernel version on the same system or, hopefully, even for a different architecture.
---------------------------------------------------------------------------

The patch was taken with respect to revision 9ce4fc6c2c3f (branch: default) of kedr-current. Command: 'hg diff -g'

Affected files:
M sources/CMakeLists.txt
M sources/cmake/modules/FindKbuild.cmake
M sources/cmake/modules/kbuild_system.cmake
M sources/cmake/modules/kmodule.cmake
M sources/cmake/modules/kmodule_files/CMakeLists.txt
M sources/cmake/modules/kmodule_files/scripts/lookup_kernel_function.sh
M sources/cmake/modules/kmodule_files/scripts/lookup_kernel_function_hard.sh
M sources/examples/counters/CMakeLists.txt
M sources/examples/custom_indicator_fsim/CMakeLists.txt
M sources/examples/custom_payload_callm/CMakeLists.txt
M sources/examples/custom_payload_fsim/CMakeLists.txt
M sources/examples/sample_fsim_payload/CMakeLists.txt
M sources/examples/sample_indicator/CMakeLists.txt
M sources/examples/sample_target/CMakeLists.txt
M sources/tools/CMakeLists.txt
M sources/tools/kedr_gen/CMakeLists.txt
M sources/tools/kedr_gen/src/CMakeLists.txt
A sources/cmake/platform/chromium-x86-generic.cmake 
---------------------------------------------------------------------------

The patch allows to build KEDR on "build machine" ("host machine") like other software components for Chromium OS. KEDR could then be transferred to the "target machine" where Chromium OS runs and work there as usual.

[WARNING] This patch may break ordinary build process, further review is needed.

For now, many parameters are hard-coded like kernel version in chromium-x86-generic.cmake. In the future we should provide a way to get around this.

It is unclear now how to build 'kedr_gen' at the configuration phase for usage on the build system. CMake seems to use wrong 'ld' (the one for the target system) in this case anyway. The ordinary build of kedr_gen works just fine but it is not necessary for Chromium OS.

For the cross-build to work, it is currently required to build kedr_gen separately in advance (for build machine rather than the target machine) and specify the path to it via KEDR_GEN variable.

Example of CMake command:

cmake \
	-DCMAKE_TOOLCHAIN_FILE=~/work/kedr-chromium/kedr-chromium/sources/cmake/platform/chromium-x86-generic.cmake \
	-DKBUILD_BUILD_DIR=/build/x86-generic/tmp/portage/sys-kernel/chromeos-kernel-9999/work/chromeos-kernel-9999/ \
	-DKEDR_GEN=/usr/local/lib/kedr/kedr_gen \
	../kedr-chromium/sources/
---------------------------------------------------------------------------

KBUILD_BUILD_DIR is where the build files of the target kernel are located. It is more reliable to rebuild kernel for this purpose:
    FEATURES="noclean" emerge-x86-generic -a kernel

If kernel is built manually (make *config, make, ...), the following options could be used:
    ARCH=i386
    CROSS_COMPILE=i686-pc-linux-gnu- 
---------------------------------------------------------------------------

To 'install' KEDR on the build system, use the following:
    sudo make DESTDIR=/build/x86-generic install

The following files should be copied to the target system (from /build/x86-generic on build system to / on the target):
    usr/local/lib/modules/*
    var/opt/kedr/*
    usr/local/bin/kedr*
---------------------------------------------------------------------------
