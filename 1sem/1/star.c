#include <mpi.h>
#include "stdio.h"


int main(int  argc, char** argv)
{
	int size = 0;
	int rank = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int buf;

	if(rank==0)
	{
		printf("Special Hello from 0!\n");

		int i;
		for(i = 1; i < size; i+=1)
		{
			MPI_Send(&rank, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Recv(&buf, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		}
	}
	else
	{
		MPI_Recv(&buf, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		printf("[%d]\n", rank);
		MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();

	return 0;
}
