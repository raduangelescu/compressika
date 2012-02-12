#include "stdafx.h"
#include "AlgorithmHuffman.h"
#include "AlgorithmsManager.h"

cAlgorithmHuffman m_globalTemplate;

cAlgorithmHuffman::cAlgorithmHuffman():cAlgorithm()
{
	m_Name="Algorithm Huffman";
	m_Ext=".huf";
	
}

//functie de comparare ce o foloseste qsort
int compareHuffman (const void * a, const void * b)
{
	return  ((cBinaryTreeNode*)a)->count-((cBinaryTreeNode*)b)->count;
}


void cAlgorithmHuffman::Compress(std::string filenameInput,std::string filenameOutput)
{
	filenameOutput+=m_Ext;
	InitTree();															//se initializeaza nodurile arborelui
	GetFileSize(filenameInput);
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");

														
	CountBytes(input);													//se numara aparitia fiecarui simbol
	ScaleCounts();														//se scaleaza numarul de aparitii si se pune in noduri
	OutputCounts( output.m_File );
	qsort (m_Nodes, 257, sizeof(cBinaryTreeNode), compareHuffman);		//se ordoneaza crescator primele 257 de elemente ale vectorului m_Nodes																			//functie standard c++ (iostream.h)
	
	BuildTree();

	ConvertTreeToCode(0, 0, f_node );
	CompressData( input, output );
	
	input.close();
	CleanUp();
	m_CompressionProgress=0;
}

void cAlgorithmHuffman::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	GetFileSize(filenameInput);
	InitTree();
	
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);
	InputCounts(input);


	qsort (m_Nodes, 257, sizeof(cBinaryTreeNode), compareHuffman);
	
	BuildTree();
	
	ExpandData( input, output, f_node );
	
	output.close();
	CleanUp();

m_CompressionProgress=0;
}

//metoda initializeaza nodurile arborelui
//primele 256 de elemente ale vectorului m_Nodes contin toate combinatiile posibile de 8 biti de la 0 la 255
//acestea sunt salvate in m_Nodes[i].symbol astfel incat atunci cand functia de scalare va introduce counturile
//va aparea m_Nodes[grup de 8 biti].symbol=grup de 8 biti si m_Nodes[grup de 8 biti].symbol=count referitor la "grupul de 8 biti"
//in acest mod simboul va fi in acelasi nod cu numarul sau de aparitii si nu se va pierde la sortare
void cAlgorithmHuffman::InitTree()
{	
	m_uiFreeNode=512;
	for(int i=0;i<514;i++)
	{	
		cBinaryTreeNode newnode(257);
		if(i<=256)
			newnode.symbol=i;
		
		//primele 256 de elemente ale vectorului contin toate combinatiile posibile de 8 biti de la 0 la 255
		m_Nodes[i]=newnode;
	}
	memset(m_uivByteCounts,0,sizeof(unsigned long int)*256);
	memset(m_Codes,0,sizeof(CODE)*257);
}

void cAlgorithmHuffman::OutputCounts ( std::fstream &output )
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

//metoda ce numara de cate ori apare o grupa de 8 biti in fisierul de input
void cAlgorithmHuffman::CountBytes(std::fstream &input)
{
	std::streampos input_marker;
	int c;

	for (int i = 0 ; i < 256; i++ )
		m_uivByteCounts[ i ] = 0;
	
	c=input.get();
	while ( c != EOF )
	{
		m_uivByteCounts[ c ]++;
		c=input.get();
	}

	input.clear();
	input.seekg(0, std::ios::beg);
}

