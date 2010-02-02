#pragma once

#define OK					0
#define DLL_LOAD_ERROR		1
#define NO_FUNC_LIST		2
#define INIT_FAILED			3
#define ALREADY_LOADED		4

void setError(int);

int getLastError();

