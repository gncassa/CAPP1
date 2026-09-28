#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_DEFECTO 1000000

/* Suma de los cuadrados de los n elementos de v */
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

    /* Reparto: N/size para todos, y los N%size primeros reciben 1 extra */
    int base = N / size, resto = N % size;
    int mi_count = base + (rank < resto ? 1 : 0);

    double partial, total = 0.0;
    double t0 = 0.0, t1 = 0.0;

    if (rank == 0) {
        /* 1. El maestro genera el array aleatorio */
        double *array = malloc((size_t)N * sizeof(double));
        if (!array) { fprintf(stderr, "malloc fallido\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        srand(12345);
        for (int i = 0; i < N; i++)
            array[i] = (double)rand() / RAND_MAX;

        t0 = MPI_Wtime();

        /* 2. Envía a cada trabajador su partición */
        int despl = mi_count;               /* la partición 0 es del maestro */
        for (int r = 1; r < size; r++) {
            int cnt = base + (r < resto ? 1 : 0);
            MPI_Send(array + despl, cnt, MPI_DOUBLE, r, 0, MPI_COMM_WORLD);
            despl += cnt;
        }

        /* 3. El maestro calcula sobre su propia partición */
        total = compute(array, mi_count);

        /* 4. Recoge los parciales de los demás y acumula */
        for (int r = 1; r < size; r++) {
            MPI_Recv(&partial, 1, MPI_DOUBLE, r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total += partial;
        }
        t1 = MPI_Wtime();

        /* Comprobación de corrección con la suma secuencial */
        double ref = compute(array, N);
        printf("N=%d procs=%d\n", N, size);
        printf("Suma paralela   = %.10e\n", total);
        printf("Suma secuencial = %.10e\n", ref);
        printf("Error relativo  = %.3e\n", fabs(total - ref) / ref);
        printf("Tiempo (reparto+calculo+recogida) = %.6f s\n", t1 - t0);

        free(array);
    } else {
        double *local = malloc((size_t)mi_count * sizeof(double));
        if (!local) { fprintf(stderr, "malloc fallido\n"); MPI_Abort(MPI_COMM_WORLD, 1); }

        MPI_Recv(local, mi_count, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        partial = compute(local, mi_count);
        MPI_Send(&partial, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);

        free(local);
    }

    MPI_Finalize();
    return 0;
}
