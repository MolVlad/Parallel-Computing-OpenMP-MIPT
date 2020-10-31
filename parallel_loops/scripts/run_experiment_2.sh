#!/bin/bash

rm -f experiment_results/results_2.txt

echo $'Num\tTime' > experiment_results/results_2.txt
for seed in $(seq 1 10);  do
	for num in $(seq 1 40); do
		export OMP_NUM_THREADS=$num
		./par_2 >> experiment_results/results_2.txt
	done
done
