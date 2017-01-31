#!/bin/bash

if ! [ -d "testfiles" ]; then
	cd tests
fi

TEST_DESCRIPTIVE_FILE="testfiles.desc"
TEST_LIST_FILE="testfiles.list"
SIMULATE_RECEIVE_LOG="simulate_receive.log"


do_fail() 
{
    echo "$1"
    echo "Watch all output:"
    cat $SIMULATE_RECEIVE_LOG
    rm -vf $SIMULATE_RECEIVE_LOG
    exit -1
}

check_for_presence()
{
    FILE=$1
    STRING=$2
    RESULT=`grep -E "$STRING" $FILE`
    if [ -z "$RESULT" ]
    then
        do_fail "    FAILED!!! Test of finding \"$STRING\" failed =( "
    else
		echo "    OK Test of finding \"$STRING\" passed "
    fi
}


echo "Lirc simulate test start in `pwd`"
echo "    Transmit data via lirc"
echo "    Try to find all required occurences in output...."

./simulate_lirc.sh ../rfsniffer/wb-homa-rfsniffer ./testfiles/ 1>/dev/null 2>"${SIMULATE_RECEIVE_LOG}"


# read file line by line
while read LINE; 
do 
	# cut first word (it's test filename)
    # and get answer 
	ANSWER=`echo $LINE | cut -d '#' -f -1 | cut -d $' ' -f 2-`
	check_for_presence $SIMULATE_RECEIVE_LOG "$ANSWER"
done < $TEST_DESCRIPTIVE_FILE


echo "All tests passed =)"
    
rm -vf $SIMULATE_RECEIVE_LOG

