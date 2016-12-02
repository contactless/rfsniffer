#!/bin/bash

converter=./ism-test-converter
log=./ism-test-converter.log

rm $converter $log
g++ -O2 -o $converter ismTestConverter.cpp


for test in "666666666666669696699969669699999999699999669699696699966996999999696966999996966666999999669600000003fff0bffffffcffffff";
do
    id=`test | md5sum`
    name="ism-test-"${id:0:10}".rcf"
    echo $name
    echo $name >> $log 
    echo $test | $converter > "./testfiles/$name" 2>> $log
done
