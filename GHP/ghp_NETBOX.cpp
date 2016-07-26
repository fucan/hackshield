#include "ghp_NETBOX.h"
#include "ghp_CORE.h"
#include "ghp_HOOK.h"
#include "..\ghp_UTILS\ghp_SYSTEM.h"

#include <RakNet\RakPeerInterface.h>
#include <RakNet\MessageIdentifiers.h>
#include <RakNet\RakSleep.h>
#include <RakNet\BitStream.h>

#pragma comment (lib, "RakNetLibStatic.lib")

UCHAR * SRV_KEY = 0;
DWORD lastPing = 0;

bool ConnectionActive = false;
bool SerialReceived = false;
RakNet::RakPeerInterface * peer = 0;
RakNet::RakNetGUID ServerGUID;

inline bool MakePacket(LPVOID DestBuffer, UINT DestSize, LPVOID OrigBuffer, UINT OrigSize, bool IgnoreSize = false)
{
	if (!IgnoreSize && (DestSize != OrigSize))
		return false;

	memcpy_s(DestBuffer, DestSize, OrigBuffer, OrigSize);
	return true;
}

inline void SendPacket(LPVOID Packet, UINT size, bool SendNow)
{
	if (ConnectionActive)
	{
		RakNet::BitStream bsOut;
		bsOut.Write((char *)Packet, size);
		
		if (SendNow)
			peer->Send(&bsOut, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, ServerGUID, false);
		else
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, ServerGUID, false);
	}
}

inline void SendInfo(UCHAR header, std::string data)
{
	PCK_INFO info(header, data);
	SendPacket(&info, info.size + 3, true);
}

inline void ProcessPacket(RakNet::Packet * packet, UCHAR * buffer, UINT size)
{
	switch (buffer[0])
	{
	case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			// Aloca a GUID do server
			ServerGUID = packet->guid;
					
			// Envia os IDs para o servidor
			for (UINT i = 0; i < hwids.size(); i++)
			{
				PCK_ID packet_id((UCHAR*)hwids.front().c_str());
				SendPacket(&packet_id, sizeof(packet_id));
				hwids.pop_front(); // remove o ID da lista
			}
					
			/* Inject USER ID if defined */
			#ifdef D_INJECT_DEBUG_LOGIN
			GameLogin = "Shiu";
			#endif
		}
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 3);
		break;
	case ID_CONNECTION_ATTEMPT_FAILED:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 2);
		break;
	case ID_DISCONNECTION_NOTIFICATION:
		Error(ERROR_DISCONNECTED_FROM_SERVER, 0);
		break;
	case ID_CONNECTION_LOST:
		Error(ERROR_DISCONNECTED_FROM_SERVER, 1);
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 4);
		break;
	case ID_CONNECTION_BANNED:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 5);
		break;
	case ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 6);
		break;
	case ID_OUR_SYSTEM_REQUIRES_SECURITY:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 7);
		break;
	case ID_PUBLIC_KEY_MISMATCH:
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 8);
		break;
	case PCK_H_ENCRYPTION_KEY:
		{
			PCK_KEY server_key;

			if(!MakePacket(&server_key, sizeof(server_key), buffer, size))
			{
				Error(ERROR_CORRUPTED_PACKET, 5);
			}
			else
			{
				if (SRV_KEY == 0)
				{
					SRV_KEY = new UCHAR[256];
					memcpy_s(SRV_KEY, 256, server_key.key, 256);
				}
				else
					Error(ERROR_CORRUPTED_PACKET, 5);
			}
		}
		break;
	case PCK_H_GAME_SERIAL:
		{
			PCK_GAME_SERIAL game_serial;

			if(!MakePacket(&game_serial, sizeof(game_serial), buffer, size, true) || !GameSerial.Init(game_serial.buffer, game_serial.size))
			{
				Error(ERROR_CORRUPTED_PACKET, 0);
			}
			else
			{
				if (GameSerial.size == 16)
				{
					if (std::string((char*)GameSerial.data, 16) != VMProtectDecryptStringA("0000000000000000"))
						SerialReceived = true;
					else
						SerialReceived = false;
				}
				else
					Error(ERROR_CORRUPTED_PACKET, 1);
			}
		}
		break;
	case PCK_H_CRC:
		{
			PCK_CRC crc_file;

			if(!MakePacket(&crc_file, sizeof(crc_file), buffer, size, true))
			{
				Error(ERROR_CORRUPTED_PACKET, 2);
			}
			else
			{
				std::wstring crc_file_name = std::wstring(crc_file.name, crc_file.name_length);
						
				ghp_UTILS::CRC temp;
				temp.fileName = crc_file_name;
				memcpy_s(temp.crc, 20, crc_file.crc, 20);
				crc.push_back(temp);
			}
		}
		break;
	case PCK_H_ENTRY:
		{
			PCK_ENTRY entry;

			if (!MakePacket(&entry, sizeof(entry), buffer, size))
			{
				Error(ERROR_CORRUPTED_PACKET, 3);
			}
			else
			{
				ghp_UTILS::Database db;
				db.type = entry.type;
				memcpy_s(db.data, 20, entry.data, 20);
				ghpsrv_hacks.push_back(db);
			}
		}
		break;
	case PCK_H_SS:
	{
		PCK_SS packet_ss;

		if (!MakePacket(&packet_ss, sizeof(packet_ss), buffer, size, true))
		{
			Error(ERROR_CORRUPTED_PACKET, 4);
		}
		else
		{
			switch (packet_ss.header_info)
			{
			case PCK_SS_H_REQUEST:
			{
#ifdef R_WARZ
				std::vector<ghp_UTILS::DataChunk> ss = ghp_UTILS::GetScreenshot((int)packet_ss.ss_quality, (int)packet_ss.ss_grayscale, d3ddev);
#else
				std::vector<ghp_UTILS::DataChunk> ss = ghp_UTILS::GetScreenshot((int)packet_ss.ss_quality, (int)packet_ss.ss_grayscale);
#endif
						
				for (UINT i = 0; i < ss.size(); i++)
				{
					PCK_SS packet_ss(PCK_SS_H_REQUEST_SUCCESS, 0, 0, &ss[i]);
					SendPacket(&packet_ss, packet_ss.buffer_size + 8);
					ss[i].FreeAll();
				}

				ss.clear();
			}
			break;
			default:
				break;
			}
		}
	}
	break;
	case PCK_H_VERIFY:
		{
			VMProtectBeginUltra("PCK_H_VERIFY");
			PCK_VERIFY packet_verify;
			if (!MakePacket(&packet_verify, sizeof(packet_verify), buffer, size))
			{
				Error(ERROR_CORRUPTED_PACKET, 6);
			}
			else
			{
				PCK_VERIFY_RET packet_verify_ret(ghp_CRYPTO::DecryptDWORD(packet_verify.data_1) ^ VERIFY_XOR_KEY, ghp_CRYPTO::DecryptDWORD(packet_verify.data_2) ^ VERIFY_XOR_KEY, ghp_CRYPTO::DecryptDWORD(packet_verify.data_3) ^ VERIFY_XOR_KEY);
				SendPacket(&packet_verify_ret, sizeof(packet_verify_ret));
				VMProtectEnd();
			}
		}
		break;
	default:
		Error(ERROR_CORRUPTED_PACKET, (DWORD)buffer[0]);
		break;
	}
}

