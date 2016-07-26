#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#define R_WARZ
/* #undef R_MUONLINE */
/* #undef D_INJECT_DEBUG_LOGIN */

#define GHP_SERVER_TITLE L"GHPServer [5.12.0]"

#define GHP_AES_KEY "\x0E\x84\x99\x65\xD6\xA9\x1D\x70\xD7\x01\xCD\xA3\xCE\x39\xE5\xAB\x21\xCB\x1D\x7E\xD4\xF4\xB8\xE8\x18\xFB\x2F\x23\x54\x00\xE1\x14\x6D\x4B\xCC\xD4\x11\x67\x94\x7E\x94\xA9\xB5\xDA\xF9\xC3\x4E\x54\x02\x8A\xB8\x76\x17\xD7\x29\x16\xC2\x96\xC5\x71\x91\x08\x37\x26\x82\x2A\x97\xF8\x2E\xE1\x69\x9F\x9E\x25\x9A\xAA\xA1\x9E\x9F\x80\xD7\x0A\xB3\x28\x43\xFE\x45\x91\x65\x39\x09\x6A\xB7\x6D\x47\x0B\xB3\x85\x38\xA7\xEA\x94\x25\x9E\xA8\x92\x54\xD4\x69\xAA\x9B\x32\x64\x38\xB7\x4B\xE0\x03\xC1\x44\xE4\x61\x32\xCC\x47\x30\x51\xA8\x01\x40\x39\x17\x23\x33\x0F\x79\xE6\xA0\x03\xCD\xC4\x2F\xB5\xAD\xAC\x50\x6B\xDE\xDD\xBB\x57\x76\x27\x89\xA7\x80\x6B\x08\x66\xA7\x4B\xD1\xC8\xBD\x84\xA8\x9B\x74\x51\x97\x51\xD8\x55\x01\xBD\xEE\x27\xD2\x1D\xC3\x18\x97\xF3\x6F\xC6\xC8\x8C\x30\x79\xB4\xB8\xCF\xF7\x4F\x41\x2D\x90\x74\x78\x6F\xFD\x61\x71\xA0\xFF\x14\xE0\xB2\x2E\x5C\xE4\x9E\xCF\x3D\x64\x09\xF3\x8A\x03\x12\x31\x1A\x9A\xCE\x39\x19\xB6\x12\x4C\xEF\xD8\x96\x4D\x6D\x09\x40\x68\x26\x7E\x80\x59\x04\x47\xEE\x3F\xDC\xF9\xF8\xC8\xFF\x7D\xDC\xBD\xAE\x99\x34"
#define GHP_PUB_KEY "-----BEGIN PUBLIC KEY-----\nMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAitNPvOqPKMu3JKPI1Fg/\nPV/JDoMzblLbli+xaRmcx+K97QhNon91k9gEovCnhUdsUq3jO5YAGUdpHBAv2aAW\n2DkNohn4wG0t2/ZKrOJjTCKZZFqNcXFxyFmlRqCSFrIbc86tDnp4TgPbzcRVh4EQ\nnS7mcDQsyQ3FssVqbF6SibOQ3aV8GsFuwu6qrbN1Juh0IprPxhch5e337P6bv/G7\n767NWnrBmg15afzQc5w2CI9Oq0Yyu1BjnaoLEwqkoOxjnK39bUp6tPfPaE5QBo05\nxNtXJmGobNPosKi5yo66f9Llc6qHMfRXG+8wSYnG5xk3D+qG3V7ovJ7W9cHg456r\n3vnSlKwHFw2I/ZtO/TGRq19YH04RSQq/VQwxXqUQZwRo+14kCopzz5en5F1sI6X1\nKqfo+dn3BH+8G9ZqAh+hC9N2ghG7rpss4A+kKYUIrB4jss5dTiRLCJw+ZGp19cYn\np3NdROvUHKLE4HK993c0xGritE5uiqHBLFmKjvukx1pI8i5423UfGkxHiPuQ+A4w\n8kgUhIc5FC5TLAW0wPt+uxA/7vdqc0lnNuyyx2hU4YZIc2bDV8GKKuRyAsdJ2NMv\nKOjq98nx3vW7m4/H48bob32WTa1KRcj/cd/e1cizwIqEcBD6+rXSR+azXLc7SRKo\nVWXn2b/No6p0+LsL95m86i0CAwEAAQ==\n-----END PUBLIC KEY-----\n"

#define GHP_KEY_1	0x5065E645
#define GHP_KEY_2	0x47CB31B8
#define GHP_KEY_3	0x46EEF9A0
#define GHP_KEY_4	0xC18B799C

#ifdef R_WARZ
#define LICENSE_NAME 	L"GHPLIC.dll"
#define DATABASE_NAME 	L"GHPDB.dll"
#define DLL_NAME_SIG	L"WZ_GHP.dll"
#define DLL_NAME_VEF	L"GHP.dll"
#endif

#ifdef R_MUONLINE
#define LICENSE_NAME 	L"GHP.lic"
#define DATABASE_NAME 	L"GHP.db"
#define DLL_NAME_SIG	L"GHP.dll"
#define DLL_NAME_VEF	L"GHP.dll"
#endif

#include <WinSock2.h>
#include <Windows.h>
#ifdef R_WARZ
#include <d3dsdk\d3d9.h>
#include <d3dsdk\d3dx9.h>
#endif
#include <Psapi.h>
#include <TlHelp32.h>
#include <IPHlpApi.h>
#include <NTSecAPI.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <list>
#include <time.h>

#include <VMProtectSDK.h>

#ifdef R_WARZ
#pragma comment (lib, "D3D9")
#pragma comment (lib, "D3DX9")
#endif
#pragma comment (lib, "PSAPI")
#pragma comment (lib, "WS2_32")
#pragma comment (lib, "IPHLPAPI")

#pragma comment (lib, "ghp_UTILS.lib")
#pragma comment (lib, "ghp_CRYPTO.lib")
#pragma comment (lib, "ghp_NET.lib")

#endif
