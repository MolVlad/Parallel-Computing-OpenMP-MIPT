#include <mpi.h>
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#define ROOT 0

enum Operation {
	MAX, MIN,
	SUM,
	PROD,
	LAND,
	BAND,
	LOR,
	BOR,
	LXOR,
	BXOR
};

int Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
int Reduce(void *send_buf, void *recv_buf, int count, MPI_Datatype datatype, enum Operation op, int root, MPI_Comm comm);
int Scatter(void *send_buf, int scount, MPI_Datatype sdatatype, void *recv_buf, int rcount, MPI_Datatype rdatatype, int root, MPI_Comm comm);
int Gather(void *send_buf, int scount, MPI_Datatype sdatatype, void *recv_buf, int rcount, MPI_Datatype rdatatype, int root, MPI_Comm comm);

int main(int  argc, char** argv)
{
//=================== Init variables ===================//

	int size = 0, rank = 0;
	double start_time = 0, end_time = 0;

	int n = 20;
	double times[n];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	double tick = MPI_Wtick();

	int *buf = calloc(size, sizeof(int));

	if(rank==ROOT)
	{
		for(int i = 0; i < size; i++)
		{
			buf[i] = i;
		}
	}

//=================== Bcast measurements ===================//

	double sum = 0, error_sum = 0, error = 0;

	for(int k = 0; k < n; k++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==ROOT)
			start_time = MPI_Wtime();

		//MPI_Bcast(buf, size, MPI_INT, ROOT, MPI_COMM_WORLD);
		Bcast(buf, size, MPI_INT, ROOT, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		if(rank == ROOT)
			end_time = MPI_Wtime();

		times[k] = end_time - start_time;
		sum += end_time - start_time;
	}
	
	if(rank==ROOT)
	{
		double avg = sum / n;
		for(int k = 0; k < n; k++)
		{ 
			error_sum += (avg - times[k])*(avg - times[k]);
		}

		error = sqrt(error_sum / (n*(n - 1)));

		printf("[BCAST]\n");
		printf("[TICK] %lf\n", tick);
		printf("[AVRG] %lf\n", avg);
		printf("[ERRr] %lf\n", error);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	printf("[%d]", rank);
	for(int i = 0; i < size; i++)
	{
		printf(" %d", buf[i]);
	}
	printf("\n");

//=================== Reduce measurements ===================//

	sum = 0, error_sum = 0, error = 0;

	int *recv_buf = calloc(size, sizeof(int));
	for(int k = 0; k < n; k++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==ROOT)
			start_time = MPI_Wtime();

		//MPI_Reduce(buf, recv_buf, size, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);
		Reduce(buf, recv_buf, size, MPI_INT, SUM, ROOT, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		if(rank == ROOT)
			end_time = MPI_Wtime();

		times[k] = end_time - start_time;
		sum += end_time - start_time;
	}
	
	if(rank==ROOT)
	{
		double avg = sum / n;
		for(int k = 0; k < n; k++)
		{ 
			error_sum += (avg - times[k])*(avg - times[k]);
		}

		error = sqrt(error_sum / (n*(n - 1)));

		printf("[REDUCE]\n");
		printf("[TICK] %lf\n", tick);
		printf("[AVRG] %lf\n", avg);
		printf("[ERRr] %lf\n", error);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	printf("[%d]", rank);
	if(rank==ROOT)
	{
		for(int i = 0; i < size; i++)
		{
			printf(" %d", recv_buf[i]);
		}
	}
	else
	{
		for(int i = 0; i < size; i++)
		{
			printf(" %d", buf[i]); 
		}
	}
	printf("\n"); 
	free(recv_buf);

//=================== Scatter measurements ===================//

	sum = 0, error_sum = 0, error = 0;

	recv_buf = calloc(1, sizeof(int));
	for(int k = 0; k < n; k++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==ROOT)
			start_time = MPI_Wtime();

		//MPI_Scatter(buf, 1, MPI_INT, recv_buf, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		Scatter(buf, 1, MPI_INT, recv_buf, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		if(rank == ROOT)
			end_time = MPI_Wtime();

		times[k] = end_time - start_time;
		sum += end_time - start_time;
	}

	if(rank==ROOT)
	{
		double avg = sum / n;
		for(int k = 0; k < n; k++)
		{ 
			error_sum += (avg - times[k])*(avg - times[k]);
		}

		error = sqrt(error_sum / (n*(n - 1)));

		printf("[SCATTER]\n");
		printf("[TICK] %lf\n", tick);
		printf("[AVRG] %lf\n", avg);
		printf("[ERRr] %lf\n", error);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(rank==ROOT)
	{
		printf("[%d]", rank);
		for(int i = 0; i < size; i++)
		{
			printf(" %d", buf[i]);
		}
		printf("\n");
	}
	else
		printf("[%d] %d\n", rank, recv_buf[0]);

	free(recv_buf);

//=================== Gather measurements ===================//

	sum = 0, error_sum = 0, error = 0;

	recv_buf = calloc(size, sizeof(int));
	for(int k = 0; k < n; k++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if(rank==ROOT)
			start_time = MPI_Wtime();

		//MPI_Gather(&rank, 1, MPI_INT, recv_buf, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		Gather(&rank, 1, MPI_INT, recv_buf, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		if(rank == ROOT)
			end_time = MPI_Wtime();

		times[k] = end_time - start_time;
		sum += end_time - start_time;
	}
	
	if(rank==ROOT)
	{
		double avg = sum / n;
		for(int k = 0; k < n; k++)
		{ 
			error_sum += (avg - times[k])*(avg - times[k]);
		}

		error = sqrt(error_sum / (n*(n - 1)));

		printf("[GATHER]\n");
		printf("[TICK] %lf\n", tick);
		printf("[AVRG] %lf\n", avg);
		printf("[ERRr] %lf\n", error);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(rank==ROOT)
	{
		printf("[%d]", rank);
		for(int i = 0; i < size; i++)
		{
			printf(" %d", recv_buf[i]);
		}
		printf("\n");
	}
	else
		printf("[%d] %d\n", rank, rank);

	free(recv_buf);

//=================== Free variables  ===================//

	MPI_Finalize();
	free(buf);

	return 0;
}

int Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
	int size = 0, rank = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Barrier(comm);

	if(rank==root)
	{
		for(int i = 1; i < size; i++)
		{
			MPI_Send(buf, count, datatype, i, 0, comm);
		}
	}
	else
	{
		MPI_Recv(buf, count, datatype, root, MPI_ANY_TAG, comm, NULL);
	}

	MPI_Barrier(comm);

	return 0;
}

int Reduce(void *send_buf, void *recv_buf, int count, MPI_Datatype datatype, enum Operation op, int root, MPI_Comm comm)
{
	if(datatype != MPI_INT)
		return -1;

	int size = 0, rank = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int *aux_buf = calloc(count, sizeof(MPI_INT));

	int *recv = recv_buf;
	int *send = send_buf;

	if(rank==root)
	{
		for(int j = 0; j < count; j++)
		{
			recv[j] = send[j];
		}

		switch (op)
		{
			case SUM:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] += aux_buf[j];
					}
				}
				break;
			case PROD:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] *= aux_buf[j];
					}
				}
				break;
			case MAX:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						if(recv[j]<aux_buf[j])
							recv[j] = aux_buf[j];
					}
				}
				break;
			case MIN:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						if(recv[j]>aux_buf[j])
							recv[j] = aux_buf[j];
					}
				}
				break;
			case LAND:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  recv[j] && aux_buf[j];
					}
				}
				break;
			case BAND:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  recv[j] & aux_buf[j];
					}
				}
				break;
			case LOR:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  recv[j] || aux_buf[j];
					}
				}
				break;
			case BOR:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  recv[j] | aux_buf[j];
					}
				}
				break;
			case LXOR:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  ((!recv[j]) && aux_buf[j]) || (recv[j] && (!aux_buf[j])) ;
					}
				}
				break;
			case BXOR:
				for(int i = 1; i < size; i++)
				{
					MPI_Recv(aux_buf, count, datatype, i, MPI_ANY_TAG, comm, NULL);

					for(int j = 1; j < count; j++)
					{
						recv[j] =  recv[j] ^ aux_buf[j];
					}
				}
				break;
			default:
				return -1;
		}
	}
	else
	{
		MPI_Send(send, count, datatype, root, 0, comm);
	}

	free(aux_buf);

	return 0;
}

