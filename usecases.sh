#!/bin/bash

while read p; do
	echo "#define FUNCTION $p" | cat - ray_march.cl > temp && mv temp ray_march.cl
	./testbed $1 $2 > ./results/$p.txt
	tail -n +2 ray_march.cl > temp && mv temp ray_march.cl
done < cases.txt
