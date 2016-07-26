#include "ghp_MU.h"

#include "ghp_ENCDEC.h"

namespace ghp_UTILS
{
	bool IsSerialPacket(UCHAR * Packet)
	{
		if (Packet[0] == 0xC3)
		{
			if (Packet[1] == 0x44 || Packet[1] == 0x65)
				return true;
			else
				return false;
		}
		else
			return false;
	}

	std::string SetSerialAndGetLogin(UCHAR * Packet, int len, std::string Serial)
	{
		VMProtectBeginUltra("SetSerialAndGetLogin");
		char login[11];
		memset(login, 0x00, sizeof(login));

		int OriginalSize = (int)Packet[1];
		int DecSize = (OriginalSize-2) * 8 / 11;
		int EncSize = (DecSize + 2) * 11 / 8;

		UCHAR * OrgBuffer = new UCHAR[OriginalSize];
		UCHAR * DecBuffer = new UCHAR[DecSize];
		UCHAR * EncBuffer = new UCHAR[EncSize];

		// Copia a packet para o buffer
		memcpy_s(OrgBuffer, OriginalSize, Packet, OriginalSize);

		// Decrypta a packet
		DecryptC3asServer(DecBuffer, OrgBuffer + 2, OriginalSize - 2);
		DecXor32(DecBuffer + 1, 2, DecSize - 1);

		// Salva o login no buffer
		EncDecLogin(DecBuffer + 3, 10); // Descripta
		memcpy_s(login, 11, DecBuffer + 3, 10);
		EncDecLogin(DecBuffer + 3, 10); // Encripta

		// Seta o serial
		if (Packet[1] == 0x44)
		{
			memcpy_s(DecBuffer + 32, 17, Serial.c_str(), 16);
		}
		else if (Packet[1] == 0x65)
		{
			memcpy_s(DecBuffer + 52, 17, Serial.c_str(), 16);
		}

		// Encrypta a packet
		EncXor32(DecBuffer + 1, 2, DecSize - 1);
		EncryptC3asClient(EncBuffer + 2, DecBuffer, DecSize);

		// Fixa o header
		EncBuffer[0] = 0xC3;
		EncBuffer[1] = (UCHAR) EncSize;

		// Copia a packet para a memoria
		if (EncSize == len)
			memcpy_s(Packet, len, EncBuffer, EncSize);

		delete [] EncBuffer;
		delete [] DecBuffer;
		delete [] OrgBuffer;

		VMProtectEnd();
		return login;
	}
}