#include <omp.h>
#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <assert.h>

#define BUF_SIZE 100

void printComplex(FILE *fp, double complex *z, int N)
{
	int i;
	for (i = 0; i < N; i++)
	{
    		fprintf(fp, "%g%+gi ", creal(z[i]), cimag(z[i]));
	}
}

void scanComplex(FILE *fp, double complex *z, int N)
{
	double real, imag;
	int i;
	for (i = 0; i < N; i++)
	{
		fscanf(fp, "%lf%lfi", &real, &imag);
		z[i] = real + imag * I;
	}
}

int main()
{
	int N = 4;

	FILE *fpin = fopen("input.txt", "r");
	if (fpin == NULL)
	{
		printf("File input.txt not found\n");
		exit(-1);
	}

	FILE *fpout = fopen("output.txt", "w");
	if (fpout == NULL)
	{
		printf("File output.txt not found\n");
		exit(-1);
	}

	double complex *input = calloc(N, sizeof(double complex));
	scanComplex(fpin, input, N);
	printComplex(fpout, input, N);

	free(input);

	return 0;
}
