#ifndef _GHP_PROTOCOL_H_
#define _GPH_PROTOCOL_H_

#include "..\include.h"
#include "ghp_DEFS.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include <RakNet\MessageIdentifiers.h>

#define MAX_BUFFER		(1024 * 386)	// Kb
#define MAX_SS_BUFFER	(1024 * 384)	// Kb

#define VERIFY_XOR_KEY	0x35F922C0

enum PckHeader
{
	PCK_H_PING = ID_USER_PACKET_ENUM + 5,
	PCK_H_ENCRYPTED_MESSAGE,
	PCK_H_ENCRYPTION_KEY,
	PCK_H_ID,
	PCK_H_USER_ID,
	PCK_H_GAME_SERIAL,
	PCK_H_CRC,
	PCK_H_ENTRY,
	PCK_H_INFO,
	PCK_H_SS,
	PCK_H_VERIFY,
	PCK_H_VERIFY_RET
};

enum PckInfoHeader
{
	PCK_INFO_H_HACK,
	PCK_INFO_H_DLL,
	PCK_INFO_H_CRC
};

enum PckSSHeader
{
	PCK_SS_H_REQUEST,
	PCK_SS_H_REQUEST_ERROR,
	PCK_SS_H_REQUEST_SUCCESS
};

#pragma pack(push, 1)

typedef struct tPCK_KEY
{
	UCHAR header;
	UCHAR key[256];

	tPCK_KEY()
	{
		header = PCK_H_ENCRYPTION_KEY;
		memset(key, 0, 256);
	}

	tPCK_KEY(UCHAR KEY[256])
	{
		header = PCK_H_ENCRYPTION_KEY;
		memcpy_s(key, 256, KEY, 256);
	}
} PCK_KEY;

typedef struct tPCK_ENCRYPTED_MESSAGE
{
	UCHAR header;
	UCHAR iv[16];
	UCHAR buffer[MAX_BUFFER];

	tPCK_ENCRYPTED_MESSAGE(UCHAR * IV, UCHAR * Buffer, UINT Size)
	{
		header = PCK_H_ENCRYPTED_MESSAGE;
		memcpy_s(iv, 16, IV, 16);
		memcpy_s(buffer, MAX_BUFFER, Buffer, Size);
	}
} PCK_ENCRYPTED_MESSAGE;

typedef struct tPCK_PING
{
	UCHAR header;

	tPCK_PING()
	{
		header = PCK_H_PING;
	}
} PCK_PING;

typedef struct tPCK_ID
{
	UCHAR header;
	UCHAR buffer[20];

	tPCK_ID()
	{
		header = PCK_H_ID;
	}

	tPCK_ID(UCHAR Buffer[20])
	{
		header = PCK_H_ID;
		memcpy_s(buffer, 20, Buffer, 20);
	}
} PCK_ID;

typedef struct tPCK_USER_ID
{
	UCHAR header;
	UCHAR size;
	UCHAR buffer[255];

	tPCK_USER_ID()
	{
		header = PCK_H_USER_ID;
		size = 0;
		memset(buffer, 0, 255);
	}

	tPCK_USER_ID(std::string UserID)
	{
		header = PCK_H_USER_ID;

		if (UserID.size() <= 250)
		{
			size = (UCHAR)UserID.size();
			memset(buffer, 0, 255);
			memcpy_s(buffer, 255, UserID.c_str(), UserID.size());
		}
		else
		{
			size = 0;
			memset(buffer, 0, 255);
		}
	}
} PCK_USER_ID;

typedef struct tPCK_GAME_SERIAL
{
	UCHAR header;
	UCHAR size;
	UCHAR buffer[255];

	tPCK_GAME_SERIAL()
	{
		header = PCK_H_GAME_SERIAL;
		size = 0;
		memset(buffer, 0, 255);
	}

	tPCK_GAME_SERIAL(std::string GameSerial)
	{
		header = PCK_H_GAME_SERIAL;
		size = GameSerial.size();
		memset(buffer, 0, 255);
		memcpy_s(buffer, 255, GameSerial.c_str(), GameSerial.size());
	}
} PCK_GAME_SERIAL;

typedef struct tPCK_CRC
{
	UCHAR header;
	UCHAR crc[20];
	UCHAR name_length;
	wchar_t name[MAX_PATH];	

	tPCK_CRC()
	{
		header = PCK_H_CRC;
		name_length = 0;
		memset(crc, 0, 20);
		memset(name, 0, MAX_PATH);
	}
	
	tPCK_CRC(std::wstring fileName, UCHAR fileCRC[20])
	{
		header = PCK_H_CRC;
		name_length = fileName.size();
		memcpy_s(crc, 20, fileCRC, 20);
		memset(name, 0, MAX_PATH);
		memcpy_s(name, MAX_PATH * sizeof(wchar_t), fileName.c_str(), fileName.size() * sizeof(wchar_t));
	}
} PCK_CRC;

