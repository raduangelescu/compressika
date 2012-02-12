#include "stdafx.h"
#include "Utile.h"
cLogger *cLogger::m_Singleton=0;
cLogger::cLogger()
	{
		m_LogFile.open(LOGFILE,std::ios::out);
		m_LogFile<<"Logger Initialized"<<"\n";
		m_LogFile.close();
	}
void cLogger::Log(std::string Logstuff)
	{
		m_LogFile.open(LOGFILE,std::ios::app);
		m_LogFile<<"--->"<<Logstuff.c_str()<<"\n";
		m_LogFile.close();
	}
cLogger::~cLogger()
{
}
unsigned int g_GetFileSize(std::string filename)
{
	std::ifstream f;
	f.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
	if (!f.good() || f.eof() || !f.is_open()) { return 0; }
	f.seekg(0, std::ios_base::beg);
	std::ifstream::pos_type begin_pos = f.tellg();
	f.seekg(0, std::ios_base::end);
	return static_cast<unsigned int>(f.tellg() - begin_pos);
	f.close();
}
