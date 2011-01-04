#include "returncode.h"

/**
	Requires a return code and a string buffer of at least 10 characters
*/
const char* RCstr(int8_t retCode)
{
	if (retCode < 0) {
		retCode = -retCode;
	}
	if (retCode < sizeof(returnStr)/sizeof(char*)) {
		return returnStr[retCode];
	} else {
		return "RCERR";
	}
}


