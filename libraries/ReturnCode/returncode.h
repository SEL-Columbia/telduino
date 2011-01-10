#ifndef RETURNCODE_H
#define RETURNCODE_H
#include <inttypes.h>

#ifdef __cplusplus
extern "C"{
#endif

#define success(X) ((X) == SUCCESS)
#define nsuccess(X) ((X) != SUCCESS)
#define ifsuccess(X) if ((X) == SUCCESS)
#define ifnsuccess(X) if ((X) != SUCCESS)

static const int8_t SUCCESS =	0;
static const int8_t FAILURE =	1;
static const int8_t ARGVALUEERR = 2;
static const int8_t PARSEERR =	3;
static const int8_t COMMERR =	4;
static const int8_t TIMEOUT =	5;

static char *returnStr[] = {"SUCCESS","FAILURE","ARGVALUEERR","PARSEERR","COMMERR","TIMEOUT"};

const char *RCstr(int8_t retCode);

#ifdef __cplusplus
}
#endif

#endif
