#!/bin/bash
NUM_CLIENTS=$1
for i in `seq 1 $NUM_CLIENTS`; do
  ./client "`hostname`.cs.wisc.edu" 22222 "/output.cgi?5" &
  echo "client $i"
done
