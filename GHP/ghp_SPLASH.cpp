#include "ghp_SPLASH.h"

/*void StartSplash()
{
	// Extrai o GHP.bin da DLL se o arquivo não existir ou for inválido
	ghp_UTILS::DataChunk check, extract;
	bool update = true;

	if (ghp_UTILS::LoadFileData(VMProtectDecryptStringW(L"GHP.bin"), &check))
	{
		if (ghp_CRYPTO::GetFormatedSHA1(&check) == updater_sha1)
			update = false;
	}

	if (update)
	{
		if (!extract.Init(updater_data, updater_size) || !ghp_UTILS::WriteFileData(VMProtectDecryptStringW(L"GHP.bin"), &extract))
		{
			Error(ERROR_GHP_BIN, 0);
		}
	}

	// Limpa a memória
	check.FreeAll();
	extract.FreeAll();
}*/