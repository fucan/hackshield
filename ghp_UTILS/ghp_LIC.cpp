#include "ghp_LIC.h"

namespace ghp_UTILS
{
	Lic::Lic()
	{
		name = "";
		ip = "";
		link = "";
		apikey = 0;
		time_1 = 0;
		time_2 = 0;
		ghpserver_raknet = 0;
		ghpserver_web = 0;
		pubkey = "";
		m_loaded = false;
	}
	
	Lic::Lic(std::string Name, std::string IP, std::string Link, DWORD APIKey, DWORD Time_1, DWORD Time_2, DWORD GHPServer_Raknet, DWORD GHPServer_Web, std::string PubKey, UCHAR RakNetPubKey[64])
	{
		name = Name;
		ip = IP;
		link = Link;
		apikey = APIKey;
		time_1 = Time_1;
		time_2 = Time_2;
		ghpserver_raknet = GHPServer_Raknet;
		ghpserver_web = GHPServer_Web;
		pubkey = PubKey;
		m_loaded = false;

		raknetpubkey.FreeAll();
		raknetpubkey.Inc(RakNetPubKey, 64);
	}

	std::string Lic::GetNAME()
	{
		return name;
	}

	std::string Lic::GetIP()
	{
		return ip;
	}

	std::string Lic::GetLINK()
	{
		return link;
	}

	std::string Lic::GetPUBKEY()
	{
		return pubkey;
	}

	DWORD Lic::GetAPIKey()
	{
		return apikey;
	}

	DWORD Lic::GetTime1()
	{
		return time_1;
	}

	DWORD Lic::GetTime2()
	{
		return time_2;
	}

	DWORD Lic::GetGHPServerRaknet()
	{
		return ghpserver_raknet;
	}

	DWORD Lic::GetGHPServerWeb()
	{
		return ghpserver_web;
	}

	UCHAR * Lic::GetRakNetPUBKEY()
	{
		return (UCHAR*)raknetpubkey.data;
	}

	bool Lic::LoadFromFile(std::wstring FileName, UCHAR * key)
	{
		VMProtectBeginUltra("Lic::LoadFromFile");
		DataChunk licChunk, signature, chunk_apikey, chunk_time_1, chunk_time_2, chunk_ghpserver_raknet, chunk_ghpserver_web;
		valid = true;
		m_loaded = false;

		dllSignature.FreeAll();

		// Descripta arquivo
		if (!ghp_CRYPTO::DecFileAESToMemory(FileName, &licChunk, key))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}
		
		// Verifica a assinatura
		if (!ghp_CRYPTO::RemoveSignatureFromDataChunk(&licChunk, &signature))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}

		// Salva o chunk, lê os dados e restaura o chunk após a leitura
		if (!licChunk.Backup() || !ReadStringBlock(&licChunk, &name) || !ReadStringBlock(&licChunk, &ip) || !ReadStringBlock(&licChunk, &link) || !ReadDataChunkBlock(&licChunk, &chunk_apikey) || !ReadDataChunkBlock(&licChunk, &chunk_time_1) || !ReadDataChunkBlock(&licChunk, &chunk_time_2) || !ReadDataChunkBlock(&licChunk, &chunk_ghpserver_raknet) || !ReadDataChunkBlock(&licChunk, &chunk_ghpserver_web) || !ReadStringBlock(&licChunk, &pubkey) || !ReadDataChunkBlock(&licChunk, &raknetpubkey) || !ReadDataChunkBlock(&licChunk, &dllSignature) || !licChunk.Restore())
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}

		// Verifica os chunks
		if (chunk_apikey.size != sizeof(apikey) || chunk_time_1.size != sizeof(time_1) || chunk_time_2.size != sizeof(time_2))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}

		// Verifica se a licença é válida
		if (!ghp_CRYPTO::VerifyDataChunkSignature(&licChunk, &signature, VMProtectDecryptStringA(GHP_PUB_KEY), VMProtectDecryptStringA("ghp_rsa")))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}

		// Verifica se a DLL é válida
		if (!ghp_CRYPTO::VerifyFileSignature(VMProtectDecryptStringW(DLL_NAME_VEF), &dllSignature, VMProtectDecryptStringA(GHP_PUB_KEY), VMProtectDecryptStringA("ghp_rsa")))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();
			signature.FreeAll();
			licChunk.FreeAll();
			return false;
		}

		// Define as variáveis
		memcpy_s(&apikey, sizeof(apikey), chunk_apikey.data, chunk_apikey.size);
		memcpy_s(&time_1, sizeof(time_1), chunk_time_1.data, chunk_time_1.size);
		memcpy_s(&time_2, sizeof(time_2), chunk_time_2.data, chunk_time_2.size);
		memcpy_s(&ghpserver_raknet, sizeof(ghpserver_raknet), chunk_ghpserver_raknet.data, chunk_ghpserver_raknet.size);
		memcpy_s(&ghpserver_web, sizeof(ghpserver_web), chunk_ghpserver_web.data, chunk_ghpserver_web.size);

		chunk_apikey.FreeAll();
		chunk_time_1.FreeAll();
		chunk_time_2.FreeAll();
		chunk_ghpserver_raknet.FreeAll();
		chunk_ghpserver_web.FreeAll();
		signature.FreeAll();
		licChunk.FreeAll();
		VMProtectEnd();

		m_loaded = true;
		return true;
	}

	bool Lic::SaveToFile(std::wstring FileName, std::string PrivateKey)
	{
		dllSignature.FreeAll();

		if (!ghp_CRYPTO::GetFileSignature(VMProtectDecryptStringW(DLL_NAME_SIG), PrivateKey, VMProtectDecryptStringA("ghp_rsa"), &dllSignature))
		{
			dllSignature.Free();
			return false;
		}

		DataChunk chunk_apikey, chunk_time_1, chunk_time_2, chunk_ghpserver_raknet, chunk_ghpserver_web;

		if (!chunk_apikey.Init(&apikey, sizeof(apikey)) || !chunk_time_1.Init(&time_1, sizeof(time_1)) || !chunk_time_2.Init(&time_2, sizeof(time_2)) || !chunk_ghpserver_raknet.Init(&ghpserver_raknet, sizeof(ghpserver_raknet)) || !chunk_ghpserver_web.Init(&ghpserver_web, sizeof(ghpserver_web)))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();

			dllSignature.Free();
			return false;
		}
		
		// Escreve as indexes no arquivo
		if (!WriteFileStringBlock(FileName, name) || !WriteFileStringBlock(FileName, ip, true) || !WriteFileStringBlock(FileName, link, true) || !WriteFileDataChunkBlock(FileName, &chunk_apikey, true) || !WriteFileDataChunkBlock(FileName, &chunk_time_1, true) || !WriteFileDataChunkBlock(FileName, &chunk_time_2, true) || !WriteFileDataChunkBlock(FileName, &chunk_ghpserver_raknet, true) || !WriteFileDataChunkBlock(FileName, &chunk_ghpserver_web, true) || !WriteFileStringBlock(FileName, pubkey, true) || !WriteFileDataChunkBlock(FileName, &raknetpubkey, true) || !WriteFileDataChunkBlock(FileName, &dllSignature, true))
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();

			dllSignature.Free();
			return false;
		}
		else
		{
			chunk_apikey.FreeAll();
			chunk_time_1.FreeAll();
			chunk_time_2.FreeAll();
			chunk_ghpserver_raknet.FreeAll();
			chunk_ghpserver_web.FreeAll();

			dllSignature.Free();
			return true;
		}
	}
}