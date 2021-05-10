#include <mpi.h>
#include "stdio.h"


int main(int  argc, char** argv)
{
	int size = 0;
	int rank = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//printf("Hello from %d (size = %d)!\n", rank, size);

	int buf;

	if(rank==0)
	{
		printf("Special Hello from 0!\n");

		int i;
		for(i = 1; i < size; i++)
		{
			MPI_Recv(&buf, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
			printf("[ROOT]: MSG %d from %d\n", buf, i);
		}
	}
	else
	{
		printf("[%d] SENDING...\n", rank);
		MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		printf("[%d] SENT\n", rank);
	}

	MPI_Finalize();

	return 0;
}
