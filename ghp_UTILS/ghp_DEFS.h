#ifndef _GHP_DEFS_H_
#define _GHP_DEFS_H_

#include "..\include.h"

#define DB_DUMP_HACK 0x00
#define DB_SHA1_HACK 0x01
#define DB_SHA1_DLL	 0x02

namespace ghp_UTILS
{
	typedef struct tpixelrgb
	{
		UCHAR r, g, b;
	} pixelrgb;

	typedef struct tpixelrgba
	{
		UCHAR r, g, b, a;
	} pixelrgba;

	typedef struct tDataChunk
	{
		LPVOID data, backupdata;
		UINT size, backupsize;

		tDataChunk()
		{
			data = NULL;
			backupdata = NULL;
			size = NULL;
			backupsize = NULL;
		}
		
		bool Init(tDataChunk * Data)
		{
			return Init(Data->data, Data->size);
		}

		bool Init(UINT Size)
		{
			Free();
			size = Size;
			data = malloc(size);

			if (data == NULL)
			{
				return false;
			}
			else
				return true;
		}

		bool Init(LPVOID Data, UINT Size)
		{
			if (!Init(Size))
				return false;

			if (memcpy_s(data, size, Data, Size) != 0)
				return false;
			else
				return true;
		}

		bool Backup()
		{
			FreeBackup();

			backupsize = size;
			backupdata = malloc(backupsize);

			if (backupdata == NULL)
			{
				FreeBackup();
				return false;
			}
			else
			{
				if (memcpy_s(backupdata, backupsize, data, size) != 0)
				{
					FreeBackup();
					return false;
				}
				else
					return true;
			}
		}

		bool Restore()
		{
			Free();

			size = backupsize;
			data = malloc(size);

			if (data == NULL)
			{
				Free();
				return false;
			}
			else
			{
				if (memcpy_s(data, size, backupdata, backupsize) != 0)
				{
					Free();
					return false;
				}
				else
					return true;
			}
		}

		void FreeAll()
		{
			Free();
			FreeBackup();
		}

		void Free()
		{
			free(data);
			data = NULL;
			size = NULL;
		}

		void FreeBackup()
		{
			free(backupdata);
			backupdata = NULL;
			backupsize = NULL;
		}

		bool Inc(tDataChunk * Data)
		{
			return Inc(Data->data, Data->size);
		}

		bool Inc(LPVOID Data, UINT Size)
		{
			LPVOID tmp = realloc(data, size + Size);

			if (tmp == NULL)
				return false;
			else
			{
				data = tmp;

				if (memcpy_s((char*)data + size, size + Size, Data, Size) != 0)
					return false;
				else
				{
					size += Size;
					return true;
				}
			}
		}

		bool EraseFront(UINT sizeToErase)
		{
			tDataChunk memTemp;

			// Verifica o size para saber se pode ou não apagar essa quantidade de dados
			if (size < sizeToErase)
			{
				memTemp.FreeAll();
				return false;
			}

			if (!MemCpy(&memTemp, size - sizeToErase, sizeToErase) || !memTemp.MemCpy(this, memTemp.size))
			{
				memTemp.FreeAll();
				return false;
			}

			memTemp.FreeAll();
			return true;
		}
		
		bool MemCpy(struct tDataChunk * chunkDst, UINT SizeToCpy, UINT OffsetToStart = 0)
		{
			chunkDst->Free();

			// Verifica se é possível copiar os bytes requisitados
			if (size >= SizeToCpy + OffsetToStart)
			{
				if (!chunkDst->Init((UCHAR*)data + OffsetToStart, SizeToCpy))
					return false;
				else
					return true;
			}
			else
				return false;
		}

		std::string toString()
		{
			return std::string((char*)data, size);
		}
	} DataChunk;

	typedef struct tDatabase
	{
		UCHAR type;
		UCHAR state;
		UINT offset;
		UCHAR data[32];
	} Database;

	typedef struct tCRC
	{
		std::wstring fileName;
		UCHAR crc[20];
	} CRC;
}

#endif