#!/bin/bash

# use ismTestConverter to convert tests
# from ism-radio format to lirc format
#

converter=./ism-test-converter
log=./ism-test-converter.log

rm $converter $log
g++ -O2 -o $converter ismTestConverter.cpp

tests=(
    '666666666666669696699969669699999999699999669699696699966996999999696966999996966666999999669600000003fff0bffffffcffffff' 
    '666666666666669696699969669699999996996669966699699669696996999999669999699999666666996969996900000000000000003fbfffffdf'
    'aaa99aa95995665696999659a595565a955a9665995000000000000000000007ffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
    'aaaa66aa56655995a5a655a969655596a556aa59555400000000000000000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
    'aaaa66aa56655995a5a6556969655596a5566a59699400000000000000000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
    'aaa99aa95995665696995565a595565a955969659a900000000000000000000fffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
    'aaaa66aa56655995a5a655a5696555555956a55965a600000000000000000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
    'aaaa66aa56655995a5a655a5696555555956a55965a600000000000000000001ffffffffffffffffffffffffffffffffffffffffffffffffffffffff'
)

IFS=$''
for test in "${tests[@]}";
do
    id=`echo $test | md5sum`
    name="ism-test-"${id:0:10}".rcf"
    echo $name $test
    echo $name >> $log 
    echo $test | $converter -v -d > "./testfiles/$name" 2>> $log
done
