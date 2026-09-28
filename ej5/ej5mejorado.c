#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_DEFECTO 1000000

double compute(const double *v, int n)
{
    double acc = 0.0;
    for (int i = 0; i < n; i++)
        acc += v[i] * v[i];
    return acc;
}

int main(int argc, char *argv[])
{
    int rank, size;
    int N = N_DEFECTO;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc > 1) N = atoi(argv[1]);
    if (N <= 0) {
        if (rank == 0) fprintf(stderr, "N debe ser > 0\n");
        MPI_Finalize();
        return 1;
    }

    /* Tablas de reparto (las necesita el root en Scatterv) */
    int *counts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int base = N / size, resto = N % size;
    for (int r = 0, d = 0; r < size; r++) {
        counts[r] = base + (r < resto ? 1 : 0);
        displs[r] = d;
        d += counts[r];
    }

    double *array = NULL;
    if (rank == 0) {
        array = malloc((size_t)N * sizeof(double));
        if (!array) { fprintf(stderr, "malloc fallido\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        srand(12345);
        for (int i = 0; i < N; i++)
            array[i] = (double)rand() / RAND_MAX;
    }

    double *local = malloc((size_t)counts[rank] * sizeof(double));

    MPI_Barrier(MPI_COMM_WORLD);            /* para medir tiempos coherentes */
    double t0 = MPI_Wtime();

    /* Reparto: el maestro también recibe su partición */
    MPI_Scatterv(array, counts, displs, MPI_DOUBLE,
                 local, counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double partial = compute(local, counts[rank]);

    /* Reducción: suma de todos los parciales en el maestro */
    double total = 0.0;
    MPI_Reduce(&partial, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    if (rank == 0) {
        double ref = compute(array, N);
        printf("N=%d procs=%d\n", N, size);
        printf("Suma paralela   = %.10e\n", total);
        printf("Suma secuencial = %.10e\n", ref);
        printf("Error relativo  = %.3e\n", fabs(total - ref) / ref);
        printf("Tiempo (reparto+calculo+reduccion) = %.6f s\n", t1 - t0);
        free(array);
    }

    free(local); free(counts); free(displs);
    MPI_Finalize();
    return 0;
}
