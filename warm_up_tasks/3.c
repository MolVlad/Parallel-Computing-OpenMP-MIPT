#include <omp.h>
#include <stdio.h>

int main()
{
	omp_set_num_threads(10);

	int shared = 0;

	#pragma omp parallel shared(shared)
	{
		int num_threads, num;

		num_threads = omp_get_num_threads();
		num = omp_get_thread_num();

		#pragma omp for ordered
		for(int i=0; i<num_threads; i++)
		{
		#pragma omp ordered
		{
			shared = shared + num;
			printf("(%d/%d): shared = %d\n", num, num_threads, shared);
		}
		}
	}

	return 0;
}
