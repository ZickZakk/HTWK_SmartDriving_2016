##########################################################################
#    Dieses Script installiert das Smart-Driving Projekt auf dem Auto    #
##########################################################################

import os
import subprocess

import shutil

CONFIG = 'DEBUG'
ADTF_DIR = '/opt/adtf/2.13.1/'
ARCHIVE_NAME = './deploy.tar.gz'
CLEANUP_FOLDER = ['./bin/', './config/', './configuration_files/', './description/', './src/']

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

print '> copying services'

if CONFIG == 'DEBUG':
    ADTF_BIN_DIR = ADTF_DIR + 'bin/debug/'
    FILTER_DIR = './bin/filter/debug/'
    SERVICE_FILES = ['htwk_world_service.srv',
                     'htwk_world_service.manifest']

    for f in SERVICE_FILES:
        shutil.copyfile(FILTER_DIR + f, ADTF_BIN_DIR + f)

    if os.path.exists(ADTF_BIN_DIR + "resources/"):
        shutil.rmtree(ADTF_BIN_DIR + "resources/")

    shutil.copytree(FILTER_DIR + "resources/", ADTF_BIN_DIR + "resources/")


if CONFIG == 'RELEASE':
    ADTF_BIN_DIR = ADTF_DIR + 'bin/'
    FILTER_DIR = './bin/filter/'
    SERVICE_FILES = ['htwk_world_service.srv',
                     'htwk_world_service.manifest']

    for f in SERVICE_FILES:
        shutil.copyfile(FILTER_DIR + f, ADTF_BIN_DIR + f)

    if os.path.exists(ADTF_BIN_DIR + "resources/"):
        shutil.rmtree(ADTF_BIN_DIR + "resources/")

    shutil.copytree(FILTER_DIR + "resources/", ADTF_BIN_DIR + "resources/")

        
print '> after build cleanup'

os.remove(ARCHIVE_NAME)

print '> done'
