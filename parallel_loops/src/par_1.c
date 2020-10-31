#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define ISIZE 1000
#define JSIZE 1000

int main(int argc, char **argv)
{
    static double a[ISIZE][JSIZE];
    int i, j;
    FILE *ff;

    #pragma omp parallel shared(a) private(i, j)
    {
		double start = omp_get_wtime();

		int num = omp_get_thread_num();
		int num_threads = omp_get_num_threads();

		// there are no dependencies here therefore we are able to parallel both loops
		#pragma omp for schedule(static) collapse(1)
		for (i=0; i<ISIZE; i++)
        {
            for (j=0; j<JSIZE; j++)
            {
                a[i][j] = 10*i +j;
            }
        }

		// there are only loop-carried dependencies here therefore we are able to parallel both loops
		// distance vector (0,0), direction vector (=,=)
		#pragma omp for schedule(static) collapse(1)
        for (i=0; i<ISIZE; i++)
        {
            for (j = 0; j < JSIZE; j++)
            {
                a[i][j] = sin(0.00001*a[i][j]);
            }
        }

		double end = omp_get_wtime();
		if (num == 0)
		{
			printf("%d\t%lf\n", num_threads, end-start);
		}
	}

    ff = fopen("program_results/par_result_1.txt","w");

	// we are unable to parallel that since it uses the same file descriptor
    for(i=0; i < ISIZE; i++)
    {
        for (j=0; j < JSIZE; j++)
        {
            fprintf(ff,"%f ",a[i][j]);
        }

        fprintf(ff,"\n");
    }

    fclose(ff);
}
