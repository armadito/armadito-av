#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <libuhuru\core.h>
#include <json.h>

#define QUARANTINE_DIR "C:\\Program Files\\Novit\\UhuruAV\\Quarantine"
#define _QUARANTINE_DIR "Quarantine"
#define ALERT_DIR "Alerts"

int MoveFileInQuarantine(char * filepath, struct uhuru_report uh_report);
int RestoreFileFromQuarantine(char* filename);
int EnumQuarantine( );
int ui_restore_quarantine_file(char * filename);