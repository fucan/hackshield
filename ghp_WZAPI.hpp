#ifndef _GHP_WZAPI_HPP_
#define _GHP_WZAPI_HPP_

#include <Windows.h>

#ifdef WO_SERVER
#include "CkHttp.h"
#include "CkHttpResponse.h"
#include "CkHttpRequest.h"
#endif

typedef VOID (* GHPSetParam) (DWORD APIKey, UINT id, UCHAR * buffer, UINT size);
typedef VOID (* GHPGetParam) (DWORD APIKey, UINT id, UCHAR * buffer, UINT size);

inline bool GHP_Load()
{
	if (LoadLibraryA("GHP.dll") != NULL)
		return true;
	else
		return false;
}

inline bool GHP_SetCustomerID(DWORD APIKey, DWORD Time1, DWORD Time2, DWORD CustomerID)
{
	HMODULE hGHP = GetModuleHandleA("GHP.dll");
	if(hGHP != NULL)
	{
		GHPSetParam SetParam = (GHPSetParam)GetProcAddress(hGHP, "GHPSetParam");
		GHPGetParam GetParam = (GHPGetParam)GetProcAddress(hGHP, "GHPGetParam");
		
		if (SetParam != NULL && GetParam != NULL)
		{
			DWORD MyTime, GHPTime, diffTime;
			
			MyTime = timeGetTime();
			GetParam(APIKey, 0, (UCHAR*)&GHPTime, sizeof(GHPTime));
			
			GHPTime ^= Time1;
			GHPTime -= Time2;
			
			diffTime = GHPTime - MyTime;
			
			if (diffTime > 2000)
				return false;
			
			char buff[32];
			memset(buff, 0, 32);
			_itoa_s((int)CustomerID, buff, 10);
			SetParam(APIKey, 0, (UCHAR*)buff, 32);
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

#ifdef WO_SERVER
inline bool GHP_CheckPlayer(DWORD CustomerID, const char * SRV_IP, long SRV_PORT, const char * C_REF)
{
	char buff[32];
	sprintf_s(buff, 32, "%d", CustomerID);
		
	CkHttp http;
	CkHttpResponse * resp = NULL;
	CkHttpRequest req;
	req.AddParam("pl_status", buff);
	req.AddParam("key", C_REF);

	resp = http.SynchronousRequest(SRV_IP, SRV_PORT, false, req);

	if (strcmp(resp->bodyStr(), "1") != 0)
		return false;
	else
		return true;
}
#endif

#endif