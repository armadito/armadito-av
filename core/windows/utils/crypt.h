#ifndef __UTILS_CRYPT_H__
#define __UTILS_CRYPT_H__

#include <stdio.h>
#include <Windows.h>

#define A6O_PUBKEY "keys\\a6o_rsa_public.pem"

void print_hexa(BYTE* data, int size);

BYTE * GetFileHash(char * data, int len, ALG_ID algo);
HCRYPTKEY import_public_key(char * filename, HCRYPTPROV hProv);
HCRYPTHASH calc_file_hash(char * filename, HCRYPTPROV hProv, ALG_ID  algo);
int verify_file_signature(char *filename, char *sigfile);


#endif