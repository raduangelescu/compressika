#ifndef M_SHANNONALGO
#define M_SHANNONALGO
#include "Algorithm.h"
#include "BinaryTree.h"
#include "BitStreamSoup.h"
// clasa cu Algoritmul Shannon-Fano .. este primul algoritm de compresie inventat
// seamana cu Huffman foarte mult , difera doar modul de construire a
// arborelui binar (aici o luam de sus in jos.. la huffman e de jos in sus)
// are rata de compresie mai slaba putin decat huffman
// nu prea se foloseste in viata reala... huffman e alegerea mai buna 
// (pe majoritatea seturilor de date.. dar nu tot timpul :) )
class cAlgorithmShannon: cAlgorithm
{
	struct CODE 
	{ 
     unsigned int code; 
     int code_bits; 
	}; 
 
	public:
		cAlgorithmShannon();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmShannon();
	private:
		//indica urmatorul nod liber in lista de noduri
		unsigned int m_uiFreeNode;
		//arata de cate ori apare fiecare simbol in fisier
		unsigned long m_uivCounts[256];
		//codurile fiecarui simbol sunt retinute aici dupa parcurgerea arborelui
		CODE m_Codes[257];
		//nodurile efective (alocare statica , mai rapida , fara fragmentari.. aliniata)
		cBinaryTreeNode m_Nodes[514]; 
		
		// initializare noduri counts si codes
		void InitTree();
		// curatam dupa ce ne-am distrat
		void CleanUp();
		// numaram bytesi din fisier de input
		void CountBytes( std::fstream &input );
		// scalam counturile sa incapa pe un byte fiecare si le bagam in noduri
		void ScaleCounts( ); 
		// construim arborele binar... sau copacul :)
		void BuildTree(unsigned int first,unsigned int last,int topNode); 
		// gasim codurile fiecarui simbol
		void ConvertTreeToCode(  unsigned int code_so_far, int bits, int node ); 
		// scriem counturile in fisier.. formatul e putin mai special
		void OutputCounts( std::fstream &output ); 
		// citim counturile din fisier
		void InputCounts(  cBitStreamSoup &input); 
		// comprima data
		void CompressData( std::fstream &input,  cBitStreamSoup &output ); 
		// decomprima data
		void ExpandData(  cBitStreamSoup &input,std::fstream &output, int root_node ); 
};
#endif