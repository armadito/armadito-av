#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <libuhuru\core.h>

#define QUARANTINE_DIR "C:\\Program Files\\Novit\\UhuruAV\\Quarantine"

int MoveFileInQuarantine(char * filepath);