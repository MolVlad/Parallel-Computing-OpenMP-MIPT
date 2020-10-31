#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define ISIZE 1000
#define JSIZE 1000

int main(int argc, char **argv)
{
    static double a[ISIZE][JSIZE];
    static double b[ISIZE][JSIZE];
    int i, j;
    FILE *ff;

    #pragma omp parallel shared(a, b) private(i, j)
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
				b[i][j] = 10*i +j;
			}
		}

		// there are loop independent dependencies here
		// distance vector (-1,6), direction vector (>,<) - anti dependency
		// we are able to parallel inner loop without constraints
		// it is also possible to parallel outer loop with preliminary data backup (a->b)
		#pragma omp for schedule(static) collapse(1)
		for (i=1; i<ISIZE-1; i++)
		{
			for (j = 6; j < JSIZE-1; j++)
			{
				a[i][j] = sin(0.00001*b[i+1][j-6]);
			}
		}

		double end = omp_get_wtime();
		if (num == 0)
		{
			printf("%d\t%lf\n", num_threads, end-start);
		}
	}

    ff = fopen("program_results/par_result_2.txt","w");

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
