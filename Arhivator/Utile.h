
#include <list>
#include <fstream>
#include <iostream>
#include <vector>
#include <time.h>
#include "WeakMemoryManager.h"
#ifndef CLOGGER123
#define CLOGGER123
#define LOGFILE "Log.txt"
#define END_OF_STREAM 256
// clasa folosita pt scris in fisierul de log
#define LOG cLogger::getSingleton()->Log
class cLogger
{
	static cLogger * m_Singleton;
	std::fstream m_LogFile;
	cLogger();
public:
	void Log(std::string Logstuff);
	static cLogger* getSingleton()
	{
		if (m_Singleton==0)
			m_Singleton= new cLogger();
		return m_Singleton;
	}
	void Release()
	{
		if(m_Singleton)
			delete m_Singleton;
	}
	~cLogger();
};
unsigned int g_GetFileSize(std::string filename);
int BitLength(unsigned long val);

#define FILE_BUFFER_SIZE 1024
#define NUM_THREADS 4

#endif