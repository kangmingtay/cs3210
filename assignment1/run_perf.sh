#!/bin/bash

FILE1=$1
FILE2=$2

echo "Computing LCS of $FILE1 and $FILE2..."
echo "Running time with $FILE1 and $FILE2..."

echo "Number of process: 5"
time ./p5 $FILE1 $FILE2
echo "Number of process: 10"
time ./p10 $FILE1 $FILE2
echo "Number of process: 20"
time ./p20 $FILE1 $FILE2

echo "Number of threads: 5"
time ./t5 $FILE1 $FILE2
echo "Number of threads: 10"
time ./t10 $FILE1 $FILE2
echo "Number of threads: 20"
time ./t20 $FILE1 $FILE2
