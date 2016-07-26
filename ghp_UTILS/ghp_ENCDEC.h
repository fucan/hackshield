#ifndef _GHP_ENCDEC_H_
#define _GHP_ENCDEC_H_

#include "..\include.h"

//////////////////////////////////////////////////////////////////////////////////////
//----------------------------User Functions------------------------------------------
extern int DecryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len);
extern int EncryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len);
extern int DecryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len);
extern int EncryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len);
extern int LoadKeys(unsigned const char * buf, unsigned long*Where);
extern void DecXor32(unsigned char*Buff,int SizeOfHeader,int Len);
extern void EncXor32(unsigned char*Buff,int SizeOfHeader,int Len);
extern void EncDecLogin(unsigned char*Buff,int Len);
//-----------------------------internal functions-------------------------------------
extern int DecryptC3(unsigned char*Dest,unsigned char*Src,int Len,unsigned long*Keys);
extern int EncryptC3(unsigned char*Dest,unsigned char*Src,int Len,unsigned long*Keys);
extern int DecC3Bytes(unsigned char*Dest,unsigned char*Src,unsigned long*Keys);
extern void EncC3Bytes(unsigned char*Dest,unsigned char*Src,int Len,unsigned long*Keys); 
extern int HashBuffer(unsigned char*Dest,int Param10,unsigned char*Src,int Param18,int Param1c);
extern void ShiftBuffer(unsigned char*Buff,int Len,int ShiftLen);

#endif