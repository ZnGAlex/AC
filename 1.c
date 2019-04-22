#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pmmintrin.h>

typedef struct {
    float w, x, y, z;
} cuaternion;

void imprimir_cuaternion(cuaternion c) {
    printf("%.2f %.2f %.2f %.2f\n", c.x, c.y, c.z, c.w);
}

void inicializar_cuaternion(cuaternion *q) {
    (*q).x = 0.0f;
    (*q).y = 0.0f;
    (*q).z = 0.0f;
    (*q).w = 0.0f;
}

cuaternion multiplicar_cuaternion(cuaternion a, cuaternion b) {
    cuaternion c;

    c.w = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    c.x = -a.w * b.y + a.x * b.z + a.y * b.w + a.z * b.x;
    c.y = a.w * b.x - a.x * b.w + a.y * b.z + a.z * b.y;
    c.z = -a.w * b.w - a.x * b.x - a.y * b.y + a.z * b.z;

    return c;
}

cuaternion sumar_cuaternion(cuaternion a, cuaternion b) {
    cuaternion c;

    c.w = a.w + b.w;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;

    return c;
}

static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

/* Set *hi and *lo to the high and low order bits of the cycle counter.
Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo){
	asm("rdtsc; movl %%edx,%0; movl %%eax,%1" /* Read cycle counter */
	: "=r" (*hi), "=r" (*lo) /* and move results to */
	: /* No input */ /* the two outputs */
	: "%edx", "%eax");
}

/* Record the current value of the cycle counter. */
void start_counter(){
	access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter(){
	unsigned ncyc_hi, ncyc_lo;
	unsigned hi, lo, borrow;
	double result;

	/* Get cycle counter */
	access_counter(&ncyc_hi, &ncyc_lo);

	/* Do double precision subtraction */
	lo = ncyc_lo - cyc_lo;
	borrow = lo > ncyc_lo;
	hi = ncyc_hi - cyc_hi - borrow;
	result = (double) hi * (1 << 30) * 4 + lo;
	if (result < 0) {
		fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
	}
	return result;
}

int main(int argc, char* argv[]) {
    double q, ck;
    int n;
    int N;
    cuaternion *a, *b, *c;
    cuaternion quat, dp;
    FILE *ptr;

    if (argc != 3) {
        printf("Debes introducir el argumento q y el nombre del archivo resultante.\n");
        return 1;
    }

    n = atoi(argv[1]);

    q = (double) n;
    N = pow(10.0, q);

    /* Apertura de fichero */
    ptr = fopen(argv[2], "a");

    if (ptr == NULL)
        exit(1);

    fprintf(ptr, "q, ciclos\n");

    /* Reserva de memoria para vectores de cuaterniones */
    a = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);
    b = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);
    c = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);

    srand(time(NULL));
    /* Inicializacion de valores de cuaterniones del vector a de cuaterniones */
    for (int i = 0; i < N; i++)
    {
        quat.w = (float) (rand() % 10);
        quat.x = (float) (rand() % 10);
        quat.y = (float) (rand() % 10);
        quat.z = (float) (rand() % 10);
        a[i] = quat;
    }
    
    /* Inicializacion de valores de cuaterniones del vector b de cuaterniones */
    for (int i = 0; i < N; i++)
    {
        quat.w = (float) (rand() % 10);
        quat.x = (float) (rand() % 10);
        quat.y = (float) (rand() % 10);
        quat.z = (float) (rand() % 10);
        b[i] = quat;
    }

    /* Multiplicacion cuaternion auxiliar */
    for (int i = 0; i < N; i++) {
        c[i] = multiplicar_cuaternion(a[i], b[i]);
    }

    /* Inicializacion cuaternion dp */
    inicializar_cuaternion(&dp);
    start_counter();
    for(int i = 0; i < N; i++)
    {
        dp = sumar_cuaternion(dp, multiplicar_cuaternion(c[i], c[i]));
    }
    ck = get_counter();
    
    /* Impresion de resultados */
    printf("cuaternion dp: ");
    imprimir_cuaternion(dp);
    printf("Ciclos: %1.lf\n", ck);
    fprintf(ptr, "%d, %1.lf\n", n, ck);

    /* Cierre de fichero */
    fclose(ptr);

    /* Liberacion de memoria de vectores de cuaterniones */
    _mm_free(a);
    _mm_free(b);
    _mm_free(c);

    return 0;
}