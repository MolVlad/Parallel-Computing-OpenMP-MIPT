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
		MPI_Send(&rank, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Recv(&buf, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		printf("[%d]\n", rank);
		if((rank+1) < size)
			MPI_Send(&rank, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();

	return 0;
}
