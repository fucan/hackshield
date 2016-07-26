#include "ghp_LOG.h"

namespace ghp_UTILS
{
	Log::Log(std::wstring FileName, bool Append)
	{
		if (!Append)
			DeleteFileW(FileName.c_str());

		filename = FileName;
	}

	void Log::LineOut(bool appendDateTime, char * data, ...)
	{
		DataChunk dateTime, logData, newLine;

		if (!dateTime.Init(1024) || !logData.Init(1024) || !newLine.Init(NEWLINE, 2))
		{
			dateTime.FreeAll();
			logData.FreeAll();
			newLine.FreeAll();
			return;
		}
		
		// Interpreta os argumentos
		va_list args;
		va_start(args, data);
		int len = vsprintf_s((char*)logData.data, logData.size, data, args);
		va_end(args);

		// Adiciona data e hora no log
		if (appendDateTime)
		{
			struct tm today;
			time_t ltime;
			time(&ltime);
			localtime_s(&today, &ltime);

			int dateTimeLen = sprintf_s((char*)dateTime.data, dateTime.size, "[%02d/%02d/%04d - %02d:%02d:%02d] ", today.tm_mday, today.tm_mon + 1, today.tm_year + 1900, today.tm_hour, today.tm_min, today.tm_sec);
			WriteFileData(filename.c_str(), &dateTime, dateTimeLen, true);
		}

		// Escreve log e nova linha no arquivo
		WriteFileData(filename.c_str(), &logData, len, true);
		WriteFileData(filename.c_str(), &newLine, 0, true);

		dateTime.FreeAll();
		logData.FreeAll();
		newLine.FreeAll();
	}
}