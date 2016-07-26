#include "ghp_SCAN.h"
#include "ghp_CORE.h"
#include "ghp_NETBOX.h"

typedef struct tdisk
{
	std::wstring letter;
	std::wstring device;

	tdisk(std::wstring Letter, std::wstring Device)
	{
		letter = Letter;
		device = Device;
	}
} disk;

bool DisksReaded = false;

std::vector<DWORD> Scanned;
std::vector<disk> Disks;

void GetDisks()
{
	wchar_t letra[3];
	wchar_t target[MAX_PATH];

	letra[1] = 0x3A;
	letra[2] = 0x00;

	Disks.clear();

	for(UINT i = 0; i < 26; i++)
	{
		ZeroMemory(target, MAX_PATH);
		letra[0] = i + 65;
		if (QueryDosDeviceW(letra, target, MAX_PATH) > 0)
			Disks.push_back(disk(letra, target));
	}
}

std::wstring GetWIN32PathFromDevice(std::wstring device)
{
	for (UINT i = 0; i < Disks.size(); i++)
	{
		if (device.find(Disks[i].device) == 0)
		{
			device.erase(0, Disks[i].device.length());
			device.insert(0, Disks[i].letter);
			return device;
		}
	}

	return L"";
}

bool IsScanned(DWORD process)
{
	for(UINT i = 0; i < Scanned.size(); i++)
	{
		if (Scanned[i] == process)
			return true;
	}
	return false;
}

extern UINT cheat_whitelist_count;
extern DWORD * cheat_whitelist;

inline bool IsCheatWhitelisted(DWORD id)
{
	for (UINT i = 0; i < cheat_whitelist_count; i++)
	{
		if (ghp_CRYPTO::DecryptDWORD(cheat_whitelist[i]) == id)
			return true;
	}

	return false;
}

void ScanThisProcess(HANDLE hProcess, DWORD processId)
{
	VMProtectBeginMutation("ScanThisProcess");
	std::wstring exe = L"";
	UCHAR sha1[20];
	wchar_t exeName[MAX_PATH];
		
	if(GetProcessImageFileNameW(hProcess, exeName, MAX_PATH) != 0)
		exe = GetWIN32PathFromDevice(exeName);
	
	if (exe != L"")
		ghp_CRYPTO::CalculeFileSHA1(exe, sha1);

	// Verifica cada entrada da DB
	UINT entries = ghpDB->GetEntryCount();
	for(UINT i = 0; i < entries; i++)
	{
		// Verifica se está no whitelist
		if (IsCheatWhitelisted(i + 1))
			continue;
		
		// Pega o entry do database
		ghp_UTILS::Database entry = ghpDB->GetEntry(i);
		
		// Verifica se o entry está ativo
		if (entry.state == 0x01)
		{
			if (entry.type == DB_DUMP_HACK)
			{
				UCHAR memory[32];
				DWORD read;
				
				if (ReadProcessMemory(hProcess, (LPCVOID)entry.offset, memory, 32, &read) == FALSE || read != 32)
					continue;

				if (memcmp(memory, entry.data, 32) == 0)
				{
					TerminateProcess(hProcess, 0);
					
					ghp_UTILS::DataChunk entryChunk;
					entryChunk.Init(&entry, sizeof(ghp_UTILS::Database));

					std::string sname = std::string(exe.begin(), exe.end());
					ghpLog.LineOut(false, VMProtectDecryptStringA("Cheat: [%d | %s] %d - %s"), i + 1, ghp_CRYPTO::GetFormatedSHA1(&entryChunk).c_str(), processId, sname.c_str());
					entryChunk.FreeAll();
					SendInfo(PCK_INFO_H_HACK, sname);
					Error(ERROR_CHEAT_FOUND, (USHORT)i + 1);
				}
			}
			else if (entry.type == DB_SHA1_HACK && exe != L"")
			{
				if (memcmp(entry.data, sha1, 20) == 0)
				{
					TerminateProcess(hProcess, 0);

					ghp_UTILS::DataChunk entryChunk;
					entryChunk.Init(&entry, sizeof(ghp_UTILS::Database));

					std::string sname = std::string(exe.begin(), exe.end());
					ghpLog.LineOut(false, VMProtectDecryptStringA("Cheat: [%d | %s] %d - %s"), i + 1, ghp_CRYPTO::GetFormatedSHA1(&entryChunk).c_str(), processId, sname.c_str());
					entryChunk.FreeAll();
					SendInfo(PCK_INFO_H_HACK, sname);
					Error(ERROR_CHEAT_FOUND, (USHORT)i + 1);
				}
			}
		}
	}

	// Verifica os hacks recebidos do GHPServer
	for (UINT i = 0; i < ghpsrv_hacks.size(); i++)
	{
		if (ghpsrv_hacks[i].type == DB_SHA1_HACK && exe != L"")
		{
			if (memcmp(ghpsrv_hacks[i].data, sha1, 20) == 0)
			{
				TerminateProcess(hProcess, 0);

				ghp_UTILS::DataChunk entryChunk;
				entryChunk.Init(&ghpsrv_hacks[i], sizeof(ghp_UTILS::Database));
				
				std::string sname = std::string(exe.begin(), exe.end());
				ghpLog.LineOut(false, VMProtectDecryptStringA("[SRV] Cheat: [%d | %s] %d - %s"), i + 1, ghp_CRYPTO::GetFormatedSHA1(&entryChunk).c_str(), processId, sname.c_str());
				SendInfo(PCK_INFO_H_HACK, sname);
				Error(ERROR_CHEAT_FOUND, (USHORT)i + 1);
			}
		}
	}
	VMProtectEnd();
}

