#ifndef __LIGB_H
#define __LIGB_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define libg_log_info(fmt, ...)		printf("INFO<%s:%u>: " fmt "", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define libg_log_warn(fmt, ...)		printf("WARN<%s:%u>: " fmt "", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define libg_log_error(fmt, ...)	printf("ERROR<%s:%u>: " fmt "", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define libg_log_debug(fmt, ...)	printf("DEBUG<%s:%u>: " fmt "", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif /* __LIGB_H */
