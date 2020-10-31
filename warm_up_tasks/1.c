#include <omp.h>
#include <stdio.h>

int main()
{
	#pragma omp parallel
	{
		int num_threads, num;

		num_threads = omp_get_num_threads();
		num = omp_get_thread_num();

		printf("(%d/%d)\n", num, num_threads);
	}

	return 0;
}
