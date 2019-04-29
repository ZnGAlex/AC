#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pmmintrin.h>

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

void _mm_imprimir(__m128 var) {
    float s[4];
    _mm_store_ps(&s[0], var);
    printf("Valor: %f %f %f %f\n", s[0], s[1], s[2], s[3]);
    
}

__m128 _mm_multiplicar(__m128 a, __m128 b) {
    float elementos_a[4];
    __m128 resultado;
    _mm_store_ps(elementos_a, a);

    /* Vectores negativos */
    __m128 neg1 = _mm_set_ps(-1.0, 1.0, -1.0, 1.0);
    __m128 neg2 = _mm_set_ps(-1.0, -1.0, 1.0, 1.0);
    __m128 neg3 = _mm_set_ps(-1.0, 1.0, 1.0, -1.0);
    __m128 neg4 = _mm_set_ps(1.0, 1.0, 1.0, 1.0);

    /* Vectores auxiliares */
    __m128 vector_w = _mm_set1_ps(elementos_a[0]);
    __m128 vector_x = _mm_set1_ps(elementos_a[1]);
    __m128 vector_y = _mm_set1_ps(elementos_a[2]);
    __m128 vector_z = _mm_set1_ps(elementos_a[3]);
        
    /* Shuffle de vector b */
    __m128 aux1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 aux2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1, 0, 3, 2));
    __m128 aux3 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 aux4;

    /* Multiplicacion por vectores negativos */
    aux1 = _mm_mul_ps(aux1, neg1);
    aux2 = _mm_mul_ps(aux2, neg2);
    aux3 = _mm_mul_ps(aux3, neg3);

    /* Multiplicacion final */
    aux1 = _mm_mul_ps(vector_w, aux1);
    aux2 = _mm_mul_ps(vector_x, aux2);
    aux3 = _mm_mul_ps(vector_y, aux3);
    aux4 = _mm_mul_ps(vector_z, b);

    resultado = _mm_add_ps(_mm_add_ps(aux1, aux2), _mm_add_ps(aux3, aux4));

    return resultado;
}

int main(int argc, char* argv[]) {
    double q, ck;
    int n;
    int N;
    __m128 *a, *b, *c;
    __m128 quat, dp, aux1, aux2, aux3, aux4;
    FILE *ptr;

    if (argc != 3) {
        printf("Debes introducir el argumento q y el nombre del archivo resultante.\n");
        return 1;
    }

    n = atoi(argv[1]);

    q = (double) n;
    N = pow(10.0, q);

    /* Apertura de fichero */
    /*ptr = fopen(argv[2], "a");

    if (ptr == NULL)
        exit(1);

    fprintf(ptr, "q, ciclos\n");
    */
    /* Reserva de memoria para vectores de cuaterniones */
    a = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);
    b = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);
    c = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);

    
    srand(time(NULL));
    /* Inicializacion de valores de cuaterniones del vector a de cuaterniones */
    for (int i = 0; i < N; i++)
    {
        float z = (float) (rand() % 10);
        float x = (float) (rand() % 10);
        float y = (float) (rand() % 10);
        float w = (float) (rand() % 10);
        a[i] = _mm_set_ps(z, y, x, w);
        a[i] = _mm_set_ps(6.0, 4.0, 2.0, 0.0);
    }
    
    /* Inicializacion de valores de cuaterniones del vector b de cuaterniones */
    for (int i = 0; i < N; i++)
    {
        float z = (float) (rand() % 10);
        float x = (float) (rand() % 10);
        float y = (float) (rand() % 10);
        float w = (float) (rand() % 10);
        b[i] = _mm_set_ps(z, y, x, w);
        b[i] = _mm_set_ps(40.0, 30.0, 20.0, 10.0);
    }

    /* Multiplicacion cuaternion auxiliar */
    for (int i = 0; i < 1; i++) {
        c[i] = _mm_multiplicar(a[i], b[i]);
    }
    

    /* Inicializacion cuaternion dp */
    /*
    start_counter();
    for(int i = 0; i < N; i++)
    {
        
    }
    ck = get_counter();
    */
    
    /* Impresion de resultados */
    /*
    printf("cuaternion dp: ");
    
    printf("Ciclos: %1.lf\n", ck);
    fprintf(ptr, "%d, %1.lf\n", n, ck);
    */
    /* Cierre de fichero */
    //fclose(ptr);
    
    /* Liberacion de memoria de vectores de cuaterniones */
    _mm_free(a);
    _mm_free(b);
    _mm_free(c);

    return 0;
}