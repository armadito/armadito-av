#include "libarmadito-config.h"

#include "statusp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int a6o_file_status_cmp(enum a6o_file_status s1, enum a6o_file_status s2)
{
	if (s1 == s2)
		return 0;

	switch(s1) {
	case ARMADITO_UNDECIDED:
		return -1;
	case ARMADITO_CLEAN:
		return (s2 == ARMADITO_UNDECIDED) ? 1 : -1;
	case ARMADITO_IERROR:
		return (s2 == ARMADITO_UNDECIDED || s2 == ARMADITO_CLEAN) ? 1 : -1;
	case ARMADITO_SUSPICIOUS:
		return (s2 == ARMADITO_UNDECIDED || s2 == ARMADITO_CLEAN || s2 == ARMADITO_IERROR) ? 1 : -1;
	case ARMADITO_WHITE_LISTED:
		return (s2 == ARMADITO_UNDECIDED || s2 == ARMADITO_CLEAN || s2 == ARMADITO_IERROR || s2 == ARMADITO_SUSPICIOUS) ? 1 : -1;
	case ARMADITO_MALWARE:
		return 1;
	}

	assert(1 == 0);

	return 0;
}

const char *a6o_file_status_str(enum a6o_file_status status)
{
	switch(status) {
#undef M
#define M(S) case S: return #S
		M(ARMADITO_UNDECIDED);
		M(ARMADITO_CLEAN);
		M(ARMADITO_UNKNOWN_FILE_TYPE);
		M(ARMADITO_EINVAL);
		M(ARMADITO_IERROR);
		M(ARMADITO_SUSPICIOUS);
		M(ARMADITO_WHITE_LISTED);
		M(ARMADITO_MALWARE);
	}

	return "UNKNOWN STATUS";
}

const char *a6o_file_status_pretty_str(enum a6o_file_status status)
{
	switch(status) {
	case ARMADITO_UNDECIDED:
		return "undecided";
	case ARMADITO_CLEAN:
		return "clean";
	case ARMADITO_UNKNOWN_FILE_TYPE:
		return "ignored";
	case ARMADITO_EINVAL:
		return "invalid argument";
	case ARMADITO_IERROR:
		return "internal error";
	case ARMADITO_SUSPICIOUS:
		return "suspicious";
	case ARMADITO_WHITE_LISTED:
		return "white listed";
	case ARMADITO_MALWARE:
		return "malware";
	}

	return "unknown status";
}

