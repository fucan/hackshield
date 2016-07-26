#include "ghp_CORE.h"
#include "ghp_NETBOX.h"
#include "ghp_SCAN.h"
#include "ghp_SPLASH.h"
#include "..\ghp_UTILS\ghp_SYSTEM.h"
#include "..\ghp_UTILS\ghp_STRUCTURED_FILE.h"
#include "..\ghp_UTILS\ghp_MEMORY_PROTECTION.h"
#include "..\ghp_NET\ghp_HTTPCLIENT.h"

HANDLE hNETBox = NULL;
HANDLE hGHP = NULL;

ghp_UTILS::Log ghpLog(L"GHP.log");
ghp_UTILS::StructuredFile ghpDBStructure;
ghp_UTILS::MemoryProtection ghpMemoryProtection;
ghp_UTILS::DB * ghpDB;
ghp_UTILS::Lic * ghpLic;

UINT cheat_whitelist_count, api_whitelist_count;
DWORD * cheat_whitelist, * api_whitelist;

std::vector<ghp_UTILS::CRC> crc;
std::list<std::string> hwids;
std::vector<ghp_UTILS::Database> ghpsrv_hacks;
ghp_UTILS::DataChunk GameSerial;
std::string GameLogin = "";
std::string LastGameLogin = "";

USHORT errorid, errorcode;
bool loadlock = false;
bool error = false;

DWORD lastLoop = 0, lastCRC = 0, mainThread = 0;
#ifdef R_WARZ
LPDIRECT3DDEVICE9 d3ddev = 0;
#endif

DWORD WINAPI GHPCore(LPVOID lpParam)
{
	VMProtectBeginVirtualization("GHPCore");
	lastLoop = ori_timeGetTime();
	
	// Seta a prioridade da thread novamente
	SetThreadPriority(GetCurrentThread(), REALTIME_PRIORITY_CLASS);

	// Scan
	ProcessScan();
	HiddenScan();
	SpeedScan();

	// Check CRC
	if (lastLoop - lastCRC > 15000)
	{
		lastCRC = lastLoop;
		UCHAR crc_buff[20];
		
		for (UINT i = 0; i < crc.size(); i++)
		{
			if (!ghp_CRYPTO::CalculeFileSHA1(crc[i].fileName, &crc_buff[0]) || memcmp(crc_buff, crc[i].crc, 20) != 0)
			{
				std::string fName(crc[i].fileName.begin(), crc[i].fileName.end());
				SendInfo(PCK_INFO_H_CRC, fName);
				Error(ERROR_CRC_INVALID, i);
			}
		}
	}

	// Memory check
	USHORT mem_check = ghpMemoryProtection.Check();
	if (mem_check != 0)
	{
		bool found = false;

		// verifica se a API está whitelisted
		for (UINT i = 0; i < api_whitelist_count; i++)
		{
			if (ghp_CRYPTO::DecryptDWORD(api_whitelist[i]) == mem_check)
			{
				found = true;
				break;
			}
		}

		if (!found)
			Error(ERROR_MEMORY_PROTECTED_WAS_CORRUPTED, mem_check);
	}
	
	// Aguarda meio segundo
	Sleep(500);

	// Cria a nova thread do GHP
	hGHP = CreateThread(NULL, 0, GHPCore, NULL, 0, 0);
	if (hGHP == NULL)
		Error(ERROR_WHILE_CREATING_THREAD, 2, GetLastError());

	// Seta a prioridade da nova thread
	SetThreadPriority(hGHP, REALTIME_PRIORITY_CLASS);

	DWORD_PTR m_mask = 1 << 3;
	SetThreadAffinityMask(hGHP, m_mask);

	VMProtectEnd();
	return 0;
}

