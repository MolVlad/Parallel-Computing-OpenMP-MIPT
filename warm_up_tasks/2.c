#include <omp.h>
#include <stdio.h>

int main()
{
	omp_set_num_threads(10);

	int N = 10000;
	float sum = 0;

	printf("Enter N: ");
	scanf("%d", &N);

	#pragma omp parallel reduction(+:sum)
	{
		int num_threads, num;

		num_threads = omp_get_num_threads();
		num = omp_get_thread_num();

		float n;

		#pragma omp for
		for(int i=1; i<=N; ++i)
		{
			n = i;
			sum = sum + 1/n;
			printf("[%d/%d]: %f\n", num, num_threads, sum);
		}

	}

	printf("sum: %f\n", sum);

	return 0;
}
