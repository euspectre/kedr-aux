import os, time, logging

from autotest_lib.client.bin import utils, test

class kedr_sample_target:
    """
    Envelop for kedr_sample_target loading and unloading commands.
    
    It makes exception handling process easier.
    """

    
    def __init__(self, kedr_sample_target_dir):
        """
        Load kedr_sample_target module
        
        kedr_sample_target_dir: directory with built target module
        """
        self._kedr_sample_target_dir = kedr_sample_target_dir
        utils.system('%s/kedr_sample_target load' % self._kedr_sample_target_dir)
        logging.info('Sample target module is loaded.')


    def __del__(self):
        """ Unload kedr_sample_target module """
        utils.system('%s/kedr_sample_target unload' % self._kedr_sample_target_dir)
        logging.info('Sample target module is unloaded')

    
    def __enter__(self):
        # Need for using 'with' with our object
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        # Need for using 'with' with our object
        return


class sample_test(test.test):
    version = 1

    def setup(self, kedr_install_dir='/usr/local'):
        """
        Setup test, building target module
        
        kedr_install_dir: directory where KEDR is installed
        """
        os.system('echo setup:')
        target_src_dir = '' + kedr_install_dir + '/share/kedr/examples/sample_target'
        target_bin_dir = '' + self.bindir + '/target'
        utils.system('cp -rp ' + target_src_dir + ' ' + target_bin_dir)
        os.chdir(target_bin_dir)
        utils.system('make')
        self._target_dir = target_bin_dir

    def run_once(self):
        os.system('echo run_once:')
        with kedr_sample_target(self._target_dir):
            utils.system('echo 123 > /dev/cfake')
            utils.system('echo 321 > /dev/cfake1')

