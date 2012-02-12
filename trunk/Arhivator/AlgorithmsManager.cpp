#include "stdafx.h"
#include "AlgorithmsManager.h"
#include "Algorithm.h"
cAlgorithmManager *cAlgorithmManager::m_Singleton=0;
void cAlgorithmManager:: Release()
{
	std::vector<cAlgorithm*>::iterator it;
	delete m_Singleton;
}
void cAlgorithmManager::AddAlgorithm(cAlgorithm *algo)
{
	Algos.push_back(algo);	
}