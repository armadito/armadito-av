#include "statusp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int umwsu_file_status_cmp(enum umwsu_file_status s1, enum umwsu_file_status s2)
{
  if (s1 == s2)
    return 0;

  switch(s1) {
  case UMWSU_UNDECIDED:
    return -1;
  case UMWSU_CLEAN:
    return (s2 == UMWSU_UNDECIDED) ? 1 : -1;
  case UMWSU_IERROR:
    return (s2 == UMWSU_UNDECIDED || s2 == UMWSU_CLEAN) ? 1 : -1;
  case UMWSU_SUSPICIOUS:
    return (s2 == UMWSU_UNDECIDED || s2 == UMWSU_CLEAN || s2 == UMWSU_IERROR) ? 1 : -1;
  case UMWSU_WHITE_LISTED:
    return (s2 == UMWSU_UNDECIDED || s2 == UMWSU_CLEAN || s2 == UMWSU_IERROR || s2 == UMWSU_SUSPICIOUS) ? 1 : -1;
  case UMWSU_MALWARE:
    return 1;
  }

  assert(1 == 0);

  return 0;
}

const char *umwsu_file_status_str(enum umwsu_file_status status)
{
  switch(status) {
#undef M
#define M(S) case S: return #S
    M(UMWSU_UNDECIDED);
    M(UMWSU_CLEAN);
    M(UMWSU_UNKNOWN_FILE_TYPE);
    M(UMWSU_EINVAL);
    M(UMWSU_IERROR);
    M(UMWSU_SUSPICIOUS);
    M(UMWSU_WHITE_LISTED);
    M(UMWSU_MALWARE);
  }

  return "UNKNOWN STATUS";
}

const char *umwsu_file_status_pretty_str(enum umwsu_file_status status)
{
  switch(status) {
  case UMWSU_UNDECIDED:
    return "undecided";
  case UMWSU_CLEAN:
    return "clean";
  case UMWSU_UNKNOWN_FILE_TYPE:
    return "ignored";
  case UMWSU_EINVAL:
    return "invalid argument";
  case UMWSU_IERROR:
    return "internal error";
  case UMWSU_SUSPICIOUS:
    return "suspicious";
  case UMWSU_WHITE_LISTED:
    return "white listed";
  case UMWSU_MALWARE:
    return "malware";
  }

  return "unknown status";
}

