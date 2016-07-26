#ifndef _GHP_NETBOX_H_
#define _GHP_NETBOX_H_

#include "..\include.h"
#include "..\ghp_UTILS\ghp_PROTOCOL.h"

extern DWORD lastPing;
extern bool SerialReceived;

extern void SendInfo(UCHAR header, std::string info);

extern DWORD WINAPI NETBoxThread(LPVOID lpParam);
extern void SendPacket(LPVOID Packet, UINT size, bool SendNow = false);

extern void ShutdownNETBox();

#endif