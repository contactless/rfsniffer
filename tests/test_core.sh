#!/bin/bash

if ! [ -d "testfiles" ]; then
    cd tests
fi

TEST_DESCRIPTIVE_FILE="testfiles.desc"
TEST_LIST_FILE="testfiles.list"
SIMULATE_RECEIVE_LOG="simulate_receive.log"
STDIN_SIMULATE_LOG="stdin_simulate.log"
RETURN_CODE="0"

do_fail()
{
    echo "Watch all output:"
    cat $SIMULATE_RECEIVE_LOG
    # During building package we don't see output, so don't remove file
    # rm -vf $SIMULATE_RECEIVE_LOG
    exit -1
}

check_for_presence()
{
    FILE=$1
    STRING=$2
    RESULT=`grep -E "$STRING" $FILE`
    if [ -z "$RESULT" ]
    then
        echo "    FAILED!!! Test of finding \"$STRING\" failed =( "
        RETURN_CODE="-1"
    else
        echo "    OK Test of finding \"$STRING\" passed "
    fi
}

check_receive_log_for_all_answers()
{
    TEST_DESCRIPTIVE_FILE=$1
    # read file line by line
    while read LINE;
    do
        # cut first word (it's test filename)
        # and get answer
        ANSWER=`echo $LINE | cut -d '#' -f -1 | cut -d $' ' -f 2-`
        if ! [ -z "$ANSWER" ]
        then
            check_for_presence $SIMULATE_RECEIVE_LOG "$ANSWER"
        fi
    done < $TEST_DESCRIPTIVE_FILE
}

do_group_tests()
{
    GROUP=$1
    OPTS=$2
    EXE=../rfsniffer/wb-homa-rfsniffer OPTS="$OPTS" \
        TESTS_FOLDER=./testfiles/$GROUP TESTS_DESCR=./testfiles/$GROUP.desc \
        ./simulate_lirc.sh 1>>"$STDIN_SIMULATE_LOG" 2>>"${SIMULATE_RECEIVE_LOG}"
    check_receive_log_for_all_answers ./testfiles/$GROUP.desc
}

echo "Lirc simulate test start in `pwd`"
echo "    Transmit data via lirc"
echo "    Try to find all required occurences in output...."

echo "Start" > "$STDIN_SIMULATE_LOG"
echo "Start" > "${SIMULATE_RECEIVE_LOG}"


do_group_tests mix ""
do_group_tests vhome "-c '{\"enabled_protocols\": [\"VHome\"]}'"


if [ "$RETURN_CODE" -ne "0" ]
then
    do_fail
fi

echo "All tests passed =)"

rm -vf $SIMULATE_RECEIVE_LOG


