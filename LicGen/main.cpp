#include "..\include.h"
#include "..\ghp_UTILS\ghp_FILE.h"
#include "..\ghp_UTILS\ghp_LIC.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include "..\ghp_UTILS\ghp_LOG.h"

#include <random>

#pragma comment (lib, "RakNetLibStatic.lib")

std::string publickey, privatekey, ghpprivatekey;

void main()
{
	// Abre os dados da licença
	char name[60], ip[100], link[1024];
	DWORD apikey, time_1, time_2, ghpserver_raknet, ghpserver_web;

	GetPrivateProfileStringA("data", "name", "", name, 60, ".\\LicData.ini");
	GetPrivateProfileStringA("data", "ip", "", ip, 100, ".\\LicData.ini");
	GetPrivateProfileStringA("data", "link", "", link, 1024, ".\\LicData.ini");

	apikey = GetPrivateProfileIntA("data", "apikey", 0, ".\\LicData.ini");
	time_1 = GetPrivateProfileIntA("data", "time_1", 0, ".\\LicData.ini");
	time_2 = GetPrivateProfileIntA("data", "time_2", 0, ".\\LicData.ini");
	ghpserver_raknet = GetPrivateProfileIntA("data", "ghpserver_raknet", 0, ".\\LicData.ini");
	ghpserver_web = GetPrivateProfileIntA("data", "ghpserver_web", 0, ".\\LicData.ini");
	
	// Se não definido as portas na ini, volta para as portas default
	if (ghpserver_raknet == 0)
		ghpserver_raknet = 55507;

	if (ghpserver_web == 0)
		ghpserver_web = 55508;

	// Verifica os valores e geram outros caso necessario
	if (apikey == 0 || time_1 == 0 || time_2 == 0)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<DWORD> dist(0, 0xFFFFFFFF);
		
		apikey = dist(mt);
		time_1 = dist(mt);
		time_2 = dist(mt);
	
		char buff[64];
		_itoa_s(apikey, buff, 16);
		sprintf_s(buff, "0x%08X", apikey);
		WritePrivateProfileStringA("data", "apikey", buff, ".\\LicData.ini");
		sprintf_s(buff, "0x%08X", time_1);
		WritePrivateProfileStringA("data", "time_1", buff, ".\\LicData.ini");
		sprintf_s(buff, "0x%08X", time_2);
		WritePrivateProfileStringA("data", "time_2", buff, ".\\LicData.ini");
	}

	// Exibe as informações
	printf("%s\n%s\n%s\n\n", name, ip, link);
	
	UCHAR raknet_publickey[64];
	UCHAR raknet_privatekey[32];

	ghp_UTILS::DataChunk dc_ghpprivatekey, dc_publickey, dc_privatekey, dc_raknetkey;

	// Abre a chave privada do GHP
	if (!ghp_UTILS::LoadFileData(L"GHP_PRV.KEY", &dc_ghpprivatekey))
		return;
	
	// Converte a chave privada do GHP para string
	ghpprivatekey = dc_ghpprivatekey.toString();
	dc_ghpprivatekey.FreeAll();

	// Tenta abrir as chaves
	if(!ghp_UTILS::LoadFileData(L"PUB.KEY", &dc_publickey) || !ghp_UTILS::LoadFileData(L"PRV.KEY", &dc_privatekey) || !ghp_UTILS::LoadFileData(L"NET.KEY", &dc_raknetkey))
	{
		if (!ghp_CRYPTO::GenerateRSAKey("ghp_public_rsa", &publickey, &privatekey))
			return;

		if (!ghp_CRYPTO::GenerateRaketNetKeys(raknet_publickey, raknet_privatekey))
			return;

		/* Salva as Keys do RakNet */
		dc_raknetkey.FreeAll();
		
		if (!dc_raknetkey.Inc(raknet_publickey, 64) || !dc_raknetkey.Inc(raknet_privatekey, 32) || !ghp_UTILS::WriteFileData(L"NET.KEY", &dc_raknetkey))
			return;
	}
	else
	{
		publickey = dc_publickey.toString();
		privatekey = dc_privatekey.toString();
		
		memcpy_s(raknet_publickey, 64, dc_raknetkey.data, 64);
		dc_raknetkey.EraseFront(64);
		memcpy_s(raknet_privatekey, 32, dc_raknetkey.data, 32);
	}
	
	// Gera a licença
	ghp_UTILS::Lic myLic(name, ip, link, ghp_CRYPTO::EncryptDWORD(apikey), ghp_CRYPTO::EncryptDWORD(time_1), ghp_CRYPTO::EncryptDWORD(time_2), ghp_CRYPTO::EncryptDWORD(ghpserver_raknet), ghp_CRYPTO::EncryptDWORD(ghpserver_web), publickey, raknet_publickey);

	if (!myLic.SaveToFile(LICENSE_NAME, ghpprivatekey) || !ghp_CRYPTO::SignFile(LICENSE_NAME, ghpprivatekey, "ghp_rsa") || !ghp_CRYPTO::EncFileAES(LICENSE_NAME, LICENSE_NAME, (UCHAR*)GHP_AES_KEY))
	{
		return;
	}

	ghp_UTILS::DataChunk prv;
	if (ghp_UTILS::LoadFileData(L"PRV.KEY", &prv))
	{
		WritePrivateProfileStringA("data", "c_ref", ghp_CRYPTO::GetFormatedSHA1(&prv).c_str(), ".\\LicData.ini");
	}

	ghp_UTILS::Lic licTest;
	if(!licTest.LoadFromFile(LICENSE_NAME, (UCHAR*)GHP_AES_KEY))
		return;

	if (licTest.GetAPIKey() != apikey || licTest.GetTime1() != time_1 || licTest.GetTime2() != time_2)
		return;

	printf("Done!");
	Sleep(1000);
}