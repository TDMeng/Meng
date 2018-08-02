#ifndef __ENV_H
#define __ENV_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define ENV_OK			0
#define ENV_ERROR		-1

int env_init(void);
int env_get(float *data, int len);
int env_set(float *data, int len);
int env_test(void);
#endif /* __ENV_H */
