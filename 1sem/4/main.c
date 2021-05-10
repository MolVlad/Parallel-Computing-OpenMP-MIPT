#include <mpi.h>
#include "stdio.h"
#include "stdlib.h"

float item(int num)
{
	return 0.607927102/(num*num);
}

int main(int  argc, char** argv)
{
	if (argc != 2)
	{
		printf("arg error\n");
		return -1;
	}

	int seriesLength = atoi(argv[1]);
	if(seriesLength > 65535)
	{
		printf("Too much...\n");
		return -1;
	}

	int size = 0;
	int rank = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int len;

	double end_time, start_time = MPI_Wtime();
	if(rank==0)
	{
		for(int i = 1; i < size; i++)
			MPI_Send(&seriesLength, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

		len = seriesLength;
	}
	else
		MPI_Recv(&len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);

	int iterations = len / size;
	float result = 0;

	for(int i = 0; i < iterations; i++)
		result += item(rank * iterations + 1 + i);

	float buf;
	if(rank==0)
	{
		for(int i = 1; i < size; i++)
		{
			MPI_Recv(&buf, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
			result += buf;
		}
	}
	else
	{
		MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();

	if(rank==0)
	{
		for(int i = len - (len % size) + 1; i <= len; i++)
			result += item(i);

		end_time = MPI_Wtime();
	
		printf("[RES] %f [TIME] %f\n", result, end_time - start_time);
	}

	return 0;
}
