#include "stdafx.h"
#include "ArchivingFileSystem.h"
cArchivingFileSystem * cArchivingFileSystem::m_Singleton=NULL;
cArchivingFileSystem::cArchivingFileSystem()
{
}
void cArchivingFileSystem::FindAllFiles(std::string searchThisDir, bool searchSubDirs)
{
	// Ce avem nevoie ca sa cautam fisierele
	WIN32_FIND_DATA FindFileData = {0};
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// Construim stringul de search
	std::string searchDir;

	//Daca avem deja path cu'\' la capat , adaugam doar *
	if(searchThisDir.c_str()[searchThisDir.size() - 1] == '\\')
	{
		searchDir.append(searchThisDir);
		searchDir.append("*");
	}
	// altfel adaugam \* la sfarsit
	else
	{
		searchDir.append(searchThisDir);
		searchDir.append("\\*");
	}

	// Gasim primul fisier din director
	hFind = FindFirstFile(searchDir.c_str(), &FindFileData);

	// Daca nu e nici un fisier returnam
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Gaseste fisiere!
	do
	{
		// daca e in directorul "." continua
		if(strcmp(FindFileData.cFileName, ".") == 0)
		{
			continue;
		}

		// la fel ".." continua
		if(strcmp(FindFileData.cFileName, "..") == 0)
		{
			continue;
		}		

		// If we find a directory
		if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			// Cautam in subdirectoare .. da deobicei da
			if(searchSubDirs)
			{
				//Noul director de cautat
				std::string searchDir2;

				//facem ca la inceput
				if(searchThisDir.c_str()[searchThisDir.size() - 1] == '\\')
				{
					searchDir2.append(searchThisDir);
					searchDir2.append(FindFileData.cFileName);
				}
				else
				{
					searchDir2.append(searchThisDir);
					searchDir2.append("\\");
					searchDir2.append(FindFileData.cFileName);
				}
				// recursivitatea bat-o vina
				FindAllFiles((char*)searchDir2.c_str(), true);
			}
			continue;
		}

		//creem path pentru fisier
		std::string filePath;
		filePath.append(searchThisDir);
		filePath.append("\\");
		filePath.append(FindFileData.cFileName);
		m_FileNames.push_back(filePath);
	}
	// Buclim  :)) cat timp gasim fisiere
	while(FindNextFile(hFind, &FindFileData) != 0);
	FindClose(hFind);

}
void cArchivingFileSystem::AddFile(std::string filePath)
{
	m_FileNames.push_back(filePath);
}
void cArchivingFileSystem::RemoveFile(std::string filePath)
{
	for(std::vector<std::string>::iterator i=m_FileNames.begin();i!=m_FileNames.end();i++)
		if(!(*i).compare( filePath))
		{
			m_FileNames.erase(i);
			break;
		}
}