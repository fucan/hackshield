#ifndef _GHP_LOG_H_
#define _GHP_LOG_H_

#include "ghp_FILE.h"

namespace ghp_UTILS
{
	static char NEWLINE[] = { 0x0D, 0x0A };

	class Log
	{
	private:
		std::wstring filename;
	public:
		Log(std::wstring FileName, bool Append = false);

		void LineOut(bool appendDateTime, char * data, ...);
	};
}

#endif