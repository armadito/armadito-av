#include "utils.h"
#include <sys/stat.h>

// Purpose:
//		This function compute the size of a file
//
// Parameters:
//		_in_ szPathFileName : the name to check
//
// Return:
//		the size of the file in a QWORD.
QWORD SizeOfFile(int fd)
{
	struct stat st;
	QWORD size = 0;

	fstat(fd, &st);
	size = st.st_size;
	return size;
}
