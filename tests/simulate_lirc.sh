#!/bin/bash

# script that helps to visually test
# how rfsniffer transfer information
# to mqtt and then to web-interface

LIRC="/dev/lirc42"

if ! [[ $# -gt 1 ]]; then
    echo "Need more arguments"
    echo "Need path to executable and path to tests"
    exit -1
fi

EXE=$1
TESTS_FOLDER=$2


if ! [ -f $EXE ]; then
    echo "Can not find $EXE"
    exit -1
fi

if ! [ -d $TESTS_FOLDER ]; then
    echo "Can not find $TESTS_FOLDER"
    exit -1
fi


rm  -v -f $LIRC

echo "create fifo ($LIRC) instead of character device"

mkfifo $LIRC

echo "take $EXE as executable rfsniffer"

CMD="$EXE -L -l $LIRC"

# stop service to avoid contradictions
service wb-homa-rfsniffer stop

echo "Start testing (run: $CMD)"

# run rfsniffer
eval "$CMD &"

for file in `find $TESTS_FOLDER -type f -name "*.rcf" | sort`
do
   wc -l $file
   cat $file >> $LIRC
   # this sends long pause and long impulse and again pause
   # so driver will be able to split packets
   echo -n -e "\x00\x0F\xFF\xFF\x01\x0F\xFF\xFF\x00\x0F\xFF\xFF" >> $LIRC
   
done

sleep 20


kill %3
kill %2
kill %1

rm -v -f $LIRC
