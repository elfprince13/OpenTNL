#!/bin/bash
LIMIT=$1

for ((a=1; a <= LIMIT ; a++))  # Double parentheses, and "LIMIT" with no "$".
do
  echo Bot $a connecting to $1
  ./zapd -name bot"$a" -jsave bot"$a".journal -loss 0.2 -lag 150 -crazybot -connect $2 &
  echo Waiting 10 seconds for bot to connect...
  sleep 4
done                           # A construct borrowed from 'ksh93'.

