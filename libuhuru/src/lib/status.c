#include "statusp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int uhuru_file_status_cmp(enum uhuru_file_status s1, enum uhuru_file_status s2)
{
  if (s1 == s2)
    return 0;

  switch(s1) {
  case UHURU_UNDECIDED:
    return -1;
  case UHURU_CLEAN:
    return (s2 == UHURU_UNDECIDED) ? 1 : -1;
  case UHURU_IERROR:
    return (s2 == UHURU_UNDECIDED || s2 == UHURU_CLEAN) ? 1 : -1;
  case UHURU_SUSPICIOUS:
    return (s2 == UHURU_UNDECIDED || s2 == UHURU_CLEAN || s2 == UHURU_IERROR) ? 1 : -1;
  case UHURU_WHITE_LISTED:
    return (s2 == UHURU_UNDECIDED || s2 == UHURU_CLEAN || s2 == UHURU_IERROR || s2 == UHURU_SUSPICIOUS) ? 1 : -1;
  case UHURU_MALWARE:
    return 1;
  }

  assert(1 == 0);

  return 0;
}

const char *uhuru_file_status_str(enum uhuru_file_status status)
{
  switch(status) {
#undef M
#define M(S) case S: return #S
    M(UHURU_UNDECIDED);
    M(UHURU_CLEAN);
    M(UHURU_UNKNOWN_FILE_TYPE);
    M(UHURU_EINVAL);
    M(UHURU_IERROR);
    M(UHURU_SUSPICIOUS);
    M(UHURU_WHITE_LISTED);
    M(UHURU_MALWARE);
  }

  return "UNKNOWN STATUS";
}

const char *uhuru_file_status_pretty_str(enum uhuru_file_status status)
{
  switch(status) {
  case UHURU_UNDECIDED:
    return "undecided";
  case UHURU_CLEAN:
    return "clean";
  case UHURU_UNKNOWN_FILE_TYPE:
    return "ignored";
  case UHURU_EINVAL:
    return "invalid argument";
  case UHURU_IERROR:
    return "internal error";
  case UHURU_SUSPICIOUS:
    return "suspicious";
  case UHURU_WHITE_LISTED:
    return "white listed";
  case UHURU_MALWARE:
    return "malware";
  }

  return "unknown status";
}

