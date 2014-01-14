#!/bin/bash

while read p; do
	echo "#define FUNCTION $p" | cat - ray_march.cl > temp && mv temp ray_march.cl
	echo $p
	while read value1 value2; do
		echo $value1 $value2 && ./testbed $value1 $value2
	done <distrib.txt
	tail -n +2 ray_march.cl > temp && mv temp ray_march.cl

done < cases.txt
