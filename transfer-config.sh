#!/bin/bash

USER=aadc
PW=aadc2016
PATH=/home/aadc/smart-driving

HOST=$1

if [ -z ${HOST} ]; then
    echo "Error: no IP address specified"
    echo "    transfer-to-car.sh [ip]"
    exit 1
fi

/usr/bin/python ./transfer-config.py \
    --host ${HOST} \
    --user ${USER} \
    --pw ${PW} \
    --path ${PATH}
