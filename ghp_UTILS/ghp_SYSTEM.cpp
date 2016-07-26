#include "ghp_SYSTEM.h"
#include "jpeg-compressor\jpge.h"
#include "jpeg-compressor\jpgd.h"

#include "ghp_FILE.h"

namespace ghp_UTILS
{
	bool PrivEnable(std::wstring PrivilegeName)
	{
		HANDLE hProc = GetCurrentProcess();
		HANDLE hToken = 0;

		if (OpenProcessToken(hProc, TOKEN_ADJUST_PRIVILEGES, &hToken) == false)
		{
			return false;
		}

		LUID luid;

		if (LookupPrivilegeValue(0, PrivilegeName.c_str(), &luid) == false)
		{
			CloseHandle(hToken);
			return false;
		}

		TOKEN_PRIVILEGES tstate;

		tstate.PrivilegeCount = 1;
		tstate.Privileges[0].Luid = luid;
		tstate.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (AdjustTokenPrivileges(hToken, 0, &tstate, 0, 0, 0) == false)
			return false;

		CloseHandle(hToken);

		return true;
	}

	bool HaveAdminPrivileges()
	{
		HKEY key;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, VMProtectDecryptStringW(L"System"), 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
			return true;
		else
			return false;
	}

	BOOL IsWow64()
	{
		BOOL bIsWow64 = FALSE;

		if (IsWow64Process(GetCurrentProcess(), &bIsWow64) == FALSE)
			return FALSE;

		return bIsWow64;
	}

	std::wstring GetWinGUID()
	{
		HKEY hCryptography;
		TCHAR WinGUID[1024];
		DWORD size = sizeof(WinGUID);

	#if defined (_WIN64)
		DWORD param = 0;
	#else
		DWORD param = IsWow64() ? KEY_WOW64_64KEY : 0;
	#endif

		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, VMProtectDecryptStringW(L"SOFTWARE\\Microsoft\\Cryptography"), 0, KEY_READ | param, &hCryptography))
		{
			RegQueryValueEx(hCryptography, VMProtectDecryptStringW(L"MachineGuid"), NULL, NULL, (LPBYTE)WinGUID, &size);
			RegCloseKey(hCryptography);
		}

		return std::wstring(WinGUID, size);
	}

	std::list<std::string> GetHWIDs()
	{
		VMProtectBeginUltra("GetHWIDs");
		ghp_UTILS::DataChunk chunk;
		std::list<std::string> ids;
		
		// GUID do Windows
		std::wstring WinGUID = GetWinGUID();

		if (WinGUID != L"")
		{
			if (chunk.Init((LPVOID)WinGUID.data(), WinGUID.size()))
			{
				ids.push_back(ghp_CRYPTO::CalculeSHA1(&chunk));
				chunk.FreeAll();
			}
		}

		// MAC's
		IP_ADAPTER_INFO AdapterInfo[32];
		DWORD dwBufLen = sizeof(AdapterInfo);

		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_SUCCESS)
		{
			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
			
			do
			{
				if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET && chunk.Init(pAdapterInfo->Address, 0x08))
				{
					ids.push_back(ghp_CRYPTO::CalculeSHA1(&chunk));
					chunk.FreeAll();
				}

				pAdapterInfo = pAdapterInfo->Next;
			} while (pAdapterInfo);
		}

		// Serial do HD
		/*DWORD serial = 0;
		GetVolumeInformation(VMProtectDecryptStringA("C:\\"), NULL, NULL, &serial, NULL, NULL, NULL, NULL);

		chunk.Init(&serial, sizeof(serial));

		ids.push_back(ghp_CRYPTO::CalculeSHA1(&chunk));*/
		
		chunk.FreeAll();
		VMProtectEnd();
		return ids;
	}

	void GetAllFiles(std::vector<std::wstring> &files, std::wstring directory)
	{
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		std::wstring filter = directory + L"*.*";
		hFind = FindFirstFileW(filter.c_str(), &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE)
			return;
	
		while (FindNextFileW(hFind, &FindFileData) != 0)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					GetAllFiles(files, directory + FindFileData.cFileName + L"\\");
				}
				else
					files.push_back(directory + FindFileData.cFileName);
			}
		}

		FindClose(hFind);
	}
	
	DataChunk GetScreen(std::wstring DeviceName, int quality, int grayscale)
	{
		VMProtectBeginUltra("GetScreen 1");
		DataChunk temp, ret;

		HDC hdcScreen = CreateDC(VMProtectDecryptStringW(L"DISPLAY"), DeviceName.c_str(), NULL, NULL);
		HDC hdcCapture = CreateCompatibleDC(hdcScreen);

		int nWidth = GetDeviceCaps(hdcScreen, HORZRES),	nHeight = GetDeviceCaps(hdcScreen, VERTRES), nBPP = GetDeviceCaps(hdcScreen, BITSPIXEL);

		LPVOID lpCapture;
		BITMAPINFO bmiCapture = { { sizeof(BITMAPINFOHEADER), nWidth, -nHeight, 1, nBPP, BI_RGB, 0, 0, 0, 0, 0 } };

		HBITMAP hbmCapture = CreateDIBSection(hdcScreen, &bmiCapture, DIB_RGB_COLORS, &lpCapture, NULL, 0);
		if (!hbmCapture)
		{
			DeleteDC(hdcCapture);
			DeleteDC(hdcScreen);
			temp.FreeAll();
			ret.FreeAll();
			return ret;
		}

		int nCapture = SaveDC(hdcCapture);
		SelectObject(hdcCapture, hbmCapture);
		BitBlt(hdcCapture, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);

		// Pixel count
		UINT pc = nWidth * nHeight;

		// Pixel variables
		pixelrgba * rgba = new pixelrgba[pc];
		pixelrgb * rgb = new pixelrgb[pc];

		// Copia o buffer pro array
		memcpy_s(rgba, sizeof(pixelrgba) * pc, lpCapture, sizeof(pixelrgba) * pc);
		VMProtectEnd();

		// Converte os pixels (BGRA >> RGB)
		for (UINT i = 0; i < pc; i++)
		{
			rgb[i].r = rgba[i].b;
			rgb[i].g = rgba[i].g;
			rgb[i].b = rgba[i].r;
		}

		VMProtectBeginUltra("GetScreen 2");
		// Parametros do JPEG
		jpge::params param;
		param.m_quality = quality;
		param.m_subsampling = grayscale == 1 ? jpge::Y_ONLY : jpge::H2V2;
		param.m_no_chroma_discrim_flag = false;
		param.m_two_pass_flag = true;

		// Inicia o buffer (pixel count * 4 bytes)
		temp.Init(pc * 4);

		// Tenta comprimir o arquivo
		int msize = temp.size;
		if (jpge::compress_image_to_jpeg_file_in_memory(temp.data, msize, nWidth, nHeight, 3, (jpge::uint8*)rgb, param))
		{
			// Copia o buffer para a variável de retorno
			if (!temp.MemCpy(&ret, msize))
			{
				temp.FreeAll();
				ret.FreeAll();
			}

			// Limpa a variável temporaria
			temp.FreeAll();
		}
		else
		{
			temp.FreeAll();
			ret.FreeAll();
		}

		delete[] rgb;
		delete[] rgba;

		RestoreDC(hdcCapture, nCapture);
		DeleteDC(hdcCapture);
		DeleteDC(hdcScreen);
		DeleteObject(hbmCapture);
		VMProtectEnd();
		return ret;
	}

