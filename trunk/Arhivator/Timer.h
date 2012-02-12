#ifndef MY_CTIMER
#define MY_CTIMER
#include "Utile.h"
////////////////////////////////////////////////////////////////////////////////
// clasa asta am facut-o sa masoare timpul de rulare pt algoritm
// nu are update real.. o sa dea timpul dupa ce dai stop (nu are rost sa facem
// thread separat.. apar alte probleme)
// momentan implementarea asta merge dupa clockurile procesorului... 
// sper ca e suficient de exacta... desi clock() are implementari aiurea
// pe anumite sisteme de operare
// UPDATE:... acum are si update real :))...
////////////////////////////////////////////////////////////////////////////////
class cMyTimer
{
public:
	clock_t m_clocks;
	clock_t m_startClocks;
public:
	cMyTimer();
	~cMyTimer();
	void Start();
	void Update();
	void GetFormattedTime(std::string &time_f);
	void GetFormattedCurrentTime(std::string &time_f);
	void Stop();
	void Reset();
};
#endif