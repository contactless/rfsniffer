#!/bin/bash

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