void ProcessScan()
{
	VMProtectBeginMutation("ProcessScan");
	// Vefifica se abriu os discos
	if (!DisksReaded)
	{
		// Abre os discos
		GetDisks();
		DisksReaded = true;
	}

	DWORD dProcesses[4096], cbNeeded, cProcesses;
	DWORD me = GetCurrentProcessId();

	if(!EnumProcesses(dProcesses, sizeof(dProcesses), &cbNeeded))
		Error(ERROR_PROCESS_SCAN, 0);

	cProcesses = cbNeeded / sizeof(DWORD);

	for(UINT i = 0; i < cProcesses; i++)
	{
		// Verifica se o processo é válido e se já foi escaneado
		if (dProcesses[i] != 0 && dProcesses[i] != me && !IsScanned(dProcesses[i]))
		{
			// Scan a memória do processo
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, false, dProcesses[i]);
			if (hProcess != NULL)
			{
				ScanThisProcess(hProcess, dProcesses[i]);
			}
			
			CloseHandle(hProcess);

			// Adiciona o processo escaneado no vetor
			Scanned.push_back(dProcesses[i]);
		}
	}
	VMProtectEnd();
}

void HiddenScan()
{
	DWORD prID = 0;
	HWND hWnd = FindWindow(0, 0);

	if (hWnd == 0)
		Error(ERROR_PROCESS_SCAN, 1);

	while(hWnd > 0)
	{
		if (GetParent(hWnd) == 0)
		{
			GetWindowThreadProcessId(hWnd, &prID);

			if (OpenProcess(PROCESS_TERMINATE, false, prID) == NULL)
			{
				if (IsWindow(hWnd))
				{
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					SendMessage(hWnd, WM_QUIT, 0, 0);
				}
			}
		}

		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}
}

void SpeedScan()
{
	DWORD t1 = timeGetTime();
	DWORD t2 = ori_timeGetTime();
	int t3 = t1 - t2;

	if (t3 < -250 || t3 > 250)
	{
		SendInfo(PCK_INFO_H_HACK, "SpeedHack heuristic");
		Error(ERROR_SPEED_SCAN, 0);
	}
}