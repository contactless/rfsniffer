#!/bin/bash

if [ $# == "1" ] 
then
    echo "Building script is run with option '$1'"

    if [ $1 == "native" ]
    then
        # build deb
        SUCCESS_CODE=0
        dh_testdir
        if [ "$?" -ne "$SUCCESS_CODE" ]; then
            echo "FAIL, bad directory"
            exit 0
        fi
        # -j8 option is suitable for build-server
        # if you have less cores you should omit it
        dpkg-buildpackage -rfakeroot -us -uc -j8
        exit 0
    fi
    
    if [ $1 == "amd64" ]
    then
        WBDEV_INSTALL_DEPS="yes" wbdev ndeb -j8
        exit 0
    fi
    
    if [ $1 == "armel" ]
    then
        WBDEV_INSTALL_DEPS="yes" WBDEV_TARGET=wheezy-armel wbdev cdeb -j8
        exit 0
    fi
    
    if [ $1 == "armhf" ]
    then
        WBDEV_INSTALL_DEPS="yes" WBDEV_TARGET=wheezy-armhf wbdev cdeb -j8
        exit 0
    fi
fi

echo "Use 'native' for build under current system, \
use 'armel', 'armhf', 'amd64' for build via wbdev under host system"


