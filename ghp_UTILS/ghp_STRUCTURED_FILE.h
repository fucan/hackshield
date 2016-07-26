#ifndef _GHP_STRUCTURED_FILE_H_
#define _GHP_STRUCTURED_FILE_H_

#include "..\include.h"
#include "ghp_DEFS.h"

namespace ghp_UTILS
{
	class StructuredFile
	{
	private:
		UINT length, size;
		DataChunk data;
	public:
		UINT GetLength();
		UINT GetSize();

		bool Get(UINT index, UINT DstLen, LPVOID DstBuffer);
		bool Set(UINT ArrayLength, UINT ArraySize, LPVOID ArrayData);

		bool LoadFromMemory(DataChunk * memory, std::string PublicKey, UCHAR * key);

		bool LoadFromFile(std::wstring FileName, std::string PublicKey, UCHAR * key);
		bool SaveToFile(std::wstring FileName, std::string PrivateKey, UCHAR * key);
	};
}

#endif