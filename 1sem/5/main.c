#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <malloc.h>

typedef enum {
  OK = 0,
  FAIL = 1
} status_t;

int main(int argc, char* argv[])
{
  status_t status = FAIL;
  int i = 0, rank = 0, size = 0;
  FILE *file;
  int digitsNumber = 0, partsNumber = 0, localPart;
  char string[9] = {0};
  int *firstNumber, *secondNumber, *result;
  int *result1, *result2;
  double startTime = 0, endTime = 0;
  int iter, carryFlag, carryFlag1, carryFlag2, carryFlagFromPrevious;

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
    MPI_Bcast(&partsNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

    startTime = MPI_Wtime();

    iter = 0;
    for (i = 1; i < size; i++)
    {
      localPart = ((partsNumber % (size - 1)) >= i) ? partsNumber / (size - 1) + 1 : partsNumber / (size - 1);

      MPI_Send(&firstNumber[iter], localPart, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&secondNumber[iter], localPart, MPI_INT, i, 0, MPI_COMM_WORLD);

      iter += localPart;
    }

    MPI_Recv(&carryFlag, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
    iter = 0;
    for (i = 1; i < size; i++)
    {
      localPart = ((partsNumber % (size - 1)) >= i) ? partsNumber / (size - 1) + 1 : partsNumber / (size - 1);

      MPI_Recv(&result[iter], localPart, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);

      iter += localPart;
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

    MPI_Bcast(&partsNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

    localPart = ((partsNumber % (size - 1)) >= rank) ? partsNumber / (size - 1) + 1 : partsNumber / (size - 1);

    firstNumber = calloc(localPart, sizeof(int));
    secondNumber = calloc(localPart, sizeof(int));
    result1 = calloc(localPart, sizeof(int));
    result2 = calloc(localPart, sizeof(int));

    MPI_Recv(firstNumber, localPart, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
    MPI_Recv(secondNumber, localPart, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);

    carryFlag1 = 0;
    for (i = localPart - 1; i >= 0; i--)
    {
      result1[i] = firstNumber[i] + secondNumber[i] + carryFlag1;
      carryFlag1 = result1[i] / 1000000000;
      result1[i] = result1[i] % 1000000000;
    }

    carryFlag2 = 1;
    for (i = localPart - 1; i >= 0; i--)
    {
      result2[i] = firstNumber[i] + secondNumber[i] + carryFlag2;
      carryFlag2 = result2[i] / 1000000000;
      result2[i] = result2[i] % 1000000000;
    }

    if (rank == (size - 1))
    {
      carryFlagFromPrevious = 0;
    }
    else
    {
      MPI_Recv(&carryFlagFromPrevious, 1, MPI_INT, rank+1, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
    }

    if (carryFlagFromPrevious == 0)
    {
      result = result1;
      carryFlag = carryFlag1;
    }
    else
    {
      result = result2;
      carryFlag = carryFlag2;
    }

    MPI_Send(&carryFlag, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
    MPI_Send(result, localPart, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  // Finish
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0)
  {
    printf("[TIME RES] %lf\n", endTime - startTime);
    free(result);
  }
  else
  {
    free(result1);
    free(result2);
  }

  MPI_Finalize();

  free(firstNumber);
  free(secondNumber);

  return 0;
}

