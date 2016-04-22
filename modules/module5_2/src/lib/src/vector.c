#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "osdeps.h"

#ifdef _WIN32
#include <windows/dirent.h>
#else
#include <dirent.h>
#endif


PVECTOR vectorNew(DWORD n){
	PVECTOR x = NULL;
	/* what is the point of having a vector of size 0? */
	if (n == 0){
		return NULL;
	}

	x = (PVECTOR)calloc(1, sizeof(VECTOR));
	if (x == NULL){
		return NULL;
	}

	x->v = (VEC_TYPE*)calloc(n, sizeof(VEC_TYPE));
	if (x->v == NULL){
		free(x);
		return NULL;
	}
	x->n = n;
	return x;
}

PVECTOR VectorCreateFromArray(VEC_TYPE* QwordArray, DWORD n){
	PVECTOR x = NULL;
	/* if QwordArray, why not use vectorNew? */
	if (QwordArray == NULL || n == 0){
		return NULL;
	}

	x = (PVECTOR)calloc(1, sizeof(VECTOR));
	if (x == NULL){
		return NULL;
	}

	x->v = QwordArray;

	x->n = n;

	return x;
}

DWORD vectorGetSize(PVECTOR v){
	if (v == NULL){
		return (DWORD)-1;
	}

	return v->n;
}

VEC_TYPE vectorGetValue(PVECTOR v, DWORD i){
	if (v == NULL || i >= v->n){
		return (VEC_TYPE)-1;
	}

	return v->v[i];
}

VEC_TYPE vectorGetValueWithPaddedVector(PVECTOR v, DWORD i){
	if (i >= v->n){
		return 0;
	}
	if (v == NULL){
		return (VEC_TYPE)-1;
	}

	return v->v[i];
}

VOID vectorDelete(PVECTOR x){
	if (x == NULL || x->v == NULL){
		return;
	}
	free(x->v);
	x->v = NULL;

	if (x == NULL){
		return;
	}
	free(x);
	x = NULL;
}

PVECTOR vectorLoad(CHAR* fileName){
	PVECTOR content = NULL;
	DWORD currentSize;
	FILE* fd = NULL;

	/*file reading*/
	fd = os_fopen(fileName, "rb");
	if (fd == NULL){
		return NULL;
	}
	else{
		/*file size calculation*/
		fseek(fd, 0L, SEEK_END);
		currentSize = ftell(fd);
		rewind(fd);

		content = vectorNew(currentSize / sizeof(VEC_TYPE));
		if (content == NULL){
			fclose(fd);
			return NULL;
		}

		/*content reading*/
		if (fread(content->v, 1, currentSize, fd) != currentSize) {
			vectorDelete(content);
			fclose(fd);
			return NULL;
		}

		fclose(fd);
	}

	return content;
}