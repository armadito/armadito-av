#include "register.h"
//#include <Wscapi.h>
//#include <comdef.h>
#include <WbemIdl.h>



/*
	This functions registers the av to Windows Sercurity Center.
	src:
		Windows Security Center.
		https://msdn.microsoft.com/en-us/library/gg537273%28v=vs.85%29.aspx
		Windows Management Instrumentation
		 https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa394582%28v=vs.85%29.aspx
		WMI Providers
		 https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa394570%28v=vs.85%29.aspx
		 (choose security provider).

		 IWbem* COM api

		 Register in WMI via the AntivirusProduct class.
		 Root\SecurityCenter or root\securityCenter2 namespaces depending upon the OS version.

		 Get WmiObject 

*/
int register_av( ) {

	int ret = 0;
	HRESULT hres = S_OK;
	PSECURITY_DESCRIPTOR sd = NULL;
	IWbemLocator *pLoc = 0;

	__try {

		// Initialize COM. (WMI Component Object Model)
		hres = CoInitializeEx(NULL,COINIT_MULTITHREADED);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: CoInitializeEx failed!\n");
			__leave;
		}

		hres = CoInitializeSecurity(sd, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: CoInitializeSecurity failed!\n");
			__leave;
		}

		printf("[+] Debug :: register_av :: WMI COM initialized successfully!\n");

		// Create connection to WMI namespace.
		//hres = CoCreateInstance(&CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, &IID_IWbemLocator, (LPVOID)&pLoc);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: CoCreateInstance failed!\n");
			__leave;
		}


	}
	__finally {

		// Un-Initialize COM.
		CoUninitialize();

	}


	return ret;
}