void Start()
{
	VMProtectBeginUltra("Start");
	// Seta os valores iniciais a variáveis importantes
	lastPing = ori_timeGetTime();
	lastLoop = lastPing;

	ghpLic = new ghp_UTILS::Lic();
	ghpDB = new ghp_UTILS::DB();

	// Hooka a API do Windows
	UINT hookRet = HookAPI();
	if (hookRet != 0)
		Error(ERROR_WHILE_HOOKING, hookRet);

	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Protege as APIs
	if (
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"WS2_32.dll"), VMProtectDecryptStringA("send"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"WS2_32.dll"), VMProtectDecryptStringA("recv"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("ExitProcess"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("QueryPerformanceCounter"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("GetTickCount"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("OpenProcess"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("CreateThread"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("QueryDosDeviceW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("ReadProcessMemory"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("TerminateProcess"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("GetCurrentProcessId"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("VirtualProtect"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Winmm.dll"), VMProtectDecryptStringA("timeGetTime"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("EnumDisplayDevicesW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("GetWindow"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("GetParent"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("IsWindow"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("GetWindowThreadProcessId"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("FindWindowW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("SendMessageW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"User32.dll"), VMProtectDecryptStringA("GetWindowPlacement"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("CreateDCW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("CreateCompatibleDC"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("GetDeviceCaps"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("CreateDIBSection"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("SaveDC"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("SelectObject"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Gdi32.dll"), VMProtectDecryptStringA("BitBlt"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Winmm.dll"), VMProtectDecryptStringA("timeGetTime"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Iphlpapi.dll"), VMProtectDecryptStringA("GetAdaptersInfo"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Advapi32.dll"), VMProtectDecryptStringA("RegOpenKeyExW"), 16) ||
		!ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Advapi32.dll"), VMProtectDecryptStringA("RegQueryValueExW"), 16))

		Error(ERROR_WHILE_HOOKING, 0xF0);

	// PSAPI or KERNEL32
	ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Kernel32.dll"), VMProtectDecryptStringA("EnumProcesses"), 16);
	ghpMemoryProtection.AddAPI(VMProtectDecryptStringW(L"Psapi.dll"), VMProtectDecryptStringA("EnumProcesses"), 16);
	
	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Verifica a hora de criação do GHP e do jogo para evitar problemas de suspend
	FILETIME start, no1, no2, no3;
	SYSTEMTIME process_start, current_time;

	GetSystemTime(&current_time);

	if (GetProcessTimes(GetCurrentProcess(), &start, &no1, &no2, &no3) == FALSE || FileTimeToSystemTime(&start, &process_start) == FALSE)
	{
		Error(ERROR_TIMER_CHECK, 0);
	}
	else
	{
		if (SystemTimeToFileTime(&process_start, &no1) == FALSE || SystemTimeToFileTime(&current_time, &no2) == FALSE)
			Error(ERROR_TIMER_CHECK, 1);
		else
		{
			ULARGE_INTEGER ul1, ul2;

			memcpy_s(&ul1, sizeof(ULARGE_INTEGER), &no1, sizeof(FILETIME));
			memcpy_s(&ul2, sizeof(ULARGE_INTEGER), &no2, sizeof(FILETIME));

			ULONGLONG total = ul2.QuadPart - ul1.QuadPart;
			
			if (total > 100000000)
			{
				Error(ERROR_TIMER_CHECK, 2);
			}
		}
	}

	// Pega os ID's da máquina
	hwids = ghp_UTILS::GetHWIDs();
	lastLoop = lastPing; // atualiza o lastLoop temporariamente
	
	// Inicia a lista de CRC
	crc.clear();

	// Abre a licença e verifica se foi aberta corretamente
	if(!ghpLic->LoadFromFile(VMProtectDecryptStringW(LICENSE_NAME), (UCHAR*)GHP_AES_KEY))
		Error(ERROR_INVALID_LIC, 0);
	
	// Baixa a lista de Whitelist
	ghp_UTILS::DataChunk chunk;
	ghp_NET::HTTPClient http;

	api_whitelist = 0;
	cheat_whitelist = 0;

	char URL[1024];
	sprintf_s(URL, "http://%s:%d/?download=cwl", ghpLic->GetIP().c_str(), ghp_CRYPTO::DecryptDWORD(ghpLic->GetGHPServerWeb()));
	
	if (!http.GetURLToDataChunk(http.SafeURL(URL), &chunk))
		Error(ERROR_WHITELIST, 0); // não conseguiu baixar o whitelist
	else
	{
		// Verifica se o whitelist está em branco
		if (chunk.size == 1)
		{
			cheat_whitelist_count = 1;
			cheat_whitelist = new DWORD[cheat_whitelist_count];
			cheat_whitelist[0] = 0;
		}
		else
		{
			// Abre o whitelist para a memória
			ghp_UTILS::StructuredFile wl;
			if (!wl.LoadFromMemory(&chunk, ghpLic->GetPUBKEY(), (UCHAR*)GHP_AES_KEY))
				Error(ERROR_WHITELIST, 1);
			
			// Cria e preenche o vetor
			cheat_whitelist_count = wl.GetLength();
			cheat_whitelist = new DWORD[cheat_whitelist_count];

			int item;

			for (UINT i = 0; i < cheat_whitelist_count; i++)
			{
				if (!wl.Get(i, sizeof(DWORD), &item))
					Error(ERROR_WHITELIST, 2);
				else
					cheat_whitelist[i] = item;
			}
		}

		// Protege a memória do whitelist
		if (!ghpMemoryProtection.Add(cheat_whitelist, cheat_whitelist_count * sizeof(DWORD)))
			Error(ERROR_WHITELIST, 3);

		if (!ghpMemoryProtection.Add(&cheat_whitelist_count, sizeof(UINT)))
			Error(ERROR_WHITELIST, 4);
	}
	chunk.FreeAll();

	// APIWhitelist
	sprintf_s(URL, "http://%s:%d/?download=awl", ghpLic->GetIP().c_str(), ghp_CRYPTO::DecryptDWORD(ghpLic->GetGHPServerWeb()));

	if (!http.GetURLToDataChunk(http.SafeURL(URL), &chunk))
		Error(ERROR_WHITELIST, 5); // não conseguiu baixar o whitelist
	else
	{
		// Verifica se o whitelist está em branco
		if (chunk.size == 1)
		{
			api_whitelist_count = 1;
			api_whitelist = new DWORD[api_whitelist_count];
			api_whitelist[0] = 0;
		}
		else
		{
			// Abre o whitelist para a memória
			ghp_UTILS::StructuredFile wl;
			if (!wl.LoadFromMemory(&chunk, ghpLic->GetPUBKEY(), (UCHAR*)GHP_AES_KEY))
				Error(ERROR_WHITELIST, 6);
			
			// Cria e preenche o vetor
			api_whitelist_count = wl.GetLength();
			api_whitelist = new DWORD[api_whitelist_count];

			int item;

			for (UINT i = 0; i < api_whitelist_count; i++)
			{
				if (!wl.Get(i, sizeof(DWORD), &item))
					Error(ERROR_WHITELIST, 7);
				else
					api_whitelist[i] = item;
			}
		}

		// Protege a memória do whitelist
		if (!ghpMemoryProtection.Add(api_whitelist, api_whitelist_count * sizeof(DWORD)))
			Error(ERROR_WHITELIST, 8);

		if (!ghpMemoryProtection.Add(&api_whitelist_count, sizeof(UINT)))
			Error(ERROR_WHITELIST, 9);
	}
	chunk.FreeAll();
	
	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Abre a estrutura do banco de dados
	if (!ghpDBStructure.LoadFromFile(VMProtectDecryptStringW(DATABASE_NAME),  ghpLic->GetPUBKEY(), (UCHAR*)GHP_AES_KEY))
		Error(ERROR_INVALID_DB, 0);

	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Cria a variável do banco de dados
	if(!ghpDB->ImportFromStructuredFile(&ghpDBStructure) || ghpDB->GetEntryCount() == 0)
		Error(ERROR_INVALID_DB, 1);
		
	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Verifica se possui privilégios de administrador
	if (!ghp_UTILS::HaveAdminPrivileges() || !ghp_UTILS::PrivEnable(VMProtectDecryptStringW(L"SeDebugPrivilege")))
		Error(ERROR_INSUFFICIENT_PRIVILEGES, 0);

	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Cria a thread de conexão com o servidor e envia a packet com a versão
	hNETBox = CreateThread(NULL, 0, NETBoxThread, NULL, 0, 0);
	if (hNETBox == NULL)
		Error(ERROR_WHILE_CREATING_THREAD, 0, GetLastError());

	lastLoop = lastPing; // atualiza o lastLoop temporariamente

	// Cria a thread do GHP
	hGHP = CreateThread(NULL, 0, GHPCore, NULL, 0, 0);
	if (hGHP == NULL)
		Error(ERROR_WHILE_CREATING_THREAD, 1, GetLastError());

	DWORD_PTR m_mask = 1 << 3;
	SetThreadAffinityMask(hGHP, m_mask);

	lastLoop = lastPing; // atualiza o lastLoop temporariamente
#ifdef R_MUONLINE
	// Abre a fonte e o minimizer
	LoadLibraryA("Fonte.dll");
	LoadLibraryA("ghpAM.dll");
#endif

	lastLoop = lastPing; // atualiza o lastLoop temporariamente
	ghpLog.LineOut(false, VMProtectDecryptStringA("GHP started!"));

	//StartSplash();

	lastLoop = lastPing; // atualiza o lastLoop temporariamente
	VMProtectEnd();
}

void Stop()
{
	VMProtectBeginUltra("Stop");
	// Desconecta e fecha a thread do NETBox
	ShutdownNETBox();
	TerminateThread(hNETBox, 0);
	Sleep(500);
	ori_ExitProcess(0);

	__asm
	{
		mov eax, 0
		jmp eax
	}
	VMProtectEnd();
}

DWORD WINAPI ErrorBrowser(LPVOID lpParam)
{
	UCHAR count = 0;

	while(true)
	{
		count++;
		
		if (count >= 100)
			Stop();

		if (!loadlock)
		{
			std::string URL = ghpLic->GetLINK();
			std::wstring wURL = std::wstring(URL.begin(), URL.end());

			wchar_t buffer[250];
			wsprintf(buffer, wURL.c_str(), errorid, errorcode);
			ShellExecute(0, VMProtectDecryptStringW(L"open"), buffer, NULL, NULL, SW_SHOWNORMAL);

			// Fecha o GHP
			Stop();
			return 0;
		}

		Sleep(10);
	}
	
	Stop();

	return 0;
}

void Error(USHORT id, USHORT code, DWORD error_code)
{
	error = true;
	errorid = id;
	errorcode = code;
	
	switch (id)
	{
	case ERROR_INVALID_LIC:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Invalid or corrupted license file. Error code: %02X%04X"), id, code);
		break;
	case ERROR_INVALID_DB:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Invalid or corrupted database file. Error code: %02X%04X"), id, code);
		break;
	case ERROR_WHILE_HOOKING:
		ghpLog.LineOut(false, VMProtectDecryptStringA("API hook error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_INSUFFICIENT_PRIVILEGES:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Insufficient privileges. Error code: %02X%04X"), id, code);
		break;
	case ERROR_WHILE_CONNECTING_TO_SERVER:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Error while connecting to server (%d). Error code: %02X%04X"), error_code, id, code);
		break;
	case ERROR_DISCONNECTED_FROM_SERVER:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Disconnected from server. Error code: %02X%04X"), id, code);
		break;
	case ERROR_CORRUPTED_PACKET:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Corrupted packet. Error code: %02X%04X"), id, code);
		break;
	case ERROR_STOPPED_WORKING:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_PROCESS_SCAN:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_CHEAT_FOUND:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Cheat found. Error code: %02X%04X"), id, code);
		break;
	case ERROR_WHILE_CREATING_THREAD:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error (%d). Error code: %02X%04X"), error_code, id, code);
		break;
	case ERROR_MEMORY_PROTECTED_WAS_CORRUPTED:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Memory corrupted. Error code: %02X%04X"), id, code);
		break;
	case ERROR_CRC_INVALID:
		{
			crc[code].fileName.erase(crc[code].fileName.begin(), crc[code].fileName.begin() + 2);
			std::string sname = std::string(crc[code].fileName.begin(), crc[code].fileName.end());
			ghpLog.LineOut(false, VMProtectDecryptStringA("Invalid or corrupted file (%s). Error code: %02X%04X"), sname.c_str(), id, code);
		}
		break;
	case ERROR_SPEED_SCAN:
		ghpLog.LineOut(false, VMProtectDecryptStringA("SpeedHack detected. Error code: %02X%04X"), id, code);
		break;
	case ERROR_TIMER_CHECK:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_INVALID_API_KEY:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_GHP_BIN:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Internal error. Error code: %02X%04X"), id, code);
		break;
	case ERROR_WHITELIST:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Whitelist corrupted. Error code: %02X%04X"), id, code);
		break;
	case ERROR_DECRYPT_FAILED:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Invalid packet format. Error code: %02X%04X"), id, code);
		break;
	default:
		ghpLog.LineOut(false, VMProtectDecryptStringA("Unknown error. Error code: %02X%04X"), id, code);
		break;
	}
	
	if (id != ERROR_INVALID_LIC)
	{
		// Inicia a thread que abre o erro no navegador
		CreateThread(NULL, 0, ErrorBrowser, NULL, 0, NULL);
		Sleep(1000);
		Stop();
	}
	else
	{
		Stop();
	}
}