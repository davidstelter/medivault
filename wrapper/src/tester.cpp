/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
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