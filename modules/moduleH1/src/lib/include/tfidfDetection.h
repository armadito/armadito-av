/**
 * @file   tfidfDetection.h
 *
 * tfidf testing.
 */

#ifndef TFIDF_DETECTION_H
#define TFIDF_DETECTION_H

#include "vector.h"
#include "windowsTypes.h"
#include "h1-errors.h"

typedef struct _TFIDF_NODE {
	QWORD NumberId;
	DWORD OccursNumber;
} TFIDF_NODE, *PTFIDF_NODE;

/**
 * load a tfidf base from a file to a PTFIDF_NODE
 * @param  szFileName        the file to load
 * @param  TotalSizeDataBase a pointer to the size of the database loaded
 * @param  nbDocsInBase      a pointer to the number of documents field of the database loaded
 * @return                   the database or NULL
 */
PTFIDF_NODE loadTFIDFBases(CHAR *szFileName, PDWORD TotalSizeDataBase, PDWORD nbDocsInBase);

/**
 * test the iat of a file with the tfidf method
 * @param  testFile                   the iat of the file to test
 * @param  DBInf                      the malware base
 * @param  DBSain                     the not malware base
 * @param  TotalSizeDataBaseTFIDFInf  the size of the malware base
 * @param  TotalSizeDataBaseTFIDFSain the size of the not malware base
 * @param  nbDocsTFIDFInf             the number of docs used to create the malware base
 * @param  nbDocsTFIDFSain            the number of docs used to create the not malware base
 * @return                            UH_NOT_MALWARE, UH_MALWARE, E_TEST_ERROR or UH_TFIDF_UNKNOWN
 */
ERROR_CODE tfidfTest(PVECTOR testFile,
	PTFIDF_NODE DBInf,
	PTFIDF_NODE DBSain,
	DWORD TotalSizeDataBaseTFIDFInf,
	DWORD TotalSizeDataBaseTFIDFSain,
	DWORD nbDocsTFIDFInf,
	DWORD nbDocsTFIDFSain);

#endif /* TFIDF_DETECTION_H */