#ifdef R_WARZ
	std::vector<DataChunk> GetScreenshot(int quality, int grayscale, LPDIRECT3DDEVICE9 dev)
#else
	std::vector<DataChunk> GetScreenshot(int quality, int grayscale)
#endif
	{
		std::vector<DataChunk> ret;
#ifdef R_WARZ
		if (dev == NULL)
#endif
		{
			VMProtectBeginUltra("GetScreenshot 1");
			DWORD device = 0;
			DISPLAY_DEVICE dd;
			dd.cb = sizeof(DISPLAY_DEVICE);
			
			while (EnumDisplayDevices(NULL, device, &dd, 0))
			{
				DWORD screen = 0;
				DISPLAY_DEVICE newdd = { 0 };
				newdd.cb = sizeof(DISPLAY_DEVICE);
			
				while (EnumDisplayDevices(dd.DeviceName, screen, &newdd, 0))
				{
					ret.push_back(GetScreen(dd.DeviceName, quality, grayscale));
					screen++;
				}
				device++;
			}
			VMProtectEnd();
		}
#ifdef R_WARZ
		else
		{
			LPDIRECT3DSURFACE9 surface = 0;
			LPD3DXBUFFER pDestBuf;
			D3DDISPLAYMODE mode;
			DataChunk d3dss, temp, normalized;
			bool gotSS = false;
			
			if (dev->TestCooperativeLevel() != D3D_OK)
				goto cleanup;

			if (FAILED(dev->GetDisplayMode(0, &mode)))
				goto cleanup;

			if (FAILED(dev->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL)))
				goto cleanup;

			if (FAILED(dev->GetFrontBufferData(0, surface)))
				goto cleanup;

			if (FAILED(D3DXSaveSurfaceToFileInMemory(&pDestBuf, D3DXIFF_JPG, surface, NULL, NULL)))
				goto cleanup;

			if (d3dss.Init(pDestBuf->GetBufferPointer(), pDestBuf->GetBufferSize()))
				gotSS = true;

			pDestBuf->Release();
			
cleanup:
			if (surface != 0)
				surface->Release();

			VMProtectBeginUltra("GetScreenshot 2");
			if (gotSS)
			{
				DataChunk asd;
				int width, height, comps;

				UCHAR * mem = jpgd::decompress_jpeg_image_from_memory((UCHAR*)d3dss.data, d3dss.size, &width, &height, &comps, 3);
				if (mem != 0)
				{
					jpge::params param;
					param.m_quality = quality;
					param.m_subsampling = grayscale == 1 ? jpge::Y_ONLY : jpge::H2V2;
					param.m_no_chroma_discrim_flag = false;
					param.m_two_pass_flag = true;
					
					// Inicia o buffer (pixel count * 4 bytes)
					temp.Init(width * height * 4);

					// Tenta comprimir o arquivo
					int msize = temp.size;
					if (jpge::compress_image_to_jpeg_file_in_memory(temp.data, msize, width, height, 3, (jpge::uint8*)mem, param))
					{
						if (temp.MemCpy(&normalized, msize))
							ret.push_back(normalized);
					}
				}
			}
			VMProtectEnd();
			temp.FreeAll();
			d3dss.FreeAll();
		}
#endif
		return ret;
	}
}