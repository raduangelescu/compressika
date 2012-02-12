#ifndef CALGORITHM_MANAGER
#define CALGORITHM_MANAGER
#include "Utile.h"
#define  AlgoManager cAlgorithmManager::getSingleton()
// Aici e managerul de algoritmi.. clasa care ii gestioneaza
// fiecare algoritm se auto-adauga in lista Algos prin constructor
// nu cred ca tb sa isi bata nimeni capul cu asta :)
// (e singleton)
class cAlgorithm;
class cAlgorithmManager
{
	public:
		std::vector<cAlgorithm*> Algos;
	private:
		static cAlgorithmManager *m_Singleton;
	private:
		cAlgorithmManager(){}
		~cAlgorithmManager(){}
	public:
		void Release();
		static cAlgorithmManager *getSingleton()
		{
			if(m_Singleton==0)
				m_Singleton =new cAlgorithmManager;
			return m_Singleton;
		}
		void AddAlgorithm(cAlgorithm *algo);
};
#endif