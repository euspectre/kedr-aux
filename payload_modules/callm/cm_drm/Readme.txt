cm_drm
========================================================================

This payload module monitors several functions provided by 'drm' kernel 
module. Such functions are typically used by graphics drivers.

[NB] "DRM" stands for "Direct Rendering Management".

To build the payload module, use "custom_payload_callm" example installed 
with KEDR but replace payload.data file with the one given here. You should 
also set module name to "cm_drm" in makefile and KBuild.

To use the payload module, you should start KEDR with it after 'drm' module 
is loaded but before the target module is loaded. This is because the 
functions we are going to intercept are exported by 'drm' module. So we 
cannot load the payload module without 'drm' in place.

Usually, a target module will be loaded by X server. You can employ the 
following trick to load KEDR at appropriate time.

1. Boot to runlevel 3 rather than 5. To do this, add
	init=/sbin/init 3
to the kernel options either at boot time or in /boot/grub/grub.conf or 
/boot/grub/menu.lst or whatever else appropriate (do not forget to change 
it back when you are done!).

You will boot to runlevel 3 where normally no graphical environments have 
started yet. Log in as root.

2. Check if 'drm' module is already loaded (it is likely it is not). If 
not, load it:
	modprobe drm

3. Start KEDR with "cm_drm" as a payload.

4. Mount debugfs if it is not mounted yet:
	mount -t debugfs none /sys/kernel/debug

5. Start trace capturing as usual.

6. Now instruct the system to continue to runlevel 5:
	init 5

X server will be loaded as well as the target module.

[NB] Rather than doing this manually, you can prepare appropriate 
rc.d-scripts to perform all the necessary operations at runlevel 3 during 
system boot. Perhaps, you can also arrange for proper cleanup (capture 
stop, kedr stop, ...) using such scripts - for runlevel 6 or whatever.
