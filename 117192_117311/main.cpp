// Celem tego programu jest prezentacja pomiaru i analizy 
// efektywnosci programu za pomoc¹  CodeAnalyst(tm).
// Implementacja mno¿enia macierzy jest realizowana za pomoca typowego 
// algorytmu podrêcznikowego. 

#define _CRT_SECURE_NO_DEPRECATE //trzeba by³o daæ bo error z plikami
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "omp.h"
#include <cstdlib>

#define USE_MULTIPLE_THREADS true
#define MAXTHREADS 128
int NumThreads;
double start;

static const int ROWS = 1000;     // liczba wierszy macierzy
static const int COLUMNS = 1000;  // lizba kolumn macierzy

float matrix_a[ROWS][COLUMNS];    // lewy operand 
float matrix_b[ROWS][COLUMNS];    // prawy operand
float matrix_r[ROWS][COLUMNS];    // wynik

float matrix_check[ROWS][COLUMNS];

FILE *result_file;

void initialize_matrices()
{
	// zdefiniowanie zawarosci poczatkowej macierzy
	//#pragma omp parallel for 
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			matrix_a[i][j] = rand() % 10;// / RAND_MAX
			matrix_b[i][j] = rand() % 10;// / RAND_MAX;
			matrix_r[i][j] = 0.0;
		}
	}
}


void initialize_matrices_check()
{
	// zdefiniowanie zawarosci poczatkowej macierzy
#pragma omp parallel for 
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			matrix_check[i][j] = 0.0;
		}
	}
}

void initialize_matricesZ()
{
	// zdefiniowanie zawarosci poczatkowej macierzy
#pragma omp parallel for 
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			matrix_r[i][j] = 0.0;
		}
	}
}

void print_result()
{
	// wydruk wyniku
	///*
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			fprintf(result_file, "%6.1f ", matrix_a[i][j]);
		}
		fprintf(result_file, "\n");
	}
	fprintf(result_file, "\n");
	fprintf(result_file, "\n");
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			fprintf(result_file, "%6.1f ", matrix_b[i][j]);
		}
		fprintf(result_file, "\n");
	}
	//*/
	fprintf(result_file, "\n");
	fprintf(result_file, "\n");
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			fprintf(result_file, "%6.1f ", matrix_r[i][j]);
		}
		fprintf(result_file, "\n");
	}
}


void multiply_matrices_KJI_sek() {
	for (int k = 0; k < COLUMNS; k++)
		for (int j = 0; j < COLUMNS; j++)
			for (int i = 0; i < ROWS; i++)
				matrix_r[i][j] += matrix_a[i][k] * matrix_b[k][j];
}


void multiply_matrices_KJI_0() {
#pragma omp parallel for
	for (int k = 0; k < COLUMNS; k++)
		for (int j = 0; j < COLUMNS; j++)
			for (int i = 0; i < ROWS; i++)
				matrix_r[i][j] += matrix_a[i][k] * matrix_b[k][j];
}


void multiply_matrices_KJI_1() {
	int k, j, i;
#pragma omp parallel shared(matrix_a,matrix_b,matrix_r) private(k,j,i)
	{
#pragma omp for
		for (k = 0; k < COLUMNS; k++)
			for (j = 0; j < COLUMNS; j++)
				for (i = 0; i < ROWS; i++)
					matrix_r[i][j] += matrix_a[i][k] * matrix_b[k][j];
	}
}


void multiply_matrices_KJI_2() {
	int k, j, i;
#pragma omp parallel shared(matrix_a,matrix_b,matrix_r) private(k,j,i)
	{
		for (k = 0; k < COLUMNS; k++)
#pragma omp for
			for (j = 0; j < COLUMNS; j++)
				for (i = 0; i < ROWS; i++)
					matrix_r[i][j] += matrix_a[i][k] * matrix_b[k][j];
	}
}


void multiply_matrices_KJI_3() {
	int k, j, i;
#pragma omp parallel shared(matrix_a,matrix_b,matrix_r) private(k,j,i)
	{
		for (k = 0; k < COLUMNS; k++)
			for (j = 0; j < COLUMNS; j++)
#pragma omp for
				for (i = 0; i < ROWS; i++)
					matrix_r[i][j] += matrix_a[i][k] * matrix_b[k][j];
	}
}

void print_elapsed_time()
{
	double elapsed;
	double resolution;
	double time;

	// wyznaczenie i zapisanie czasu przetwarzania
	elapsed = (double)clock() / CLK_TCK;
	resolution = 1.0 / CLK_TCK;
	time = elapsed - start;
	printf("Czas: %8.4f sec \n",
		time);

	fprintf(result_file,
		"Czas : %8.4f sec (%6.4f sec rozdzielczosc pomiaru)\n",
		time, resolution);
	//return time;
}

int main(int argc, char* argv[])
{
	//srand((unsigned int)time(NULL));
	//	 start = (double) clock() / CLK_TCK ;
	if ((result_file = fopen("classic.txt", "w")) == NULL) { // "a" oznacza append
		fprintf(stderr, "nie mozna otworzyc pliku wyniku \n");
		perror("classic");
		return(EXIT_FAILURE);
	}


	//Determine the number of threads to use
	if (USE_MULTIPLE_THREADS) {
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		NumThreads = SysInfo.dwNumberOfProcessors;
		if (NumThreads > MAXTHREADS)
			NumThreads = MAXTHREADS;
	}
	else
		NumThreads = 1;
	fprintf(result_file, "Klasyczny algorytm mnozenia macierzy, liczba watkow %d \n", NumThreads);
	printf("liczba watkow  = %d\n\n", NumThreads);

	double min_time = 999999.0;
	initialize_matrices_check();
	initialize_matrices();
	start = (double)clock() / CLK_TCK;
	multiply_matrices_KJI_sek();
	fprintf(result_file, "KJI sekwencyjnie\t");
	printf("KJI sekwencyjnie\n");
	print_elapsed_time();
	print_result();


	initialize_matricesZ();
	start = (double)clock() / CLK_TCK;
	multiply_matrices_KJI_1();
	fprintf(result_file, "KJI parallel w 1\t");
	printf("KJI parallel w 1\n");
	print_elapsed_time();
	print_result();

	initialize_matricesZ();
	start = (double)clock() / CLK_TCK;
	multiply_matrices_KJI_2();
	fprintf(result_file, "KJI parallel w 2\t");
	printf("KJI parallel w 2\n");
	print_elapsed_time();
	print_result();

	initialize_matricesZ();
	start = (double)clock() / CLK_TCK;
	multiply_matrices_KJI_3();
	fprintf(result_file, "KJI parallel w 3\t");
	printf("KJI parallel w 3\n");
	print_elapsed_time();
	print_result();

	fclose(result_file);
	system("PAUSE");
	return(0);
}
