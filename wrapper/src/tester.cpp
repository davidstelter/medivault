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
			
			test.selectCard(0);

			delete [] slots;
		}
	}
	printf("about to encrypt...\n");
	test.encryptFile( "bob.txt", "bob.enc", "bob" );
	test.signFile("bob.txt", "bob.sign", "bob");
}