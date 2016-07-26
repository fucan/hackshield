#ifndef _GHP_LIC_H_
#define _GHP_LIC_H_

#include "..\include.h"
#include "ghp_FILE.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"

namespace ghp_UTILS
{
	class Lic
	{
	private:
		std::string name, ip, link, pubkey;
		DWORD apikey, time_1, time_2, ghpserver_raknet, ghpserver_web;
		DataChunk raknetpubkey, dllSignature;
		bool valid, m_loaded;
	public:
		Lic();
		Lic(std::string Name, std::string IP, std::string Link, DWORD APIKey, DWORD Time_1, DWORD Time_2, DWORD GHPServer_Raknet, DWORD GHPServer_Web, std::string PubKey, UCHAR RakNetPubKey[64]);
		
		bool IsLoaded()
		{
			return m_loaded;
		}

		std::string GetNAME();
		std::string GetIP();
		std::string GetLINK();
		std::string GetPUBKEY();
		DWORD GetAPIKey();
		DWORD GetTime1();
		DWORD GetTime2();
		DWORD GetGHPServerRaknet();
		DWORD GetGHPServerWeb();
		UCHAR * GetRakNetPUBKEY();
				
		bool LoadFromFile(std::wstring FileName, UCHAR * key);
		bool SaveToFile(std::wstring FileName, std::string PrivateKey);
	};
}

#endif