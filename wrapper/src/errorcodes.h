/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * errorcodes.h
 * This file contains the definitions of the error codes used by the 
 * CryptoWrapper system.
 * */

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
#define FAILED_TO_OPEN_READ_FILE	11
#define FAILED_TO_OPEN_WRITE_FILE	12
#define FAILED_TO_ENCRYPT			13
#define COULD_NOT_INIT_ENCRYPT		14
#define WRONG_PASSWORD				15
#define COULD_NOT_INIT_SIGN			16
#define FAILED_TO_SIGN				17
#define FAILED_TO_READ_FILE			18
#define COULD_NOT_INIT_DIGEST		19
#define FAILED_TO_DIGEST			20
#define FAILED_TO_VERIFY			21

