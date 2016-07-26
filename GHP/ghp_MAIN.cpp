#include "..\include.h"
#include "ghp_CORE.h"

bool running = false;

extern "C" __declspec (dllexport) void GHPRUN()
{
	if (!running)
	{
		running = true;

		// Inicia o GHP
		Start();
	}
}

extern "C" __declspec (dllexport) void GHPSTOP(DWORD APIKey)
{
	VMProtectBeginUltra("GHPSTOP");
	if (!ghpLic->IsLoaded())
		return;
	if (APIKey != ghp_CRYPTO::DecryptDWORD(ghpLic->GetAPIKey()))
	{
		Error(ERROR_INVALID_API_KEY, 2);
		return;
	}
	if (running)
	{
		// Fecha o GHP
		Stop();
	}
	VMProtectEnd();
}

extern "C" __declspec (dllexport) void GHPSetParam(DWORD APIKey, UINT id, UCHAR * buffer, UINT size)
{
	VMProtectBeginUltra("GHPSetParam");
	if (!ghpLic->IsLoaded())
		return;
	if (APIKey != ghp_CRYPTO::DecryptDWORD(ghpLic->GetAPIKey()))
	{
		Error(ERROR_INVALID_API_KEY, 0);
		return;
	}

	switch (id)
	{
	case 0: // SET LOGIN
		GameLogin = std::string((char*)buffer, strlen((char*)buffer));
		break;
	case 1: // SET D3DDEVICE
#ifdef R_WARZ
		d3ddev = (LPDIRECT3DDEVICE9)buffer;
#endif
		break;
	default:
		break;
	}
	VMProtectEnd();
}

extern "C" __declspec (dllexport) void GHPGetParam(DWORD APIKey, UINT id, UCHAR * buffer, UINT size)
{
	VMProtectBeginUltra("GHPGetParam");
	if (!ghpLic->IsLoaded())
		return;
	if (APIKey != ghp_CRYPTO::DecryptDWORD(ghpLic->GetAPIKey()))
	{
		Error(ERROR_INVALID_API_KEY, 1);
		return;
	}
	
	switch (id)
	{
	case 0: // GET GHP TIME
		{
			DWORD time = timeGetTime();

			time += ghp_CRYPTO::DecryptDWORD(ghpLic->GetTime2());
			time ^= ghp_CRYPTO::DecryptDWORD(ghpLic->GetTime1());

			memcpy_s(buffer, size, &time, sizeof(time));
		}
		break;
	default:
		break;
	}
	VMProtectEnd();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	VMProtectBeginUltra("DllMain");
	loadlock = true;
	
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			mainThread = GetCurrentThreadId();
			GHPRUN();
		}
		break;
	}
	
	loadlock = false;
	VMProtectEnd();
	return TRUE;
}