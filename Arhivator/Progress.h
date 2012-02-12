#pragma once
#include <windows.h>
#include "resource.h"
class ThreadData
{
public:
	HINSTANCE hInst;
	std::string FolderPath;
	std::string FolderPathOut;
	BOOL bCompress;
	HWND hParentWindow;
	ThreadData()
	{
	}
	~ThreadData()
	{
	}
};
DWORD WINAPI ProgressDialog(void *Data);
void UpdateFileProgressBar(unsigned int progress,unsigned int filesize);