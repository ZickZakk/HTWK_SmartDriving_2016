#!/bin/bash

USER=aadc
PW=aadc2016
PATH=/home/aadc/smart-driving

HOST=$1
COMPRESS=$2

if [ -z ${HOST} ]; then
    echo "Error: no IP address specified"
    echo "    transfer-to-car.sh [ip]"
    exit 1
fi

if [ ! -z ${COMPRESS} ]; then
    COMPRESS="--compress"
else
    COMPRESS=""
fi

/usr/bin/python ./transfer-to-car.py \
    --host ${HOST} \
    --user ${USER} \
    --pw ${PW} \
    --path ${PATH} \
    ${COMPRESS}
