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
 * Tester.cpp
 * This file contains the tests for the CryptoWrapper system.
 * */

#include <iostream>

#include "CryptoWrapper.h"

void main() {
	CryptoWrapper test;
	if(test.initCrypto()){
		int TokenCount = test.getTokenCount();
		printf("Token count: %d\n", TokenCount);

		if (TokenCount != 0){
			string *slots;
			slots = test.enumerateCards();
			CK_UTF8CHAR temp[9] = "12345678";
			test.selectCard(0,temp,8);

			delete [] slots;
		}
	}
	printf("about to encrypt...\n");
	test.encryptFile( "bob.txt", "enc.txt", "PSU Captsone" );
	cout << test.LoadFile("enc.txt") << endl;
	test.signFile("bob.txt", "sign.txt", "PSU Captsone" );
	string temp = test.LoadFile("sign.txt");
	if( temp.compare("") == 0 ) {
		cout << "failed" << endl;
	} else {
		cout << temp << endl;
	}
	test.finalizeCrypto();
}
