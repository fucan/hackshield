#ifndef _GHP_SYSTEM_H_
#define _GHP_SYSTEM_H_

#include "..\include.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include "ghp_DEFS.h"

namespace ghp_UTILS
{
	extern bool PrivEnable(std::wstring PrivilegeName);
	extern bool HaveAdminPrivileges();
	extern std::list<std::string> GetHWIDs();
	extern void GetAllFiles(std::vector<std::wstring> &files, std::wstring directory);
#ifdef R_WARZ
	extern std::vector<DataChunk> GetScreenshot(int quality, int grayscale, LPDIRECT3DDEVICE9 dev);
#else
	extern std::vector<DataChunk> GetScreenshot(int quality, int grayscale);
#endif
}

#endif