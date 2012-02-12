#include "stdafx.h"
#include "AlgorithmShannon.h"
#include "AlgorithmsManager.h"
#include <algorithm>
cAlgorithmShannon m_globalTemplate;
cAlgorithmShannon::cAlgorithmShannon():cAlgorithm()
{
	m_Name="Algorithm Shannon-Fano";
	m_Ext=".shf";
}
void cAlgorithmShannon::CleanUp()
{
	memset(m_Nodes,0,sizeof(cBinaryTreeNode)*514);
	memset(m_uivCounts,0,sizeof(unsigned long int)*256);
	memset(m_Codes,0,sizeof(CODE)*257);
}
void cAlgorithmShannon::InitTree()
{

	m_uiFreeNode=512;
	for(int i=0;i<514;i++)
	{
		cBinaryTreeNode newnode;
		if(i<=256)
			newnode.symbol=i;
		
		m_Nodes[i]=newnode;
	}
	memset(m_uivCounts,0,sizeof(unsigned long int)*256);
	memset(m_Codes,0,sizeof(CODE)*257);
}
int compare (const void * a, const void * b)
{
	return  ((cBinaryTreeNode*)b)->count-((cBinaryTreeNode*)a)->count;
}

void cAlgorithmShannon::Compress(std::string filenameInput,std::string filenameOutput)
{
		m_CompressionProgress=0;
	GetFileSize(filenameInput);
	filenameOutput+=m_Ext;
	InitTree();
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");
	int root_node=513;
	
	//numaram bytesii
	CountBytes( input );
	
	ScaleCounts();
	
	OutputCounts( output.m_File );
	qsort (m_Nodes, 257, sizeof(cBinaryTreeNode), compare);
	BuildTree(0,257,513);
	ConvertTreeToCode(0, 0, root_node );
	
	CompressData( input, output );
	input.close();
	CleanUp();
		m_CompressionProgress=0;
}
void cAlgorithmShannon::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	InitTree();
	int root_node=513;
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);
	InputCounts( input);

	qsort (m_Nodes, 257, sizeof(cBinaryTreeNode), compare);
	BuildTree(0,257,513);
	
	ExpandData( input, output, root_node );
	output.close();
	CleanUp();
	m_CompressionProgress=0;
}
void cAlgorithmShannon::OutputCounts ( std::fstream &output )
{
	int first;
	int last;
	int next;
	int i;
	first = 0;
	while ( first < 255 && m_Nodes[ first ].count == 0 )
		first++;
	/*
	Scriem counturile in fisier intr-un mod putin mai destept ... desi
	as fi putut scrie tot vectorul.. scriem doar portiuni diferite de zero
	daca exista nedumeriri , sunt disponibil pt explicatii
	To Do: o sa le adaug aici daca chiar e nevoie
	*/
	for ( ; first < 256 ; first = next) {
		last = first + 1;
		for ( ; ; ) {
			for ( ; last < 256 ; last ++ )
				if ( m_Nodes[ last ].count == 0 )
					break;
			last--;
			for ( next = last + 1; next < 256 ; next++ )
				if ( m_Nodes[ next ]. count != 0 )
					break;
			if ( next > 255 )
				break;
			if ( ( next - last ) > 3 )
				break;
			last = next;
		};
		
		//Aici scriu pe primul si pe ultimul si pe toate alea dintre ele	
		output.put( first);
		output.put( last);
		for ( i = first ; i <= last ; i++ )
		{
			output.put( m_Nodes[ i ]. count);
		}
	}
	output.put( 0);
}
void cAlgorithmShannon::InputCounts( cBitStreamSoup &input)
{
	int first;
	int last;
	int i;
	int c;
	for ( i = 0 ; i < 256 ; i++ )
		m_Nodes[ i ]. count = 0;
	if ( (first = input.m_File.get()) == EOF)
		LOG( "Error reading byte m_uivCounts\n" );
	if ( ( last =  input.m_File.get()) == EOF )
		LOG( "Error reading byte m_uivCounts\n");
		while(1) 
		{
			for ( i = first ; i <= last ; i++ )
				if ( ( c =  input.m_File.get() ) == EOF)
					LOG( "Error reading byte m_uivCounts\n" );
				else
					m_Nodes[ i ]. count = (unsigned int) c;
			if ( ( first = input.m_File.get() ) == EOF )
				LOG( "Error reading byte m_uivCounts\n" );
			if ( first == 0)
				break;
			if ( ( last = input.m_File.get() ) == EOF )
				LOG( "Error reading byte m_uivCounts\n" );
		}
		m_Nodes[ END_OF_STREAM ].count = 1;
}
void cAlgorithmShannon::CountBytes(std::fstream &input)
{
	std::streampos input_marker;
	int c;
	for (int i = 0 ; i < 256; i++ )
		m_uivCounts[ i ] = 0;
	input_marker = input.tellg();
	c=input.get();
	while ( c != EOF )
	{
		m_uivCounts[ c ]++;
		c=input.get();
	}
	input.clear();
	input.seekg(0, std::ios::beg);
}
void cAlgorithmShannon:: ScaleCounts()
{
	unsigned long max_count;
	int i;
	max_count = 0;
	for ( i = 0 ; i < 256 ; i++ )
		if ( m_uivCounts[ i ] > max_count )
			max_count = m_uivCounts[ i ] ;
	
	if ( max_count == 0 )
	{
		m_uivCounts[ 0 ] = 1;
		max_count = 1;
	}
	max_count = max_count / 255;
	max_count = max_count + 1;
	for ( i = 0 ; i < 256 ; i++ )
	{
		m_Nodes[ i ].count = (unsigned int)( m_uivCounts[ i ] / max_count );
		if ( m_Nodes[ i ].count == 0 && m_uivCounts[ i ] !=0 )
		m_Nodes[ i ].count = 1;
	}
	m_Nodes[ END_OF_STREAM ]. count = 1;
}
void cAlgorithmShannon::BuildTree(unsigned int first,unsigned int last,int topNode )
{
    unsigned int k, size, size_a, size_b, last_a, first_b;


	switch(last-first)
	{
	case 0:
		return;
		break;
	case 1:
		m_Nodes[topNode].child_0=first;
		m_Nodes[topNode].child_1=last;
		return;
		break;
	case 2:
		m_Nodes[topNode].child_0=first;
		m_Nodes[topNode].child_1=m_uiFreeNode;
		m_Nodes[m_uiFreeNode].child_0=first+1;
		m_Nodes[m_uiFreeNode].child_1=last;
		m_uiFreeNode--;
		return;
		break;
	default:
		m_Nodes[topNode].child_0=m_uiFreeNode;
		m_Nodes[topNode].child_1=m_uiFreeNode-1;
		m_uiFreeNode-=2;
		break;
	}

    // marime interval
    size = 0;
	
    for( k = first; k <= last; ++ k )
    {
        size += m_Nodes[k].count;
    }

    /*Gaseste marimea branchului  a */
    size_a = 0;
	// gasim mijlocul in functie de suma frecventelor
	// pozitia mijlocului e retinuta in k
    for( k = first; size_a <((size+1)>>1) && k < last; ++ k )
    {
        size_a += m_Nodes[k].count;
    }

    /* Avem elemente in ramura a */
    if( size_a > 0 )
    {
        // bagam prima ramura 
        last_a  = k-1;
		if(last_a-first==0)// avem un singur element
			m_Nodes[topNode].child_0=first;
        BuildTree(first, last_a,m_Nodes[topNode].child_0 );
    }
    // calculam marimea ramurei b 
    size_b = size - size_a;

    // Avem elemente? 
    if( size_b > 0 )
    {
        // bagam a doua ramura
        first_b = k;
		if(last-first_b==0)// avem un singur element
			m_Nodes[topNode].child_1=first;
        // o facem 
        BuildTree( first_b, last,m_Nodes[topNode].child_1);
    }	
	

}
//neah neah recursiv
void cAlgorithmShannon::ConvertTreeToCode(  unsigned int code_so_far, int bits, int node )
{
	if ( node <= END_OF_STREAM ) 
	{
		m_Codes[ m_Nodes[node].symbol ].code = code_so_far;
		m_Codes[ m_Nodes[node].symbol ].code_bits = bits;
		return;
	}
	code_so_far <<= 1;
	bits++;
	ConvertTreeToCode(code_so_far, bits,
		m_Nodes[ node ]. child_0 );
	ConvertTreeToCode(code_so_far | 1,
		bits, m_Nodes[ node ].child_1 );
}
void cAlgorithmShannon::CompressData(std::fstream &input,cBitStreamSoup &output )
{

	int c;
	c=input.get();
	m_CompressionProgress++;
	while ( c!= EOF )
	{
		output.OutputBits((unsigned long) m_Codes[ c ].code,m_Codes[ c ].code_bits );
		c=input.get();
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
	}
	output.OutputBits((unsigned long) m_Codes[ END_OF_STREAM ].code,m_Codes[ END_OF_STREAM ].code_bits );
}
void cAlgorithmShannon::ExpandData( cBitStreamSoup &input,std::fstream &output,int root_node )
{
	int iNode;                 // indexul nodului arborelui huffman/shannon-fano (daca o sa il folosesti la huffman)
	// simbolul curent 

	while (1)
	{
		iNode = root_node;

		do 
		{
			if (input.InputBit())
				iNode = m_Nodes[iNode].child_1;
			else
				iNode = m_Nodes[iNode].child_0;

		} 
		while (iNode > END_OF_STREAM);

		if (m_Nodes[iNode].symbol == END_OF_STREAM)
			break;
		output.put(m_Nodes[iNode].symbol);
	}
}
cAlgorithmShannon::~cAlgorithmShannon()
{
}