typedef struct tPCK_ENTRY
{
	UCHAR header;
	UCHAR type;
	UCHAR data[20];

	tPCK_ENTRY()
	{
		header = PCK_H_ENTRY;
		type = 0x00;
		memset(data, 0, 20);
	}

	tPCK_ENTRY(UCHAR newtype, UCHAR * newdata)
	{
		header = PCK_H_ENTRY;
		type = newtype;
		memset(data, 0, 20);
		memcpy_s(data, 20, newdata, 20);
	}
} PCK_ENTRY;

typedef struct tPCK_INFO
{
	UCHAR header;
	UCHAR header_info;
	UCHAR size;
	UCHAR buffer[MAX_PATH];

	tPCK_INFO()
	{
		header = PCK_H_INFO;
		size = 0;
		memset(buffer, 0, MAX_PATH);
	}

	tPCK_INFO(UCHAR info_header, std::string info)
	{
		header = PCK_H_INFO;
		header_info = info_header;
		size = info.size();
		memset(buffer, 0, MAX_PATH);
		memcpy_s(buffer, MAX_PATH, info.c_str(), info.size());
	}
} PCK_INFO;

typedef struct tPCK_SS
{
	UCHAR header;
	UCHAR header_info;
	UCHAR ss_quality;
	UCHAR ss_grayscale;
	UINT buffer_size;
	UCHAR buffer[MAX_SS_BUFFER];

	tPCK_SS()
	{
		header = PCK_H_SS;
		ss_quality = 0;
		ss_grayscale = 0;
		buffer_size = 0;
	}

	tPCK_SS(UCHAR info_header, UCHAR quality, UCHAR grayscale, ghp_UTILS::DataChunk * chunk)
	{
		header = PCK_H_SS;
		header_info = info_header;
		ss_quality = quality;
		ss_grayscale = grayscale;

		if (chunk == NULL)
		{
			buffer_size = 0;
			memset(buffer, 0, MAX_SS_BUFFER);
		}
		else
		{
			buffer_size = chunk->size;

			if (buffer_size >= MAX_SS_BUFFER)
			{
				header = PCK_H_SS;
				header_info = PCK_SS_H_REQUEST_ERROR;
				ss_quality = 0;
				memset(buffer, 0, MAX_SS_BUFFER);
			}
			else
				memcpy_s(buffer, MAX_SS_BUFFER, chunk->data, buffer_size);
		}
	}
} PCK_SS;

typedef struct tPCK_VERIFY
{
	UCHAR header;
	UCHAR garbage_1;
	UCHAR garbage_2;
	UINT data_1;
	UCHAR garbage_3;
	UCHAR garbage_4;
	UINT data_2;
	UCHAR garbage_5;
	UCHAR garbage_6;
	UINT data_3;
	UCHAR garbage_7;
	UCHAR garbage_8;

	tPCK_VERIFY()
	{
		header = PCK_H_VERIFY;
		srand((UINT)time((time_t)0));
		data_1 = ghp_CRYPTO::EncryptDWORD(rand() % 4294967296);
		data_2 = ghp_CRYPTO::EncryptDWORD(rand() % 4294967296);
		data_3 = ghp_CRYPTO::EncryptDWORD(rand() % 4294967296);
		GenGarbage();
	}

	inline void GenGarbage()
	{
		garbage_1 = rand() % 256;
		garbage_2 = rand() % 256;
		garbage_3 = rand() % 256;
		garbage_4 = rand() % 256;
		garbage_5 = rand() % 256;
		garbage_6 = rand() % 256;
		garbage_7 = rand() % 256;
		garbage_8 = rand() % 256;
	}
} PCK_VERIFY;

typedef struct tPCK_VERIFY_RET
{
	UCHAR header;
	UCHAR garbage_1;
	UCHAR garbage_2;
	UINT data_1;
	UCHAR garbage_3;
	UCHAR garbage_4;
	UINT data_2;
	UCHAR garbage_5;
	UCHAR garbage_6;
	UINT data_3;
	UCHAR garbage_7;
	UCHAR garbage_8;

	tPCK_VERIFY_RET()
	{
	}

	tPCK_VERIFY_RET(UINT d1, UINT d2, UINT d3)
	{
		header = PCK_H_VERIFY_RET;
		data_1 = ghp_CRYPTO::EncryptDWORD(d1);
		data_2 = ghp_CRYPTO::EncryptDWORD(d2);
		data_3 = ghp_CRYPTO::EncryptDWORD(d3);
		GenGarbage();
	}

	inline void GenGarbage()
	{
		garbage_1 = rand() % 256;
		garbage_2 = rand() % 256;
		garbage_3 = rand() % 256;
		garbage_4 = rand() % 256;
		garbage_5 = rand() % 256;
		garbage_6 = rand() % 256;
		garbage_7 = rand() % 256;
		garbage_8 = rand() % 256;
	}
} PCK_VERIFY_RET;

#pragma pack(pop)

#endif