#!/bin/bash

# script that helps to visually test
# how rfsniffer transfer information
# to mqtt and then to web-interface

LIRC="./lirc_rfm_test_device"

if ! [[ $# -gt 1 ]]; then
    echo "Need more arguments"
    echo "Need path to executable and path to tests"
    exit -1
fi

EXE=$1
TESTS_FOLDER=$2
TESTS_DESCR=""

if [[ $# -gt 2 ]]; then
    TESTS_DESCR=$3
fi


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

CMD="$EXE -T -l $LIRC"


# Make sure only root can run our script
if [[ $EUID -ne 0 ]]; then
    echo "Don't try to stop service wb-homa-rfsniffer"
	echo "It's probably not WB device"
else
	echo "Stop service wb-homa-rfsniffer to avoid contradictions"
	service wb-homa-rfsniffer stop
fi

# too long too wait
#echo "Clean mqtt tree"
#mqtt-delete-retained '/devices/#'

echo "Start testing (run: $CMD)"

# run rfsniffer
eval "$CMD &"

if [ -z "$TESTS_DESCR" ]; then
    for file in `find $TESTS_FOLDER -type f -name "*.rcf" | sort`
    do
        # do not print file name so it will not be mixed with rfsniffer output
        wc -l $file
        cat $file >> $LIRC
        # this sends long pause and long impulse and again pause
        # so driver will be able to split packets
        echo -n -e "\x40\x42\x0F\x01\x40\x42\x0F\x00" >> $LIRC
    done
else
    # read file line by line
    while read LINE; 
    do 
        # cut first word (it's test filename)
        # and get answer 
        FILE_NAME=`echo $LINE | cut -d '#' -f -1 | cut -d $' ' -f -1`
        if [ -z "$FILE_NAME" ]; then
            continue
        fi
        file="$TESTS_FOLDER/$FILE_NAME"
        # do not print file name so it will not be mixed with rfsniffer output
        wc -l $file
        cat $file >> $LIRC
        # this sends long pause and long impulse and again pause
        # so driver will be able to split packets
        echo -n -e "\x40\x42\x0F\x01\x40\x42\x0F\x00" >> $LIRC
    done < $TESTS_DESCR
fi




# wait for job (CMD) completion
if ! wait -n
then
	sleep 5
	kill %1
	kill %2
	kill %3
fi

rm -v -f $LIRC
