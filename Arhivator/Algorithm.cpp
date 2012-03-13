#include "stdafx.h"
#include "AlgorithmsManager.h"
#include "stdafx.h"
#include "Algorithm.h"
#include "Crc.h"
#define BLOCK_SIZE 256
cAlgorithm::cAlgorithm()
{
	AlgoManager->AddAlgorithm(this);
	m_CompressionProgress=0;
	m_isLossy = false;
}

int cAlgorithm::GenerateCRCVector(std::fstream &filein,std::fstream &out,CRC_Generate_Mode Mode)
{
	char block[BLOCK_SIZE];// facem crc checkingul pe blocuri de BLOCK_SIZE.. putem pe oricat
	unsigned int curr=0;
	block[curr++]=filein.get();
	unsigned int res=0;
	while(block[curr-1]!=EOF)
	{
		if(curr<BLOCK_SIZE-1)
			block[curr++]=filein.get();
		else
		{
			res = CRC::Hash(reinterpret_cast<unsigned char*>(block), curr);
			curr=0;
			if(Mode==GENERATE)
				out<<res<<" ";
			else
				if(Mode==CHECK)
				{
					unsigned int test;
					out>>test;
					if(test!=res)
						return 0;
				}
		}
	}
	if(curr!=0)
	{
		res = CRC::Hash(reinterpret_cast<unsigned char*>(block), curr);
			if(Mode==GENERATE)
				out<<res<<" ";
			else
				if(Mode==CHECK)
				{
					unsigned int test;
					out>>test;
					if(test!=res)
						return 0;
				}
	}
	return 1;
}
int cAlgorithm::CheckCRCFile(std::string filenameInput,std::string fileout)
{
	if(!m_isLossy)
	{
		std::fstream in(filenameInput.c_str(),std::ios::in|std::ios::binary);
		std::string crcf=fileout;
		crcf+=".crc";
		std::fstream out(crcf.c_str(),std::ios::in);
		int ret=GenerateCRCVector(in,out,CHECK);
		in.close();
		out.close();
		return ret;
	}
	return 1;
}
void cAlgorithm::GenerateCRCFile(std::string filenameInput,std::string filenameOutput)
{
	if(!m_isLossy)
	{
		std::fstream in(filenameInput.c_str(),std::ios::in|std::ios::binary);
		std::string crcf=filenameOutput;
		crcf+=".crc";
		std::fstream out(crcf.c_str(),std::ios::out);
		GenerateCRCVector(in,out,GENERATE);
		in.close();
		out.close();
	}
}
// Functia asta nu e neaparat exacta.. adica poate arata mai marf fisierele
// decat sunt , si mai are dezavantajul ca maxim o sa putem pe unsigned int
// ceea ce inseamna fisiere de 4 giga... asta se poate schimba inlocuind totul
// cu __int64 (dar dupaia nu merge decat pe visual), deocamdata o lasam asa
// si daca e modificam dupa , oricum orice ai face la getfilesize.. o sa 
// existe intotdeauna fisiere atat de mari incat sa nu returneze cum tb valoarea
void cAlgorithm::GetFileSize(std::string filename)
{
  std::ifstream f;
  f.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
  if (!f.good() || f.eof() || !f.is_open()) { return ; }
  f.seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = f.tellg();
  f.seekg(0, std::ios_base::end);
  m_FileSizeInBytes =static_cast<unsigned int>(f.tellg() - begin_pos);
  f.close();
}