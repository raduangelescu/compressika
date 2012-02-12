#ifndef M_ARCHIVINGFILESYSTEM
#define  M_ARCHIVINGFILESYSTEM
#include "Utile.h"
#define ARCHIVE_FILE_SYSTEM cArchivingFileSystem::getSingleton()

class cArchivingFileSystem
{
	static cArchivingFileSystem * m_Singleton;
	cArchivingFileSystem();
	std::vector<std::string> m_FileNames;
public:
	
	static cArchivingFileSystem* getSingleton()
	{
		if (m_Singleton==0)
			m_Singleton= new cArchivingFileSystem();
		return m_Singleton;
	}
	void FindAllFiles(std::string searchThisDir, bool searchSubDirs=true);
	void AddFile(std::string filePath);
	void RemoveFile(std::string filePath);
	void Release()
	{
		if(m_Singleton)
			delete m_Singleton;
	}
	~cArchivingFileSystem();
};
#endif