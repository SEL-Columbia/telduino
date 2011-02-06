#ifndef STATISTICS_H
#define STATISTICS_H
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif
void variancef(float *value, float *mean, float *variance, int32_t *nOld);

#ifdef __cplusplus
}
#endif
#endif