//metoda scaleaza numarul de aparitii ale byte-ilor astfel incat sa incapa pe un byte
void cAlgorithmHuffman::ScaleCounts()
{
	unsigned long max_count;
	int i;

	//max_count va memora maximul numarului de aparitii ale unui byte din vectorul m_uivByteCounts
	max_count = 0;
	for ( i = 0 ; i < 256 ; i++ )
		if ( m_uivByteCounts[ i ] > max_count )
			max_count = m_uivByteCounts[ i ] ;
	
	//daca max_count este 0 atunci el trebuie egalat cu 1 pt ca se va face o impartire si impartirea la 0 nu are sens
	if ( max_count == 0 )
	{
		m_uivByteCounts[ 0 ] = 1;
		max_count = 1;
	}

	max_count = max_count / 255;
	max_count = max_count + 1;

	
	for ( i = 0 ; i < 256 ; i++ )
	{
		//scalarea se face doar daca exista un numar de aparitii mai mare de 255 
		//se pune in noduri numarul de aparitii scalat
		m_Nodes[ i ].count = (unsigned int)( m_uivByteCounts[ i ] / max_count );
		if ( m_Nodes[ i ].count == 0 && m_uivByteCounts[ i ] !=0 )
		m_Nodes[ i ].count = 1;
	}
	m_Nodes[ END_OF_STREAM ]. count = 1;
}                                    

void cAlgorithmHuffman::BuildTree()
{
	//cauta primul element cu numar de aparitii mai mare ca 0
	int i;
	int k=257;
	int BottomNode;
	for(i=0; i<257; i++)
	{
		if(m_Nodes[i].count>0){
			//se pun in arbore primele 2 noduri
			m_Nodes[k].child_0=i;
			m_Nodes[k].child_1=i+1;
			m_Nodes[k].count = m_Nodes[i].count+m_Nodes[i+1].count;
			k++;
			BottomNode=i;
			break;
		}
	}

	for(int i=BottomNode+2; i<514; i=i+2)
	{
		if(m_Nodes[i+1].count!=0) {

			qsort (m_Nodes+i, k-i, sizeof(cBinaryTreeNode), compareHuffman);

			m_Nodes[k].child_0=i;
			m_Nodes[k].child_1=i+1;
			m_Nodes[k].count = m_Nodes[i].count+m_Nodes[i+1].count;
			k++;

		} else {
			f_node=k-1;
			break;
		}
	}
}
void cAlgorithmHuffman::ConvertTreeToCode(  unsigned int code_so_far, int bits, int node )
{
	if (m_Nodes[node].symbol <= END_OF_STREAM) 
	{
		m_Codes[ m_Nodes[node].symbol ].code = code_so_far;
		m_Codes[ m_Nodes[node].symbol ].code_bits = bits;

		return;
	}
	code_so_far <<= 1;
	bits++;
	ConvertTreeToCode(code_so_far, bits, m_Nodes[ node ]. child_0 );
	ConvertTreeToCode(code_so_far | 1, bits, m_Nodes[ node ].child_1 );
}

void cAlgorithmHuffman::CompressData(std::fstream &input,cBitStreamSoup &output )
{
	int c;
	c=input.get();
	while ( c!= EOF )
	{
		output.OutputBits((unsigned long) m_Codes[ c ].code,m_Codes[ c ].code_bits );
		c=input.get();
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
	}
	output.OutputBits((unsigned long) m_Codes[ END_OF_STREAM ].code,m_Codes[ END_OF_STREAM ].code_bits );
}

void cAlgorithmHuffman::InputCounts( cBitStreamSoup &input)
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

void cAlgorithmHuffman::ExpandData( cBitStreamSoup &input,std::fstream &output,int root_node )
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
		while (m_Nodes[iNode].symbol > END_OF_STREAM);

		if (m_Nodes[iNode].symbol == END_OF_STREAM)
			break;
		output.put(m_Nodes[iNode].symbol);
	}
}

void cAlgorithmHuffman::CleanUp()
{
	memset(m_Nodes,0,sizeof(cBinaryTreeNode)*514);
	memset(m_uivByteCounts,0,sizeof(unsigned long int)*256);
	memset(m_Codes,0,sizeof(CODE)*257);
}

cAlgorithmHuffman::~cAlgorithmHuffman()
{
}