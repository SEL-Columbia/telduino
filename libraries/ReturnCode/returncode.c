#include "returncode.h"

/**
	Takes a return code and returns a short description and otherwise returns "RCERR".
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


/**
	Returns a short desciption of the error code.
  */
const char* RCstr(int8_t retCode)
{
	return _RCstr(retCode);
}
