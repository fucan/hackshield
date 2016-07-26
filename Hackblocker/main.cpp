#include "..\include.h"
#include <random>

#include "..\ghp_UTILS\ghp_LOG.h"
#include "..\ghp_UTILS\ghp_SYSTEM.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"

typedef struct tExeInfo
{
	DWORD ImageBase;
	DWORD CodeBase;
	DWORD SizeOfCode;
	DWORD SizeOfImage;
	DWORD EntryPoint;

	tExeInfo()
	{
		ImageBase = 0;
		CodeBase = 0;
		SizeOfCode = 0;
		SizeOfImage = 0;
		EntryPoint = 0;
	}
} ExeInfo;

ghp_UTILS::Log logHacks(L"hacks.log");
ghp_UTILS::Log logHacksError(L"hacks_error.log");

bool IsEXE(std::wstring fileName)
{
	int len = fileName.length();

	if ((fileName[len-3] == 'E' || fileName[len-3] == 'e') &&
		(fileName[len-2] == 'X' || fileName[len-2] == 'x') &&
		(fileName[len-1] == 'E' || fileName[len-1] == 'e'))
		return true;
	else
		return false;
}

bool IsDLL(std::wstring fileName)
{
	int len = fileName.length();

	if ((fileName[len-3] == 'D' || fileName[len-3] == 'd') &&
		(fileName[len-2] == 'L' || fileName[len-2] == 'l') &&
		(fileName[len-1] == 'L' || fileName[len-1] == 'l'))
		return true;
	else
		return false;
}

std::string GetFileNameA(std::string name)
{
	std::string ret = "";

	for (UINT i = name.length() - 1; i > 0; i--)
	{
		// Verifica se existe alguma quebra de folder e caso exista retorna o texto até aqui
		if (name[i] == '\\')
			break;

		// Incrementa a letra no começo do texto
		ret = name[i] + ret;
	}

	return ret;
}

std::wstring GetFileNameW(std::wstring name)
{
	std::wstring ret = L"";

	for (UINT i = name.length() - 1; i > 0; i--)
	{
		// Verifica se existe alguma quebra de folder e caso exista retorna o texto até aqui
		if (name[i] == L'\\')
			break;

		// Incrementa a letra no começo do texto
		ret = name[i] + ret;
	}

	return ret;
}

void TerminateProcess(DWORD dwProcessID)
{
	TerminateProcess(OpenProcess(PROCESS_TERMINATE, false, dwProcessID), 0);
}

DWORD CreateProcessSimple(std::wstring name)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessW(name.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) == FALSE)
	{
		logHacksError.LineOut(false, "CreateProcess error: %ls (%08X)\n", name.c_str(), GetLastError());
		return 0;
	}
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return pi.dwProcessId;
}

ExeInfo GetExeInfo(std::wstring name)
{
	HANDLE hFile;
	HANDLE hMap;
	PBYTE pData;
	ExeInfo info;
	
	if ((hFile = CreateFileW(name.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		return info;
	}

	if ((hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL))	== NULL)
	{
		CloseHandle(hFile);
		return info;
	}
	
	if ((pData = (PBYTE)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 8192)) == NULL)
	{
		CloseHandle(hMap);
		CloseHandle(hFile);
		return info;
	}

	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pData;

	if (pDos->e_magic == IMAGE_DOS_SIGNATURE)
	{
		PIMAGE_NT_HEADERS32 pHdr32 = (PIMAGE_NT_HEADERS32)(((PBYTE)(pDos)) + pDos->e_lfanew);
		PIMAGE_NT_HEADERS64 pHdr64 = (PIMAGE_NT_HEADERS64)(((PBYTE)(pDos)) + pDos->e_lfanew);

		if (((PBYTE)pHdr32 - pData) <= 4096 && pHdr32->Signature == IMAGE_NT_SIGNATURE && pHdr32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
		{
			info.ImageBase = (DWORD)pHdr32->OptionalHeader.ImageBase;
			info.CodeBase = (DWORD)pHdr32->OptionalHeader.BaseOfCode;
			info.SizeOfCode = (DWORD)pHdr32->OptionalHeader.SizeOfCode;
			info.SizeOfImage = (DWORD)pHdr32->OptionalHeader.SizeOfImage;
			info.EntryPoint = (DWORD)pHdr32->OptionalHeader.AddressOfEntryPoint;
		}
		else
		{
			if (((PBYTE)pHdr64 - pData) <= 4096 && pHdr64->Signature == IMAGE_NT_SIGNATURE && pHdr64->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
			{
				info.ImageBase = (DWORD)pHdr64->OptionalHeader.ImageBase;
				info.CodeBase = (DWORD)pHdr64->OptionalHeader.BaseOfCode;
				info.SizeOfCode = (DWORD)pHdr64->OptionalHeader.SizeOfCode;
				info.SizeOfImage = (DWORD)pHdr64->OptionalHeader.SizeOfImage;
				info.EntryPoint = (DWORD)pHdr64->OptionalHeader.AddressOfEntryPoint;
			}
		}
	}

	UnmapViewOfFile(pData);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return info;
}

bool GetDump(DWORD prID, DWORD offset, PUCHAR memory)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, false, prID);

	if (hProcess != NULL)
	{
		DWORD read;
		if (ReadProcessMemory(hProcess, (LPCVOID)offset, memory, 32, &read) == FALSE || read != 32)
		{
			CloseHandle(hProcess);
			return false;
		}
		else
		{
			CloseHandle(hProcess);
			return true;
		}
	}
	else
		return false;
}

