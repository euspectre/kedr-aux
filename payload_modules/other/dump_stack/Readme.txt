payload_dump_stack
========================================================================

This payload module uses "point-indicator" infrastructure (which is used, 
for example, for fault simulation) to obtain stack traces for the calls of 
interest. 

To restrict the number of calls for which stack is to be dumped, common 
techniques from fault simulation can be used: restriction by process ID, by 
an expression involving parameters of the target function, etc.

Namely, this module processes the calls to memory allocation and 
deallocation functions and dumps stack trace for __kmalloc calls if the 
indicator triggers. "kmalloc" indicator from KEDR distribution can be used 
with this module to set a scenario (for which calls to dump stack and in 
what conditions).

To build the payload module, use "custom_payload_fsim" example installed 
with KEDR but replace payload.data file with the one given here. You should 
also set module name appropriately in makefile and KBuild.

A typical configuration file that can be used for KEDR to properly use this
module may look like this (the paths may differ on your system):

-------------------------------------
on_load mkdir -p "/tmp/kedr/debugfs"

# Mount debugfs to enable tracepoints later
on_load mount debugfs -t debugfs "/tmp/kedr/debugfs"

# This allows to roll back in case of error while loading a payload
on_unload mount | grep "/tmp/kedr/debugfs"; if test $? -eq 0; then umount "/tmp/kedr/debugfs"; fi

# Fault simulation infrastructure
module /usr/local/lib/modules/2.6.34.7-0.5-default/misc/kedr_fault_simulation.ko

# Payload modules
payload /home/tester/work/kedr/payload_dump_stack/payload_dump_stack.ko 
on_load echo 1 > /tmp/kedr/debugfs/tracing/events/payload_dump_stack/enable

# Indicators 
# We could use kedr_fsim_indicator_common.ko as well because we are 
# not going to set scenarios involving restrictions on the arguments 
# of __kmalloc.
module /usr/local/lib/modules/2.6.34.7-0.5-default/misc/kedr_fsim_indicator_kmalloc.ko

# After tracepoints are enabled for all payloads, unmount debugfs
on_load umount "/tmp/kedr/debugfs"
-------------------------------------
