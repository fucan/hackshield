#include "ghp_HOOK.h"
#include "ghp_CORE.h"
#include "ghp_NETBOX.h"
#include "..\ghp_UTILS\ghp_MU.h"

#include "Detours\detours.h"

typedef NTSTATUS(NTAPI * LdrLoadDll)
(
	PWCHAR				PathToFile,
	ULONG				Flags,
	PUNICODE_STRING		ModuleFileName,
	PHANDLE				ModuleHandle 
);

LdrLoadDll ori_LdrLoadDll;
DWORD	(WINAPI * ori_timeGetTime)				() = timeGetTime;
int		(WINAPI * ori_send)						(SOCKET s, const char * buf, int len, int flags) = send;
int		(WINAPI * ori_recv)						(SOCKET s, char * buf, int len, int flags) = recv;
HANDLE	(WINAPI * ori_CreateThread)				(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) = CreateThread;
VOID	(WINAPI * ori_ExitProcess)				(UINT uExitCode) = ExitProcess;

DWORD lastCheck = 0;
std::vector<DWORD> threads;

void CheckGHP(DWORD time)
{
	/*VMProtectBeginMutation("CheckGHP");
	DWORD now = time;

	if (now - lastCheck > 2500)
	{
		lastCheck = now;
		// Resume a thread, mesmo que já esteja rodando
		ResumeThread(hGHP);
		
		// Verifica a thread do GHP
		if (now - lastLoop > 7500)
			Error(ERROR_STOPPED_WORKING, 0);

		// Verifica o NETBox
		if (now - lastPing > 30000)
			Error(ERROR_STOPPED_WORKING, 1);
	}
	VMProtectEnd();*/
}

NTSTATUS NTAPI ghp_LdrLoadDll(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle)
{
	VMProtectBeginMutation("ghp_LdrLoadDll");
	CheckGHP(ori_timeGetTime());

	// Get names
	std::wstring wFile = std::wstring(ModuleFileName->Buffer);

	// Fix feito para WarZ (o jogo crashava após 2~5 minutos quando essa dll era aberta
#ifdef R_WARZ
	if (wFile == VMProtectDecryptStringW(L"WINHTTP.dll") || wFile == VMProtectDecryptStringW(L"winhttp.dll"))
		return ori_LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);
#endif

	// Verify theads (Anti-Inject)
	bool threadFound = false;
	DWORD currentThreadId = GetCurrentThreadId();

	for (UINT i = 0; i < threads.size(); i++)
	{
		if (threads[i] == currentThreadId)
		{
			threadFound = true;
			break;
		}
	}

	if (currentThreadId == mainThread)
	{
		threadFound = true;
	}

	if (!threadFound)
		return 0xC0000022; // ACCESS_DENIED
	
	// Get SHA1
	UCHAR sha1[20];
	memset(&sha1, 0, 20);

	if (ghp_CRYPTO::CalculeFileSHA1(wFile, sha1))
	{
		// Scan DB table

		UINT entries = ghpDB->GetEntryCount();
		for (UINT i = 0; i < entries; i++)
		{
			// Pega o entry do database
			ghp_UTILS::Database entry = ghpDB->GetEntry(i);
		
			// Verifica se o entry está ativo
			if (entry.state == 0x01)
			{
				// Verifica se é uma entrada de DLL SHA1
				if (entry.type == DB_SHA1_DLL)
				{
					// Compara a memória
					if (memcmp(entry.data, sha1, 20) == 0)
					{
						std::string sFile = std::string(wFile.begin(), wFile.end());

						ghp_UTILS::DataChunk chunk;
						chunk.Init(&entry.data, 32);

						ghpLog.LineOut(false, VMProtectDecryptStringA("Dll detected: [%s] %s"), ghp_CRYPTO::GetFormatedSHA1(&chunk).c_str(), sFile.c_str());
						chunk.FreeAll();
						SendInfo(PCK_INFO_H_DLL, sFile);

						Error(ERROR_CHEAT_FOUND, 0xF001);
						return 0xC0000022; // ACCESS_DENIED
					}
				}
			}
		}

		// Scaneia com base na lista recebida do GHPServer
		for (UINT i = 0; i < ghpsrv_hacks.size(); i++)
		{
			if (ghpsrv_hacks[i].type == DB_SHA1_DLL)
			{
				// Compara a memória
				if (memcmp(ghpsrv_hacks[i].data, sha1, 20) == 0)
				{
					std::string sFile = std::string(wFile.begin(), wFile.end());

					ghp_UTILS::DataChunk chunk;
					chunk.Init(&ghpsrv_hacks[i].data, 32);

					ghpLog.LineOut(false, VMProtectDecryptStringA("[SRV] Dll detected: [%s] %s"), ghp_CRYPTO::GetFormatedSHA1(&chunk).c_str(), sFile.c_str());
					chunk.FreeAll();
					SendInfo(PCK_INFO_H_DLL, sFile);

					Error(ERROR_CHEAT_FOUND, 0xF001);
					return 0xC0000022; // ACCESS_DENIED
				}
			}
		}
	}

	// Load DLL
	NTSTATUS ret = ori_LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);
	VMProtectEnd();
	return ret;
}

