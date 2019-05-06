#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pmmintrin.h>
#include <omp.h>

typedef struct {
	float w, x, y, z;
} cuaternion;
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

//Funciones cuaterniones
void imprimir_cuaternion(cuaternion c);
void inicializar_cuaternion(cuaternion *q);
void inicializar_cuaternion_rand(cuaternion *q);
cuaternion multiplicar_cuaternion(cuaternion a, cuaternion b);
cuaternion sumar_cuaternion(cuaternion a, cuaternion b);

//Funciones tiempo
void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
double get_counter();

//MAIN
int main(int argc, char* argv[]) {
	double ck;
	int N, q, k;
	cuaternion *a, *b, *c;
	cuaternion quat, dp;
	FILE *ptr;

	if (argc != 4) {
		printf("Debes introducir el argumento q, el nombre del archivo resultante y el numero de hilos.\n");
		return 1;
	}
	/* Apertura de fichero */
	if((ptr = fopen(argv[2], "a")) == NULL){
		printf("Error abrindo archivo\n");
		exit(1);
	}
	fprintf(ptr, "q, ciclos\n");	

	srand(time(NULL));
	q = atoi(argv[1]);
	N = pow(10.0, q);
	k = atoi(argv[3]);

	/* Reserva de memoria para vectores de cuaterniones */
	a = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);
	b = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);
	c = (cuaternion *) _mm_malloc(N * sizeof(cuaternion), 16);

	
	/* Inicializacion de valores de cuaterniones del vector a y b de cuaterniones */
	for (int i = 0; i < N; i++) {
		inicializar_cuaternion_rand(&quat);
		a[i] = quat;
		inicializar_cuaternion_rand(&quat);
		b[i] = quat;	
	}
	
	/* Inicializacion cuaternion dp */
	inicializar_cuaternion(&dp);
		

	start_counter();  // START COUNTER

	#pragma omp parallel num_threads(k)
	{
		
		#pragma omp for
		/* MULTIPLICACION C = A * B */
		for (int i = 0; i < 1; i++) {
			c[i] = multiplicar_cuaternion(a[i], b[i]);
		}
		

		#pragma omp barrier
		
		#pragma omp for
		/* SUMA Y MULTIPLICACION DP = DP + C * C */
		for(int i = 0; i < N; i++){
			#pragma omp critical
			{
				dp = sumar_cuaternion(dp, multiplicar_cuaternion(c[i], c[i]));
		
			}
		}

	}
	ck = get_counter(); // GET COUNTER
	
	/* Impresion de resultados */
	printf("Cuaternion dp: ");
	imprimir_cuaternion(dp);
	printf("Ciclos: %1.lf\n", ck);
	fprintf(ptr, "%d, %1.lf\n", q, ck);
	/* Cierre de fichero */
	fclose(ptr);

	/* Liberacion de memoria de vectores de cuaterniones */
	_mm_free(a);
	_mm_free(b);
	_mm_free(c);

	return 0;
}


void imprimir_cuaternion(cuaternion c) {
	printf("%.2f %.2f %.2f %.2f\n", c.w, c.x, c.y, c.z);
}

void inicializar_cuaternion(cuaternion *q) {
	(*q).w = 0.0f;
	(*q).x = 0.0f;
	(*q).y = 0.0f;
	(*q).z = 0.0f;
}

void inicializar_cuaternion_rand(cuaternion *q) {
	(*q).w = (float) (rand() % 10);
	(*q).x = (float) (rand() % 10);
	(*q).y = (float) (rand() % 10);
	(*q).z = (float) (rand() % 10);
}

cuaternion multiplicar_cuaternion(cuaternion a, cuaternion b) {
	cuaternion c;

	c.w =  a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	c.x = -a.w * b.y + a.x * b.z + a.y * b.w + a.z * b.x;
	c.y =  a.w * b.x - a.x * b.w + a.y * b.z + a.z * b.y;
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
