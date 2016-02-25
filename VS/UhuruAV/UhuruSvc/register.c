#include "register.h"
//#include <Wscapi.h>
//#include <comdef.h>
#include <WbemIdl.h>

char * hres2String( HRESULT hres) {

	switch (hres) {
		case WBEM_E_ACCESS_DENIED:
			return "WBEM_E_ACCESS_DENIED";
			break;
		case WBEM_E_FAILED:
			return "WBEM_E_FAILED";
			break;
		case WBEM_E_INVALID_NAMESPACE:
			return "WBEM_E_INVALID_NAMESPACE";
			break;
		case WBEM_E_INVALID_PARAMETER:
			return "WBEM_E_INVALID_PARAMETER";
			break;
		case WBEM_E_OUT_OF_MEMORY:
			return "WBEM_E_OUT_OF_MEMORY";
			break;
		case WBEM_E_TRANSPORT_FAILURE:
			return "WBEM_E_TRANSPORT_FAILURE";
			break;
		case WBEM_E_LOCAL_CREDENTIALS:
			return "WBEM_E_LOCAL_CREDENTIALS";
			break;
		case WBEM_E_INVALID_OBJECT_PATH:
			return "WBEM_E_INVALID_OBJECT_PATH";
			break;
		case WBEM_E_INVALID_CLASS:
			return "WBEM_E_INVALID_CLASS";
			break;
		case WBEM_E_SHUTTING_DOWN:
			return "WBEM_E_SHUTTING_DOWN";
			break;
		case WBEM_S_NO_ERROR:
			return "WBEM_S_NO_ERROR";
			break;
		case WBEM_E_NOT_FOUND:
			return "WBEM_E_NOT_FOUND";
			break;
		case WBEM_E_ILLEGAL_NULL:
			return "WBEM_E_ILLEGAL_NULL";
			break;
		case WBEM_E_ALREADY_EXISTS:
			return "WBEM_E_ALREADY_EXISTS";
			break;
		case WBEM_E_INVALID_OBJECT:
			return "WBEM_E_INVALID_OBJECT";
			break;
		default :
			return "ERROR_TO_DEFINE";
	}

	return NULL;

}

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
	IWbemLocator *pLoc = NULL;
	IWbemClassObjectVtbl *pALoc = NULL;
	IWbemServices *pSvc = NULL; 
	IWbemClassObject * pClas = NULL;
	VARIANT v = {0};

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
		hres = CoCreateInstance(&CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, &IID_IWbemLocator, (LPVOID *)&pLoc);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: CoCreateInstance failed!\n");
			__leave;
		}
		

		// Connect to the WMI		
		hres = pLoc->lpVtbl->ConnectServer(pLoc, L"ROOT\\SECURITYCENTER2",NULL, NULL, 0,NULL,0,0,&pSvc);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: ConnectServer failed! :: GLE = 0x%x :: %s\n",hres,hres2String(hres));
			__leave;
		}

		printf("[+] Debug :: register_av :: Connection to WMI created successfully!\n");

		// Setting the security levels on a WMI Connection.
		hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: CoSetProxyBlanket failed! :: GLE = 0x%x :: %s\n",hres,hres2String(hres));
			__leave;
		}

		printf("[+] Debug :: register_av :: Security level set successfully!\n");

		// https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa393946%28v=vs.85%29.aspx
		// Get AntivirusProduct class Object
		hres = pSvc->lpVtbl->GetObjectA(pSvc, L"AntivirusProduct", WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pClas, NULL);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: Get object [AntivirusProduct] failed! :: GLE = 0x%x :: %s\n",hres,hres2String(hres));
			__leave;
		}

		printf("[+] Debug :: register_av :: Get [AntivirusProduct] object path successfully!\n");

		// get the class value.
		hres = pClas->lpVtbl->Get(pClas, L"__CLASS", 0, &v, 0, 0);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: Get [AntivirusProduct] class properties failed! :: GLE = 0x%x :: %s\n",hres,hres2String(hres));
			__leave;
		}

		if (V_VT(&v) == VT_BSTR ) {

			wprintf(L"[+] Debug :: register_av :: The class name is :: %s\n",V_BSTR(&v) );

		}

		// Create a new instance from the current class.

		//pClas->lpVtbl->
		
		// modify instance
		//pClas->lpVtbl->Put( );

		
		/*hres = pSvc->lpVtbl->PutInstance(pSvc, pObj, WBEM_FLAG_UPDATE_ONLY, NULL, NULL);
		if (FAILED(hres)) {
			printf("[-] Error :: register_av :: Put/Modify instance [AntivirusProduct] failed! :: GLE = 0x%x :: %s\n",hres,hres2String(hres));
			__leave;
		}

		printf("[+] Debug :: register_av :: Put/Modify [AntivirusProduct] instance successfully!\n");
		*/

		// https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa392115%28v=vs.85%29.aspx

		// Create instance on AntivirusProduct class
		/*
			// Security center 1
			class AntiVirusProduct
			{
				string  companyName;              // Vendor name
				string  displayName;              // Application name
				string  instanceGuid;             // Unique identifier
				boolean onAccessScanningEnabled;  // Real-time protection
				boolean productUptoDate;          // Definition state
				string  versionNumber;            // Application version
			}

			// Security center 2
			class AntiVirusProduct
			{
				string  displayName;              // Application name
				string  instanceGuid;             // Unique identifier
				string  pathToSignedProductExe;   // Path to application
				string  pathToSignedReportingExe; // Path to provider
				uint32  productState;             // Real-time protection & defintion state
			}

		*/

		//pLoc->lpVtbl->

	}
	__finally {

		// release
		pSvc->lpVtbl->Release(pSvc);
		pLoc->lpVtbl->Release(pLoc);
		

		// Un-Initialize COM.
		CoUninitialize();

		VariantClear(&v);

		

	}


	return ret;
}