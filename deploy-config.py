################################################################################
#    Dieses Script installiert die Smart-Driving Konfiguration auf dem Auto    #
################################################################################

import os
import subprocess

import shutil

CONFIG = 'DEBUG'
ADTF_DIR = '/opt/adtf/2.13.1/'
ARCHIVE_NAME = './deploy.tar.gz'
CLEANUP_FOLDER = ['./configuration_files/']

print ''
print '> deploying'

print '> pre build cleanup'

for f in CLEANUP_FOLDER:
    if os.path.isdir(f):
        shutil.rmtree(f)

print '> executing unpack'

extract_process = subprocess.Popen(['tar', '-xf', ARCHIVE_NAME],
                                   cwd='./',
                                   env={'PATH': os.environ['PATH']})
return_code = extract_process.wait()

if return_code > 0:
    exit(return_code)

print '> after build cleanup'

os.remove(ARCHIVE_NAME)

print '> done'
