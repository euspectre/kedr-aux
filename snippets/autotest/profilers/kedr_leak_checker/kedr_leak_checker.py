"""
KEDR leak check payload as profiler
"""
import os
from autotest_lib.client.bin import utils, profiler
from autotest_lib.client.common_lib import error
import logging


class kedr_leak_check_class:
    """ Envelop for KEDR control script, which makes exception handling process easier """

    _debugfs_dir = '/sys/kernel/debug'

    def __init__(self, kedr_dir, target_name):
        """
        Analogue of 'kedr start' service command.
        
        kedr_dir: directory where KEDR is installed
        target_name: name of the target module for watched for
        
        Exactly, execute "kedr start target_name=`target_name` -f leak_check.conf".
        """
        self._kedr_dir = kedr_dir
        utils.system('%s/bin/kedr start %s -f leak_check.conf' % (self._kedr_dir, target_name))
        logging.info('KEDR leak checker is loaded.')

    
    def __del__(self):
        """ Analogue of 'kedr stop' service command. """
        utils.system('%s/bin/kedr stop' % self._kedr_dir)
        logging.info('KERD leak checker is unloaded.')

    def store_results(self, result_dir):
        """ Store results of KEDR leak check payload into `result_dir`"""
        utils.system('mkdir -p %s' % result_dir)
        utils.system('rm -rf %s/*' % result_dir)
        utils.system('cp -r %s/kedr_leak_check/* %s' % (self._debugfs_dir, result_dir))
        logging.info('KEDR leak checker report was stored into \'%s\'' % result_dir)

class kedr_leak_checker(profiler.profiler):
    """ KEDR leak check profiler """
    version = 1

    def initialize(self, target_name, kedr_install_dir='/usr/local'):
        """
        Initialize profiler
        
        target_name: name of the kernel module for profiling
        kedr_install_dir: directory where KEDR is installed
        """
        os.system('echo Profiler initialize:')
        self._target_name = target_name
        self._kedr_install_dir = kedr_install_dir

    def start(self, test):
        os.system('echo Profiler start:')
        self._kedr_leak_check_instance = kedr_leak_check_class(
            self._kedr_install_dir, self._target_name)


    def stop(self, test):
        os.system('echo Profiler stop:')
        self._kedr_leak_check_instance.store_results(test.profdir + '/kedr_leak_checker')
        del self._kedr_leak_check_instance

    def report(self, test):
        # All work is done in the `stop` method
        pass
