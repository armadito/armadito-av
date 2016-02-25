#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <libuhuru\core.h>
#include <json.h>

#define QUARANTINE_DIR "C:\\Program Files\\Novit\\UhuruAV\\Quarantine"
#define _QUARANTINE_DIR "Quarantine"

int MoveFileInQuarantine(char * filepath);
int RestoreFileFromQuarantine(char* filename);