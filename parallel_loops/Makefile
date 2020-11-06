all: first second third

first:
	gcc src/seq_1.c -o seq_1 -lm -fopenmp
	mpicc -o par_1 src/par_1.c -lm
	gcc src/par_omp_1.c -o par_omp_1 -lm -fopenmp

second:
	gcc src/seq_2.c -o seq_2 -lm -fopenmp
	mpicc -o par_2 src/par_2.c -lm
	gcc src/par_omp_2.c -o par_omp_2 -lm -fopenmp

third:
	gcc src/seq_3.c -o seq_3 -lm -fopenmp
	gcc src/par_3.c -o par_3 -lm -fopenmp
clean:
	rm seq_1 par_1 seq_2 par_2 seq_3 par_3
