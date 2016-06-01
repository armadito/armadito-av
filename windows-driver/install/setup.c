/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito windows driver.

Reproduction, distribution and derivative works are permitted under the terms of the Microsoft Public License
See file COPYING.MSPL for terms of license.

***/

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <Msi.h>
#include <process.h>
#include <SetupAPI.h>

#define LOG_FILE "C:\\armadito-install.log"
#define INF_FILE "\\ArmaditoGuard.inf"

int gfirst_time = 0;

// TODO :: add timestamp.
void a6oLogFile(const char *fmt, ...)
{
	
	FILE * f = NULL;
	const char *mode = "a";
	errno_t err;
	va_list ap;


	if (gfirst_time) {
		mode = "w";
		gfirst_time = 0;
	}
	err = fopen_s(&f, LOG_FILE, mode);
	if (err != 0)
		return;

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);
	fclose(f);

	return;
}

char * GetBinaryDirectory( ) {

	char * dirpath = NULL;
	char filepath[MAX_PATH];
	char * ptr = NULL;
	int len = 0;

	if (!GetModuleFileNameA(NULL, (LPSTR)&filepath, MAX_PATH)) {		
		a6oLogFile("[-] Error :: GetBinaryDirectory!GetModuleFileName() failed :: %d\n", GetLastError());
		return NULL;
	}

	// get the file name from the complete file path
	ptr = strrchr(filepath,'\\');
	if (ptr == NULL) {		
		a6oLogFile("[-] Error :: GetBinaryDirectory!strrchr() failed :: backslash not found in the path :: %s.\n", filepath);
		return NULL;
	}
	
	// calc the dir buffer length.
	len = (int)(ptr - filepath);

	dirpath = (char*)(calloc(len+1,sizeof(char)));
	dirpath[len] = '\0';

	memcpy_s(dirpath, len, filepath, len);

	return dirpath;
}

char * BuildInfCmd(char * pre_command ) {

	char * infpath = NULL;
	char * binpath = NULL;
	char * cmd = NULL;


	int pre_len = 0, post_len = 0;
	int binpath_len = 0;
	int len = 0;
	int cmd_len = 0;

	__try {

		binpath = GetBinaryDirectory();
		if (binpath == NULL) {
			__leave;
		}
		//printf("[+] Debug :: binpath = %s\n", binpath);
		binpath_len = strnlen_s(binpath,MAX_PATH);		

		// DefaultInstall 132 [inf_file_path]	
		// DefaultUninstall 132 [inf_file_path]

		pre_len = strnlen_s(pre_command,MAX_PATH);
		post_len = strnlen_s(INF_FILE,MAX_PATH);		
		cmd_len = pre_len + binpath_len + post_len +1 ;
		//printf("[+] Debug :: binpath len = %d\n", len);
		cmd = (char*)calloc(cmd_len+1,sizeof(char));
		cmd[cmd_len] = '\0';		
		strncat_s(cmd,cmd_len,pre_command,pre_len);
		strncat_s(cmd,cmd_len,binpath,binpath_len);
		strncat_s(cmd,cmd_len,INF_FILE,post_len);
		printf("[+] Debug :: cmd = %s\n", cmd);
		

	}
	__finally {

		if (binpath != NULL) {
			free(binpath);
			binpath = NULL;
		}

	}
	


	return cmd;
}

