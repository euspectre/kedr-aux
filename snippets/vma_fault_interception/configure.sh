#! /bin/sh

example_dir=$(dirname $(readlink -f $0))

if test -z "$KEDR_INSTALL_PREFIX" -o -z "$KEDR_COI_INSTALL_PREFIX"; then
    printf "'KEDR_INSTALL_PREFIX' and 'KEDR_COI_INSTALL_PREFIX' should be set.\n"
    exit 1
fi

cat > "${example_dir}/config.mk" << eof
KEDR_INSTALL_PREFIX = ${KEDR_INSTALL_PREFIX}
KEDR_COI_INSTALL_PREFIX = ${KEDR_COI_INSTALL_PREFIX}
eof

cat > "${example_dir}/vma_fault_interception.conf" << eof
module /home/andrew/kedr-coi-builds/work/install/lib/modules/`uname -r`/misc/kedr_coi.ko
payload ${example_dir}/vma_fault_interception.ko
eof