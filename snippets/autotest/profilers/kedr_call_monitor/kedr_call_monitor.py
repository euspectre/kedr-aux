"""
KEDR call monitor profiler
"""
import os
import signal
import subprocess
from autotest_lib.client.bin import utils, profiler
from autotest_lib.client.common_lib import error
import logging


class kedr_call_monitor_class:
    """ Envelop for KEDR control script, which makes exception handling process easier """


    def __init__(self, kedr_dir, target_name):
        """
        Analogue of 'kedr start' service command.
        
        kedr_dir: directory where KEDR is installed
        target_name: name of the target module for watched for
        
        Exactly, execute "kedr start target_name=`target_name` -f callm.conf".
        """
        self._kedr_dir = kedr_dir
        utils.system('%s/bin/kedr start %s -f callm.conf' % (self._kedr_dir, target_name))
        logging.info('KEDR core module is loaded.')
    

    def __del__(self):
        """ Analogue of 'kedr stop' service command. """
        utils.system('%s/bin/kedr stop' % self._kedr_dir)
        logging.info('KERD core module is unloaded.')


class capture_trace_class:
    """
    Envelop for capture trace tool, which makes exception handling process easier
    
    This is lazy variant for capturing - all trace is capturing at object deleting.
    """
    _kedr_core = None
    _kedr_dir = None
    _trace_file = None
    
    def __init__(self, kedr_call_monitor_instance, kedr_dir, trace_file):
        """
        Prepare for capture trace.
        
        kedr_call_monitor_instance: object of type kedr_call_monitor_class
            This reference is used for prevent KEDR core unloading while trace is being collected.
        kedr_dir: directory where KEDR is installed
        trace_file: file for store trace
        """
        self._kedr_call_monitor_instance = kedr_call_monitor_instance
        self._kedr_dir = kedr_dir
        self._trace_file = trace_file
        utils.system('mkdir -p ' + os.path.dirname(trace_file))
        utils.system('rm -f ' + trace_file)
        logging.info('Capturing trace is started, trace file is \'%s\'' % self._trace_file)

    def __del__(self):
        """ Preform capturing trace """
        try:
            utils.system('%s/bin/kedr_capture_trace -s -f %s' % (self._kedr_dir, self._trace_file),
                timeout = 10)
        except error.CmdError:
            logging.error('Failed to capture trace or test case didn\'t load target module.')
            raise
        else:
            logging.info('Capturing trace is stopped.')
        finally:
            del self._kedr_call_monitor_instance
    

class capture_trace_runtime_class:
    """
    Envelop for capture trace tool, which makes exception handling process easier
    
    This is dynamic variant for capturing - capturing begin in constructor and stop in destructor.
    """

    _kedr_dir = None
    _trace_file = None
    
    _trace_process = None
    
    def __init__(self, kedr_call_monitor_instance, kedr_dir, trace_file):
        """
        Start capturing trace.
        
        kedr_call_monitor_instance: object of type kedr_call_monitor_class
            This reference is used for prevent KEDR core unloading while trace is being collected.
        kedr_dir: directory where KEDR is installed
        trace_file: file for store trace
        """
        self._kedr_call_monitor_instance = kedr_call_monitor_instance
        self._kedr_dir = kedr_dir
        self._trace_file = trace_file
        utils.system('mkdir -p ' + os.path.dirname(trace_file))
        utils.system('rm -f ' + trace_file)
        cmd = '%s/bin/kedr_capture_trace' % self._kedr_dir
        self._trace_process = subprocess.Popen([cmd, '-f', self._trace_file])
        logging.info('Capturing trace is started, trace file is \'%s\'' % self._trace_file)


    def __del__(self):
        """ Stop capturing trace """
        # Need to wait until last trace lines are collected
        os.system('sleep 1')
        self._trace_process.send_signal(signal.SIGINT)
        self._trace_process.wait()
        logging.info('Capturing trace is stopped.')
        del self._kedr_call_monitor_instance


class kedr_call_monitor(profiler.profiler):
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
        self._kedr_call_monitor_instance = kedr_call_monitor_class(
            self._kedr_install_dir, self._target_name)
        self._capture_trace_instance = capture_trace_runtime_class(
            self._kedr_call_monitor_instance, self._kedr_install_dir,
            test.profdir + '/kedr_call_monitor/trace')


    def stop(self, test):
        os.system('echo Profiler stop:')
        del self._capture_trace_instance
        del self._kedr_call_monitor_instance


    def report(self, test):
        # All work is done in the `stop` method
        pass