DWORD WINAPI NETBoxThread(LPVOID lpParam)
{
	VMProtectBeginUltra("NETBoxThread 1");
	peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet * packet;

	RakNet::SocketDescriptor sd;

	peer->SetOccasionalPing(true);
	peer->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	
	RakNet::StartupResult startupResult = peer->Startup(1, &sd, 1);
	if (startupResult != RakNet::RAKNET_STARTED)
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 0, startupResult);
	
	RakNet::PublicKey pk;
	pk.remoteServerPublicKey = (char*)ghpLic->GetRakNetPUBKEY();
	pk.publicKeyMode = (RakNet::PublicKeyMode)2; // RakNet::PublicKeyMode::PKM_USE_KNOWN_PUBLIC_KEY

	if (peer->Connect(ghpLic->GetIP().c_str(), (USHORT)ghp_CRYPTO::DecryptDWORD(ghpLic->GetGHPServerRaknet()), "", 0, &pk) != RakNet::CONNECTION_ATTEMPT_STARTED)
		Error(ERROR_WHILE_CONNECTING_TO_SERVER, 1);
	
	ConnectionActive = true;
	VMProtectEnd();
		
	while (ConnectionActive)
	{
		RakSleep(5);
		
		VMProtectBeginVirtualization("NETBoxThread 2");
		DWORD now = ori_timeGetTime();
		
		// Verifica se precisa enviar o ping
		if (now - lastPing > 5000)
		{
			PCK_PING packet_ping;
			SendPacket(&packet_ping, sizeof(packet_ping));

			lastPing = now;
		}
		
		if (GameLogin != LastGameLogin && ServerGUID.systemIndex != RakNet::UNASSIGNED_PLAYER_INDEX)
		{
			if (GameLogin != "")
			{
				PCK_USER_ID packet_user_id(GameLogin);
				SendPacket(&packet_user_id, sizeof(packet_user_id));
			}

			LastGameLogin = GameLogin;
		}
		VMProtectEnd();

		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			/*switch(packet->data[0])
			{
			case PCK_H_ENCRYPTED_MESSAGE:  DISABLED
			{
				VMProtectBeginMutation("PCK_H_ENCRYPTED_MESSAGE");
				if (SRV_KEY != 0)
				{
					UCHAR * out = new UCHAR[packet->length - 17];

					size_t ivoff = 0;
					aes_context ctx;
					aes_setkey_enc(&ctx, SRV_KEY, 256);
					int ret = aes_crypt_cfb128(&ctx, AES_DECRYPT, packet->length - 17, &ivoff, &packet->data[1], &packet->data[17], out);

					if (ret == 0)
						ProcessPacket(packet, out, packet->length - 17);
					else
						Error(ERROR_DECRYPT_FAILED, 1);

					delete [] out;
				}
				else
					Error(ERROR_DECRYPT_FAILED, 0);
				VMProtectEnd();
			}
			break;
			default:*/
				VMProtectBeginVirtualization("NETBoxThread 3");
				ProcessPacket(packet, packet->data, packet->length);
				VMProtectEnd();
				/*break;
			}*/
		}
	}
	
	// Aguarda 500 ms e chama a função que fecha o game
	Sleep(500);
	Stop();

	return 0;
}

// Fecha o NETBox
void ShutdownNETBox()
{
	// Se a conexão estiver ativa, desconecta e destroi as variáveis
	if (ConnectionActive && peer != 0)
	{
		ConnectionActive = false;
		peer->Shutdown(500, '\000', IMMEDIATE_PRIORITY);
		RakNet::RakPeerInterface::DestroyInstance(peer);
		peer = 0;
	}
}