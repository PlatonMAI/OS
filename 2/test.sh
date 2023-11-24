#!/bin/bash
for (( i=1; i<=50; i++ ))
do
	time1=$(date +'%s.%N')
	{
		./main $i < pattern.txt
	} &> /dev/null
	time2=$(date +'%s.%N')
	time=$(bc<<<"scale=9;$time2-$time1")
	echo "$i $time"
done