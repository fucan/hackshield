#ifndef _GHP_MEMORY_PROTECTION_H_
#define _GHP_MEMORY_PROTECTION_H_

#include "..\include.h"

namespace ghp_UTILS
{
	typedef struct tMemoryEntry
	{
		LPVOID addr;
		UINT size;
		std::vector<UCHAR> memory;

		tMemoryEntry(LPVOID Addr, UINT Size, UCHAR * buffer)
		{
			addr = Addr;
			size = Size;
			memory.clear();

			for (UINT i = 0; i < Size; i++)
			{
				memory.push_back(buffer[i]);
			}
		}
	} MemoryEntry;

	class MemoryProtection
	{
	private:
		std::vector<MemoryEntry> entries;

		bool GetMemory(LPVOID Addr, UINT Size, UCHAR * outBuffer);
	public:
		MemoryProtection();

		bool Add(LPVOID Addr, UINT Size);
		bool AddAPI(std::wstring DLL, std::string API, UINT Size);
		USHORT Check();
	};
}

#endif