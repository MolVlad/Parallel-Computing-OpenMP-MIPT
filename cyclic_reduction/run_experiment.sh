#!/bin/bash

rm -f results.txt

echo $'Num\tTime' > results.txt
for seed in $(seq 1 100);  do
	for num in $(seq 1 10); do
		./a.out 1 $num 10 1 >> results.txt
	done
done
