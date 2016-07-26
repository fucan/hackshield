#ifndef _GHP_FILE_H_
#define _GHP_FILE_H_

#include "ghp_DEFS.h"

namespace ghp_UTILS
{
	extern bool LoadFileData(std::wstring FileName, DataChunk * chunk);
	extern bool WriteFileData(std::wstring FileName, DataChunk * chunk, UINT BytesToWrite = 0, bool Append = false);
	extern bool WriteFileDataChunkBlock(std::wstring FileName, DataChunk * data, bool Append = false);
	extern bool ReadDataChunkBlock(DataChunk * dataChunkIn, DataChunk * dataChunkOut);
	extern bool WriteFileStringBlock(std::wstring FileName, std::string data, bool Append = false);
	extern bool ReadStringBlock(DataChunk * memory, std::string * string);
}

#endif