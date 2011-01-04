#ifndef RETURNCODE_H
#define RETURNCODE_H
#include <inttypes.h>

#ifdef __cplusplus
extern "C"{
#endif

#define SUCCESS 0
#define FAILURE 1
#define ARGVALUEERR 2
#define PARSEERR 3
#define COMMERR 4

static char *returnStr[5] = {"SUCCESS","FAILURE","ARGVALUEERR","PARSEERR","COMMERR"};

const char *RCstr(int8_t retCode);

#ifdef __cplusplus
}
#endif

#endif
