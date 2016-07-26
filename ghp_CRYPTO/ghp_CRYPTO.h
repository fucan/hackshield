#include "..\include.h"
#include "..\ghp_UTILS\ghp_DEFS.h"

#include "polarssl\config.h"
#include "polarssl\pk.h"
#include "polarssl\entropy.h"
#include "polarssl\ctr_drbg.h"
#include "polarssl\sha1.h"

#pragma comment (lib, "PolarSSL.lib")
#pragma comment (lib, "RakNetLibStatic.lib")

namespace ghp_CRYPTO
{
	/*extern inline DWORD EncryptDWORD(DWORD value);
	extern inline DWORD DecryptDWORD(DWORD value);
	extern bool GenerateRaketNetKeys(UCHAR * publickey, UCHAR * privatekey);
	extern bool GenerateRSAKey(std::string Peer, std::string * publickey, std::string * privatekey);
	extern bool GetFileSignature(std::wstring FileName, std::string PrivateKey, std::string Peer, ghp_UTILS::DataChunk * signature);
	extern bool SignFile(std::wstring FileName, std::string PrivateKey, std::string Peer);
	extern bool RemoveSignatureFromDataChunk(ghp_UTILS::DataChunk * memory, ghp_UTILS::DataChunk * signature);
	extern bool VerifyDataChunkSignature(ghp_UTILS::DataChunk * memory, ghp_UTILS::DataChunk * signature, std::string PublicKey, std::string Peer);
	extern bool VerifyFileSignature(std::wstring FileName, ghp_UTILS::DataChunk * signature, std::string PublicKey, std::string Peer);
	extern bool EncFileAES(std::wstring FileNameIn, std::wstring FileNameOut, UCHAR * key);
	extern bool DecAES(ghp_UTILS::DataChunk * in, ghp_UTILS::DataChunk * out, UCHAR * key);
	extern bool DecFileAESToMemory(std::wstring FileName, ghp_UTILS::DataChunk * memory, UCHAR * key);*/
	extern std::string CalculeSHA1(ghp_UTILS::DataChunk * memory);
	extern std::string GetFormatedSHA1(ghp_UTILS::DataChunk * memory);
	extern bool CalculeFileSHA1(std::wstring FileName, UCHAR * hash);
}