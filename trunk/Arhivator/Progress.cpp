#include "stdafx.h"
#include "Progress.h"
#include "Algorithm.h"
#include "AlgorithmsManager.h"
#include "Timer.h"
HWND hProgressWnd;
cMyTimer g_timer;// Timerul 
DWORD ThreadID=NULL;
DWORD ThreadIDTimer=NULL;
HANDLE ThreadHandle=NULL;
HANDLE ThreadHandleTimer=NULL;
ThreadData* emptyData=NULL;
BOOL g_bRunning=FALSE;
BOOL CALLBACK ProgressDialogProc (HWND hwnd, 
								  UINT message, 
								  WPARAM wParam, 
								  LPARAM lParam);
DWORD WINAPI timer_Update(void *Data)//tre sa fie null
{
	clock_t oldtime=g_timer.m_clocks;
	while(g_bRunning)
	{
		g_timer.Update();
		if(g_timer.m_clocks-oldtime>200)
		{
			oldtime=g_timer.m_clocks;
			// nu facem updateul la timer non stop fiindca flikare..
		std::string ftime;
		g_timer.GetFormattedCurrentTime(ftime);
		SetDlgItemText( hProgressWnd,IDC_STATICTIMER,ftime.c_str());
		}
	}
	return 1;

}
BOOL InitInstance(HINSTANCE hInstance);
DWORD WINAPI Compress(void *Data)
{

		ThreadData*m_data=(ThreadData*)Data;
		int currentAlgo= SendDlgItemMessage(m_data-> hParentWindow, IDC_LIST1, LB_GETCURSEL, 0, 0);
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"GENERATING CRC FILE..");
		AlgoManager->Algos[currentAlgo]->GenerateCRCFile(m_data->FolderPath,m_data->FolderPathOut);
		g_timer.Stop();
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"COMPRESSING..");
		g_timer.Start();
		AlgoManager->Algos[currentAlgo]->Compress(m_data->FolderPath,m_data->FolderPathOut);
		g_timer.Stop();
		g_bRunning=FALSE;
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"Ready..");
		delete m_data;
		emptyData=NULL;
		SetDlgItemText( hProgressWnd,	IDC_CANCELCOMPRESS,"Finish");
		return 1;
}
DWORD WINAPI Decompress(void *Data)
{
		
		ThreadData*m_data=(ThreadData*)Data;
		int currentAlgo= SendDlgItemMessage(m_data-> hParentWindow, IDC_LIST1, LB_GETCURSEL, 0, 0);
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"DECOMPRESSING..");
		g_timer.Start();
		AlgoManager->Algos[currentAlgo]->DeCompress(m_data->FolderPath,m_data->FolderPathOut);
		g_timer.Stop();
		g_timer.Start();
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"CHECKING CRC FILE..");
		int test=AlgoManager->Algos[currentAlgo]->CheckCRCFile(m_data->FolderPathOut,m_data->FolderPath);
		g_timer.Stop();
		g_bRunning=FALSE;
		if(test)
			MessageBox(hProgressWnd,"ok","Totul e aparent OK!",0);
		else
			MessageBox(hProgressWnd,"eroare","CRC ERROR!!",0);
		SetDlgItemText( hProgressWnd,IDC_DOGroup,"Ready..");
		delete Data;
		emptyData=NULL;
		SetDlgItemText( hProgressWnd,	IDC_CANCELCOMPRESS,"Finish");
		return 1;
}
unsigned int last_progress_update=0;
void UpdateFileProgressBar(unsigned int progress,unsigned int filesize)
{
	if(((progress*100)/filesize)-last_progress_update>=2)
	{
		last_progress_update=((progress*100)/filesize);
		SendDlgItemMessage( hProgressWnd, IDC_FILE_PROGRESS_BAR,  PBM_SETPOS ,(progress*100)/filesize, 0);
	}
//hmmm
}
DWORD WINAPI ProgressDialog(void *Data)
{
	ThreadData*m_data=(ThreadData*)Data;
	 emptyData=m_data;
	if(!InitInstance( m_data->hInst))
		return 0;
	SendDlgItemMessage( hProgressWnd, IDC_FILE_PROGRESS_BAR, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	if(m_data->bCompress)
	{
		g_timer.Start();
		g_bRunning=TRUE;
			ThreadHandle=CreateThread( 
				 NULL,                   // default security attributes
				  0,                      // use default stack size  
				Compress,       // thread function name
				 Data,          // argument to thread function 
				 0,                      // use default creation flags 
				&ThreadID);   // returns the thread identifier 
			ThreadHandleTimer=CreateThread( 
				 NULL,                   // default security attributes
				  0,                      // use default stack size  
				 timer_Update,       // thread function name
				 NULL,          // argument to thread function 
				 0,                      // use default creation flags 
				&ThreadIDTimer);   // returns the thread identifier 
	
		/*std::string endcomp="Compreesion took: ";
		std::string timeform;
		g_timer.GetFormattedTime(timeform);
		endcomp+=timeform;
		SetDlgItemText(hwnd,IDC_STATUSTXT,endcomp.c_str());*/
	}
	else
	{
		g_bRunning=TRUE;
		ThreadHandle=CreateThread( 
				 NULL,                   // default security attributes
				  0,                      // use default stack size  
				Decompress,       // thread function name
				 Data,          // argument to thread function 
				 0,                      // use default creation flags 
				&ThreadID);   // returns the thread identifier 
		ThreadHandleTimer=CreateThread( 
				 NULL,                   // default security attributes
				  0,                      // use default stack size  
				 timer_Update,       // thread function name
				 NULL,          // argument to thread function 
				 0,                      // use default creation flags 
				&ThreadIDTimer);   // returns the thread identifier 

		/*std::string endcomp="Decompreesion took: ";
		std::string timeform;
		g_timer.GetFormattedTime(timeform);
		endcomp+=timeform;
		SetDlgItemText(hwnd,IDC_STATUSTXT,endcomp.c_str());*/
		//DestroyWindow (hProgressWnd);
	}
	return 1;
}
BOOL CALLBACK ProgressDialogProc (HWND hwnd, 
								  UINT message, 
								  WPARAM wParam, 
								  LPARAM lParam)
{
	

	switch (message)
	{
	case WM_INITDIALOG:	
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_CANCELCOMPRESS:
				TerminateThread (ThreadHandleTimer,0);
				TerminateThread (ThreadHandle,0);
				MEMORY_MANAGER->CleanUp();
				CloseHandle(ThreadHandleTimer);
				CloseHandle(ThreadHandle);
				DestroyWindow (hwnd);
				delete emptyData;
			break;

		} 
	case WM_HSCROLL:
		return 0;
	case WM_DESTROY:
		//PostQuitMessage(0);
		return TRUE;
	case WM_CLOSE:
		TerminateThread (ThreadHandleTimer,0);
		TerminateThread (ThreadHandle,0);	
		MEMORY_MANAGER->CleanUp();
		CloseHandle(ThreadHandleTimer);
		CloseHandle(ThreadHandle);
		DestroyWindow (hwnd);
		if(emptyData)
			delete emptyData;
		return TRUE;
	}

	return FALSE;
}
BOOL InitInstance(HINSTANCE hInstance)
{
	

	 hProgressWnd = CreateDialog (hInstance, 
		MAKEINTRESOURCE (IDD_PROGRESS ), 
		0, 
		ProgressDialogProc);
	if (! hProgressWnd)
	{
		return FALSE;
	}

	ShowWindow( hProgressWnd, 1);
	UpdateWindow( hProgressWnd);
	return TRUE;
}