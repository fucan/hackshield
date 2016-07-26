#ifndef _GHP_DB_H_
#define _GHP_DB_H_

#include "..\include.h"
#include "ghp_STRUCTURED_FILE.h"

namespace ghp_UTILS
{
	class DB
	{
	private:
		std::vector<Database> db;
	public:
		bool ImportFromStructuredFile(StructuredFile * structured);

		UINT GetEntryCount();
		Database GetEntry(UINT index);
	};
}

#endif