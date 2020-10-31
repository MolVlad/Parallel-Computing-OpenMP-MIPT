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

    double start = omp_get_wtime();

    for (i=0; i<ISIZE; i++)
    {
        for (j=0; j<JSIZE; j++)
        {
            a[i][j] = 10*i +j;
        }
    }

    for (i=8; i<ISIZE; i++)
    {
        for (j = 0; j < JSIZE-3; j++)
        {
            a[i][j] = sin(0.00001*a[i-8][j+3]);
        }
    }

    double end = omp_get_wtime();
    printf("%lf\n", end-start);

    ff = fopen("program_results/seq_result_3.txt","w");

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
