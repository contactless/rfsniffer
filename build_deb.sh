#!/bin/bash

# build deb
SUCCESS_CODE=0
dh_testdir
if [ "$?" -ne "$SUCCESS_CODE" ]; then
	echo "FAIL, bad directory"
 	exit 0
fi
dpkg-buildpackage -rfakeroot -us -uc

