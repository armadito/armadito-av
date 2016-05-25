/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module H1.

Armadito module H1 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module H1 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module H1.  If not, see <http://www.gnu.org/licenses/>.

***/

/**
 * @file   model.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows/dirent.h>
#else
#include <dirent.h>
#endif
#include <string.h>

#include "model.h"
#include "utils.h"
#include "miniz.h" /* for modelLoadFromZip */

PMODEL modelLoadFromZip(CHAR* zipName){
	mz_zip_archive zip_archive;
	mz_bool status;
	DWORD i = 0, j = 0, k = 0;
	PVOID p = NULL;
	QWORD* s = NULL;
	PMODEL modelArray = NULL;
	size_t uncomp_size;

	modelArray = (PMODEL)calloc(1, sizeof(MODEL));
	if (modelArray == NULL){
		return NULL;
	}

	memset(&zip_archive, 0, sizeof(zip_archive));

	status = mz_zip_reader_init_file(&zip_archive, zipName, 0);
	if (!status){
		printf("mz_zip_reader_init_file() failed for file %s ! This file might be empty.\n ", zipName );
		free(modelArray);
		return NULL;
	}

	modelArray->modelFiles = (PVECTOR*)calloc((DWORD)mz_zip_reader_get_num_files(&zip_archive), sizeof(PVECTOR));
	if (modelArray->modelFiles == NULL){
		free(modelArray);
		return NULL;
	}
	modelArray->numberOfFile = (DWORD)mz_zip_reader_get_num_files(&zip_archive);

	for (i = 0; i < (DWORD)mz_zip_reader_get_num_files(&zip_archive); i++){
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)){
			printf("mz_zip_reader_file_stat() failed for file %s !\n", zipName );
			mz_zip_reader_end(&zip_archive);
			for (j = 0; j < i; j++){
				vectorDelete(modelArray->modelFiles[j]);
			}
			free(modelArray);
			return NULL;
		}

		if (!mz_zip_reader_is_file_a_directory(&zip_archive, i)){
			p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncomp_size, 0);
			s = (QWORD*)p;
			if (!p){
				printf("mz_zip_reader_extract_file_to_heap() failed for file %s !\n", zipName );
				mz_zip_reader_end(&zip_archive);
				return NULL;
			}
			modelArray->modelFiles[i] = vectorNew((DWORD)(uncomp_size / 8));
			for (k = 0; k < uncomp_size / 8; k++){
				modelArray->modelFiles[i]->v[k] = s[k];
			}
			if (modelArray->modelFiles[i] == NULL){
				for (j = 0; j < i; j++){
					vectorDelete(modelArray->modelFiles[j]);
				}
				free(modelArray->modelFiles);
				free(modelArray);
				return NULL;
			}

			mz_free(p);
			p = NULL;
		}
	}

	mz_zip_reader_end(&zip_archive);

	return modelArray;
}

PMODEL modelLoad(CHAR* dirName){
	DWORD nbFilesInDir = 0, i = 0, j = 0;
	DIR* rep = NULL;
	PMODEL modelArray = NULL;
	CHAR fileName[NAMESIZE];
	struct dirent* file = NULL;

	modelArray = (PMODEL)calloc(1, sizeof(MODEL));
	if (modelArray == NULL){
		return NULL;
	}

	/* opening of the folder */
	rep = opendir(dirName);
	if (rep == NULL){
		free(modelArray);
		return NULL;
	}

	/* Count the number of files in the model */
	while ((file = readdir(rep)) != NULL){
		if (strcmp(file->d_name, "..") && strcmp(file->d_name, ".") && strcmp(file->d_name, ".directory")){ /* avoid files . and .. and .directory*/
			nbFilesInDir++;
		}
	}
	rewinddir(rep);

	/* init of the model array */
	modelArray->modelFiles = (PVECTOR*)calloc(nbFilesInDir, sizeof(PVECTOR));
	if (modelArray->modelFiles == NULL){
		free(modelArray);
		return NULL;
	}
	modelArray->numberOfFile = nbFilesInDir;

	/* Load all the files of the model in an array */
	while ((file = readdir(rep)) != NULL && i < nbFilesInDir){
		if (strcmp(file->d_name, "..") && strcmp(file->d_name, ".") && strcmp(file->d_name, ".directory")){
#ifndef _MSC_VER
			snprintf(fileName, strlen(dirName) + strlen(file->d_name), "%s/%s", dirName, file->d_name);
#else
			sprintf_s(fileName, strlen(dirName) + strlen(file->d_name), "%s/%s", dirName, file->d_name);
#endif
			/* we add each file read to the array */
			modelArray->modelFiles[i] = vectorLoad(fileName);
			if (modelArray->modelFiles[i] == NULL){
				for (j = 0; j < i; j++){
					vectorDelete(modelArray->modelFiles[j]);
				}
				free(modelArray);
				free(modelArray->modelFiles);
				return NULL;
			}

			i++;
		}
	}
	closedir(rep);
	return modelArray;
}

VOID modelDelete(PMODEL M){
	DWORD i = 0;
	if (M == NULL || M->modelFiles == NULL){
		return;
	}
	for (i = 0; i < M->numberOfFile; i++){
		vectorDelete(M->modelFiles[i]);
	}
	free(M->modelFiles);
	if (M == NULL){
		return;
	}
	free(M);
}
