/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito core.

Armadito core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Armadito core.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef __UTILS_CRYPT_H__
#define __UTILS_CRYPT_H__

#include <stdio.h>
#include <Windows.h>

//#define A6O_PUBKEY "keys\\a6o_rsa_public.pem"
#define A6O_PUB_KEY "http://db.armadito.org/keys/a6o_rsa_pub.pem"

void print_hexa(BYTE* data, int size);
char * download_pub_key( );
BYTE * GetFileHash(char * data, int len, ALG_ID algo);
HCRYPTKEY import_public_key(char * filename, HCRYPTPROV hProv);
HCRYPTHASH calc_file_hash(char * filename, HCRYPTPROV hProv, ALG_ID  algo);
int verify_file_signature(char *filename, char *sigfile);


#endif