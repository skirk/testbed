#!/bin/bash

while read p; do
	echo "#define FUNCTION $p" | cat - ray_march.cl > temp && mv temp ray_march.cl
	printf "%-15s" $p
	while read value1 value2; do
		./testbed $value1 $value2
	done <distrib.txt
	tail -n +2 ray_march.cl > temp && mv temp ray_march.cl
	printf "\n"

done < cases.txt
