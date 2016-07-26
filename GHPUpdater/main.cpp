#include "..\include.h"
#include "..\ghp_UTILS\ghp_FILE.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"

#define NEW_LINE	"\x0d\x0a"
#define TAB			"\x09"

#include "..\ghp_UTILS\jpeg-compressor\jpge.h"
#include "..\ghp_UTILS\jpeg-compressor\jpgd.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// Gera o arquivo CPP com o Updater
	if (std::wstring(lpCmdLine) == VMProtectDecryptStringW(L"-generate"))
	{
		ghp_UTILS::DataChunk in, out;
		
		if (ghp_UTILS::LoadFileData(VMProtectDecryptStringW(L"GHPUpdater.exe"), &in))
		{
			char buffer[256];
			int counter = 0;
			VMProtectBegin("SHA1");
			std::string sha1 = ghp_CRYPTO::GetFormatedSHA1(&in);
			VMProtectEnd();

			// Escreve o header
			memset(buffer, 0, 256);
			sprintf_s(buffer, VMProtectDecryptStringA("#include \"ghp_UPDATER.h\"%schar * updater_sha1 = \"%s\";%sUINT updater_size = %d;%sUCHAR updater_data[] = {%s"), VMProtectDecryptStringA(NEW_LINE NEW_LINE), sha1.c_str(), VMProtectDecryptStringA(NEW_LINE NEW_LINE), in.size, VMProtectDecryptStringA(NEW_LINE NEW_LINE), VMProtectDecryptStringA(NEW_LINE TAB));
			out.Inc(buffer, strlen(buffer));
			
			// Escreve o buffer
			for (UINT i = 0; i < in.size; i++)
			{
				UCHAR byte;
				memcpy_s(&byte, sizeof(byte), (LPVOID)((DWORD)in.data + i), sizeof(byte));
				
				counter++;
				memset(buffer, 0, 256);
				
				if (counter == 32)
				{
					counter = 0;
					sprintf_s(buffer, VMProtectDecryptStringA("0x%02X,%s"), byte, VMProtectDecryptStringA(NEW_LINE TAB));
				}
				else
					sprintf_s(buffer, VMProtectDecryptStringA("0x%02X, "), byte);

				out.Inc(buffer, strlen(buffer));
			}

			memset(buffer, 0, 256);
			sprintf_s(buffer, VMProtectDecryptStringA("};"));
			out.Inc(buffer, strlen(buffer));

			// Escreve o arquivo final
			ghp_UTILS::WriteFileData(VMProtectDecryptStringW(L"ghp_UPDATER.cpp"), &out);
		}

		in.FreeAll();
		out.FreeAll();

		return 0;
	}

	return 0;
}