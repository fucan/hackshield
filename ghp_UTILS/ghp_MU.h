#ifndef _GHP_MU_H_
#define _GHP_MU_H_

#include "..\include.h"

namespace ghp_UTILS
{
	extern bool IsSerialPacket(UCHAR * Packet);
	extern std::string SetSerialAndGetLogin(UCHAR * Packet, int len, std::string Serial);
}

#endif