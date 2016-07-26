#include "ghp_STRUCTURED_FILE.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include "ghp_FILE.h"

namespace ghp_UTILS
{
	UINT StructuredFile::GetLength()
	{
		return length;
	}

	UINT StructuredFile::GetSize()
	{
		return size;
	}

	bool StructuredFile::Get(UINT index, UINT DstLen, LPVOID DstBuffer)
	{
		if (DstLen > data.size)
			return false;

		memcpy_s(DstBuffer, DstLen, data.data, DstLen);
		if(!data.EraseFront(DstLen))
			return false;

		return true;
	}

	bool StructuredFile::Set(UINT ArrayLength, UINT ArraySize, LPVOID ArrayData)
	{
		length = ArrayLength;
		size = ArraySize;

		if (!data.Init(ArrayData, length * size))
		{
			data.FreeAll();
			return false;
		}
		
		return true;
	}

	bool StructuredFile::LoadFromMemory(DataChunk * memory, std::string PublicKey, UCHAR * key)
	{
		VMProtectBeginUltra("StructuredFile::LoadFromMemory");
		DataChunk decrypted, sign;
		
		// Remove a assinatura do arquivo
		if (!ghp_CRYPTO::DecAES(memory, &decrypted, key) || !ghp_CRYPTO::RemoveSignatureFromDataChunk(&decrypted, &sign))
		{
			decrypted.FreeAll();
			sign.FreeAll();
			return false;
		}

		// Verifica a assinatura do buffer
		if (!ghp_CRYPTO::VerifyDataChunkSignature(&decrypted, &sign, PublicKey, "ghp_public_rsa"))
		{
			decrypted.FreeAll();
			sign.FreeAll();
			return false;
		}
		
		// Salva no buffer da class
		data.FreeAll();
		if (!data.Init(&decrypted))
		{
			decrypted.FreeAll();
			sign.FreeAll();
			return false;
		}
		
		// Seta as variáveis de size e lenght
		memcpy_s(&length, sizeof(length), data.data, sizeof(length));
		memcpy_s(&size, sizeof(size), (char*)data.data + 4, sizeof(size));
		
		// Remove o header do buffer
		data.EraseFront(sizeof(size) + sizeof(length));
		decrypted.FreeAll();
		sign.FreeAll();

		VMProtectEnd();
		return true;
	}

	bool StructuredFile::LoadFromFile(std::wstring FileName, std::string PublicKey, UCHAR * key)
	{
		VMProtectBeginUltra("StructuredFile::LoadFromFile");
		DataChunk file;

		// Tenta abrir o arquivo
		if (!ghp_UTILS::LoadFileData(FileName, &file))
		{
			file.FreeAll();
			return false;
		}
		else
		{
			if (!LoadFromMemory(&file, PublicKey, key))
			{
				file.FreeAll();
				return false;
			}
		}

		file.FreeAll();
		VMProtectEnd();
		return true;
	}

	bool StructuredFile::SaveToFile(std::wstring FileName, std::string PrivateKey, UCHAR * key)
	{
		DataChunk file;

		// Cria o arquivo no buffer
		if(!file.Init(&length, sizeof(length)) || !file.Inc(&size, sizeof(size)) || !file.Inc(&data))
		{
			file.FreeAll();
			return false;
		}
				
		// Escreve o buffer no arquivo
		if(!WriteFileData(FileName, &file))
		{
			file.FreeAll();
			return false;
		}

		// Limpa o arquivo da memoria
		file.FreeAll();

		// Assina o arquivo com a chave privada
		if(!ghp_CRYPTO::SignFile(FileName, PrivateKey, "ghp_public_rsa"))
			return false;

		// Encripta o arquivo
		if(!ghp_CRYPTO::EncFileAES(FileName, FileName, key))
			return false;

		return true;
	}
}