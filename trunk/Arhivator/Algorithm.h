#ifndef MY_CALGORITHM
#define MY_CALGORITHM
#include "Utile.h"
#include "Progress.h"
// clasa de baza pentru algoritmi... orice algoritm tb sa o mosteneasca
enum CRC_Generate_Mode
{
	GENERATE,
	CHECK
};
class cAlgorithm
{

public:
	std::string m_Ext;
	cAlgorithm();
	virtual ~cAlgorithm(){}
	virtual void Compress(std::string filenameInput,std::string filenameOutput)=0;
	int GenerateCRCVector(std::fstream &filein,std::fstream &out,CRC_Generate_Mode Mode);
	void GenerateCRCFile(std::string filenameInput,std::string filenameOutput);
	int CheckCRCFile(std::string filenameInput,std::string fileout);
	virtual void DeCompress(std::string filenameInput,std::string filenameOutput)=0;
	std::string getName(){return m_Name;}
	void GetFileSize(std::string filename);
protected:
	std::string m_Name;
public:
	unsigned int m_FileSizeInBytes;
	unsigned int m_CompressionProgress;
};
#endif