#ifndef M_LZSSALGO
#define M_LZSSALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"

#define INDEX_BIT_COUNT      12
#define LENGTH_BIT_COUNT     4
#define WINDOW_SIZE          ( 1 << INDEX_BIT_COUNT )
#define RAW_LOOK_AHEAD_SIZE  ( 1 << LENGTH_BIT_COUNT )
#define BREAK_EVEN           ( ( 1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT ) / 9 )
#define LOOK_AHEAD_SIZE      ( RAW_LOOK_AHEAD_SIZE + BREAK_EVEN )
#define TREE_ROOT            WINDOW_SIZE
#define END_OF_STREAM_LZZ       0
#define UNUSED               0
#define MOD_WINDOW( a )      ( ( a ) & ( WINDOW_SIZE - 1 ) )



class cAlgorithmLZSS: cAlgorithm
{
private:
	struct TREE
	{
		int parent;
		int smaller_child;
		int larger_child;
	} ;
	TREE tree [WINDOW_SIZE + 1 ];
	unsigned char window [WINDOW_SIZE ];
public:
	cAlgorithmLZSS();
	void Compress(std::string filenameInput,std::string filenameOutput);
	void DeCompress(std::string filenameInput,std::string filenameOutput);
	virtual ~cAlgorithmLZSS();
	void InitTree( int r );
	void ContractNode( int old_node, int new_node );
	void ReplaceNode( int old_node, int new_node );
	int FindNextNode( int node );
	void DeleteString( int p );
	int AddString( int new_node,int *match_position);
	void CompressFile( std::fstream &input,cBitStreamSoup &output );
	void ExpandFile( cBitStreamSoup &input,  std::fstream &output);

};
#endif