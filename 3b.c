#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pmmintrin.h>

static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

//Funciones tiempo
void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
double get_counter();

//Funciones cuaterniones
void _mm_imprimir(__m128 var);
__m128 *_mm_multiplicar(__m128 a[4], __m128 b[4]);

int main(int argc, char* argv[]) {
	double ck;
	int q;
	int N;
	__m128 *a, *b, *c;
	__m128 quat, dp, aux1[4];
	__m128 *aux;
	FILE *ptr;

	if (argc != 3) {
		printf("Debes introducir el argumento q y el nombre del archivo resultante.\n");
		return 1;
	}

	srand(time(NULL));
	q = atoi(argv[1]);
	N = pow(10.0, q);
	if (N%4 != 0){
		printf("q no válido\n");
		exit(1);
	}

	/* Apertura de fichero */
	if((ptr = fopen(argv[2], "a")) == NULL){
		printf("Error al abrir el archivo");
		exit(1);
	}
	fprintf(ptr, "q, ciclos\n");
	
	
	/* Reserva de memoria para vectores de cuaterniones */
	a = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);
	b = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);
	c = (__m128 *) _mm_malloc(N * sizeof(__m128), 16);		
	
	/* Inicializacion de valores de cuaterniones del vector a de cuaterniones */
	for (int i = 0; i < N; i++)
	{
		float z = (float) (rand() % 10);
		float x = (float) (rand() % 10);
		float y = (float) (rand() % 10);
		float w = (float) (rand() % 10);
		a[i] = _mm_set_ps(z, y, x, w);
	}
	
	/* Inicializacion de valores de cuaterniones del vector b de cuaterniones */
	for (int i = 0; i < N; i++)
	{
		float z = (float) (rand() % 10);
		float x = (float) (rand() % 10);
		float y = (float) (rand() % 10);
		float w = (float) (rand() % 10);
		b[i] = _mm_set_ps(z, y, x, w);
	}

	/* Inicializacion cuaternion dp */
	dp = _mm_setzero_ps();

	start_counter();
	/* MULTIPLICACION C = A * B */
	for (int i = 0; i < N; i+=4) {
		aux = _mm_multiplicar(&a[i], &b[i]);
		c[i] = aux[0];
		c[i+1] = aux[1];
		c[i+2] = aux[2];
		c[i+3] = aux[3];

		free(aux);
	}


	/*SUMA Y MULTIPLICACION DP = DP + A * B */
	for (int i = 0; i < N; i+=4){
		aux1[0] = c[i];
		aux1[1] = c[i+1];
		aux1[2] = c[i+2];
		aux1[3] = c[i+3];
		aux = _mm_multiplicar(&c[i], aux1);
		dp = _mm_add_ps(dp, aux[0]);
		dp = _mm_add_ps(dp, aux[1]);
		dp = _mm_add_ps(dp, aux[2]);
		dp = _mm_add_ps(dp, aux[3]);

		free(aux);
	}
	ck = get_counter();
	
	
	/* Impresion de resultados */
	
	printf("cuaternion dp: ");
	_mm_imprimir(dp);
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

void juntarWXYZ(__m128 a[]){
	float a0[4],a1[4],a2[4],a3[4];
	
	_mm_store_ps(a0, a[0]);
	_mm_store_ps(a1, a[1]);
	_mm_store_ps(a2, a[2]);
	_mm_store_ps(a3, a[3]);

	a[0] = _mm_set_ps(a3[0], a2[0], a1[0], a0[0]);
	a[1] = _mm_set_ps(a3[1], a2[1], a1[1], a0[1]);
	a[2] = _mm_set_ps(a3[2], a2[2], a1[2], a0[2]);
	a[3] = _mm_set_ps(a3[3], a2[3], a1[3], a0[3]);
}

void separarWXYZ(__m128 a[]){
	juntarWXYZ(a);
}

void _mm_imprimir(__m128 var) {
	float s[4];
	_mm_store_ps(&s[0], var);
	printf("%f %f %f %f\n", s[0], s[1], s[2], s[3]);
	
}

__m128 *_mm_multiplicar(__m128 a[4], __m128 b[4]) {
	__m128 aux[4][4], *c;
	int i, j;

	c = (__m128*)malloc(4*sizeof(__m128));

	juntarWXYZ(a);
	juntarWXYZ(b);
	
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			aux[i][j] = _mm_mul_ps(a[i], b[j]);
		}
	}

	//c.w =  a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;
	c[0] = _mm_add_ps(aux[0][3],_mm_add_ps(aux[1][2],_mm_sub_ps(aux[3][0], aux[2][1])));
	//c.x = -a.w*b.y + a.x*b.z + a.y*b.w + a.z*b.x;
	c[1] = _mm_add_ps(aux[1][3],_mm_add_ps(aux[2][0],_mm_sub_ps(aux[3][1], aux[0][2])));
	//c.y =  a.w*b.x - a.x*b.w + a.y*b.z + a.z*b.y;
	c[2] = _mm_add_ps(aux[0][1],_mm_add_ps(aux[2][3],_mm_sub_ps(aux[3][2], aux[1][0])));
	//c.z = -a.w*b.w - a.x*b.x - a.y*b.y + a.z*b.z;
	c[3] = _mm_sub_ps(_mm_sub_ps(_mm_sub_ps(aux[3][3],aux[2][2]),aux[1][1]), aux[0][0]);
	
	separarWXYZ(c);

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

