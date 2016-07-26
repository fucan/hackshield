#include "ghp_CRYPTO.h"
#include "..\ghp_UTILS\ghp_FILE.h"

//#include <RakNet\SecureHandshake.h>

#define KEY_BUFFER_SIZE 1024 * 32

namespace ghp_CRYPTO
{
/*	inline DWORD EncryptDWORD(DWORD value)
	{
		value += GHP_KEY_1;
		value ^= GHP_KEY_2;
		value -= GHP_KEY_3;
		value ^= GHP_KEY_4;
		return value;
	}

	inline DWORD DecryptDWORD(DWORD value)
	{
		value ^= GHP_KEY_4;
		value += GHP_KEY_3;
		value ^= GHP_KEY_2;
		value -= GHP_KEY_1;
		return value;
	}

	bool GenerateRaketNetKeys(UCHAR * publickey, UCHAR * privatekey)
	{
		if (!cat::EasyHandshake::Initialize())
			return false;

		cat::EasyHandshake handshake;

		if (!handshake.GenerateServerKey(publickey, privatekey))
			return false;

		return true;
	}

	bool GenerateRSAKey(std::string Peer, std::string * publickey, std::string * privatekey)
	{
		pk_context key;
		entropy_context entropy;
		ctr_drbg_context ctr_drbg;

		pk_init(&key);

		// Inicia o rand
		entropy_init(&entropy);
		if (ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, (const unsigned char *)Peer.c_str(), Peer.size()) != 0)
		{
			pk_free(&key);
			return false;
		}

		// Cria as chaves
		pk_init_ctx(&key, pk_info_from_type(POLARSSL_PK_RSA));
		if (rsa_gen_key(pk_rsa(key), ctr_drbg_random, &ctr_drbg, 4096, 65537) != 0)
		{
			pk_free(&key);
			entropy_free(&entropy);
			return false;
		}

		// Converte as chaves
		ghp_UTILS::DataChunk buffer_prv, buffer_pub;

		if (!buffer_prv.Init(KEY_BUFFER_SIZE) || !buffer_pub.Init(KEY_BUFFER_SIZE))
		{
			buffer_prv.FreeAll();
			buffer_pub.FreeAll();

			pk_free(&key);
			entropy_free(&entropy);
			return false;
		}

		if (pk_write_key_pem(&key, (UCHAR*)buffer_prv.data, KEY_BUFFER_SIZE) != 0 || pk_write_pubkey_pem(&key, (UCHAR*)buffer_pub.data, KEY_BUFFER_SIZE) != 0)
		{
			buffer_prv.FreeAll();
			buffer_pub.FreeAll();

			pk_free(&key);
			entropy_free(&entropy);
			return false;
		}

		// Escreve as chaves nos arquivos
		if (!ghp_UTILS::WriteFileData(L"PRV.KEY", &buffer_prv, strlen((char*)buffer_prv.data)) || !ghp_UTILS::WriteFileData(L"PUB.KEY", &buffer_pub, strlen((char*)buffer_pub.data)))
		{
			buffer_prv.FreeAll();
			buffer_pub.FreeAll();

			pk_free(&key);
			entropy_free(&entropy);
			return false;
		}


		// Escfeve as chaves nas strings
		publickey->clear();
		privatekey->clear();

		publickey->append((char*)buffer_pub.data);
		privatekey->append((char*)buffer_prv.data);
		
		// Limpa a memória
		buffer_prv.FreeAll();
		buffer_pub.FreeAll();

		pk_free(&key);
		entropy_free(&entropy);
		return true;
	}

	bool GetFileSignature(std::wstring FileName, std::string PrivateKey, std::string Peer, ghp_UTILS::DataChunk * signature)
	{
		pk_context pk;
		entropy_context entropy;
		ctr_drbg_context ctr_drbg;

		UCHAR buff[POLARSSL_MPI_MAX_SIZE];
		UCHAR hash[20];
		size_t len = 0;

		signature->Free();

		// Inicia o rand
		entropy_init(&entropy);
		if (ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, (const unsigned char *)Peer.c_str(), Peer.size()) != 0)
		{
			return false;
		}

		// Abre a key
		pk_init(&pk);
		if (pk_parse_key(&pk, (const unsigned char*)PrivateKey.c_str(), PrivateKey.length(), (const unsigned char*)"", 0) != 0)
		{
			pk_free(&pk);
			entropy_free(&entropy);
			return false;
		}

		// Pega o hash do arquivo
		std::string sFileName;
		sFileName.assign(FileName.begin(), FileName.end());

		if (sha1_file(sFileName.c_str(), hash) != 0)
		{
			pk_free(&pk);
			entropy_free(&entropy);
			return false;
		}

		// Pega a assinatura do arquivo
		if (pk_sign(&pk, POLARSSL_MD_SHA1, hash, 0, buff, &len, ctr_drbg_random, &ctr_drbg) != 0)
		{
			pk_free(&pk);
			entropy_free(&entropy);
			return false;
		}

		if (!signature->Init(buff, len))
		{
			signature->Free();
			pk_free(&pk);
			entropy_free(&entropy);
			return false;
		}

		pk_free(&pk);
		entropy_free(&entropy);
		return true;
	}

	bool SignFile(std::wstring FileName, std::string PrivateKey, std::string Peer)
	{
		ghp_UTILS::DataChunk fileToSign, signature, signatureSize;
		DWORD dwSignatureSize;

		if (!GetFileSignature(FileName, PrivateKey, Peer, &signature))
		{
			fileToSign.FreeAll();
			signature.FreeAll();
			return false;
		}

		if (!ghp_UTILS::LoadFileData(FileName.c_str(), &fileToSign))
		{
			fileToSign.FreeAll();
			signature.FreeAll();
			return false;
		}

		dwSignatureSize = signature.size;
		signatureSize.Init(&dwSignatureSize, sizeof(DWORD));

		// Escreve assinatura no arquivo
		if (!ghp_UTILS::WriteFileData(FileName.c_str(), &signatureSize) || !ghp_UTILS::WriteFileData(FileName.c_str(), &signature, 0, true) || !ghp_UTILS::WriteFileData(FileName.c_str(), &fileToSign, 0, true))
		{
			fileToSign.FreeAll();
			signature.FreeAll();
			return false;
		}

		fileToSign.FreeAll();
		signature.FreeAll();
		return true;
	}

	bool RemoveSignatureFromDataChunk(ghp_UTILS::DataChunk * memory, ghp_UTILS::DataChunk * signature)
	{
		ghp_UTILS::DataChunk signSize, memoryTemp;
		
		// Extrai o sign
		if (!memory->MemCpy(&signSize, sizeof(DWORD)) || !memory->MemCpy(signature, *(DWORD*)signSize.data, sizeof(DWORD)))
		{
			signSize.FreeAll();
			memoryTemp.FreeAll();
			signature->FreeAll();
			return false;
		}

		// Recorta a memória
		if (!memory->EraseFront(signature->size + sizeof(DWORD)))
		{
			signSize.FreeAll();
			memoryTemp.FreeAll();
			signature->FreeAll();
			return false;
		}

		signSize.FreeAll();
		memoryTemp.FreeAll();
		return true;
	}

	bool VerifyDataChunkSignature(ghp_UTILS::DataChunk * memory, ghp_UTILS::DataChunk * signature, std::string PublicKey, std::string Peer)
	{
		pk_context pk;
		UCHAR hash[20];
		
		// Abre a key
		pk_init(&pk);
		if (pk_parse_public_key(&pk, (const unsigned char*)PublicKey.c_str(), PublicKey.length()) != 0)
		{
			pk_free(&pk);
			return false;
		}

		// Calcula o hash da memória
		sha1_context ctx;
		sha1_starts(&ctx);
		sha1_update(&ctx, (UCHAR*)memory->data, memory->size);
		sha1_finish(&ctx, hash);

		// Verifica a assinatura
		if (pk_verify(&pk, POLARSSL_MD_SHA1, hash, 0, (const UCHAR*)signature->data, signature->size) != 0)
		{
			pk_free(&pk);
			return false;
		}

		pk_free(&pk);
		return true;
	}

	bool VerifyFileSignature(std::wstring FileName, ghp_UTILS::DataChunk * signature, std::string PublicKey, std::string Peer)
	{
		ghp_UTILS::DataChunk fileDataChunk;

		if(!ghp_UTILS::LoadFileData(FileName, &fileDataChunk) || !VerifyDataChunkSignature(&fileDataChunk, signature, PublicKey, Peer))
		{
			fileDataChunk.FreeAll();
			return false;
		}
		else
		{
			fileDataChunk.FreeAll();
			return true;
		}
	}

	bool EncFileAES(std::wstring FileNameIn, std::wstring FileNameOut, UCHAR * key)
	{
		ghp_UTILS::DataChunk bufferIn, bufferOut, bufferIV;

		if (!ghp_UTILS::LoadFileData(FileNameIn, &bufferIn) || !bufferOut.Init(bufferIn.size))
		{
			bufferIn.FreeAll();
			bufferOut.FreeAll();
			return false;
		}
		
		// Gera o IV
		UCHAR iv[16];
		srand((UINT)time(NULL));
		for (UCHAR i = 0; i < 16; i++)
		{
			iv[i] = rand() % 256;
		}
		bufferIV.Init(&iv, 16);

		// Encrita
		size_t ivoff = 0;
		aes_context ctx;
		aes_setkey_enc(&ctx, key, 256);
		int ret = aes_crypt_cfb128(&ctx, AES_ENCRYPT, bufferIn.size, &ivoff, iv, (const UCHAR*)bufferIn.data, (UCHAR*)bufferOut.data);

		// Verifica e escreve o arquivo encriptado
		if (ret == 0)
		{
			if (!ghp_UTILS::WriteFileData(FileNameOut, &bufferIV) || !ghp_UTILS::WriteFileData(FileNameOut, &bufferOut, 0, true))
			{
				bufferIn.FreeAll();
				bufferOut.FreeAll();
				bufferIV.FreeAll();
				return false;
			}

			bufferIn.FreeAll();
			bufferOut.FreeAll();
			bufferIV.FreeAll();
			return true;
		}
		else
		{
			bufferIn.FreeAll();
			bufferOut.FreeAll();
			bufferIV.FreeAll();
			return false;
		}
	}

	bool DecAES(ghp_UTILS::DataChunk * in, ghp_UTILS::DataChunk * out, UCHAR * key)
	{
		VMProtectBeginUltra("DecAES");
		ghp_UTILS::DataChunk iv;
		
		if (!in->MemCpy(&iv, 16, 0) || !out->Init(in->size - 16))
		{
			out->FreeAll();
			iv.FreeAll();
			return false;
		}
		
		size_t ivoff = 0;
		aes_context ctx;
		aes_setkey_enc(&ctx, key, 256);

		int ret = aes_crypt_cfb128(&ctx, AES_DECRYPT, out->size, &ivoff, (UCHAR*)iv.data, (UCHAR*)in->data + 16, (UCHAR*)out->data);

		iv.FreeAll();
		VMProtectEnd();

		if (ret == 0)
			return true;
		else
			return false;
	}

	bool DecFileAESToMemory(std::wstring FileName, ghp_UTILS::DataChunk * memory, UCHAR * key)
	{
		VMProtectBeginUltra("DecFileAESToMemory");
		ghp_UTILS::DataChunk fileData;

		if (!ghp_UTILS::LoadFileData(FileName, &fileData))
		{
			fileData.FreeAll();
			memory->FreeAll();
			return false;
		}
		else
		{
			if (!DecAES(&fileData, memory, key))
			{
				fileData.FreeAll();
				memory->FreeAll();
				return false;
			}
		}

		fileData.FreeAll();
		VMProtectEnd();
		return true;
	}*/