bool VerifyDump(std::wstring name, PDWORD offset, PUCHAR memory)
{
	DWORD prID = CreateProcessSimple(name);

	if (prID != 0)
	{
		DWORD newoffset = *offset;
		UCHAR newmemory[32];

		if (!GetDump(prID, newoffset, newmemory))
		{
			TerminateProcess(prID);
			return false;
		}

		if (memcmp(memory, newmemory, 32) != 0)
		{
			TerminateProcess(prID);
			return false;
		}

		TerminateProcess(prID);
		return true;
	}
	else
		return false;
}

bool GetExeDump(std::wstring name, PDWORD offset, PUCHAR memory)
{
	DWORD prID = CreateProcessSimple(name);

	if (prID != 0)
	{
		ExeInfo info = GetExeInfo(name);

		if (info.ImageBase != 0)
		{
			//std::default_random_engine generator((ULONG)time(0));
			//std::uniform_int_distribution<DWORD> distribution(0, info.SizeOfCode + info.CodeBase - 32);
			*offset = info.ImageBase + info.EntryPoint; //distribution(generator);
			
			if (!GetDump(prID, *offset, memory))
			{
				TerminateProcess(prID);
				return false;
			}

			TerminateProcess(prID);

			if (!VerifyDump(name, offset, memory))
			{
				return false;
			}
			
			return true;
		}
		
		TerminateProcess(prID);
		return false;
	}
	else
		return false;
}

void main()
{
	std::vector<std::wstring> files;
	ghp_UTILS::GetAllFiles(files, L".\\hacks\\");

	UCHAR hash[20];
	
	for (UINT i = 0; i < files.size(); i++)
	{
		if (!ghp_CRYPTO::CalculeFileSHA1(files[i], hash))
		{
			std::string sname = std::string(files[i].begin(), files[i].end());
			logHacksError.LineOut(false, "File hash error: %s\n", sname.c_str());
			Sleep(500);
			continue;
		}
		
		if (IsEXE(files[i]))
		{
			std::string sname = GetFileNameA(std::string(files[i].begin(), files[i].end()));
			logHacks.LineOut(false, "	{DB_SHA1_HACK, 0x01,  0x00000000, {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}, // %s", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19], sname.c_str());

			// Captura a dump do executavel
			DWORD offset;
			UCHAR memory[32];
			if (!GetExeDump(files[i], &offset, memory))
			{
				logHacksError.LineOut(false, "GetExeDump error: %s\n", sname.c_str());
			}
			else
			{
				logHacks.LineOut(false, "	{DB_DUMP_HACK, 0x01,  0x%08X, {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}}, // %s", offset, memory[0], memory[1], memory[2], memory[3], memory[4], memory[5], memory[6], memory[7], memory[8], memory[9], memory[10], memory[11], memory[12], memory[13], memory[14], memory[15], memory[16], memory[17], memory[18], memory[19], memory[20], memory[21], memory[22], memory[23], memory[24], memory[25], memory[26], memory[27], memory[28], memory[29], memory[30], memory[31], sname.c_str());
			}
		}
		else if (IsDLL(files[i]))
		{
			std::string sname = GetFileNameA(std::string(files[i].begin(), files[i].end()));
			logHacks.LineOut(false, "	{DB_SHA1_DLL , 0x01,  0x00000000, {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}, // %s", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19], sname.c_str());
		}
	}

	Sleep(500);
}