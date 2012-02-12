#ifndef M_WEAKMEMORYMANAGER
#define  M_WEAKMEMORYMANAGER
#include "Utile.h"
#define MEMORY_MANAGER cWeakMemoryManager::getSingleton()
enum VariableType
{
	MMW_INTEGER,
	MMW_CHAR,
	MMW_LONG,
	MMW_DICTIONARY
};
struct cMemBlock
{
	void * m_adress;
	VariableType m_type;
	cMemBlock(void*adress,VariableType type):m_adress(adress),m_type(type){}
	void DeleteBlock();
	~cMemBlock();

};
class cWeakMemoryManager
{
	static cWeakMemoryManager * m_Singleton;
	std::vector<cMemBlock> m_MemoryBlocks;
	std::vector<std::fstream *> m_Files;// foarte periculoasa treaba asta
	cWeakMemoryManager();
public:
	static cWeakMemoryManager* getSingleton()
	{
		if (m_Singleton==0)
			m_Singleton= new cWeakMemoryManager();
		return m_Singleton;
	}
	void RegisterVariable(void *adress,VariableType tip);
	void UnregisterVariable(void *adress);
	void RegisterFile(std::fstream *file);
	void UnregisterFile(std::fstream *file);
	void CleanUp();
	void Release()
	{
		if(m_Singleton)
			delete m_Singleton;
	}
	~cWeakMemoryManager();
};
#endif