	void GetSHA1(ghp_UTILS::DataChunk * memory, UCHAR * hash)
	{
		sha1_context ctx;
		sha1_starts(&ctx);
		sha1_update(&ctx, (UCHAR*)memory->data, memory->size);
		sha1_finish(&ctx, hash);
	}

	std::string CalculeSHA1(ghp_UTILS::DataChunk * memory)
	{
		UCHAR hash[20];

		sha1_context ctx;
		sha1_starts(&ctx);
		sha1_update(&ctx, (UCHAR*)memory->data, memory->size);
		sha1_finish(&ctx, hash);

		return std::string((char*)hash, 20);
	}

	std::string GetFormatedSHA1(ghp_UTILS::DataChunk * memory)
	{
		UINT sha1[5];
		char buff[255];
		std::string temp = CalculeSHA1(memory);

		memcpy_s(&sha1, sizeof(sha1), temp.c_str(), temp.size());
		sprintf_s(buff, VMProtectDecryptStringA("%08X-%08X-%08X-%08X-%08X"), sha1[0], sha1[1], sha1[2], sha1[3], sha1[4]);

		return buff;
	}

	/*bool CalculeFileSHA1(std::string FileName, UCHAR * hash)
	{
		if (sha1_file(FileName.c_str(), hash) != 0)
			return false;
		else
			return true;
	}*/

	bool CalculeFileSHA1(std::wstring FileName, UCHAR * hash)
	{
		ghp_UTILS::DataChunk data;

		if (!ghp_UTILS::LoadFileData(FileName, &data))
		{
			data.FreeAll();
			return false;
		}
		else
		{
			GetSHA1(&data, hash);
			data.FreeAll();
			return true;
		}
	}
}