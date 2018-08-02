#include "AudioFilter.h"


void idft( double complex *H, int N, double complex *h ){
	int i,j;
	double complex sum;
	for(i = 0; i < N; i++){
		sum = 0;
		for(j = 0; j < N; j++){
			sum = sum + H[j] * cexp(I * 2 * PI * i * j / N);
		}
		h[i] = sum / N;
	}
}


int *eqfilter_int(int N, int sr, double *banks, double *eqfactors, int numbanks)
{
	/*Parameter*/
	int M, i, j;
	M = 2 * N;

	/*transfer frequencies in banks to position points in nbanks*/
	double *nbanks = (double *)calloc(numbanks, sizeof(double));
	for(i=0; i < numbanks; i++){
		nbanks[i] = M * banks[i] / (sr / 2);
	}

	/*transfer db in eq-factors to amplitude factor in eq*/
	double *eq = (double *)calloc(numbanks, sizeof(double));
	for(i = 0; i < numbanks; i++){
		eq[i] = pow(10, eqfactors[i] / 20);
	}

	/*calculate ideal amplitude frequency response*/
	double *FILTER = (double *)calloc(M, sizeof(double));
	for(j = 0; j < (int)((nbanks[0] + nbanks[1]) / 2 + 0.5);  j++){
		FILTER[j] = eq[0];
	}
	for(i = 1; i < numbanks - 1; i++){
		for(; j < (int)((nbanks[i] + nbanks[i + 1]) / 2 + 0.5); j++){
			FILTER[j] = eq[i];
		}
	}
	for(; j < M; j++){
		FILTER[j] = eq[i];
	}
	/*calculate rad and frequency response*/
	double complex rad = 0;
	double complex *Filter = (double complex*)calloc(2 * M - 2, sizeof(double complex));
	for(i = 0; i < M; i++){
		rad = -1 * (N / 2) * I * PI * i / (M - 1);
		Filter[i] = FILTER[i] * cexp(rad);
	}
	for(; i < 2 * M - 2; i++){
		Filter[i] = conj(Filter[2 * M - 2 - i]);
	}
	/*calculate hamming function*/
	double *hamming = (double *)calloc(N, sizeof(double));
	double alpha = 0.46;
	for(i = 0; i < N; i++){
		hamming[i] = (1 - alpha) - alpha * cos(2 * PI * i / (N - 1));
	}
	/*calculate pulse response*/
	double complex *filter_d = (double complex *)calloc(2 * M - 2, sizeof(double complex));
	idft(Filter, 2 * M - 2, filter_d);

	/*cut off filter_d by hamming window*/
	double *filter = (double *)calloc(N, sizeof(double));
	for(i = 0; i < N; i++){
		filter[i] = creal(filter_d[i]) * hamming[i];
	}

	double max = 0;
	for (i = 0; i < N; i++){
		if(max < fabs(filter[i]))	max = fabs(filter[i]);
	}
	int *filter_int = (int *)calloc(N, sizeof(int));
	for(i = 0; i < N; i++){
		filter_int[i] = (int)((filter[i] / max) * (pow(2, 31) - 1));
	}
	/*free buff*/
	free(nbanks);
	free(eq);
	free(FILTER);
	free(Filter);
	free(hamming);
	/*return filter pointer*/
	return filter_int;

}


__int64 convolution(short *history, int *filter, int store_ptr)
{
    __int64 result = 0;
    int ptr2, j;
    for (j = 0; j < SIZE_OF_SUM_FILTER_256; j++)
    {

        ptr2 = (store_ptr - j);

        if (ptr2 < 0)
        {
            ptr2 += HISTORY_NUM;
        }
				
        result += ((__int64)filter[j] * (__int64)history[ptr2]);
				//printf("filter[%d] = %d, history[%d] = %d\n", j, filter[j], j, history[ptr2]);
    }
    return result;
}

void cleanVolumeHistoryBuffer(void)
{
    for(int i = 0; i < HISTORY_NUM; i++)
    {
        volume_float_left[i] = 0;
        volume_float_right[i] = 0;
    }
    volume_store_ptr = 0;
}
