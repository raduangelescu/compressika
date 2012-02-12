#ifndef M_HUFFMANALGO
#define M_HUFFMANALGO
#include "Algorithm.h"
#include "BinaryTree.h"
#include "BitStreamSoup.h"

// ar cam fi acelasi cod ca la Shannon .. mai putin functia
// Build Tree .. din motivul asta ma gandeam sa facem o clasa de baza comuna
// sa nu fie bloated code ... desi nu stiu daca merita , imi zici si tu parerea
// oricum daca pui codul din Shannon aici , poate treci prin el si gasesti 
// chestii de optimizat

class cAlgorithmHuffman: cAlgorithm
{
	struct CODE 
	{ 
     unsigned int code; 
     int code_bits; 
	}; 

	private:
		unsigned int m_uiFreeNode;				//indica urmatorul nod liber in lista de noduri
		CODE m_Codes[257];						//codurile fiecarui simbol sunt retinute aici dupa parcurgerea arborelui
		unsigned long m_uivByteCounts[256];		//vector ce memoreaza toate grupele de 8 biti si numarul lor de aparitii
		cBinaryTreeNode m_Nodes[514];			//nodurile efective
		int f_node;								//primul nod din arborele huffman

	public:
		cAlgorithmHuffman();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmHuffman();

	private:
		//metoda initializeaza primele 256 elemente ale vectorului m_Nodes
		void InitTree();

		//numara bytesii din fisier pt fiecare grup de 8 biti
		void CountBytes(std::fstream &input);	
		
		//scalazeaza numarul de aparitii astfel incat sa incapa pe un byte
		void ScaleCounts();
		
		//construieste arborele de compresie
		//void BuildTree(unsigned int BottomNode,int x);
		///construieste arborele de compresie
		void BuildTree();
		
		//codeaza fiecare simbol
		void ConvertTreeToCode(  unsigned int code_so_far, int bits, int node ); 
		
		// comprima data
		void CompressData( std::fstream &input,  cBitStreamSoup &output ); 
		
		// decomprima data
		void ExpandData(  cBitStreamSoup &input,std::fstream &output, int root_node ); 
		
		//face curatenie
		void CleanUp();
		
		//citim counturile din fisier
		void InputCounts(  cBitStreamSoup &input);

		void OutputCounts ( std::fstream &output );
		
		
		

};
#endif