/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
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

