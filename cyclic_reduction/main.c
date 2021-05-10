#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>

#define ITERATION_LIMIT 100

double h, EPS = 0.00000001;
int threads, N, degree;

void forward(int iter, int start, int step, double* alpha, double* beta, double* gamma, double* F);
void backward(int iter, int start, int step, double* y, double *alpha, double* beta, double* gamma, double* F);

double* cyclic_reduction(double* alpha, double* beta, double* gamma, double* y, double* F);

double b;

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf ("use: %s b threads degree verbose\n", argv[0]);
        return -1;
    }

    b = strtof (argv[1], NULL);
    threads = atoi (argv[2]);
    degree = atoi(argv[3]);
    N = pow(2, degree);
    h = 1./N;
    int verbose = atoi (argv[4]);

    double* y = (double*) calloc((N+1), sizeof(double));
    double* next = (double*) calloc((N+1), sizeof(double));
    double* alpha = (double*) calloc((N+1), sizeof(double));
    double* beta  = (double*) calloc((N+1), sizeof(double));
    double* gamma = (double*) calloc((N+1), sizeof(double));
    double* F = (double*) calloc((N+1), sizeof(double));

    for (int i = 0; i < N + 1; i++)
    {
        y[i] = (b-1)*i/N+1;
        next[i] = (b-1)*i/N+1;
        alpha[i] = 1.;
        gamma[i] = 1.;
    }

    double * save;
    double diff = -1;
    double residual = -1;
    int i = 0;

    double start = omp_get_wtime ();
    while (i < ITERATION_LIMIT)
    {
        #pragma omp parallel shared(beta, F, y)
        {
        #pragma omp  for schedule(dynamic)
            for (int i = 1; i < N; i++)
            {
                beta[i] = -2 - h*h * exp(y[i]);
                F[i] = h*h * ((exp(y[i-1]) + 10*exp(y[i]) + exp(y[i+1]))/12  - y[i] * exp(y[i]));
            }
        }
        #pragma omp parallel for shared(alpha, gamma) schedule(guided)
        for (int i = 0; i < N + 1; i++)
        {
            alpha[i] = 1.;
            gamma[i] = 1.;
        }

        cyclic_reduction(alpha, beta, gamma, next, F);

        next[0] = 1;
        next[N] = b;

        residual = 0;
        #pragma omp parallel for num_threads(threads) shared(y, next) reduction(+: residual)
        for (int i = 0; i < N+1; i++)
        {
            residual += fabsl(y[i] - next[i]);
        }

        save = y;
        y = next;
        next = save;

        if(verbose)
        {
            printf("[%d]\tdifference: %.30lf\n", i, residual);
            fflush(stdout);
        }

        if(residual < EPS)
            break;
        i++;
    }

    double end = omp_get_wtime ();

    printf("b: %lf\tdifference: %.30lf\titerations: %d\tthreads: %d\ttime: %lf\n", b, residual, i, threads, end - start);
    //printf("%d\t%lf\n", threads, end - start);

    FILE * file = fopen("x.txt", "w");
    double x = 0;
    for (int i = 0; i < N+1; ++i)
    {
        fprintf(file, "%0.30lf\n", x);
        x += (double) h;
    }
    fclose(file);

    file = fopen("y.txt", "w");
    for (int i = 0; i < N+1; ++i)
    {
        fprintf(file, "%0.30lf\n", next[i]);
    }
    fclose(file);

    free(y);
    free(next);
    free(alpha);
    free(beta);
    free(gamma);
    free(F);

    return 0;
}

void forward(int iter, int start, int step,
        double *alpha, double* beta, double* gamma, double* F)
{
    double next_alpha, next_beta, next_gamma, next_F;
    int k = start * (iter + 1);

    next_alpha = -alpha[k] * alpha[k - step] / beta[k - step];
    next_beta = beta[k] - gamma[k - step] * alpha[k] / beta[k - step] - gamma[k] * alpha[k + step] / beta[k + step];
    next_gamma = -gamma[k] * gamma[k + step] / beta[k + step];
    next_F = F[k] - alpha[k] * F[k - step] / beta[k - step] - gamma[k] * F[k + step] / beta[k - step];

    alpha[k] = next_alpha;
    beta[k] = next_beta;
    gamma[k] = next_gamma;
    F[k] = next_F;
}

void backward(int iter, int start, int step, double* y,
                 double *alpha, double* beta, double* gamma, double* F)
{
    int k = start * (2*iter + 1);
    y[k] = (F[k] - gamma[k]*y[k+step] - alpha[k]*y[k-step]) / beta[k];
}

double* cyclic_reduction(double *alpha, double *beta, double *gamma, double* y, double *F)
{
    int elements = N - 1;
    int start = 2;
    int step = 1;

    // forward
    for (int j = 0; j < degree; j++)
    {
        elements = (elements - 1) / 2;

        #pragma omp parallel for num_threads(threads) shared(alpha, beta, gamma, F, elements, start, step)
        for (int i = 0; i < elements; i++)
        {
            forward(i, start, step, alpha, beta, gamma, F);
        }
        #pragma omp barrier
        start = 2 * start;
        step = 2 * step;
    }

    // backward
    elements = 1;
    start = N / 2;
    step = start;
    for (int j = degree-1; j >= 0 ; j--)
    {
        #pragma omp parallel for num_threads(threads) shared(alpha, beta, gamma, F, elements, start, step, y)
        for (int i = 0; i < elements; i++)
        {
            backward(i, start, step, y, alpha, beta, gamma, F);
        }
        #pragma omp barrier
        start = start / 2;
        step = start;
        elements = elements * 2;
    }
    return y;
}

