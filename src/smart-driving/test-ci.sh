#!/bin/bash

########################################################################
# Dieses Script führt die HTWK Smart-Driving Tests im TeamCity aus.    #
########################################################################

INSTALL_PATH=./../../bin/

echo "Starting tests"
${INSTALL_PATH}/test/run_tests
