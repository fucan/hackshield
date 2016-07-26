#include "ghp_FILE.h"
//#include "..\ghp_UTILS\lzma\LzmaLib.h"

namespace ghp_UTILS
{
	bool LoadFileData(std::wstring FileName, DataChunk * chunk)
	{
		HANDLE hFile = CreateFileW(FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		chunk->FreeAll();

		if (hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER size;

			if (GetFileSizeEx(hFile, &size) == FALSE)
			{
				chunk->FreeAll();
				CloseHandle(hFile);
				return false;
			}

			DWORD dwRead;
			DWORD FileSize = (DWORD)size.QuadPart;

			chunk->data = malloc(FileSize);
			chunk->size = FileSize;

			if (chunk->data == NULL)
			{
				chunk->FreeAll();
				CloseHandle(hFile);
				return false;
			}

			if (ReadFile(hFile, chunk->data, chunk->size, &dwRead, NULL) == FALSE || dwRead != chunk->size)
			{
				chunk->FreeAll();
				CloseHandle(hFile);
				return false;
			}

			CloseHandle(hFile);
			return true;
		}
		else
		{
			chunk->FreeAll();
			return false;
		}
	}
	
	bool WriteFileData(std::wstring FileName, DataChunk * chunk, UINT BytesToWrite, bool Append)
	{
		UINT len;
		DWORD mode;

		// Verifica se é para escrever todos os bytes ou somente o size enviado na função
		if (BytesToWrite == 0)
			len = chunk->size;
		else
			len = BytesToWrite;

		// Verifica o modo que irá escrever os dados
		if (Append == false)
			mode = CREATE_ALWAYS;
		else
			mode = OPEN_ALWAYS;

		// Abre ou cria o arquivo
		HANDLE hFile = CreateFileW(FileName.c_str(), GENERIC_WRITE, 0, NULL, mode, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwWrote;

			// Vai para o final do arquivo caso requisitado na função
			if (Append)
				SetFilePointer(hFile, 0, NULL, FILE_END);

			if (WriteFile(hFile, chunk->data, len, &dwWrote, NULL) == FALSE || dwWrote != len)
			{
				CloseHandle(hFile);
				return false;
			}

			CloseHandle(hFile);
			return true;
		}
		else
			return false;
	}

	bool WriteFileDataChunkBlock(std::wstring FileName, DataChunk * data, bool Append)
	{
		DataChunk sizeChunk;
		DWORD dataSize = data->size;

		if (!sizeChunk.Init((LPVOID)&dataSize, sizeof(DWORD)) || !WriteFileData(FileName, &sizeChunk, 0, Append) || !WriteFileData(FileName, data, 0, true))
		{
			sizeChunk.FreeAll();
			return false;
		}

		sizeChunk.FreeAll();
		return true;
	}

	bool ReadDataChunkBlock(DataChunk * dataChunkIn, DataChunk * dataChunkOut)
	{
		DataChunk sizeChunk;
		dataChunkOut->FreeAll();
		
		if (!dataChunkIn->MemCpy(&sizeChunk, sizeof(DWORD)) || !dataChunkIn->MemCpy(dataChunkOut, *(DWORD*)sizeChunk.data, sizeof(DWORD)) || !dataChunkIn->EraseFront(*(DWORD*)sizeChunk.data + sizeof(DWORD)))
		{
			sizeChunk.FreeAll();
			dataChunkOut->FreeAll();
			return false;
		}
		
		sizeChunk.FreeAll();
		return true;
	}

	bool WriteFileStringBlock(std::wstring FileName, std::string data, bool Append)
	{
		DataChunk dataChunk;

		if (!dataChunk.Init((LPVOID)data.c_str(), data.size()) || !WriteFileDataChunkBlock(FileName, &dataChunk, Append))
		{
			dataChunk.FreeAll();
			return false;
		}

		dataChunk.FreeAll();
		return true;
	}

	bool ReadStringBlock(DataChunk * memory, std::string * string)
	{
		DataChunk dataChunk;
		string->clear();

		if (!ReadDataChunkBlock(memory, &dataChunk))
		{
			dataChunk.FreeAll();
			return false;
		}

		string->append((char*)dataChunk.data, dataChunk.size);

		dataChunk.FreeAll();
		return true;
	}

	bool LZMA_Compress(DataChunk * memoryOrg, DataChunk * memoryDst)
	{
		/*memoryDst->FreeAll();
		memoryDst->Init(memoryOrg->size + 999999);

		UINT props_size = LZMA_PROPS_SIZE;

		if(LzmaCompress((UCHAR*)memoryDst->data + LZMA_PROPS_SIZE, &memoryDst->size, (UCHAR*)memoryOrg->data, memoryOrg->size, (UCHAR*)memoryDst->data, &props_size, -1, 0, -1, -1, -1, -1, -1) == SZ_OK)
			return true;
		else*/
			return false;
	}

	bool LZMA_Decompress(DataChunk * memoryOrg, DataChunk * memoryDst)
	{
		/*memoryDst->FreeAll();
		memoryDst->Init(memoryOrg->size + 999999);

		UINT props_size = LZMA_PROPS_SIZE;

		if (LzmaUncompress((UCHAR*)memoryDst->data, &memoryDst->size, (UCHAR*)memoryOrg->data + LZMA_PROPS_SIZE, &memoryOrg->size, (UCHAR*)memoryOrg->data, props_size) == SZ_OK)
			return true;
		else*/
			return false;
	}
}