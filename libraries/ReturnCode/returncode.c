#include "returncode.h"

/**
	Requires a return code otherwise returns "RCERR".
*/
const char* _RCstr(int8_t retCode)
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


const char* RCstr(int8_t retCode)
{
	return _RCstr(retCode);
}
