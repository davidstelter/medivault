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
	test.encryptFile( "bob.txt", "bob.enc", "bob" );
	test.signFile("bob.txt", "bob.sign", "bob");
	test.listKeys();
}