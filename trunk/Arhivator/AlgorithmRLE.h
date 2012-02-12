#ifndef M_RLEANALGO
#define M_RLEALGO
#include "Algorithm.h"
//NEIMPLEMENTAT.. poate o sa avem RLE simplu.. desi ma gandesc sa bagam doar
//de z7 .. lsw sau cum era..
class cAlgorithmRLE: cAlgorithm
{
	public:
		cAlgorithmRLE();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmRLE();

};
#endif