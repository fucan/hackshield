#ifndef _SERVER_CORE_H_
#define _SERVER_CORE_H_

#include "..\include.h"
#include "..\ghp_UTILS\ghp_DEFS.h"

#include <fstream>
#include <RakNet\RakPeerInterface.h>
#include <RakNet\MessageIdentifiers.h>
#include <RakNet\RakSleep.h>
#include <RakNet\BitStream.h>

#pragma comment (lib, "RakNetLibStatic.lib")

typedef struct tPlayer
{
	RakNet::RakNetGUID guid;
	std::string userid, ip;
	std::vector<std::string> hwids;
	DWORD connectionTime, lastPing, lastSS;
	int connectionPing;
	HANDLE mutex;
	UCHAR key[256];
	UINT ping_data1, ping_data2, ping_data3;
	
	tPlayer()
	{
		userid = "";
		mutex = NULL;
		ping_data1 = 0;
		ping_data2 = 0;
		ping_data3 = 0;
	}
	
	tPlayer(RakNet::RakNetGUID GUID, std::string IP, UCHAR * KEY)
	{
		memcpy_s(key, 256, KEY, 256);
		guid = GUID;
		userid = "";
		mutex = NULL;
		ip = IP;
		hwids.clear();
		lastPing = timeGetTime();
		connectionTime = lastPing;
		lastSS = lastPing;
		connectionPing = 0;
		ping_data1 = 0;
		ping_data2 = 0;
		ping_data3 = 0;
	}
} Player;

enum BanType { BAN_HWID, BAN_USERID, BAN_IP, BAN_NONE };
enum BanResult { BANR_OK, BANR_ERROR, BANR_ALREADY_BANNED };

extern RakNet::RakPeerInterface * peer;

extern std::string RawHWIDtoFmtHWID(std::string HWID);
extern BanResult BanData(BanType banType, std::string data, std::string reason, std::string ip, std::string userid);
extern std::vector<std::string> GetHWIDSFromFile(std::string userid);

extern int BIND_GHPSERVER;
extern bool acc_autoban, hwid_autoban, ip_autoban, call_api, force_api;
extern std::string ACC_BAN_URL;
extern std::string STATUS_NOTIFY;

extern std::string GAME_SERIAL;
extern std::vector<Player> players;
extern std::vector<ghp_UTILS::CRC> crc;
extern std::vector<ghp_UTILS::Database> hacks;
extern DWORD WINAPI PlayerCheckWorkerThread(LPVOID plParam);
extern DWORD WINAPI ServerWorkerThread(LPVOID lpParam);

extern Player GetPlayerByGUID(std::string GUID);
extern Player GetPlayerByUserID(std::string USERID);

extern bool SetPlayerLastSSByGUID(RakNet::RakNetGUID GUID);

extern bool acc_multisession, hwid_multisession;

#endif _TCP_SERVER_H_