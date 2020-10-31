#!/bin/bash

rm -f experiment_results/results_1.txt

echo $'Num\tTime' > experiment_results/results_1.txt
for seed in $(seq 1 10);  do
	for num in $(seq 1 40); do
		export OMP_NUM_THREADS=$num
		./par_1 >> experiment_results/results_1.txt
	done
done
