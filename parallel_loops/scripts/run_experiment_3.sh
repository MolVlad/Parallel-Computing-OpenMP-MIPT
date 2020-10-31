#!/bin/bash

rm -f experiment_results/results_3.txt

echo $'Num\tTime' > experiment_results/results_3.txt
for seed in $(seq 1 10);  do
	for num in $(seq 1 40); do
		export OMP_NUM_THREADS=$num
		./par_3 >> experiment_results/results_3.txt
	done
done
