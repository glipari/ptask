#!/bin/bash

function test {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo -e ">> test " $@ "\t\t \e[00;31m[FAIL]\e[00m"
    else 
	echo -e ">> test " $@ "\t\t\e[01;32m[OK]\e[00m" 
    fi
    return $status
}

test ./time
test ./key
test ./key_opt
test ./test_cores
test ./task_create
test ./task_aperiodic