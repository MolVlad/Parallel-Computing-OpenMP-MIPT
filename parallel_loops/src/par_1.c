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

	double *array;
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
		array = calloc(sizeof(double), job_size);
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

		MPI_Scatter(array, per_proc, MPI_DOUBLE, sub_array, per_proc, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
		for (i=0; i<per_proc; i++)
        {
			sub_array[i] = sin(0.00001*sub_array[i]);
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
		FILE *ff = fopen("program_results/par_result_1.txt","w");
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
