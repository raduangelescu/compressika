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
/* =============================================================
* Da numarul de biti necesar reprezentarii unui intreg:
* 0 to 1 -> 1,
* 2 to 3 -> 2,
* 3 to 7 -> 3, etc...
* Se poate mai rapid cu tabel de lookup.
*/
int BitLength(unsigned long val)
{
	int bits = 1;
	if (val > 0xffff)
	{
		bits += 16;
		val >>= 16;
	}	

	if (val > 0xff)
	{
		bits += 8;
		val >>= 8;
	}
	if (val > 0xf)
	{
		bits += 4;
		val >>= 4;
	}
	if (val > 0x3)
	{
		bits += 2;
		val >>= 2;
	}
	if (val > 0x1) 
		bits += 1;
	return bits;
}