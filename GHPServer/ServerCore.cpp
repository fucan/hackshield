#include "ServerCore.h"
#include "..\ghp_UTILS\ghp_PROTOCOL.h"
#include "..\ghp_UTILS\ghp_LOG.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include "..\ghp_NET\ghp_HTTPCLIENT.h"

extern std::string BLCK_BY_ADMIN_REASON_LNG, BLCK_HACK_USE_LNG;
extern int max_connections, ss_quality, ss_grayscale, auto_ss_interval;

RakNet::RakPeerInterface * peer;

bool acc_multisession = false;
bool hwid_multisession = false;

std::string GAME_SERIAL;
std::vector<Player> players;
std::vector<ghp_UTILS::CRC> crc;
std::vector<ghp_UTILS::Database> hacks;

CRITICAL_SECTION critical; 
ghp_UTILS::Log servLog(L"GHPServer.log");

DWORD WINAPI APIWorkerThread(LPVOID lpParam)
{
	char * URL = (char*)lpParam;
	CURL * curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, URL);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "GHP_NET");
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 15);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10);

	CURLcode res = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);

	free(lpParam);
	return 0;
}

void CallAPI(std::string URL)
{
	int size = URL.size() + 1;
	char * url = (char*)malloc(size);
	memset(url, 0, size);
	memcpy_s(url, size, URL.c_str(), URL.size());
	CreateThread(0, 0, APIWorkerThread, url, 0, 0);
}

// ##############################################################################
// Critical Section functions
void EnterCritical()
{
	while (true)
	{
		if (TryEnterCriticalSection(&critical) == TRUE)
			return;
	}
}
void LeaveCritical()
{
	LeaveCriticalSection(&critical);
}
// ##############################################################################
// Core functions

