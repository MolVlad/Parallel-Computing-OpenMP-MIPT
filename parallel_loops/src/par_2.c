#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define ISIZE 5000
#define JSIZE 10000

#define ROOT 0

int main(int argc, char **argv)
{
    int i, j;
    double **a;

	int ind, ind_i, ind_j;
	
	int size = 0;
	int rank = 0;

	double start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	long long unsigned int job_size = ISIZE*JSIZE;
	int per_proc = job_size / size;
	if (job_size % size)
	{
		per_proc++;
	}
	
	job_size = per_proc * size;

	double *array = calloc(sizeof(double), job_size);
	double *sub_array = calloc(sizeof(double), per_proc);

	if(rank==0)
	{
		// --------- INIT ------- //

		a = calloc(sizeof(double *), ISIZE);
		for (i = 0; i < ISIZE; i++)
		{
			a[i] = calloc(sizeof(double), JSIZE);
		}

		for (i=0; i<ISIZE; i++)
        {
            for (j=0; j<JSIZE; j++)
            {
                a[i][j] = 10*i +j;
            }
        }

		// --------- FLATTEN ------- //
		for (i=0; i<ISIZE; i++)
        {
            for (j=0; j<JSIZE; j++)
            {
				array[i*JSIZE+j] = a[i][j];
            }
        }

		start = MPI_Wtime();
	}

		// --------- COMPUTE ------- //

		MPI_Bcast(array, job_size, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
		for (i = 0; i < per_proc; i++)
        {
			ind = rank * per_proc + i;
			ind_i = ind / JSIZE;
			ind_j = ind % JSIZE;
			if ((ind_j >= 6) && (ind_j < JSIZE - 1) && (ind_i >= 1) && (ind_i < ISIZE - 1))
			{
				sub_array[i] = sin(0.00001*array[(ind_i + 1) * JSIZE + (ind_j - 6)]);
			}
			else
			{
				sub_array[i] = array[ind_i * JSIZE + ind_j];
			}
        }
		MPI_Gather(sub_array, per_proc, MPI_DOUBLE, array, per_proc, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

	if (rank==0)
	{
		end = MPI_Wtime();
		printf("%d\t%lf\n", size, end-start);

		// --------- RESHAPE ------- //
		for (i=0; i<ISIZE; i++)
        {
            for (j=0; j<JSIZE; j++)
            {
				a[i][j] = array[i*JSIZE+j];
            }
        }

		// --------- SAVE ------- //
		FILE *ff = fopen("program_results/par_result_2.txt","w");
		for(i=0; i < ISIZE; i++)
		{
			for (j=0; j < JSIZE; j++)
			{
				fprintf(ff,"%f ",a[i][j]);
			}

			fprintf(ff,"\n");
		}

		fclose(ff);

		for (i = 0; i < ISIZE; i++)
		{
			free(a[i]);
		}

		free(a);
		free(array);
	}

	free(sub_array);
	
	MPI_Finalize();
}