int Scatter(void *send_buf, int scount, MPI_Datatype sdatatype, void *recv_buf, int rcount, MPI_Datatype rdatatype, int root, MPI_Comm comm)
{
	int size = 0, rank = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Barrier(comm);

	int *send = send_buf;
	int *recv = recv_buf;

	if(rank==root)
	{
		for(int k = 0; k < rcount; k++)
			recv[k] = send[k];

		for(int i = 1; i < size; i++)
		{
			MPI_Send(send+scount*i, scount, sdatatype, i, 0, comm);
		}
	}
	else
	{
		MPI_Recv(recv_buf, rcount, rdatatype, root, MPI_ANY_TAG, comm, NULL);
	}

	MPI_Barrier(comm);

	return 0;
}

int Gather(void *send_buf, int scount, MPI_Datatype sdatatype, void *recv_buf, int rcount, MPI_Datatype rdatatype, int root, MPI_Comm comm)
{
	int size = 0, rank = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Barrier(comm);

	int *send = send_buf;
	int *recv = recv_buf;

	if(rank==root)
	{
		for(int k = 0; k < rcount; k++)
			recv[k] = send[k];

		for(int i = 1; i < size; i++)
		{
			MPI_Recv(recv+rcount*i, rcount, rdatatype, i, MPI_ANY_TAG, comm, NULL);
		}
	}
	else
	{
		MPI_Send(send_buf, scount, sdatatype, root, 0, comm);
	}

	MPI_Barrier(comm);

	return 0;
}

/*
[BCAST]
[TICK] 0.000001
[AVRG] 0.000005
[ERRr] 0.000001
[0] 0 1 2 3
[1] 0 1 2 3
[2] 0 1 2 3
[3] 0 1 2 3
[REDUCE]
[TICK] 0.000001
[AVRG] 0.000003
[ERRr] 0.000001
[0] 0 4 8 12
[1] 0 1 2 3
[2] 0 1 2 3
[3] 0 1 2 3
[SCATTER]
[TICK] 0.000001
[AVRG] 0.000003
[ERRr] 0.000000
[0] 0 1 2 3
[1] 1
[2] 2
[3] 3
[GATHER]
[TICK] 0.000001
[AVRG] 0.000003
[ERRr] 0.000000
[0] 0 1 2 3
[1] 1
[2] 2
[3] 3
*/
