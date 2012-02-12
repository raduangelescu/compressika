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

