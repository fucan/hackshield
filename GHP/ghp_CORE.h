#ifndef _GHP_CORE_H_
#define _GHP_CORE_H_

#include "..\include.h"
#include "ghp_HOOK.h"
#include "..\ghp_UTILS\ghp_LOG.h"
#include "..\ghp_UTILS\ghp_LIC.h"
#include "..\ghp_UTILS\ghp_DB.h"

enum GHP_ERRORS
{
	ERROR_INVALID_LIC,
	ERROR_INVALID_DB,
	ERROR_WHILE_HOOKING,
	ERROR_INSUFFICIENT_PRIVILEGES,
	ERROR_WHILE_CONNECTING_TO_SERVER,
	ERROR_DISCONNECTED_FROM_SERVER,
	ERROR_CORRUPTED_PACKET,
	ERROR_STOPPED_WORKING,
	ERROR_PROCESS_SCAN,
	ERROR_CHEAT_FOUND,
	ERROR_WHILE_CREATING_THREAD,
	ERROR_MEMORY_PROTECTED_WAS_CORRUPTED,
	ERROR_CRC_INVALID,
	ERROR_SPEED_SCAN,
	ERROR_TIMER_CHECK,
	ERROR_INVALID_API_KEY,
	ERROR_GHP_BIN,
	ERROR_WHITELIST,
	ERROR_DECRYPT_FAILED
};

extern HANDLE hGHP;

extern ghp_UTILS::Log ghpLog;
extern ghp_UTILS::Lic * ghpLic;
extern ghp_UTILS::DB * ghpDB;

extern ghp_UTILS::DataChunk GameSerial;
extern std::string GameLogin;
extern std::string LastGameLogin;
extern std::list<std::string> hwids;
extern std::vector<ghp_UTILS::CRC> crc;
extern std::vector<ghp_UTILS::Database> ghpsrv_hacks;

extern DWORD lastLoop, mainThread;
extern bool loadlock;
#ifdef R_WARZ
extern LPDIRECT3DDEVICE9 d3ddev;
#endif

extern DWORD WINAPI GHPCore(LPVOID lpParam);

extern void Start();
extern void Stop();
extern void Error(USHORT id, USHORT code, DWORD error_code = 0);

#endif