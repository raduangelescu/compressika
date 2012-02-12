// Arhivator.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "Arhivator.h"
#include "Progress.h"
#include "Algorithm.h"
#include "AlgorithmsManager.h"
#include "Timer.h"
BOOL bThreadActive=FALSE;



// da.. stiu ca arata urat... am putea sa impartim pe aici in clase
// doar ca kkt-ul de proiect Win32 ai nevoie de CALLBACKURI dinalea de c
// si orice am face tot treubie sa avem functii globale
// insa de variabilele globale am putea sa scapam.. ca sunt nasoale
#define MAX_LOADSTRING 100
unsigned int currentAlgo=0;// index de algoritm .. pt selectie din listbox
std::string FolderPath;// director de intrare
std::string FolderPathOut;// director de iesire

// Global Variables:
HINSTANCE hInst;								// instanta curenta
TCHAR szTitle[MAX_LOADSTRING];					// textul din title bar
TCHAR szWindowClass[MAX_LOADSTRING];			// numele clasei ferestrei principale
void PopulateAlgoList(HWND hWnd);// bagam algoritmii in lisbox
bool GetFolder(std::string& folderpath, // facem rost de folder.. asta daca o sa baga vreodata optiune de arhivare folder
										
			   char* szCaption = NULL, 
			   HWND hOwner = NULL);

bool GetOutFile(std::string &filepath,HWND hwnd)
{
	OPENFILENAME ofn;      
	char szFile[260];      

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// display pt dialogul de open file
	if (GetSaveFileName(&ofn)==TRUE)
	{
		filepath=ofn.lpstrFile;
		return true;
	}
	return false;
}
bool GetFile(std::string &filepath,HWND hwnd)
{
	OPENFILENAME ofn;      
	char szFile[260];      

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;

	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	// asta o sa tb sa il schimb.. adica txt? wtf.. sa decidem si format de arhiva
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//la fel ca mai sus
	if (GetOpenFileName(&ofn)==TRUE)
	{
		filepath=ofn.lpstrFile;
		return true;
	}
	return false;
}

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int,char*);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK MainDialogProc (HWND hwnd, 
							  UINT message, 
							  WPARAM wParam, 
							  LPARAM lParam);
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// initializare stringuri globale

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ARHIVATOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Initializare aplicatie:
	if (!InitInstance (hInstance, nCmdShow,lpCmdLine))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARHIVATOR));
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	cLogger::getSingleton()->Release();
	return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	//wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ARHIVATOR);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


bool GetFolder(std::string& folderpath, 
			   char* szCaption, 
			   HWND hOwner)
{
	bool retVal = false;

	

	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));

	bi.ulFlags   = BIF_USENEWUI;
	bi.hwndOwner = hOwner;
	bi.lpszTitle = szCaption;

	::OleInitialize(NULL);


	LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

	if(pIDL != NULL)
	{


		char buffer[_MAX_PATH] = {'\0'};
		if(::SHGetPathFromIDList(pIDL, buffer) != 0)
		{

			folderpath = buffer;
			retVal = true;
		}



		CoTaskMemFree(pIDL);
	}

	::OleUninitialize();

	return retVal;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow,char *setup)
{
	HWND hWnd;

	hInst = hInstance; 
	hWnd = CreateDialog (hInst, 
		MAKEINTRESOURCE (IDD_MAINWINDOW ), 
		0, 
		MainDialogProc );
	PopulateAlgoList(hWnd);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	return TRUE;
}

void PopulateAlgoList(HWND hWnd)
{
	std::vector<cAlgorithm*>::iterator it;
	for(it=AlgoManager->Algos.begin();it!=AlgoManager->Algos.end();it++)
	{
		SendDlgItemMessage(hWnd,
			IDC_LIST1,
			LB_ADDSTRING,
			0,
			(LPARAM)(LPTSTR)((*it)->getName().c_str()));
	
	}
	SendDlgItemMessage(hWnd, IDC_LIST1, LB_SETCURSEL, 0, 0);

}


// deocamdata asta nici nu apare :)
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
BOOL CALLBACK MainDialogProc (HWND hwnd, 
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
		case IDC_BUTTONBROWSE:
			GetFile(FolderPath,hwnd);
			SetDlgItemText( hwnd,IDC_FILEINPUTEDIT,FolderPath.c_str());
			//GetFolder(FolderPath);	
			break;
		case IDC_BUTTONOUTPUT:
				GetOutFile(FolderPathOut,hwnd);
				SetDlgItemText( hwnd,IDC_FILEOUTPUTEDIT,FolderPathOut.c_str());
			break;
		case IDC_BUTTONCOMPRESS:
			if(FolderPath.empty()||FolderPathOut.empty())
				MessageBox(hwnd,"Nu ai selectat fisier!!","Selecteaza fisier",0);
			else
			{			
				ThreadData* Data=new ThreadData;
				Data->bCompress=TRUE;
				Data->FolderPath=FolderPath;
				Data->FolderPathOut=FolderPathOut;
				Data->hInst=hInst;
				Data->bStat=FALSE;
				Data->hParentWindow=hwnd;
				ProgressDialog(Data);
 
			}
			break;
		case IDC_BUTTONUNCOMPRESS:
			if(FolderPath.empty()||FolderPathOut.empty())
				MessageBox(hwnd,"Nu ai selectat fisier!!","Selecteaza fisier",0);
			else
			{
				ThreadData* Data=new ThreadData;
				Data->bCompress=FALSE;
				Data->FolderPath=FolderPath;
				Data->FolderPathOut=FolderPathOut;
				Data->hInst=hInst;
				Data->bStat=FALSE;
				Data->hParentWindow=hwnd;
				ProgressDialog(Data);
	
			
			}			
			break;
		case IDC_GENSTAT:
			if(FolderPath.empty()||FolderPathOut.empty())
				MessageBox(hwnd,"Nu ai selectat fisier!!","Selecteaza fisier",0);
			else
			{
				ThreadData* Data=new ThreadData;
				Data->bCompress=FALSE;
				Data->FolderPath=FolderPath;
				Data->FolderPathOut=FolderPathOut;
				Data->hInst=hInst;
				Data->bStat=TRUE;
				Data->hParentWindow=hwnd;
				ProgressDialog(Data);
	
			
			}	
			break;

		} 
	case WM_HSCROLL:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_CLOSE:
		AlgoManager->Release();
		MEMORY_MANAGER->Release();
		DestroyWindow (hwnd);
		return TRUE;
	}

	return FALSE;
}