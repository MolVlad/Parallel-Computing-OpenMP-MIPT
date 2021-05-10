#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <malloc.h>

typedef enum {
  OK = 0,
  FAIL = 1
} status_t;

typedef enum {
  IDLE = 0,
  VALID = 1
} packet_status_t;

typedef struct {
  int firstNumber;
  int secondNumber;
  int partNumber;
} task_t;

typedef struct {
  int result1;
  int result2;
  int carryFlag1;
  int carryFlag2;
  int partNumber;
} response_t;

int main(int argc, char* argv[])
{
  status_t status = FAIL;
  MPI_Status MPIstatus = {};
  int i = 0, rank = 0, size = 0;
  FILE *file;
  int digitsNumber = 0, partsNumber = 0, localPart;
  char string[9] = {0};
  int *firstNumber, *secondNumber, *result;
  int *result1, *result2;
  double startTime = 0, endTime = 0;
  int iter, carryFlag, carryFlag1, carryFlag2;
  response_t response;
  task_t task;
  int slave = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0)
  {
    // Root branch
    if (argc != 3)
    {
      printf("Usage:\n [input file] [output file]\n");
      // Exit with fail status
      status = FAIL;
      MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Finalize();
      return 0;
    }

    file = fopen(argv[1], "r");
    fscanf(file, "%d\n", &digitsNumber);
    partsNumber = digitsNumber / 9;
    if (((digitsNumber % 9) != 0) || (partsNumber < (size - 1)))
    {
      printf("Error in reading file:\n");
      printf("1)Number of digits must be divisible by 9;\n");
      printf("2)Number of parts must be greater then number of processes.\n");
      // Exit with fail status
      status = FAIL;
      MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Finalize();
      return -1;
    }
    
    firstNumber = calloc(partsNumber, sizeof(int));
    secondNumber = calloc(partsNumber, sizeof(int));
    result = calloc(partsNumber, sizeof(int));

    for (i = 0; i < partsNumber; i++)
    {
      fscanf(file, "%9s", string);
      firstNumber[i] = atoi(string);
    }
 
    fscanf(file, "\n");

    for (i = 0; i < partsNumber; i++)
    {
      fscanf(file, "%9s", string);
      secondNumber[i] = atoi(string);
    }

    fclose(file);

    // Send status of arguments
    status = OK;
    MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);

    task_t *taskSchedule = calloc(partsNumber, sizeof(task_t));
    response_t *responses = calloc(partsNumber, sizeof(response_t));

    for (i = 0; i < partsNumber; i++)
    {
      taskSchedule[i].firstNumber = firstNumber[i];
      taskSchedule[i].secondNumber = secondNumber[i];
      taskSchedule[i].partNumber = i;
    }

    startTime = MPI_Wtime();

    for (i = 0; i < partsNumber; i++)
    {
      // Find slave
      MPI_Recv(&response, sizeof(response_t)/sizeof(int), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIstatus);
      slave = MPIstatus.MPI_SOURCE;
      if (MPIstatus.MPI_TAG == VALID)
      {
        responses[response.partNumber] = response;
      }
      MPI_Send(&taskSchedule[i], sizeof(task_t)/sizeof(int), MPI_INT, slave, OK, MPI_COMM_WORLD);
    }

    // End calc
    for (i = 1; i < size; i++)
    {
      MPI_Recv(&response, sizeof(response_t)/sizeof(int), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIstatus);
      slave = MPIstatus.MPI_SOURCE;
      if (MPIstatus.MPI_TAG == VALID)
      {
        responses[response.partNumber] = response;
      }
      MPI_Send(taskSchedule, sizeof(task_t)/sizeof(int), MPI_INT, slave, FAIL, MPI_COMM_WORLD);
    }

    carryFlag = 0;
    for (i = partsNumber - 1; i >= 0; i--)
    {
      printf("result1: %d\n", responses[i].result1);
      printf("carry1: %d\n", responses[i].carryFlag1);
      printf("result2: %d\n", responses[i].result2);
      printf("carry2: %d\n", responses[i].carryFlag2);

      result[i] = carryFlag ? responses[i].result2 : responses[i].result1;
      carryFlag = carryFlag ? responses[i].carryFlag2 : responses[i].carryFlag1;

      printf("result: %d\n", result[i]);
      printf("carry: %d\n", carryFlag);
    }

    endTime = MPI_Wtime();

    file = fopen(argv[2], "w");
    if (carryFlag == 1)
      fprintf(file, "1");
    for (i = 0; i < partsNumber; i++)
    {
      fprintf(file, "%d", result[i]);
    }
    fprintf(file, "\n");

    fclose(file);
    free(result);
    free(firstNumber);
    free(secondNumber);
    free(taskSchedule);
  }
  else
  {
    // Slave branch
    //
    // Check status
    MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (status == FAIL)
    {
      MPI_Finalize();
      return -1;
    }

    MPI_Send(&response, sizeof(response_t)/sizeof(int), MPI_INT, 0, IDLE, MPI_COMM_WORLD);

    while (1)
    {
      MPI_Recv(&task, sizeof(task_t)/sizeof(int), MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIstatus);
      if (MPIstatus.MPI_TAG == OK)
      {
        response.result1 = task.firstNumber + task.secondNumber;
        response.carryFlag1 = response.result1 / 1000000000;
        response.result1 = response.result1 % 1000000000;

        response.result2 = task.firstNumber + task.secondNumber + 1;
        response.carryFlag2 = response.result2 / 1000000000;
        response.result2 = response.result2 % 1000000000;

        response.partNumber = task.partNumber;
      }
      else
      {
        break;
      }

      MPI_Send(&response, sizeof(response_t)/sizeof(int), MPI_INT, 0, VALID, MPI_COMM_WORLD);
    }
  }

  // Finish
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0)
  {
    printf("[TIME RES] %lf\n", endTime - startTime);
  }

  MPI_Finalize();


  return 0;
}

