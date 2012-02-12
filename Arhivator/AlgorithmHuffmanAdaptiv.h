#ifndef M_HUFFADAPTANALGO
#define M_HUFFADAPTANALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"
//Algoritmul Huffman Adaptiv seamana cu algoritmul huffman simplu insa 
// difera destul de mult :)) . Ideea de baza e aceeasi cu arborele binar
// construirea in functie de statistici etc , insa in cazul asta facem
// statisticile dinamic. Pe masura ce facem rost de un simbol , il adaugam
// in copac si facem update pe copac.(mergem in sus si adaugam la couturile
// de pe ramura.. si daca nu mai respecta pozitia rearanjam copacul)
// Foarte important!! Ca sa mearga algoritmul asta esential este ca
// functia UpdateTree si functia InitializeTree sa fie identice
// si la compresie si la decompresie
#define END_OF_STREAM 256
#define ESCAPE 257
#define SYMBOL_COUNT 258
#define NODE_TABLE_COUNT ( ( SYMBOL_COUNT * 2 ) - 1 )
#define ROOT_NODE 0
#define MAX_WEIGHT 0X8000
		//putin diferita decat din binary tree... 
struct node {
			unsigned int weight;
			int parent;
			int child_is_leaf;
			int child;
		};
class cAlgorithmHuffAdaptiv: cAlgorithm
{
	private:

	struct TREE {
		int leaf[ SYMBOL_COUNT ];
		int next_free_node;
		node nodes [ NODE_TABLE_COUNT ];
	};
	TREE m_Tree;
	public:
		cAlgorithmHuffAdaptiv();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmHuffAdaptiv();
private:
	void CompressFile( std::fstream &input, cBitStreamSoup &output);
	void ExpandFile( cBitStreamSoup &input, std::fstream &output );
	void InitializeTree();
	void EncodeSymbol(unsigned int c,  cBitStreamSoup &output );
	int  DecodeSymbol(  cBitStreamSoup &input );
	void UpdateModel(  int c );
	void RebuildTree(  );
	void swap_nodes(  int i, int j );
	void add_new_node( int c );

};
#endif