#pragma once

#include "stdafx.h"

BOOL CReadFile(PCHAR File, byte** Out, PDWORD pLength);
BOOL CWriteFile(PCHAR File, PBYTE Buffer, DWORD Length);
void XNotify(wchar_t* Message);
void XamShowKeyboardUI(LPCWSTR titleText, LPCWSTR descriptionText, int* error, char* out, int length, LPCWSTR defText = L"");
bool DirectoryExists(const char* path);
DWORD XUserFindUser(XUID qwXuid, int dwIndex, char* szGamertag, int dwGamertagLen, XUID* qwOutXuid, int r8);
bool DoesFileExist(const char* pFilename, DWORD* pSize);