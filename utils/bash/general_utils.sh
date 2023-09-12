#!/bin/bash
#
#   Global variables
CERNBOX_ADDRESS=https://cernbox.cern.ch/remote.php/dav/public-files/svthPKrpowQLtBz/
GLOBAL_FOLDER="bologna"
#
# set env variables in bashrc
set_env_variables() {
    if grep -q "SIPM4EIC_WORKDIR" ~/.bashrc; then
        echo "[INFO] Skip making alias, as one is already in ~/.bashrc"
    else
        if [ -z $1 ]; then
            echo "[INFO] Setting the work and data folder to $(pwd)"
            echo -e "export SIPM4EIC_WORKDIR=\"$(pwd)\"" >>~/.bashrc
            mkdir -p $(pwd)/Data
            echo -e "export SIPM4EIC_DATA=\"$(pwd)/Data\"" >>~/.bashrc
        else
            echo "[INFO] Setting the work and data folder to $1"
            echo -e "export SIPM4EIC_WORKDIR=\"$1\"" >>~/.bashrc
            mkdir -p $1/Data
            echo -e "export SIPM4EIC_DATA=\"$1/Data\"" >>~/.bashrc
        fi
    fi
}
#
fetch_data() {
    if grep -q "SIPM4EIC_DATA" ~/.bashrc; then
    else
        set_env_variables
    fi
    RUN_TAG=$1
    RUN_TYPE=$2
    RUN_MEASURE=$3
    RUN_PLATFORM=$4
    RUN_CONDITIONS=$5
    RUN_TYPE2=$6
    curl $CERNBOX_ADDRESS/$GLOBAL_FOLDER/$RUN_TAG/$RUN_TYPE/$RUN_MEASURE/$RUN_PLATFORM/$RUN_CONDITIONS.$RUN_TYPE2.csv -o $SIPM4EIC_DATA/$GLOBAL_FOLDER/$RUN_TAG/$RUN_TYPE/$RUN_MEASURE/$RUN_PLATFORM/$RUN_CONDITIONS.$RUN_TYPE2.csv
    if [ $? -ne 0 ]; then
        echo "[ERROR] Invalid directory, no data here"
    fi
}
#
