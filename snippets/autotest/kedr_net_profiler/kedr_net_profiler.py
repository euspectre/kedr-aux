"""
Use KEDR leak checker as profiler for test which involve given kernel module as support for net interface
"""

import time, os, subprocess, logging, re
import contextlib
import collections
from autotest_lib.client.bin import profiler
from autotest_lib.client.common_lib import utils, error


class Unloadable(collections.Hashable):
    """
    Represent object which can be unload and load again multiple times.
    """
    def is_loaded(self):
        # Verify whether object is loaded now
        pass
    
    def unload(self):
        # Unload object
        pass
    
    def load(self):
        # Load object (again)
        pass
    
    def get_depended_objects(self):
        # Return list of 'Unloadable' objects, which depend from this
        return []

class kernel_module(Unloadable):
    """
    Describe kernel module with given name, which can be unload
    
    Module name should be known to modprobe.
    """
    _modprobe_command = '/sbin/modprobe'
    _lsmod_command = '/sbin/lsmod'

    _module_name = None
    
    def __init__(self, module_name):
        # Verify that module is known to modprobe
        utils.system('%s --show-depends %s' % (self._modprobe_command, module_name))
        self._module_name = module_name
    
    def is_loaded(self):
        modules = subprocess.Popen(self._lsmod_command, stdout=subprocess.PIPE).communicate()[0].splitlines()
        # <module_name> <usage> ...
        module_re = re.compile('^(\w+)')
        for module in modules:
            module_match = module_re.match(module)
            if not module_match:
                # Header
                continue
            if module_match.group(1) != self._module_name:
                # Not our module
                continue
            return True
        return False
    
# Modules with equal names should be interpreted as one object
    def __hash__(self):
        return self._module_name.__hash__()
    
    def __eq__(self, other):
        if isinstance(other, kernel_module):
            return self._module_name == other._module_name
        else:
            return False

    def unload(self):
        utils.system('%s -r %s' % (self._modprobe_command, self._module_name))
        logging.info('Module %s was unloaded' % self._module_name)
    
    def load(self):
        utils.system('%s %s' % (self._modprobe_command, self._module_name))
        logging.info('Module %s was loaded' % self._module_name)
    
    def get_depended_objects(self):
        modules = subprocess.Popen(self._lsmod_command, stdout=subprocess.PIPE).communicate()[0].splitlines()
        # <module_name> <usage> <refcount> <depend-modules-names>
        module_re = re.compile('^(\w+)\s+\d+\s+\d+(\s+(\S+))')
        for module in modules:
            module_match = module_re.match(module)
            if not module_match:
                # Header
                continue
            if module_match.group(1) != self._module_name:
                # Not our module
                continue
            
            if module_match.group(2) != None:
                depend_modules_names = module_match.group(3).split(',')
                return [kernel_module(name) for name in depend_modules_names]
            else:
                # There is no modules which depends from this
                return []
        # Module is not loaded
        return []

@contextlib.contextmanager
def object_unloaded(object_for_unload, static_dependencies = None):
    """
    Provide possibility for doing something while given object is 'unloaded' using 'with' statement
    """
    if static_dependencies == None:
        static_dependencies = {}

    # Array of unloaded object (them should be loaded again)
    unloaded_objects = []

    def unload_object(obj):
        for depended_object in obj.get_depended_objects():
            if depended_object.is_loaded():
                unload_object(depended_object)
        
        for depended_object in static_dependencies.get(obj, []):
            if depended_object.is_loaded():
                unload_object(depended_object)
        
        obj.unload()
        unloaded_objects.append(obj)
    
    try:
        if object_for_unload.is_loaded():
            unload_object(object_for_unload)
        yield
    finally:
        while unloaded_objects:
            unloaded_objects.pop().load()


class net_interface(Unloadable):
    """
    Describe network interface with given name which can be unloaded ('DOWN')
    """
    _ifconfig_command = '/sbin/ifconfig'

    _seconds_for_restore_connection = 120
    _interface_name = None
    
    def __init__(self, interface_name, seconds_for_restore_connection = None):
        # Verify whether interface is known to the system
        utils.system('%s %s' % (self._ifconfig_command, interface_name))
        
        self._interface_name = interface_name
        if seconds_for_restore_connection:
            self._seconds_for_restore_connection = seconds_for_restore_connection

