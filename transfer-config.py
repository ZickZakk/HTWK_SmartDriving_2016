import argparse
import os
import re
import subprocess
import tarfile
from os import walk

import time

import sys

__author__ = 'pbachmann'

WHITELIST = ['configuration_files/']

ARCHIVE_FILENAME = 'deploy.tar.gz'
SCRIPT_FILENAME = 'deploy-config.py'

SCP_HOST = '{0}@{1}:{2}'
SSH_HOST = '{0}@{1}'


class Settings(object):
    def __init__(self, args):
        self.pw = args.pw
        self.user = args.user
        self.host = args.host

        self.path = str(args.path)
        if not self.path.endswith('/'):
            self.path += '/'


def main():
    print 'transfer-config.py'
    print 'Working dir is ' + os.getcwd()
    print
    sys.stdout.flush()

    parser = setup_argparser()
    args = parser.parse_args()

    settings = Settings(args)

    print 'Checking host ' + settings.host
    return_code = subprocess.call('/bin/ping -c 1 -W 1 ' + settings.host, shell=True, stdout=subprocess.PIPE)

    if return_code > 0:
        print settings.host + ' is down!'
        exit(return_code)
    else:
        print 'Ok'
        print

    print 'Collecting files'
    abs_filepaths = collect_files()

    print 'Compressing files'
    sys.stdout.flush()

    with tarfile.open('.' + os.sep + ARCHIVE_FILENAME, mode='w') as archive:
        for abs_filepath in abs_filepaths:
            rel_filepath = os.path.relpath(abs_filepath, '.')
            archive.add(abs_filepath, rel_filepath)

    print 'Created .' + os.sep + ARCHIVE_FILENAME
    print
    sys.stdout.flush()

    print 'Copying archive'
    sys.stdout.flush()

    host = SCP_HOST.format(settings.user, settings.host, settings.path) + ARCHIVE_FILENAME
    copy_process = subprocess.Popen(['/usr/bin/scp', '-o', 'StrictHostKeyChecking=no',
                                     '.' + os.sep + ARCHIVE_FILENAME, host],
                                    cwd='./',
                                    env={'PATH': os.environ['PATH']})
    return_code = copy_process.wait()

    if return_code > 0:
        cleanup()
        exit(return_code)

    print
    print 'Copying deploy script'
    sys.stdout.flush()

    host = SCP_HOST.format(settings.user, settings.host, settings.path) + SCRIPT_FILENAME
    copy_process = subprocess.Popen(['/usr/bin/scp', '-o', 'StrictHostKeyChecking=no',
                                     '.' + os.sep + SCRIPT_FILENAME, host],
                                    cwd='./',
                                    env={'PATH': os.environ['PATH']})
    return_code = copy_process.wait()

    if return_code > 0:
        cleanup()
        exit(return_code)

    print
    print 'Executing deploy script'
    sys.stdout.flush()

    host = SSH_HOST.format(settings.user, settings.host)
    command = 'cd {0} && /usr/bin/python ./{1}'.format(settings.path, SCRIPT_FILENAME)
    deploy_process = subprocess.Popen(['/usr/bin/ssh', '-o', 'StrictHostKeyChecking=no',
                                       host, command],
                                      cwd='./',
                                      env={'PATH': os.environ['PATH']})
    return_code = deploy_process.wait()

    if return_code > 0:
        cleanup()
        exit(return_code)

    cleanup()


def cleanup():
    print('')
    print('Cleanup')
    print('Removing ' + ARCHIVE_FILENAME)
    print('')
    os.remove('.' + os.sep + ARCHIVE_FILENAME)


def collect_files():
    files = []
    for (dirpath, dirnames, filenames) in walk('.'):
        for full_filename in filenames:
            abs_filepath = dirpath + os.sep + full_filename
            rel_filepath = os.path.relpath(abs_filepath, '.')

            for f in WHITELIST:
                if re.match(f, rel_filepath):
                    files.append(abs_filepath)
    return files


def setup_argparser():
    parser = argparse.ArgumentParser(description="My argument pareser")
    parser.add_argument('--host',
                        required=True,
                        help='Specifies address of the remote host.')
    parser.add_argument('--user',
                        required=True,
                        help='Specifies user for the login on the remote host.')
    parser.add_argument('--pw',
                        required=True,
                        help='Specifies password for the login on the remote host.')
    parser.add_argument('--path',
                        required=True,
                        help='Specifies path where the files should be deployed on the remote host.')
    return parser


if __name__ == '__main__':
    start = time.time()
    main()
    elapsedTime = time.time() - start

    print 'Deployment finished in ' + str(elapsedTime)
    sys.stdout.flush()
