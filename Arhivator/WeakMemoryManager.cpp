#include "stdafx.h"
#include "WeakMemoryManager.h"
cWeakMemoryManager* cWeakMemoryManager::m_Singleton=NULL;
struct dictionary
	{
		int code_value;
		int parent_code;
		char character;
	};
void cMemBlock::DeleteBlock()	
{
		switch (m_type)
		{
		case MMW_INTEGER:
				delete [] ((int*)m_adress);
				break;
			case MMW_CHAR:
				delete [] ((char*)m_adress);
			break;
			case MMW_LONG:
				delete [] ((long*)m_adress);
			break;
			case 	MMW_DICTIONARY:
				delete[]((dictionary*)m_adress);
			break;
		}
}
cMemBlock::~cMemBlock()
{
}
cWeakMemoryManager::cWeakMemoryManager()
{
}
cWeakMemoryManager::~cWeakMemoryManager()
{
}
void cWeakMemoryManager::RegisterVariable(void *adress,VariableType tip)
{
	cMemBlock new_block(adress,tip);
	m_MemoryBlocks.push_back(new_block);
}
void cWeakMemoryManager::UnregisterVariable(void *adress)
{
	for(std::vector<cMemBlock>::iterator i=m_MemoryBlocks.begin();i!=m_MemoryBlocks.end();i++)
	{
		if(adress==(i->m_adress))
		{
			m_MemoryBlocks.erase(i);
			break;
		}
	}
}
void cWeakMemoryManager::CleanUp()
{

	for(std::vector<std::fstream*>::iterator i=m_Files.begin();i!=m_Files.end();i++)
	{
		(*i)->close();
	}
	m_Files.clear();
		for(std::vector<cMemBlock>::iterator i=m_MemoryBlocks.begin();i!=m_MemoryBlocks.end();i++)
	{
		i->DeleteBlock();
	}
	m_MemoryBlocks.clear();
}
void cWeakMemoryManager::UnregisterFile(std::fstream *file)
{

	for(std::vector<std::fstream*>::iterator i=m_Files.begin();i!=m_Files.end();i++)
	{
		if(file==(*i))
		{
			m_Files.erase(i);
			break;
		}
	}

}
void  cWeakMemoryManager::RegisterFile(std::fstream* file)
{
	m_Files.push_back(file);
}