#include "ghp_MEMORY_PROTECTION.h"

namespace ghp_UTILS
{
	MemoryProtection::MemoryProtection()
	{
		VMProtectBeginUltra("MemoryProtection::MemoryProtection");
		entries.clear();
		VMProtectEnd();
	}

	bool MemoryProtection::GetMemory(LPVOID Addr, UINT Size, UCHAR * outBuffer)
	{
		/* Seta as permissões de leitura ao ADDR */
		DWORD oldProtect = 0;
		if (VirtualProtect((LPVOID)Addr, Size, PAGE_EXECUTE_READWRITE, &oldProtect) == 0)
			return false;

		/* Lê a memória do ADDR */
		if (memcpy_s(outBuffer, Size, Addr, Size) != 0)
			return false;

		/*DWORD read = 0;
		if (ReadProcessMemory(GetCurrentProcess(), Addr, outBuffer, Size, &read) == 0 || read != Size)
			return false;*/

		/* Reseta as permissões originais ao ADDR */
		if (VirtualProtect((LPVOID)Addr, Size, oldProtect, &oldProtect) == 0)
			return false;

		return true;
	}

	bool MemoryProtection::Add(LPVOID Addr, UINT Size)
	{
		VMProtectBeginUltra("MemoryProtection::Add");

		/* Lê as informações da memória */
		UCHAR * buffer = new UCHAR[Size];
		if (!GetMemory(Addr, Size, buffer))
		{
			delete [] buffer;
			return false;
		}

		/* Adiciona na lista de entries */
		entries.push_back(MemoryEntry(Addr, Size, buffer));
		delete [] buffer;

		VMProtectEnd();
		return true;
	}

	bool MemoryProtection::AddAPI(std::wstring DLL, std::string API, UINT Size)
	{
		VMProtectBeginUltra("MemoryProtection::AddAPI");

		/* Tenta encontrar a DLL */
		HMODULE hModule = GetModuleHandle(DLL.c_str());

		if (hModule == NULL)
		{
			/* Se não encontrou, abre a DLL */
			hModule = LoadLibrary(DLL.c_str());

			/* Retorna false pois não foi possível abrir a DLL */
			if (hModule == NULL)
				return false;
		}

		/* Pega o ADDR da API */
		DWORD addr = (DWORD)GetProcAddress(hModule, API.c_str());

		/* Verifica se encontrou a API */
		if (addr == NULL)
			return false;

		/* Lê os dados e Adciona ao entry */
		bool ret = Add((LPVOID)addr, Size);

		VMProtectEnd();
		return ret;
	}

	USHORT MemoryProtection::Check()
	{
		VMProtectBeginMutation("MemoryProtection::Check");
		UCHAR * buffer;

		for (UINT i = 0; i < entries.size(); i++)
		{
			buffer = new UCHAR[entries[i].size];

			/* Lê a memória atual */
			if (!GetMemory(entries[i].addr, entries[i].size, buffer))
			{
				delete [] buffer;
				return (USHORT)i + 1;
			}

			/* Compara com a memória protegida */
			if (memcmp(&entries[i].memory[0], buffer, entries[i].size) != 0)
			{
				delete [] buffer;
				return (USHORT)i + 1;
			}

			delete [] buffer;
		}
		
		VMProtectEnd();
		return 0;
	}
}