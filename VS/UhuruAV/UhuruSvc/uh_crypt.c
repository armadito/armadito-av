#include "uh_crypt.h"


BYTE * _GetFileContent(char * filename, int * retsize) {

	BYTE * content = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER fileSize = {0};
	int size = 0, read = 0, ret = 0;

	if (filename == NULL || retsize == NULL) {
		printf("[-] Error :: GetFileContent :: Invalid parameter\n");
		return NULL;
	}

	__try {

		hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("[-] Error :: GetFileContent :: Opening the file failed! :: error = %d\n",GetLastError());
			ret = -1;
			__leave;
		}

		if (GetFileSizeEx(hFile, &fileSize) == FALSE) {
			printf("[-] Error :: GetFileContent :: Get file size failed! :: error = %d\n",GetLastError());
			ret = -2;
			__leave;
		}

		size = fileSize.QuadPart;
		*retsize = size;

		//printf("[+] Debug :: GetFileContent :: file size = %d\n",size);

		content = (char*)calloc(size+1,sizeof(char));
		content[size]='\0';

		if (ReadFile(hFile, content, size, &read, NULL) == FALSE) {
			printf("[-] Error :: GetFileContent :: Read file content failed! :: error = %d\n",GetLastError());
			ret = -4;
			__leave;
		}

	}
	__finally {

		
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			hFile = NULL;
		}

		if (content != NULL && ret < 0) {
			free(content);
			content = NULL;
		}

	}

	return content;
}


