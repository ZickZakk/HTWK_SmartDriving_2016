########################################################################
# Dieses Script baut den HTWK Smart-Driving Code im TeamCity.          #
########################################################################

import os
import subprocess
import shutil

# no release logic added
import sys

CONFIG = 'DEBUG'

ADTF_DIR = '/opt/adtf/'
ADTF_DISPLAY_DIR = '/opt/adtf/addons/adtf-display-toolbox/'
OSG_DIR = '/opt/osg/3.2.0/'
ARUCO_DIR = '/opt/aruco/1.3.0/build/'
OPENCV_DIR = '/opt/opencv/3.0.0/build/'
BUILD_PATH = './build/'

# relativ zu BUILD_PATH.
# Cmake die erzeugt build files immer im aktuellen Ordner.
# Spaeter im script wird in den BUILD_PATH gewechselt.
INSTALL_PATH = './../../../bin/'

if ADTF_DIR is '':
    print 'ADTF_DIR must not be empty'
    exit(1)

if OPENCV_DIR is '':
    print 'OPENCV_DIR must not be empty'
    exit(1)

if BUILD_PATH is '':
    print 'BUILD_PATH must not be empty'
    exit(1)

if INSTALL_PATH is '':
    print 'INSTALL_PATH must not be empty'
    exit(1)

if not os.path.isdir(BUILD_PATH):
    os.mkdir(BUILD_PATH, 0777)

print('build.py')
print('Working dir is ' + os.getcwd())
print
sys.stdout.flush()

generate_process = subprocess.Popen(['/usr/bin/cmake',
                                     '-G', 'Unix Makefiles',
                                     '-D', 'ADTF_DIR=' + ADTF_DIR,
                                     '-D', 'ADTF_DISPLAY_TOOLBOX_DIR=' + ADTF_DISPLAY_DIR,
                                     '-D', 'OSG_DIR=' + OSG_DIR,
                                     '-D', 'OpenCV_DIR=' + OPENCV_DIR,
                                     '-D', 'aruco_DIR=' + ARUCO_DIR,
                                     '-D', 'CMAKE_INSTALL_PREFIX=' + INSTALL_PATH,
                                     '-D', 'CMAKE_BUILD_TYPE=' + CONFIG,
                                     '-D', 'CMAKE_C_COMPILER=gcc-4.9',
                                     '-D', 'CMAKE_CXX_COMPILER=g++-4.9',
                                     '..'],
                                    cwd='./build/',
                                    env={'PATH': os.environ['PATH']})
exit_code = generate_process.wait()

print
print('Generating build files done.')
print('Exit code: ' + str(exit_code))
print
sys.stdout.flush()

if exit_code > 0:
    exit(exit_code)

build_process = subprocess.Popen(['/usr/bin/cmake',
                                  '--build', '.',
                                  '--target', 'install',
                                  '--config', CONFIG,
                                  '--', '-j4'],
                                 cwd='./build/',
                                 env={'PATH': os.environ['PATH']})
exit_code = build_process.wait()

print
print('Building project done.')
print('Exit code: ' + str(exit_code))
print

if exit_code > 0:
    exit(exit_code)