# Interfaces with equal names should be interpreted as one object
    def __hash__(self):
        return self._interface_name.__hash__()
    
    def __eq__(self, other):
        if isinstance(other, net_interface) and self._interface_name == other._interface_name:
            return True
        else:
            return False
    
    def is_loaded(self):
        interface_info = subprocess.Popen(['/sbin/ifconfig', self._interface_name], stdout=subprocess.PIPE).communicate()[0]
    
        if 'UP' in interface_info.split():
            logging.info('Interface %s is up' % self._interface_name)
            return True
        else:
            logging.info('Interface %s is down' % self._interface_name)
            return False
    
    def unload(self):
        utils.system('%s %s down' % (self._ifconfig_command, self._interface_name))
        logging.info('Interface %s was down' % self._interface_name)

    def load(self):
        utils.system('%s %s up' % (self._ifconfig_command, self._interface_name))
        logging.info('Interface %s was up' % self._interface_name)
        # Wait for recover connection
        time.sleep(self._seconds_for_restore_connection)


class kedr:
    """
    Represent KEDR as a mechanism for watching for a kernel module
    """
    _kedr_install_dir = None
    
    def __init__(self, kedr_install_dir):
        self._kedr_install_dir = kedr_install_dir
    
    def start(self, target_name, config_file):
        utils.system('%s/bin/kedr start %s -f %s' % (self._kedr_install_dir, target_name, config_file))
    
    def stop(self):
        utils.system('%s/bin/kedr stop' % self._kedr_install_dir)

class kedr_leak_check:
    """
    Represent KEDR leak checker as a mechanism for verification of the kernel module
    """
    _kedr_instance = None
    _debugfs_mount_point = '/sys/kernel/debug'
    
    def __init__(self, kedr_install_dir):
        self._kedr_instance = kedr(kedr_install_dir)
    
    def load(self, target_name):
        self._kedr_instance.start(target_name, 'leak_check.conf')
        self._target_name = target_name
    
    def parse_info(self):
        with open('%s/kedr_leak_check/info' % self._debugfs_mount_point) as f:
            leak_check_info = f.readlines()
        if len(leak_check_info) < 4:
            logging.warn('Module \'%s\' watched by KEDR wasn\'t loaded during the test' % self._target_name)
            return None
        try:
            target_module = re.match('Target module:\s*"(\w+)"', leak_check_info[0]).group(1)
            memory_allocations = re.match('Memory allocations:\s*(\w+)',leak_check_info[1]).group(1)
            possible_leaks = re.match('Possible leaks:\s*(\w+)', leak_check_info[2]).group(1)
            unallocated_frees = re.match('Unallocated frees:\s*(\w+)', leak_check_info[3]).group(1)
        except:
            logging.warn('Failed to parse information about leaks')
            return None
        else:
            return {'target_module' : target_module,
                    'memory_allocations' : memory_allocations,
                    'possible_leaks' : possible_leaks,
                    'unallocated_frees' : unallocated_frees}

    def report(self, dest_dir = None, test = None):
        if test:
            leak_check_info = self.parse_info()
            if leak_check_info:
                test.write_perf_keyval(leak_check_info)

        if dest_dir:
            utils.system('mkdir -p %s' % dest_dir)
            utils.system('rm -rf %s/*' % dest_dir)
            utils.system('cp -rp %s/kedr_leak_check/* %s' % (self._debugfs_mount_point, dest_dir))
            logging.info('KEDR leak checker result was stored into %s' % dest_dir)


    def unload(self):
        self._target_name = None
        self._kedr_instance.stop()
        

class kedr_net_profiler(profiler.profiler):
    version = 1
    _kedr_leak_check_instance = None
    _module_name = None
    _unload_dependencies = None
    
    def initialize(self, module_name, module_interface_dependencies = None, kedr_install_dir = '/usr/local'):
        self._module_name = module_name

        self._unload_dependencies = {}
        if module_interface_dependencies:
            for module_interface_dependency in module_interface_dependencies:
                module_dep = kernel_module(module_interface_dependency[0])
                interface_dep = net_interface(module_interface_dependency[1])
                
                if module_dep in self._unload_dependencies:
                    self._unload_dependencies[module_dep].append(interface_dep)
                else:
                    self._unload_dependencies[module_dep] = [interface_dep]

        self._kedr_leak_check_instance = kedr_leak_check(kedr_install_dir)

    def start(self, test):
        with object_unloaded(kernel_module(self._module_name), self._unload_dependencies):
            self._kedr_leak_check_instance.load(self._module_name)

    def stop(self, test):
        with object_unloaded(kernel_module(self._module_name), self._unload_dependencies):
            self._kedr_leak_check_instance.report(test.profdir + '/kedr_leak_checker', test)
            self._kedr_leak_check_instance.unload()
        
