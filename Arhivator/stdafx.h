// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN             // nu includem toate prostiile
// Windows Header Files:
#include <windows.h>
//#define _HAS_ITERATOR_DEBUGGING 0 //pentru ca pm... s-ar putea sa faca incet
// daca folosesti algoritmii std de sortare si gasire in std::vectori sau std::list
// astia o sa ruleze foarte incet pe versiunea Debug daca  nu faci _HAS_ITERATOR_DEBUGGING 0
// C RunTime Header Files

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include<shlobj.h>

#include <fstream>

#define _CRTDBG_MAP_ALLOC // nuj ce are asta .. tb sa arate si numele fisierului de leak :(.. mai vedem
						// poate gasesti tu ce are... poate nici nu o sa avem leakuri si nu avem nevoie :))

#include <stdlib.h>
#include <Commdlg.h> // pentru dialogurile de open file, save file (browse)

#include <crtdbg.h>
#define new DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)