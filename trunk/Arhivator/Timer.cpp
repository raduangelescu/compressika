#include <stdafx.h>
#include "Timer.h"
#include <sstream>
cMyTimer::cMyTimer()
{
}
cMyTimer::~cMyTimer()
{
}
void  cMyTimer::Update()
{
m_clocks=clock();
}
void cMyTimer::Reset()
{
	m_startClocks=clock();
	m_clocks=clock();
}
void cMyTimer::GetFormattedCurrentTime(std::string &time_f)
{
	m_clocks=clock()-m_startClocks;
	GetFormattedTime(time_f);
}
void cMyTimer::Start()
{
	m_clocks=clock();
	m_startClocks=clock();

}
void cMyTimer::GetFormattedTime(std::string &time_f)
{
	std::ostringstream o;
	time_f.clear();
	time_f="";
	double secunde=(double(m_clocks)/CLOCKS_PER_SEC);
	unsigned int minute=(unsigned int)secunde/60;
	unsigned int ore=minute/60;
	secunde=secunde-minute*60-ore*60*60;
	minute=minute-ore*60;

	o<<ore<<": "<<minute<<": "<<secunde;
	time_f=o.str();
	int t=0;
}
void cMyTimer::Stop()
{
	m_clocks=clock()-m_startClocks;
}