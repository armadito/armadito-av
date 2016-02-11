#ifndef __UH_CRYPT_H__
#define __UH_CRYPT_H__

#include <stdio.h>
#include <Windows.h>

// for test purpose only. UhuruSvc.exe --crypt
#define PUBKEY "C:\\users\\david\\Desktop\\crypt_test\\openssl\\rsa_public.pem"
#define PRIVKEY "C:\\users\\david\\Desktop\\crypt_test\\openssl\\private.pem"
#define SIGNATURE_FILE "C:\\users\\david\\Desktop\\crypt_test\\openssl\\uhurudbvirus.json.sha1.sig"
#define FILE2SIGN "C:\\users\\david\\Desktop\\crypt_test\\openssl\\uhurudbvirus.json"

#define UH_PUBKEY "keys\\uh_rsa_public.pem"

HCRYPTKEY import_public_key(char * filename, HCRYPTPROV hProv);
HCRYPTHASH calc_file_hash(char * filename, HCRYPTPROV hProv, ALG_ID  algo);
int verify_file_signature(char *filename, char *sigfile);


#endif