int install( ) {

	int ret = 0;
	LPCWSTR w_cmd = NULL;
	size_t count = 0;
	char * cmd= NULL;
	PVOID OldValue = NULL;

	cmd = BuildInfCmd("DefaultInstall 132 ");
	if (cmd == NULL){
		return -1;
	}

	// convert ascii string to wide char string.
	w_cmd = (WCHAR*)calloc(MAX_PATH,sizeof(WCHAR));
	mbstowcs_s(&count, w_cmd,MAX_PATH, cmd,MAX_PATH);

	//wprintf(L"[i] Debug :: install ::  wide string =  %ls\n",w_cmd);

	// https://msdn.microsoft.com/en-us/library/aa384187.aspx
	// https://msdn.microsoft.com/en-us/library/aa365743.aspx
	// Disables file system redirection for the calling thread.
	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {		
		a6oLogFile("[-] Error :: Disable Wow64 file system redirection failed !\n");
		return -2;
	}

	a6oLogFile("[+] Debug :: install :: installing the driver...\n");
	InstallHinfSectionW(NULL, NULL, w_cmd,0);
	a6oLogFile("[+] Debug :: install :: InstallHinfSection executed!\n");

	if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ){
        //  Failure to re-enable redirection should be considered
        //  a criticial failure and execution aborted.
		a6oLogFile("[-] Error :: Revert Wow64 file system redirection failed !\n");
        return -3;
    }

	free(cmd);

	return ret;
}

int uninstall( ) {

	int ret = 0;
	char * cmd = NULL;
	LPCWSTR w_cmd = NULL;
	size_t count = 0;
	PVOID OldValue = NULL;

	cmd = BuildInfCmd("DefaultUnInstall 132 ");
	if (cmd == NULL){
		return -1;
	}

	a6oLogFile("[i] Debug :: uninstall :: Uninstalling the driver...\n");

	w_cmd = (WCHAR*)calloc(MAX_PATH,sizeof(WCHAR));
	mbstowcs_s(&count, w_cmd,MAX_PATH, cmd,MAX_PATH);

	wprintf(L"[i] Debug :: uninstall ::  cmd =  %ls\n",w_cmd);

	if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
		a6oLogFile("[-] Error :: Disable Wow64 file system redirection failed !\n");
		return -1;
	}

	InstallHinfSectionW(NULL, NULL, w_cmd,0);

	if (Wow64RevertWow64FsRedirection(OldValue) == FALSE ){
        //  Failure to re-enable redirection should be considered
        //  a criticial failure and execution aborted.
		a6oLogFile("[-] Error :: Revert Wow64 file system redirection failed !\n");
        return -2;
    }

	a6oLogFile("[i] Debug :: uninstall :: InstallHinfSection executed!\n");


	free(cmd);

	return ret;
}

int start( ) {

	int ret = 0;
	system("sc start ArmaditoGuard");
	return ret;
}

int stop( ) {

	int ret = 0;
	system("sc stop ArmaditoGuard");
	return ret;
}

void Help( ) {
	printf("USAGE :: ArmaditoGuard-setup.exe --[install|start|stop|uninstall]\n" );
	return;
}

int main(int argc, char ** argv) {


	// check parameter.
	if (argc < 2) {
		a6oLogFile("[-] Error :: ArmaditoGuard-setup :: missing parameter\n");
		Help( );
		return ERROR_INSTALL_FAILURE;
	}

	if (strncmp(argv[1],"--install",9) == 0) {

		if (install( ) < 0) {			
			a6oLogFile("[-] Error :: ArmaditoGuard installation failed!\n");
			return ERROR_INSTALL_FAILURE;
		}

	}
	else if (strncmp(argv[1],"--start",7) == 0) {

		if (start( ) < 0) {			
			a6oLogFile("[-] Error :: ArmaditoGuard start failed!\n");
			return -1;
		}

	}
	else if (strncmp(argv[1],"--stop",6) == 0) {

		if (stop( ) < 0) {			
			a6oLogFile("[-] Error :: ArmaditoGuard stop failed!\n");
			return -1;
		}

	}
	else if (strncmp(argv[1],"--uninstall",11) == 0) {

		if (uninstall( ) < 0) {			
			a6oLogFile("[-] Error :: ArmaditoGuard uninstallation failed!\n");
			return -1;
		}

	}
	else {
		a6oLogFile("[-] Error :: ArmaditoGuard-setup :: Invalid parameter\n");
		Help( );
		return ERROR_INSTALL_FAILURE;
	}
	
	return 0;	
}