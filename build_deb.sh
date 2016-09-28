#!/bin/sh

echo "DH_FIXPERMS"
dh_fixperms

echo "DH_MAKESHLIBS"
dh_makeshlibs

echo "DH_INSTALLDEB"
dh_installdeb

echo "DH_SHLIBDEPS"
dh_shlibdeps

echo "DH_GENCONTROL"
dh_gencontrol

echo "DH_MD5SUMS"
dh_md5sums

echo "DH_BUILDDEP"
dh_builddeb