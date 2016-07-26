#ifndef _GHP_HTTPCLIENT_H_
#define _GHP_HTTPCLIENT_H_

#include "..\include.h"
#include "..\ghp_UTILS\ghp_DEFS.h"
#include <curl\curl.h>

#pragma comment (lib, "libcurl.lib")

namespace ghp_NET
{
	enum HTTP_STATUS
	{
		HTTP_STATUS_NONE = 0,
		HTTP_STATUS_DOWNLOADING
	};

	class HTTPClient
	{
	private:
		HTTP_STATUS status;

		bool GetURL(std::string URL, ghp_UTILS::DataChunk * data);
	public:
		HTTPClient();
		~HTTPClient();

		bool GetURLToFile(std::string URL, std::wstring FileName);
		bool GetURLToString(std::string URL, std::string * string);
		bool GetURLToDataChunk(std::string URL, ghp_UTILS::DataChunk * data);

		std::string SafeURL(std::string url);
		HTTP_STATUS Status();
		float GetCompletion();
	};
}

#endif