void SendPacket(UCHAR * key, RakNet::AddressOrGUID addr, LPVOID Packet, UINT size, bool sendNow = false)
{
	RakNet::BitStream bsOut;

	// Key? Encrypta
	/*if (key != 0)
	{
		UCHAR * out = new UCHAR[size];
		UCHAR iv[16], ivTemp[16];

		// Cria o IV
		for (UCHAR i = 0; i < 16; i++)
		{
			iv[i] = rand() % 256;
			ivTemp[i] = iv[i];
		}

		// Encripta
		size_t ivoff = 0;
		aes_context ctx;
		aes_setkey_enc(&ctx, key, 256);
		int ret = aes_crypt_cfb128(&ctx, AES_ENCRYPT, size, &ivoff, iv, (UCHAR*)Packet, out);

		if (ret == 0)
		{
			PCK_ENCRYPTED_MESSAGE encrypted(ivTemp, out, size);
			bsOut.Write((char *)&encrypted, size + 17);
		}
		else
			bsOut.Write((char *)Packet, size);

		delete [] out;
	}
	else*/
		bsOut.Write((char *)Packet, size);
	
	if (sendNow)
		peer->Send(&bsOut, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
	else
		peer->Send(&bsOut, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
}

void AddInfoLog(std::wstring type, std::string data, std::string info)
{
	std::wstring wdata(data.begin(), data.end());
	ghp_UTILS::Log log(std::wstring(L".\\data\\logs\\" + type + L"\\" + wdata + L".log"), true);
	log.LineOut(true, "%s", info.c_str());
}

void InfoLog(Player player, std::string info)
{
	std::vector<std::string> hwids = GetHWIDSFromFile(player.userid);
	for (UINT i = 0; i < hwids.size(); i++)
	{
		AddInfoLog(L"hwid", hwids[i], info);
	}

	if (player.userid != "")
		AddInfoLog(L"login", player.userid, info);
	AddInfoLog(L"ip", player.ip, info);
}

void SetLoginDateTime(std::string userid)
{
	time_t raw;
	time(&raw);
		
	char buff[64];
	sprintf_s(buff, "%I64u", raw);
	
	std::string path = ".\\data\\info\\" + userid + ".ini";
	WritePrivateProfileStringA("info", "lastlogin", buff, path.c_str());
}

bool IsDataBanned(BanType banType, std::string data)
{
	std::string banTypeS = "";

	switch(banType)
	{
	case BAN_HWID:
		banTypeS = "hwid\\";
		break;
	case BAN_USERID:
		banTypeS = "login\\";
		break;
	case BAN_IP:
		banTypeS = "ip\\";
		break;
	default:
		banTypeS = "none\\";
		break;
	}

	std::string path = ".\\data\\block\\" + banTypeS + data + ".ini";
	int banned = GetPrivateProfileIntA("block", "status", 0, path.c_str());

	if (banned == 1)
		return true;
	else
		return false;
}

BanResult BanData(BanType banType, std::string data, std::string reason, std::string ip, std::string userid)
{
	// Verifica se a data é valida
	if (data == "no data")
		return BANR_ERROR;

	// Se estiver bloqueando o player e estiver ativo a api, chama a api (cuidado com ping alto)
	if ((banType == BAN_USERID || banType == BAN_NONE) && ACC_BAN_URL != "" && (call_api == true || force_api == true))
	{
		ghp_NET::HTTPClient http;
		std::string ret;

		char link[4096];
		sprintf_s(link, ACC_BAN_URL.c_str(), userid.c_str(), reason.c_str());
		http.GetURLToString(link, &ret);
	}

	if (!IsDataBanned(banType, data))
	{
		std::wstring wdata(data.begin(), data.end());
		std::string banTypeS = "";

		switch(banType)
		{
		case BAN_HWID:
			banTypeS = "hwid\\";
			break;
		case BAN_USERID:
			banTypeS = "login\\";
			break;
		case BAN_IP:
			banTypeS = "ip\\";
			break;
		default:
			return BANR_OK;
			break;
		}

		std::string path = ".\\data\\block\\" + banTypeS + data + ".ini";

		if (WritePrivateProfileStringA("block", "status", "1", path.c_str()) == FALSE)
			return BANR_ERROR;
		if (WritePrivateProfileStringA("block", "reason", reason.c_str(), path.c_str()) == FALSE)
			return BANR_ERROR;
		if (WritePrivateProfileStringA("block", "ip", ip.c_str(), path.c_str()) == FALSE)
			return BANR_ERROR;
		if (WritePrivateProfileStringA("block", "userid", userid.c_str(), path.c_str()) == FALSE)
			return BANR_ERROR;

		// Salva no log que foi banido
		switch (banType)
		{
		case BAN_HWID:
		{
			ghp_UTILS::Log log(std::wstring(L".\\data\\logs\\hwid\\" + wdata + L".log"), true);
			log.LineOut(true, "HWID banned: %s - %s", reason.c_str(), data.c_str());
			servLog.LineOut(true, "HWID banned: %s - %s", reason.c_str(), data.c_str());
		}
		break;
		case BAN_USERID:
		{
			ghp_UTILS::Log log(std::wstring(L".\\data\\logs\\login\\" + wdata + L".log"), true);
			log.LineOut(true, "USERID banned: %s - %s", reason.c_str(), userid.c_str());
			servLog.LineOut(true, "USERID banned: %s - %s", reason.c_str(), userid.c_str());
		}
		break;
		case BAN_IP:
		{
			ghp_UTILS::Log log(std::wstring(L".\\data\\logs\\ip\\" + wdata + L".log"), true);
			log.LineOut(true, "IP banned: %s - %s", reason.c_str(), ip.c_str());
			servLog.LineOut(true, "IP banned: %s - %s", reason.c_str(), ip.c_str());
		}
		break;
		default:
			break;
		}

		return BANR_OK;
	}
	else
		return BANR_ALREADY_BANNED;
}

std::string RawHWIDtoFmtHWID(std::string HWID)
{
	ghp_UTILS::DataChunk hwid_chunk;
	std::string hwid_fmt = "";

	if (!hwid_chunk.Init((LPVOID)HWID.c_str(), HWID.size()))
		return "";
	
	hwid_fmt = ghp_CRYPTO::GetFormatedSHA1(&hwid_chunk);
	hwid_chunk.FreeAll();

	return hwid_fmt;
}

std::vector<std::string> GetHWIDSFromFile(std::string userid)
{
	std::vector<std::string> ret;
	ret.clear();

	char hwid_buffer[128];
	char itoa_buffer[16];

	memset(hwid_buffer, 0, 128);
	memset(itoa_buffer, 0, 16);

	std::string path = ".\\data\\hwid\\" + userid + ".ini";

	int count = GetPrivateProfileIntA("hwids", "count", 0, path.c_str());

	if (count == 0)
		return ret;
	else
	{
		for (int i = 0; i < count; i++)
		{
			_itoa_s(i+1, itoa_buffer, 10);

			GetPrivateProfileStringA("hwids", itoa_buffer, "", hwid_buffer, 128, path.c_str());

			if (strcmp(hwid_buffer, "") == 0)
				break;
			else
			{
				ret.push_back(hwid_buffer);
			}
		}

		return ret;
	}
}

bool IsHWIDSaved(std::string userid, std::string HWID)
{
	std::vector<std::string> hwids = GetHWIDSFromFile(userid);

	if (hwids.size() > 0)
	{
		for (UINT i = 0; i < hwids.size(); i++)
		{
			if (hwids[i] == HWID)
				return true;
		}

		return false;
	}
	else
		return false;
}

void SaveHWID(std::string userid, std::string HWID)
{
	char itoa_buffer[16];
	memset(itoa_buffer, 0, 16);

	std::string path = ".\\data\\hwid\\" + userid + ".ini";

	int count = GetPrivateProfileIntA("hwids", "count", 0, path.c_str());
	
	_itoa_s(count+1, itoa_buffer, 10);
	WritePrivateProfileStringA("hwids", "count", itoa_buffer, path.c_str());
	WritePrivateProfileStringA("hwids", itoa_buffer, HWID.c_str(), path.c_str());
}

void SaveHWIDs(Player player)
{
	if (player.userid == "" || player.hwids.size() == 0)
		return;
	
	std::string hwid_fmt = "";

	for (UINT i = 0; i < player.hwids.size(); i++)
	{
		hwid_fmt = RawHWIDtoFmtHWID(player.hwids[i]);

		// Se o HWID não foi inserido na lista, insere.
		if (!IsHWIDSaved(player.userid, hwid_fmt))
		{
			SaveHWID(player.userid, hwid_fmt);
		}
	}
}

void AddPlayer(RakNet::RakNetGUID guid, std::string ip, UCHAR key[256])
{
	EnterCritical();

	players.push_back(Player(guid, ip, key));

	LeaveCritical();
}

bool RemovePlayerByGUID(RakNet::RakNetGUID GUID)
{
	EnterCritical();

	// ###########################################################################
	for (UINT i = 0; i < players.size(); i++)
	{
		if (players[i].guid == GUID)
		{
			// Chama a API
			if (players[i].mutex != NULL)
			{
				std::string mutexName = "ghp_pl_" + players[i].userid;
				HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, false, mutexName.c_str());

				if (ReleaseMutex(hMutex) == FALSE)
					servLog.LineOut(true, "Player release mutex error: %s (%d)", players[i].userid.c_str(), GetLastError());

				CloseHandle(players[i].mutex);
				CloseHandle(hMutex);
				players[i].mutex = NULL;
			}

			// Atualiza o horário do login e notifica
			if (players[i].userid != "")
			{
				SetLoginDateTime(players[i].userid);

				if (STATUS_NOTIFY != "" && (call_api == true || force_api == true))
				{
					//ghp_NET::HTTPClient http;
					//std::string ret;

					char link[4096];
					sprintf_s(link, STATUS_NOTIFY.c_str(), players[i].userid.c_str(), "0");
					CallAPI(link);
					/*if (!http.GetURLToString(link, &ret) || ret != "0")
						servLog.LineOut(true, "Player disconnection notify error: %s >> %s", players[i].userid.c_str(), ret.c_str());*/
				}
			}
			
			players.erase(players.begin() + i);
			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

RakNet::RakNetGUID GetPlayerGUIDByHWID(std::string HWID)
{
	EnterCritical();

	for (UINT i = 0; i < players.size(); i++)
	{
		for (UINT j = 0; j < players[i].hwids.size(); j++)
		{
			if (players[i].hwids[j] == HWID)
			{
				LeaveCritical();
				return players[i].guid;
			}
		}
	}

	LeaveCritical();
	return RakNet::RakNetGUID();
}

RakNet::RakNetGUID GetPlayerGUIDByUserID(std::string UserID)
{
	EnterCritical();

	for (UINT i = 0; i < players.size(); i++)
	{
		if (players[i].userid == UserID)
		{
			LeaveCritical();
			return players[i].guid;
		}
	}

	LeaveCritical();
	return RakNet::RakNetGUID();
}

Player GetPlayerByGUID(std::string GUID)
{
	EnterCritical();

	for (UINT i = 0; i < players.size(); i++)
	{
		if (GUID == players[i].guid.ToString())
		{
			LeaveCritical();
			return players[i];
		}
	}

	LeaveCritical();
	return Player();
}

Player GetPlayerByUserID(std::string USERID)
{
	EnterCritical();

	for (UINT i = 0; i < players.size(); i++)
	{
		if (USERID == players[i].userid)
		{
			LeaveCritical();
			return players[i];
		}
	}

	LeaveCritical();
	return Player();
}

bool AssignHWIDToPlayer(RakNet::RakNetGUID GUID, std::string hwid)
{
	EnterCritical();

	if (!hwid_multisession)
	{
		// Procura o ID no vetor
		RakNet::RakNetGUID guid = GetPlayerGUIDByHWID(hwid);

		// Verifica se encontrou algum player
		if (guid.systemIndex != RakNet::UNASSIGNED_PLAYER_INDEX) // RakNet::UNASSIGNED_PLAYER_INDEX == não encontrado
		{
			LeaveCritical();
			return false;
		}
	}

	// Adiciona o ID ao player
	for (UINT k = 0; k < players.size(); k++)
	{
		if (players[k].guid == GUID)
		{
			players[k].hwids.push_back(hwid);

			// Salva a lista de HWID's
			SaveHWIDs(players[k]);

			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

bool AssignUserIDToPlayer(RakNet::RakNetGUID GUID, std::string UserID)
{
	EnterCritical();

	// Chama a API
	if (STATUS_NOTIFY != "" && (call_api == true || force_api == true))
	{
		//ghp_NET::HTTPClient http;
		std::string ret;

		char link[4096];
		sprintf_s(link, STATUS_NOTIFY.c_str(), UserID.c_str(), "1");
		CallAPI(link);
		/*if (!http.GetURLToString(link, &ret) || ret != "0")
		{
			servLog.LineOut(true, "Player connection notify error: %s >> %s", UserID.c_str(), ret.c_str());
			peer->CloseConnection(GUID, true);
			LeaveCritical();
			return true;
		}*/
	}

	if (!acc_multisession)
	{
		// Procura o ID no vetor
		RakNet::RakNetGUID guid = GetPlayerGUIDByUserID(UserID);

		// Verifica se encontrou algum player
		if (guid.systemIndex != RakNet::UNASSIGNED_PLAYER_INDEX && guid != GUID) // RakNet::UNASSIGNED_PLAYER_INDEX == não encontrado
		{
			LeaveCritical();
			return false;
		}
	}

	// Seta o UserID do player
	for (UINT k = 0; k < players.size(); k++)
	{
		if (players[k].guid == GUID)
		{
			players[k].userid = UserID;

			// Cria o Mutex do player
			std::string mutex = "ghp_pl_" + UserID;
			players[k].mutex = CreateMutexA(0, true, mutex.c_str());

			if (players[k].mutex == NULL)
				servLog.LineOut(true, "Player mutex create error: %s (%d)", mutex.c_str(), GetLastError());

			// Salva a lista de HWID's
			SaveHWIDs(players[k]);

			// Atualiza o horário do login
			SetLoginDateTime(UserID);

			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

bool SetPlayerPingByGUID(RakNet::RakNetGUID GUID, int ConnectionPing)
{
	EnterCritical();

	// Seta o lastPing do player
	for (UINT i = 0; i < players.size(); i++)
	{
		if (players[i].guid == GUID)
		{
			players[i].lastPing = timeGetTime();
			players[i].connectionPing = ConnectionPing;

			// Verifica se precisa requisitar uma nova screenshot
			if (auto_ss_interval > 0)
			{
				if (players[i].lastPing - players[i].lastSS > ((DWORD)auto_ss_interval * 60000))
				{
					players[i].lastSS = players[i].lastPing;

					PCK_SS packet_ss(PCK_SS_H_REQUEST, (UCHAR)ss_quality, (UCHAR)ss_grayscale, NULL);
					SendPacket(players[i].key, players[i].guid, &packet_ss, packet_ss.buffer_size + 8);
				}
			}

			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

bool SetPlayerLastSSByGUID(RakNet::RakNetGUID GUID)
{
	EnterCritical();

	// Seta o lastPing do player
	for (UINT i = 0; i < players.size(); i++)
	{
		if (players[i].guid == GUID)
		{
			players[i].lastSS = timeGetTime();
			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

bool SetPlayerPingDataInfo(RakNet::RakNetGUID GUID, PCK_VERIFY * info)
{
	EnterCritical();

	// Seta o ping data info do player
	for (UINT i = 0; i < players.size(); i++)
	{
		if (players[i].guid == GUID)
		{
			players[i].ping_data1 = info->data_1;
			players[i].ping_data2 = info->data_2;
			players[i].ping_data3 = info->data_3;
			LeaveCritical();
			return true;
		}
	}

	LeaveCritical();
	return false;
}

// ##############################################################################

bool MakePacket(LPVOID DestBuffer, UINT DestSize, LPVOID OrigBuffer, UINT OrigSize, bool IgnoreSize = false)
{
	if (!IgnoreSize && (DestSize != OrigSize))
		return false;

	memcpy_s(DestBuffer, DestSize, OrigBuffer, OrigSize);
	return true;
}

DWORD WINAPI PlayerCheckWorkerThread(LPVOID plParam)
{
	while(true)
	{
		DWORD now = timeGetTime();

		EnterCritical();
		for (UINT i = 0; i < players.size(); i++)
		{
			if (now - players[i].lastPing > 30000)
			{
				RakNet::RakNetGUID guid = players[i].guid;

				servLog.LineOut(true, "PlayerCheck -> Disconnecting player %s %s", players[i].userid.c_str(), guid.ToString());

				peer->CloseConnection(guid, true);
				if (!RemovePlayerByGUID(guid))
					servLog.LineOut(true, "PLAYERCHECK -> RemovePlayerByGUID error | %s", guid.ToString());
			}
		}
		LeaveCritical();

		Sleep(1000);
	}

	return 0;
}

void BlockAll(Player player)
{
	if (acc_autoban && player.userid != "")
	{
		BanData(BAN_USERID, player.userid, BLCK_HACK_USE_LNG, player.ip, player.userid);
	}
	if (hwid_autoban)
	{
		std::vector<std::string> hwids = GetHWIDSFromFile(player.userid);

		for (UINT i = 0; i < hwids.size(); i++)
		{
			BanData(BAN_HWID, hwids[i], BLCK_HACK_USE_LNG, player.ip, player.userid);
		}
	}
	if (ip_autoban)
	{
		BanData(BAN_IP, player.ip, BLCK_HACK_USE_LNG, player.ip, player.userid);
	}

	if (force_api && player.userid != "")
	{
		BanData(BAN_NONE, player.ip, BLCK_HACK_USE_LNG, player.ip, player.userid);
	}
}

void ProcessPacket(RakNet::Packet * packet, UCHAR * buffer, UINT size)
{
	switch (buffer[0])
	{
	case ID_NEW_INCOMING_CONNECTION:
		{
			// Verifica se o IP está banido
			if (IsDataBanned(BAN_IP, packet->systemAddress.ToString(false)))
			{
				servLog.LineOut(true, "IP banned -> %s", packet->systemAddress.ToString(false));
				peer->CloseConnection(packet->systemAddress, true);
			}
			else
			{
				// Cria a chave do player
				UCHAR key[256];
				for (UINT i = 0; i < 256; i++)
				{
					key[i] = (UCHAR)(rand() % 256);
				}
				
				// Adiciona o player na struct
				AddPlayer(packet->guid, packet->systemAddress.ToString(false), key);

				// Envia a chave para o player
				PCK_KEY player_key(key);
				SendPacket(0, packet->guid, &player_key, 257, true);

				// Envia o serial para o player
				PCK_GAME_SERIAL game_serial(GAME_SERIAL);
				SendPacket(key, packet->guid, &game_serial, game_serial.size + 2, true);

				// Envia os CRC para o player
				for (UINT i = 0; i < crc.size(); i++)
				{
					PCK_CRC file_crc(crc[i].fileName, crc[i].crc);
					SendPacket(key, packet->guid, &file_crc, (file_crc.name_length * sizeof(wchar_t)) + 22);
				}

				// Envia os Hacks para o player
				for (UINT i = 0; i < hacks.size(); i++)
				{
					PCK_ENTRY entry(hacks[i].type, hacks[i].data);
					SendPacket(key, packet->guid, &entry, sizeof(entry));
				}
			}
		}
		break;
	case ID_DISCONNECTION_NOTIFICATION:
		if(!RemovePlayerByGUID(packet->guid))
			servLog.LineOut(true, "ID_DISCONNECTION_NOTIFICATION -> RemovePlayerByGUID error | %s", packet->guid.ToString());
		break;
	case ID_CONNECTION_LOST:
		if (!RemovePlayerByGUID(packet->guid))
			servLog.LineOut(true, "ID_CONNECTION_LOST -> RemovePlayerByGUID error | %s", packet->guid.ToString());
		break;
	case PCK_H_PING:
		{
			PCK_VERIFY packet_verify;
			if (!SetPlayerPingDataInfo(packet->guid, &packet_verify))
			{
				servLog.LineOut(true, "SetPlayerPingDataInfo error -> %s", packet->systemAddress.ToString(false));
				peer->CloseConnection(packet->systemAddress, true);
			}
			else
				SendPacket(0, packet->guid, &packet_verify, sizeof(packet_verify));
		}
		break;
	case PCK_H_VERIFY_RET:
		{
			PCK_VERIFY_RET packet_verify_ret;

			if (!MakePacket(&packet_verify_ret, sizeof(packet_verify_ret), buffer, size))
			{
				servLog.LineOut(true, "PCK_VERIFY_RET error -> %s", packet->systemAddress.ToString(false));
				peer->CloseConnection(packet->systemAddress, true);
			}
			else
			{
				Player p = GetPlayerByGUID(packet->guid.ToString());

				UINT p_d1 = ghp_CRYPTO::DecryptDWORD(p.ping_data1) ^ VERIFY_XOR_KEY;
				UINT p_d2 = ghp_CRYPTO::DecryptDWORD(p.ping_data2) ^ VERIFY_XOR_KEY;
				UINT p_d3 = ghp_CRYPTO::DecryptDWORD(p.ping_data3) ^ VERIFY_XOR_KEY;

				UINT v_d1 = ghp_CRYPTO::DecryptDWORD(packet_verify_ret.data_1);
				UINT v_d2 = ghp_CRYPTO::DecryptDWORD(packet_verify_ret.data_2);
				UINT v_d3 = ghp_CRYPTO::DecryptDWORD(packet_verify_ret.data_3);

				if (p_d1 == v_d1 && p_d2 == v_d2 && p_d3 == v_d3)
				{
					if(!SetPlayerPingByGUID(packet->guid, peer->GetLastPing(packet->guid)))
						servLog.LineOut(true, "PCK_H_PING -> Set player ping error | %s", packet->guid.ToString());
				}
				else
					servLog.LineOut(true, "PCK_VERIFY_RET failed -> %s", packet->systemAddress.ToString(false));
			}
		}
		break;
	case PCK_H_ID:
		{
			PCK_ID packet_id;

			if (!MakePacket(&packet_id, sizeof(packet_id), buffer, size))
			{
				servLog.LineOut(true, "PCK_H_ID invalid. Size recv: %d", size);
				peer->CloseConnection(packet->guid, true);
			}

			std::string hwid_fmt = RawHWIDtoFmtHWID(std::string((char*)packet_id.buffer, 20));

			// Verifica se o HWID está banido
			if (IsDataBanned(BAN_HWID, hwid_fmt))
			{
				servLog.LineOut(true, "HWID banned -> %s | %s", hwid_fmt.c_str(), packet->systemAddress.ToString(false));
				peer->CloseConnection(packet->guid, true);
			}
			else
			{
				if (!AssignHWIDToPlayer(packet->guid, std::string((char*)packet_id.buffer, 20)))
				{
					servLog.LineOut(true, "HWID already in use | %s", packet->systemAddress.ToString(false));
					peer->CloseConnection(packet->guid, true);
				}
			}
		}
		break;
	case PCK_H_USER_ID:
	{
		PCK_USER_ID packet_user_id;
				
		if (!MakePacket(&packet_user_id, sizeof(packet_user_id), buffer, size))
		{
			servLog.LineOut(true, "PCK_H_USER_ID invalid. Size recv: %d", size);
			peer->CloseConnection(packet->guid, true);
		}

		// Verifica se o USERID está banido
		if (IsDataBanned(BAN_USERID, std::string((char*)packet_user_id.buffer, packet_user_id.size)))
		{
			servLog.LineOut(true, "USERID banned -> %s | %s", std::string((char*)packet_user_id.buffer, packet_user_id.size).c_str(), packet->systemAddress.ToString(false));
			peer->CloseConnection(packet->guid, true);
		}
		else
		{
			if (!AssignUserIDToPlayer(packet->guid, std::string((char*)packet_user_id.buffer, packet_user_id.size)))
			{
				servLog.LineOut(true, "USERID already in use | %s -> %s", packet->systemAddress.ToString(false), std::string((char*)packet_user_id.buffer, packet_user_id.size).c_str());
				peer->CloseConnection(packet->guid, true);
			}
		}
	}
	break;
	case PCK_H_INFO:
	{
		PCK_INFO packet_info;

		if (!MakePacket(&packet_info, sizeof(packet_info), buffer, size, true))
		{
			servLog.LineOut(true, "PCK_INFO invalid. Size recv: %d", size);
		}
		else
		{
			switch (packet_info.header_info)
			{
			case PCK_INFO_H_HACK:
			{
				Player player = GetPlayerByGUID(packet->guid.ToString());

				char info[4096];
				sprintf_s(info, "HACK detected: %s", packet_info.buffer);
				InfoLog(player, info);

				if (player.userid != "")
					servLog.LineOut(true, "HACK detected: %s (%s)", player.userid.c_str(), packet_info.buffer);
				else
					servLog.LineOut(true, "HACK detected: %s (%s)", packet->systemAddress.ToString(false), packet_info.buffer);

				// Bloqueia tudo que estiver ativado
				BlockAll(player);
			}
			break;
			case PCK_INFO_H_DLL:
			{
				Player player = GetPlayerByGUID(packet->guid.ToString());

				char info[4096];
				sprintf_s(info, "DLL detected: %s", packet_info.buffer);
				InfoLog(player, info);

				if (player.userid != "")
					servLog.LineOut(true, "DLL detected: %s (%s)", player.userid.c_str(), packet_info.buffer);
				else
					servLog.LineOut(true, "DLL detected: %s (%s)", packet->systemAddress.ToString(false), packet_info.buffer);

				// Bloqueia tudo que estiver ativado
				BlockAll(player);
			}
			break;
			case PCK_INFO_H_CRC:
			{
				Player player = GetPlayerByGUID(packet->guid.ToString());

				char info[4096];
				sprintf_s(info, "Modified file: %s", packet_info.buffer);
				InfoLog(player, info);

				if (player.userid != "")
					servLog.LineOut(true, "Modified file: %s (%s)", player.userid.c_str(), packet_info.buffer);
				else
					servLog.LineOut(true, "Modified file: %s (%s)", packet->systemAddress.ToString(false), packet_info.buffer);
			}
			break;
			default:
				servLog.LineOut(true, "PCK_INFO invalid. Size recv: %d | Header info: ", size, packet_info.header_info);
				break;
			}
		}
	}
	break;
	case PCK_H_SS:
	{
		PCK_SS packet_ss;

		if (!MakePacket(&packet_ss, sizeof(packet_ss), buffer, size, true))
		{
			servLog.LineOut(true, "PCK_SS invalid. Size recv: %d", size);
		}
		else
		{
			switch (packet_ss.header_info)
			{
			case PCK_SS_H_REQUEST_ERROR:
			{
				Player player = GetPlayerByGUID(packet->guid.ToString());
				InfoLog(player, "SCREENSHOT request error");

				servLog.LineOut(true, "SCREENSHOT request error: %s (%s)", player.ip.c_str(), player.userid == "" ? "?" : player.userid.c_str());
			}
			break;
			case PCK_SS_H_REQUEST_SUCCESS:
			{
				Player player = GetPlayerByGUID(packet->guid.ToString());

				if (player.userid == "")
				{
					InfoLog(player, "SCREENSHOT request error");
					servLog.LineOut(true, "SCREENSHOT request error: %s (%s)", player.ip.c_str(), player.userid == "" ? "?" : player.userid.c_str());
					return;
				}
						
				ghp_UTILS::DataChunk ss;
				ss.Init(packet_ss.buffer, packet_ss.buffer_size);
						
				// Salva a SS
				time_t rawtime;
				struct tm timeinfo;
				time(&rawtime);
				localtime_s(&timeinfo, &rawtime);

				WCHAR buff[MAX_PATH];
						
				std::wstring wuserid(player.userid.begin(), player.userid.end());

				// Força o diretorio
				swprintf_s(buff, L".\\data\\ss\\%ls", wuserid.c_str());
				_wmkdir(buff);

				int id = 0;
				bool free_name = false;

				while (!free_name)
				{
					swprintf_s(buff, L".\\data\\ss\\%ls\\%04d_%02d_%02d_%02d_%02d_%02d_%d.jpg", wuserid.c_str(), timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, id);
					ghp_UTILS::DataChunk tempchunk;
					if (ghp_UTILS::LoadFileData(buff, &tempchunk))
					{
						tempchunk.FreeAll();
						id++;
					}
					else
						free_name = true;
				}
						
				ghp_UTILS::WriteFileData(buff, &ss);
				ss.FreeAll();
			}
			break;
			default:
				servLog.LineOut(true, "PCK_SS invalid. Size recv: %d | Header info: ", size, packet_ss.header_info);
				break;
			}
		}
	}
	break;
	default:
		servLog.LineOut(true, "ERROR | INVALID PACKET: %02X at size %d", buffer[0], size);
		break;
	}
}

DWORD WINAPI ServerWorkerThread(LPVOID lpParam)
{
	srand((UINT)time(0));
	InitializeCriticalSection(&critical);

	peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet * packet;

	// RakNet Keys
	VMProtectBeginUltra("ServerWorkerThread");
	ghp_UTILS::DataChunk raknetkeys;
	UCHAR raknet_publickey[64];
	UCHAR raknet_privatekey[32];

	if (!ghp_UTILS::LoadFileData(L"NET.KEY", &raknetkeys))
	{
		servLog.LineOut(true, "NET.KEY error");
		ExitProcess(0);
	}

	memcpy_s(raknet_publickey, 64, raknetkeys.data, 64);
	raknetkeys.EraseFront(64);
	memcpy_s(raknet_privatekey, 32, raknetkeys.data, 32);
	raknetkeys.FreeAll();
	
	if(!peer->InitializeSecurity((char*)raknet_publickey, (char*)raknet_privatekey))
	{
		servLog.LineOut(true, "peer->InitializeSecurity error");
		ExitProcess(0);
	}
	VMProtectEnd();

	RakNet::SocketDescriptor sd(BIND_GHPSERVER, 0);

	peer->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	peer->SetMaximumIncomingConnections(max_connections);	

	servLog.LineOut(true, "Initializing GHPServer! Max Connections: %d", max_connections);
	RakNet::StartupResult ret = peer->Startup(max_connections, &sd, 1);
	if (ret != RakNet::RAKNET_STARTED)
	{
		servLog.LineOut(true, "peer->Startup error: %d", ret);
		ExitProcess(0);
	}
	
	while (true)
	{
		RakSleep(5);
		
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			switch (packet->data[0])
			{
			case PCK_H_ENCRYPTED_MESSAGE:
				servLog.LineOut(true, "PCK_H_ENCRYPTED_MESSAGE");
				ProcessPacket(packet, packet->data, packet->length);
				break;
			default:
				ProcessPacket(packet, packet->data, packet->length);
			};
		}
	}

	DeleteCriticalSection(&critical);
	RakNet::RakPeerInterface::DestroyInstance(peer);
	return 0;
}