void print_hexa(BYTE* data, int size) {

	int i = 0;
	if (data == NULL || size <= 0)
		return;

	for(i = 0 ; i < size ; i++){
        printf("%02x ",data[i]);
    }

	return;
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
		printf("[-] Error :: GetFileHash :: Invalid parameters!\n");
		return NULL ; 
	}


	__try {

		// Acquire a handle to a particular key container within a particular cryptographic service provider.
		if (CryptAcquireContextA(&hProv,NULL, NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE) {
			printf("[-] Error :: GetFileHash :: Acquire crypt context failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		if (CryptCreateHash(hProv, algo, 0, 0, &hHash) == FALSE) {
			printf("[-] Error :: GetFileHash :: Create hash failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		if (CryptHashData(hHash, data, len, 0) == FALSE) {
			printf("[-] Error :: GetFileHash :: Crypt hash data failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		// determine the size of the hash value
		if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &hashsize, 0) == FALSE) {
			printf("[-] Error :: GetFileHash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		//printf("[+] Debug :: GetFileHash :: Hash size = %d\n",hashsize);

		hashval = (BYTE*)calloc(hashsize + 1, sizeof(BYTE));
		hashval[hashsize] = '\0';

		if (CryptGetHashParam(hHash, HP_HASHVAL, hashval, &hashsize, 0) == FALSE) {
			printf("[-] Error :: GetFileHash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			ret = -1;
			__leave;
		}


		/*printf("[+] Debug :: GetFileHash :: Hash value = ");
		print_hexa(hashval, hashsize);
		printf("\n");*/


	}
	__finally {

		if (CryptReleaseContext(hProv, 0) == FALSE) {
			printf("[-] Error :: GetFileHash :: Release crypt context failed. :: GLE = %d\n",GetLastError);
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
		printf("[-] Error :: import_public_key :: invalid parameters\n");
		return 0;
	}

	__try {
		
		// import public key from file.

		content = _GetFileContent(filename,&size);
		if (content == NULL) {
			printf("[-] Error :: uh_verify_file_signature :: Get public key content failed\n");
			__leave;
		}

		//printf("[+] Debug :: import_public_key :: pksize = %d :: content = \n%s\n\n", size,content);


		if (CryptStringToBinaryA(content, size, CRYPT_STRING_BASE64_ANY, NULL, &buf_size, 0, NULL) == FALSE) {
			printf("[-] Error :: import_public_key :: Crypt String to Binary failed :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		//printf("[+] Debug :: import_public_key :: public key buf size = %d bytes\n", buf_size);

		buf = (BYTE*)calloc(buf_size,sizeof(BYTE));

		if (CryptStringToBinaryA(content, size, CRYPT_STRING_BASE64_ANY, buf, &buf_size, NULL, NULL) == FALSE) {
			printf("[-] Error :: import_public_key :: Crypt String to Binary failed :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		printf("\n");
		printf("[+] Debug :: import_public_key :: Public key value = \n");
		print_hexa(buf, buf_size);
		printf("\n");
		printf("\n");


		if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buf, buf_size,0, NULL, NULL, &cbPubKeyBlob) == FALSE) {
			printf("[-] Error :: import_public_key :: Crypt Decode public key Object failed 1 :: GLE = 0x%x\n",GetLastError());
			__leave;
		}
		
		//printf("[+] Debug :: import_public_key :: public key Blob size = %d bytes\n",cbPubKeyBlob);

		pubKeyBlob = (BYTE*)calloc(cbPubKeyBlob ,sizeof(BYTE));

		if (CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buf, buf_size,0, NULL, pubKeyBlob, &cbPubKeyBlob) == FALSE) {
			printf("[-] Error :: import_public_key :: Crypt Decode public key Object failed 2 :: GLE = 0x%x\n",GetLastError());
			__leave;
		}

		if (!CryptImportKey(hProv, pubKeyBlob, cbPubKeyBlob, 0, 0, &hPubKey)){
			printf("[-] Error :: import_public_key :: CryptImportKey for public key failed :: GLE = 0x%x\n", GetLastError());
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


		content = _GetFileContent(filename, &size);
		if (content == NULL) {
			printf("[-] Error :: calc_file_hash :: Get file content failed\n");
			__leave;
		}

		if (CryptCreateHash(hProv, algo, 0, 0, &hHash) == FALSE) {
			printf("[-] Error :: calc_file_hash :: Create hash failed. :: GLE = 0x%x\n", GetLastError());
			__leave;
		}

		if (CryptHashData(hHash, content, size, 0) == FALSE) {
			printf("[-] Error :: calc_file_hash :: Crypt hash data failed. :: GLE = %d\n",GetLastError);			
			__leave;
		}

		// display file hash (for debug only).
		if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &hashsize, 0) == FALSE) {
			printf("[-] Error :: calc_file_hash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
			__leave;
		}

		hashval = (BYTE*)calloc(hashsize + 1, sizeof(BYTE));
		hashval[hashsize] = '\0';

		if (CryptGetHashParam(hHash, HP_HASHVAL, hashval, &hashsize, 0) == FALSE) {
			printf("[-] Error :: calc_file_hash :: Crypt Get hash Param failed. :: GLE = %d\n",GetLastError);
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
uh_verify_file_signature
This function verify the signature of a file with public key.
returns 0 on success or a value less than zero on error.
*/
int verify_file_signature(char *filename, char *sigfile){

	int ret = 0;
	BYTE * content = NULL, * signature = NULL, * invert = NULL;

	HCRYPTKEY hPubKey = 0;
	HCRYPTHASH hHash = 0;
	HCRYPTPROV hProv = 0;
	int size = 0, i = 0, j =0, sig_size = 0;
	int fsize = 0;

	if (filename == NULL || sigfile == NULL) {
		printf("[-] Error :: uh_verify_file_signature :: invalid parameters\n");
		return -1;
	}

	__try {

		// Acquire crypt context
		if (CryptAcquireContextA(&hProv,NULL, MS_ENHANCED_PROV,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE) {
			printf("[-] Error :: import_public_key :: Acquire crypt context failed. :: GLE = 0x%x\n",GetLastError());
			__leave;
		}
		

		// import public key from file.
		hPubKey = import_public_key(UH_PUBKEY,hProv);
		if (hPubKey == 0) {
			printf("[-] Error :: uh_verify_file_signature :: import public key failed\n");
			ret = -2;
			__leave;
		}

		// calc hash for the file.
		hHash = calc_file_hash(filename,hProv, CALG_SHA1);
		if (hHash == 0) {
			printf("[-] Error :: uh_verify_file_signature :: calc file hash failed\n");
			ret = -3;
			__leave;
		}

		// get file signature.
		signature = _GetFileContent(sigfile, &sig_size);
		if (signature == NULL) {
			printf("[-] Error :: uh_verify_file_signature :: Get signature content failed\n");
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
		printf("[+] Debug :: uh_verify_file_signature :: signature value = \n");
		print_hexa(signature, sig_size);
		printf("\n");
		printf("\n");

		printf("[+] Debug :: uh_verify_file_signature :: invert value = \n");
		print_hexa(invert, sig_size);
		printf("\n");
		printf("\n");
		*/

		// verify signature
		if (CryptVerifySignature(hHash, invert, sig_size, hPubKey, NULL, 0) == FALSE) {
			printf("[-] Error :: uh_verify_file_signature :: Crypt verify Signature failed. :: GLE = %d :: 0x%x\n",GetLastError(),GetLastError());
			ret = -9;
			__leave;
		}


		
		


	}
	__finally {

		if (signature != NULL) {
			free(signature);
			signature = NULL;
		}

		if (invert != NULL) {
			free(invert);
			invert = NULL;
		}		
		
		if(CryptDestroyKey(hPubKey) == FALSE){
			printf("[-] Error :: uh_verify_file_signature :: Crypt Destroy Key failed! :: GLE = 0x%x\n",GetLastError());
		}

		if (hHash != 0)
			CryptDestroyHash(hHash);

		if (CryptReleaseContext(hProv, 0) == FALSE) {
			printf("[-] Error :: uh_verify_file_signature :: Release crypt context failed. :: GLE = %d\n",GetLastError);			
		}

	}

	return ret;
}