DWORD WINAPI ghp_timeGetTime(VOID)
{
	DWORD ret = ori_timeGetTime();
	CheckGHP(ret);
	return ret;
}

int WINAPI ghp_send(SOCKET s, const char * buf, int len, int flags)
{
	CheckGHP(ori_timeGetTime());

#ifdef R_MUONLINE
	if (SerialReceived && ghp_UTILS::IsSerialPacket((UCHAR*)buf))
	{
		VMProtectBeginUltra("ghp_send");
		GameLogin = ghp_UTILS::SetSerialAndGetLogin((UCHAR*)buf, len, std::string((char*)GameSerial.data, 16));
		VMProtectEnd();
	}
#endif

	return ori_send(s, buf, len, flags);
}

int WINAPI ghp_recv(SOCKET s, char * buf, int len, int flags)
{
	CheckGHP(ori_timeGetTime());
	return ori_recv(s, buf, len, flags);
}

VOID WINAPI ghp_ExitProcess(UINT uExitCode)
{
	VMProtectBeginUltra("ghp_ExitProcess");
	Stop();
	ori_ExitProcess(uExitCode);
	VMProtectEnd();
}

HANDLE WINAPI ghp_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	VMProtectBeginMutation("ghp_CreateThread");
	CheckGHP(ori_timeGetTime());
	HANDLE ret;
	
	if (lpThreadId == 0)
	{
		DWORD thID;
		ret = ori_CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, &thID);
		threads.push_back(thID);
	}
	else
	{
		ret = ori_CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
		threads.push_back(*lpThreadId);
	}
	VMProtectEnd();
	
	return ret;
}

UINT HookAPI()
{
	VMProtectBeginUltra("HookAPI");

	if(DetourTransactionBegin() != NO_ERROR)
		return 0xFF;
	if(DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
		return 0xFF;

	/* Get LdrLoadDll Address */
	ori_LdrLoadDll = (LdrLoadDll)GetProcAddress(GetModuleHandleA(VMProtectDecryptStringA("ntdll.dll")), VMProtectDecryptStringA("LdrLoadDll"));

	/* Check and hook */
	if (ori_LdrLoadDll == NULL)
		return 1;
	if (DetourAttach(&(PVOID&) ori_LdrLoadDll, ghp_LdrLoadDll) != NO_ERROR)
		return 2;
	if (DetourAttach(&(PVOID&) ori_timeGetTime, ghp_timeGetTime) != NO_ERROR)
		return 3;
	if (DetourAttach(&(PVOID&) ori_send, ghp_send) != NO_ERROR)
		return 4;
	if (DetourAttach(&(PVOID&) ori_recv, ghp_recv) != NO_ERROR)
		return 5;
	if (DetourAttach(&(PVOID&) ori_CreateThread, ghp_CreateThread) != NO_ERROR)
		return 6;
	if (DetourAttach(&(PVOID&) ori_ExitProcess, ghp_ExitProcess) != NO_ERROR)
		return 7;
	
	if(DetourTransactionCommit() != NO_ERROR)
		return 0xFF;

	VMProtectEnd();
	return 0;
}