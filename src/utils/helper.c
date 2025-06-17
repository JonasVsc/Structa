#include "helper.h"


const char* stStringResult(StResult result)
{
	switch (result)
	{
	case ST_SUCCESS:
		return "Success";
	case ST_ERROR:
		return "Error";
	default:
		return "Invalid or unknown code result";
	}
}