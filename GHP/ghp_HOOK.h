#ifndef _GHP_HOOK_H_
#define _GHP_HOOK_H_

#include "..\include.h"

extern DWORD lastCore, lastNET;
extern UINT HookAPI();

extern DWORD	(WINAPI * ori_timeGetTime)	();
extern VOID		(WINAPI * ori_ExitProcess)	(UINT uExitCode);

#endif