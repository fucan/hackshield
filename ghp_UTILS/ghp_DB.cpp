#include "ghp_DB.h"

namespace ghp_UTILS
{
	bool DB::ImportFromStructuredFile(StructuredFile * structured)
	{
		VMProtectBeginUltra("DB::ImportFromStructuredFile");
		db.clear();

		// Verifica se está importando os dados corretos
		if (sizeof(Database) != structured->GetSize())
			return false;

		// Abre as entradas e salva no vetor
		for(UINT i = 0; i < structured->GetLength(); i++)
		{
			Database entry;

			if (!structured->Get(i, sizeof(entry), &entry))
				return false;

			db.push_back(entry);
		}

		// Verifica se abriu todos os indexes
		if (structured->GetLength() != db.size())
			return false;

		VMProtectEnd();
		
		return true;
	}

	UINT DB::GetEntryCount()
	{
		return db.size();
	}

	Database DB::GetEntry(UINT index)
	{
		return db[index];
	}
}