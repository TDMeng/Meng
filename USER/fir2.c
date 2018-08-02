//
//  fir2.c
//  Fir2
//
//  Created by Yudong He on 2018/4/3.
//  Copyright © 2018 Yudong He. All rights reserved.
//
#include "fir2.h"


void eqfilter(int N, int sr, float *banks, float *eqfactors, int numbanks, float * filter)
{
	/*Parameter*/
	int M, i, j;
	M = 2 * N + 1;

	/*transfer frequencies in banks to position points in nbanks*/
	float *nbanks = (float *)calloc(numbanks, sizeof(float));
	for (i = 0; i < numbanks; i++)
	{
		nbanks[i] = M * banks[i] / (sr / 2);
	}

	/*transfer db in eqfactors to amplitude factor in eq*/
	float *eq = (float *)calloc(numbanks, sizeof(float));
	for (i = 0; i < numbanks; i++)
	{
		eq[i] = pow(10, eqfactors[i] / 20);
	}

	/*calculate ideal amplitude frequency response*/
	float *FILTER = (float *)calloc(M, sizeof(float));
	for (j = 0; j < (int)(pow(10, log10(nbanks[i] * nbanks[i + 1]) / 2) + 0.5); j++)
	{
		FILTER[j] = eq[0];
	}
	for (i = 1; i < numbanks - 1; i++)
	{
		for (; j < (int)(pow(10, log10(nbanks[i] * nbanks[i + 1]) / 2) + 0.5); j++)
		{
			FILTER[j] = eq[i];
		}
	}
	for (; j < M; j++)
	{
		FILTER[j] = eq[i];
	}

	/*calculate rad and frequency response*/
	float complex rad = 0;
	float complex *Filter = (float complex *)calloc(2 * M - 2, sizeof(float complex));
	for (i = 0; i < M; i++)
	{
		rad = -1 * (N / 2) * I * PI * i / (M - 1);
		Filter[i] = FILTER[i] * cexp(rad);
	}

	float ifftInput[1024];

	ifftInput[0] = creal(Filter[0]);
	
	ifftInput[1] = creal(Filter[M - 1]);
	for (i = 1; i < M - 1; i++)
	{
		ifftInput[2 * i] = creal(Filter[i]);
		ifftInput[2 * i + 1] = cimag(Filter[i]);
	}

	/*calculate pulse response*/
	float filter_d[1024];
	
	arm_rfft_fast_instance_f32 S;
	int fftSize = 1024;
	int ifftFlag = 1;
	arm_rfft_fast_init_f32(&S, fftSize);
	arm_rfft_fast_f32(&S, ifftInput, filter_d, ifftFlag);

	/*calculate hamming function*/
	float *hamming = (float *)calloc(N, sizeof(float));
	float alpha = 0.46;
	for (i = 0; i < N; i++)
	{
		hamming[i] = (1 - alpha) - alpha * cos(2 * PI * i / (N - 1));
	}

	/*cut off filter_d by hamming window*/
	for (i = 0; i < N; i++)
	{
		filter[i] = creal(filter_d[i]) * hamming[i];
	}

	/*free buff*/
	free(nbanks);
	free(eq);
	free(FILTER);
	free(Filter);
	free(hamming);

}

