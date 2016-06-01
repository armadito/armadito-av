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

#include <libarmadito.h>
#include "crypt.h"
#include "others.h"


void print_hexa(BYTE* data, int size) {

	int i = 0;
	if (data == NULL || size <= 0)
		return;

	for(i = 0 ; i < size ; i++){
        printf("%02x ",data[i]);
    }

	return;
}

char * download_pub_key( ) {

	int ret = 0, len= 0;
	char * pubkey_path = NULL;
	HRESULT hres = S_OK;

	len = MAX_PATH;
	pubkey_path = (char*)calloc(len +1, sizeof(char));
	pubkey_path[len] = '\0';
	
	hres = URLDownloadToCacheFileA(NULL, A6O_PUB_KEY, pubkey_path,MAX_PATH, 0,NULL);
	if (FAILED(hres)) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: download_pub_key :: URLDownloadToCacheFileA failed :: error = 0x%x\n",hres);
		free(pubkey_path);
		pubkey_path = NULL;
		return pubkey_path;
	}

	printf("[+] Debug :: URLDownloadToCacheFileA ::  pubkey_path = %s\n", pubkey_path);

	return pubkey_path;
}

BYTE * GetFileHash(char * data, int len, ALG_ID algo ) {

	BYTE * hashval = NULL;
	int hashsize = 0;
	int i = 0;
	int ret = 0;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	
	
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380%28v=vs.85%29.aspx
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa382453%28v=vs.85%29.aspx
	
	if (data == NULL || len <= 0) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Invalid parameters!\n");
		return NULL ; 
	}


	__try {

		// Acquire a handle to a particular key container within a particular cryptographic service provider.
		if (CryptAcquireContextA(&hProv,NULL, NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Acquire crypt context failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		if (CryptCreateHash(hProv, algo, 0, 0, &hHash) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Create hash failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		if (CryptHashData(hHash, data, len, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Crypt hash data failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		// determine the size of the hash value
		if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &hashsize, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		//printf("[+] Debug :: GetFileHash :: Hash size = %d\n",hashsize);

		hashval = (BYTE*)calloc(hashsize + 1, sizeof(BYTE));
		hashval[hashsize] = '\0';

		if (CryptGetHashParam(hHash, HP_HASHVAL, hashval, &hashsize, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			ret = -1;
			__leave;
		}


		/*printf("[+] Debug :: GetFileHash :: Hash value = ");
		print_hexa(hashval, hashsize);
		printf("\n");*/


	}
	__finally {

		if (CryptReleaseContext(hProv, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: GetFileHash :: Release crypt context failed. :: GLE = %d\n",GetLastError);
			//return NULL;
		}

		CryptDestroyHash(hHash);

		if (hashval != NULL && ret<0 ) {
			free(hashval);
			hashval = NULL;
		}

	}
	


	// Get a handle to a hash value
	//CryptCreateHash()
	

	return hashval;
}


HCRYPTKEY import_public_key(char * filename, HCRYPTPROV hProv) {

	HCRYPTKEY hPubKey = 0;
	BYTE * content = NULL, * buf = NULL;
	VOID * pubKeyBlob = NULL;
	int size = 0 , cbPubKeyBlob = 0, buf_size = 0,  i = 0;

	if (filename == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: invalid parameters\n");
		return 0;
	}

	__try {
		
		// import public key from file.
		content = GetFileContent_b(filename,&size);
		if (content == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Get public key content failed\n");
			__leave;
		}

		//printf("[+] Debug :: import_public_key :: pksize = %d :: content = \n%s\n\n", size,content);


		if (CryptStringToBinaryA(content, size, CRYPT_STRING_BASE64_ANY, NULL, &buf_size, 0, NULL) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: Crypt String to Binary failed :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		//printf("[+] Debug :: import_public_key :: public key buf size = %d bytes\n", buf_size);

		buf = (BYTE*)calloc(buf_size,sizeof(BYTE));

		if (CryptStringToBinaryA(content, size, CRYPT_STRING_BASE64_ANY, buf, &buf_size, NULL, NULL) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: Crypt String to Binary failed :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		printf("\n");
		printf("[+] Debug :: import_public_key :: Public key value = \n");
		print_hexa(buf, buf_size);
		printf("\n");
		printf("\n");


		if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buf, buf_size,0, NULL, NULL, &cbPubKeyBlob) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: Crypt Decode public key Object failed 1 :: GLE = 0x%x\n",GetLastError());
			__leave;
		}
		
		//printf("[+] Debug :: import_public_key :: public key Blob size = %d bytes\n",cbPubKeyBlob);

		pubKeyBlob = (BYTE*)calloc(cbPubKeyBlob ,sizeof(BYTE));

		if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buf, buf_size,0, NULL, pubKeyBlob, &cbPubKeyBlob) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: Crypt Decode public key Object failed 2 :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		if (!CryptImportKey(hProv, pubKeyBlob, cbPubKeyBlob, 0, 0, &hPubKey)){
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: import_public_key :: CryptImportKey for public key failed :: GLE = 0x%x\n", GetLastError());
			__leave;
		}
		


	}
	__finally {

		if (content != NULL) {
			free(content);
			content = NULL;
		}

		if (buf != NULL) {
			free(buf);
			buf = NULL;
		}

		if (pubKeyBlob != NULL) {
			free(pubKeyBlob);
			pubKeyBlob = NULL;
		}

	}

	
	return hPubKey;

}


HCRYPTHASH calc_file_hash(char * filename,HCRYPTPROV hProv, ALG_ID  algo) {

	HCRYPTHASH hHash = 0;
	BYTE * content = NULL , * hashval = NULL;
	int size = 0, hashsize = 0, i =0;

	__try {


		content = GetFileContent_b(filename, &size);
		if (content == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: calc_file_hash :: Get file content failed\n");
			__leave;
		}

		if (CryptCreateHash(hProv, algo, 0, 0, &hHash) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: calc_file_hash :: Create hash failed. :: GLE = 0x%x\n", GetLastError());
			__leave;
		}

		if (CryptHashData(hHash, content, size, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: calc_file_hash :: Crypt hash data failed. :: GLE = %d\n",GetLastError);			
			__leave;
		}

		// display file hash (for debug only).
		if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &hashsize, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: calc_file_hash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		hashval = (BYTE*)calloc(hashsize + 1, sizeof(BYTE));
		hashval[hashsize] = '\0';

		if (CryptGetHashParam(hHash, HP_HASHVAL, hashval, &hashsize, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: calc_file_hash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		printf("\n");
		printf("[+] Debug :: calc_file_hash :: Hash value = \n");
		print_hexa(hashval, hashsize);
		printf("\n");
		printf("\n");


	}
	__finally {

		if (content != NULL) {
			free(content);
			content = NULL;
		}

		/*if (CryptReleaseContext(hProv, 0) == FALSE) {
			printf("[-] Error :: calc_file_hash :: Release crypt context failed. :: GLE = %d\n",GetLastError);			
		}*/

	}

	return hHash;

}

/*
verify_file_signature
This function verify the signature of a file with public key.
returns 0 on success or a value less than zero on error.
*/
int verify_file_signature(char *filename, char *sigfile){

	int ret = 0;
	BYTE * content = NULL, * signature = NULL, * invert = NULL;
	char * pubkeyfile = NULL;
	HCRYPTKEY hPubKey = 0;
	HCRYPTHASH hHash = 0;
	HCRYPTPROV hProv = 0;
	int size = 0, i = 0, j =0, sig_size = 0;
	int fsize = 0;

	if (filename == NULL || sigfile == NULL) {
		a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: invalid parameters\n");
		return -1;
	}

	__try {

		// Acquire crypt context
		if (CryptAcquireContextA(&hProv,NULL, MS_ENHANCED_PROV,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Acquire crypt context failed. :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		// get pub key file path.
		pubkeyfile = download_pub_key();
		if (pubkeyfile == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Can't download public key from armadito server!\n");
			__leave;
		}
		

		// import public key from file.
		hPubKey = import_public_key(pubkeyfile,hProv);
		if (hPubKey == 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: import public key failed\n");
			ret = -2;
			__leave;
		}

		// calc hash for the file.
		hHash = calc_file_hash(filename,hProv, CALG_SHA1);
		if (hHash == 0) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: calc file hash failed\n");
			ret = -3;
			__leave;
		}

		// get file signature. (bytes)
		signature = GetFileContent_b(sigfile, &sig_size);
		if (signature == NULL) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Get signature content failed\n");
			ret = -4;
			__leave;
		}

		// invert signature (little endian to big endian).
		invert = (BYTE*)calloc(sig_size,sizeof(BYTE));
		j = 0;
		for (i = sig_size-1; i >= 0; i--) {
			invert[j] = signature[i] ;
			j++;
		}

		/*printf("\n");
		printf("[+] Debug :: verify_file_signature :: signature value = \n");
		print_hexa(signature, sig_size);
		printf("\n");
		printf("\n");

		printf("[+] Debug :: verify_file_signature :: invert value = \n");
		print_hexa(invert, sig_size);
		printf("\n");
		printf("\n");
		*/

		// verify signature
		if (CryptVerifySignature(hHash, invert, sig_size, hPubKey, NULL, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Crypt verify Signature failed. :: GLE = %d :: 0x%x\n",GetLastError(),GetLastError());
			ret = -9;
			__leave;
		}



	}
	__finally {

		if (signature != NULL) {
			free(signature);
			signature = NULL;
		}

		if (pubkeyfile != NULL) {
			free(pubkeyfile);
			pubkeyfile = NULL;
		}

		if (invert != NULL) {
			free(invert);
			invert = NULL;
		}		
		
		if(CryptDestroyKey(hPubKey) == FALSE){
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Crypt Destroy Key failed! :: GLE = 0x%x\n",GetLastError());
		}

		if (hHash != 0)
			CryptDestroyHash(hHash);

		if (CryptReleaseContext(hProv, 0) == FALSE) {
			a6o_log(ARMADITO_LOG_SERVICE, ARMADITO_LOG_LEVEL_ERROR,"[-] Error :: verify_file_signature :: Release crypt context failed. :: GLE = %d\n",GetLastError);			
		}

	}

	return ret;
}
