#include "ghp_HTTPCLIENT.h"
#include "..\ghp_UTILS\ghp_FILE.h"

namespace ghp_NET
{
	HTTPClient::HTTPClient()
	{
		status = HTTP_STATUS_NONE;
		curl_global_init(CURL_GLOBAL_ALL);
	}

	HTTPClient::~HTTPClient()
	{
		curl_global_cleanup();
	}

	// Static vars
	static float percent = 0.0f;

	static size_t WriteMemoryCallback(void * contents, size_t size, size_t nmemb, void * userp)
	{
		size_t realsize = size * nmemb;
		ghp_UTILS::DataChunk * mem = (ghp_UTILS::DataChunk*)userp;

		if (!mem->Inc(contents, realsize))
			return 0;
		else
			return realsize;
	}

	static int ProgressCallback(void * clientp, double dltotal, double dlnow, double ultotal, double ulnow)
	{
		if (dlnow != 0 && dltotal != 0)
			percent = ((float)dlnow / (float)dltotal) * 100.0f;
		else
			percent = 0.0f;

		return 0;
	}

	bool HTTPClient::GetURL(std::string URL, ghp_UTILS::DataChunk * data)
	{
		// Fixa a URL
		size_t position = 0;
		for (position = URL.find(" "); position != std::string::npos; position = URL.find(" ", position))
		{
			URL.replace(position, 1, "%20");
		}

		CURL * curl_handle = curl_easy_init();

		data->FreeAll();
		
		curl_easy_setopt(curl_handle, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, VMProtectDecryptStringA("GHP_NET"));
		curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 30);
		//curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 4);

		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);		
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, data);
		
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);

		CURLcode res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK)
		{
			curl_easy_cleanup(curl_handle);
			return false;
		}
		else
		{
			curl_easy_cleanup(curl_handle);
			return true;
		}
	}

	bool HTTPClient::GetURLToFile(std::string URL, std::wstring FileName)
	{
		status = HTTP_STATUS_DOWNLOADING;

		// Lê os dados do site
		ghp_UTILS::DataChunk data;
		if (!GetURL(URL, &data))
		{
			data.FreeAll();
			status = HTTP_STATUS_NONE;
			return false;
		}

		// Escreve os dados no arquivo
		if (!ghp_UTILS::WriteFileData(FileName, &data))
		{
			data.FreeAll();
			status = HTTP_STATUS_NONE;
			return false;
		}
		else
		{
			data.FreeAll();
			status = HTTP_STATUS_NONE;
			return true;
		}
	}

	bool HTTPClient::GetURLToString(std::string URL, std::string * string)
	{
		status = HTTP_STATUS_DOWNLOADING;

		// Lê os dados do site
		ghp_UTILS::DataChunk data;
		if (!GetURL(URL, &data))
		{
			data.FreeAll();
			status = HTTP_STATUS_NONE;
			return false;
		}
		else
		{
			string->clear();
			string->append((char*)data.data, data.size);

			data.FreeAll();
			status = HTTP_STATUS_NONE;
			return true;
		}
	}

	bool HTTPClient::GetURLToDataChunk(std::string URL, ghp_UTILS::DataChunk * data)
	{
		status = HTTP_STATUS_DOWNLOADING;

		if (!GetURL(URL, data))
		{
			data->FreeAll();
			status = HTTP_STATUS_NONE;
			return false;
		}
		else
		{
			status = HTTP_STATUS_NONE;
			return true;
		}
	}

	std::string HTTPClient::SafeURL(std::string url)
	{
		size_t addr = -1;

		while (true)
		{
			addr = url.find("+", addr + 1);

			if (addr == -1)
				break;

			url.replace(addr, 1, "%2B");
		}

		return url;
	}

	HTTP_STATUS HTTPClient::Status()
	{
		return status;
	}

	float HTTPClient::GetCompletion()
	{
		if (status == HTTP_STATUS_DOWNLOADING)
			return percent;
		else
			return 0.0f;
	}
}