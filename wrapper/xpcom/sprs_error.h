#pragma once

#define OK							0
#define DLL_LOAD_ERROR				1
#define NO_FUNC_LIST				2
#define INIT_FAILED					3
#define ALREADY_LOADED				4
#define OPEN_SESSION_FAILED			5
#define KEY_NOT_FOUND				6
#define	INVALID_ATTRIBUTE			7
#define COULD_NOT_INIT_DECRYPTION	8
#define FAILED_TO_DECRYPT			9
#define NO_DEVICES_FOUND			10
#define FAILED_TO_OPEN_READ_FILE		11
#define FAILED_TO_OPEN_WRITE_FILE		12
#define WRONG_PASSWORD 13

void setError(int);

int getLastError();

