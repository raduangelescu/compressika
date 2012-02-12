#include "stdafx.h"
#include "AlgorithmLZSS.h"
#include "AlgorithmsManager.h"
cAlgorithmLZSS m_globalTemplate;
cAlgorithmLZSS::cAlgorithmLZSS():cAlgorithm()
{
	m_Name="Algorithm LZSS";
	m_Ext=".lzs";
}
void cAlgorithmLZSS::Compress(std::string filenameInput,std::string filenameOutput)
{
	GetFileSize(filenameInput);
filenameOutput+=m_Ext;
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");
	CompressFile(input,output);
	input.close();
		m_CompressionProgress=0;
}
void cAlgorithmLZSS::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);
	ExpandFile(input,output);
	output.close();
	m_CompressionProgress=0;
}
cAlgorithmLZSS::~cAlgorithmLZSS()
{
}
/*
Pt ca arborele este alocat static, il alocam cu 0 peste tot
... chestie ok fiindca 0 este si UNUSED. Ca sa facem arborele
utilizabil adaugam o fraza ca sa aiba nod radacina
*/
void cAlgorithmLZSS::InitTree(int  r )
{
    tree[ TREE_ROOT ].larger_child = r;
    tree[ r ].parent = TREE_ROOT;
    tree[ r ].larger_child = UNUSED;
    tree[ r ].smaller_child = UNUSED;
}
/*
Folosim functia asta cand se sterge un nod. Legatura cu descendentii lui
e rupta tragandu-i pe toti in sus ca sa se suprapuna pe linkul existent
*/
void cAlgorithmLZSS::ContractNode(int old_node,int new_node )

{
 tree[ new_node ].parent = tree[ old_node ].parent;
    if ( tree[ tree[ old_node ].parent ].larger_child == old_node )
        tree[ tree[ old_node ].parent ].larger_child = new_node;
    else
        tree[ tree[ old_node ].parent ].smaller_child = new_node;
    tree[ old_node ].parent = UNUSED;
}
/*
Rutina asta este folosita cand un nod e sters. In cazul asta este 
inlocuit de un nod care nu era in copac
*/
void cAlgorithmLZSS::ReplaceNode( int old_node, int new_node )
{
	int parent;

    parent = tree[ old_node ].parent;
    if ( tree[ parent ].smaller_child == old_node )
        tree[ parent ].smaller_child = new_node;
    else
        tree[ parent ].larger_child = new_node;
    tree[ new_node ] = tree[ old_node ];
    tree[ tree[ new_node ].smaller_child ].parent = new_node;
    tree[ tree[ new_node ].larger_child ].parent = new_node;
    tree[ old_node ].parent = UNUSED;
}
/*
Functia asta e folosita sa gasim urmatorul cel mai mic nod dupa argument.
Se presupune ca nodul are smaller_child. Gasim urmatorul cel mai mic copil
mergand la smaller_child si apoi la ultimul larger_child
*/
int cAlgorithmLZSS::FindNextNode(int node )
{
	int next;

    next = tree[ node ].smaller_child;
    while ( tree[ next ].larger_child != UNUSED )
        next = tree[ next ].larger_child;
    return( next );
}
/*
Functia asta  sterge clasic din arborele binar.Daca nodul sters are linkul null
in orice directie trebuie sa tragem linkul non null in sus si sa inlocuim cu ala
existent. Daca ambele exista atunci stergem urmatorul link in ordine care e garantat
sa aiba un link null si facem ca mai sus
*/
void cAlgorithmLZSS::DeleteString( int p )
{
	int  replacement;

    if ( tree[ p ].parent == UNUSED )
        return;
    if ( tree[ p ].larger_child == UNUSED )
        ContractNode( p, tree[ p ].smaller_child );
    else if ( tree[ p ].smaller_child == UNUSED )
        ContractNode( p, tree[ p ].larger_child );
    else {
        replacement = FindNextNode( p );
        DeleteString( replacement );
        ReplaceNode( p, replacement );
    }
}
/*
Asta e functia care munceste cel mai mult la encodat. E responsabila
pentru adaugarea unui nod nou in arborele binar. De asemenea trebuie sa 
gaseasca cel mai apropiat nod (de stringul nou) si sa il intoarca.
Ca sa fie si mai naspa :) , daca nodul nou are un duplicat in arbore
ala vechi e sters, pt eficienta
*/
int cAlgorithmLZSS::AddString(int new_node,int *match_position )
{
	int i;
    int test_node;
    int delta;
    int match_length;
    int *child;

    if ( new_node == END_OF_STREAM_LZZ )
        return( 0 );
    test_node = tree[ TREE_ROOT ].larger_child;
    match_length = 0;
    while (true)
	{
        for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ ) {
            delta = window[ MOD_WINDOW( new_node + i ) ] -
                    window[ MOD_WINDOW( test_node + i ) ];
            if ( delta != 0 )
                break;
        }
        if ( i >= match_length ) 
		{
            match_length = i;
            *match_position = test_node;
            if ( match_length >= LOOK_AHEAD_SIZE )
			{
                ReplaceNode( test_node, new_node );
                return( match_length );
            }
        }
        if ( delta >= 0 )
            child = &tree[ test_node ].larger_child;
        else
            child = &tree[ test_node ].smaller_child;
        if ( *child == UNUSED )
		{
            *child = new_node;
            tree[ new_node ].parent = test_node;
            tree[ new_node ].larger_child = UNUSED;
            tree[ new_node ].smaller_child = UNUSED;
            return( match_length );
        }
        test_node = *child;
    }
}
/*
Functia de compresie. Mai intai incarca bufferul look_ahead si apoi intra in
bucla de compresie. Bucla de compresie decide daca baga pe output un singur
caracter sau un index care defineste fraza. Odata ce caracterul sau fraza
au fost trimise pe output tb sa mai facem o bucla. A doua bucla citeste
noile caractere, sterge frazele care sunt rescrise de noul caracter , apoi
adauga frazele create de noul caracter
*/
void cAlgorithmLZSS::CompressFile(std::fstream &input,cBitStreamSoup & output)
{
	 int i;
    int c;
    int look_ahead_bytes;
    int current_position;
    int replace_count;
    int match_length;
    int match_position;
    current_position = 1;
    for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ )
	{
		c=input.get();

		m_CompressionProgress++;
        UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
		if ( c   == EOF )
            break;
        window[ current_position + i ] = (unsigned char) c;
    }
    look_ahead_bytes = i;
    InitTree( current_position );
    match_length = 0;
    match_position = 0;
    while ( look_ahead_bytes > 0 ) 
	{
        if ( match_length > look_ahead_bytes )
            match_length = look_ahead_bytes;
        if ( match_length <= BREAK_EVEN )
		{
            replace_count = 1;
            output.OutputBit(  1 );
            output.OutputBits((unsigned long) window[ current_position ], 8 );
        }
		else 
		{
             output.OutputBit( 0 );
             output.OutputBits( (unsigned long) match_position, INDEX_BIT_COUNT );
             output.OutputBits( (unsigned long) ( match_length - ( BREAK_EVEN + 1 ) ), LENGTH_BIT_COUNT );
            replace_count = match_length;
        }
        for ( i = 0 ; i < replace_count ; i++ ) 
		{
            DeleteString( MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) );
            c=input.get();
			
			m_CompressionProgress++;
			 UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
			if ( c  == EOF )
                look_ahead_bytes--;
            else
                window[ MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) ]= (unsigned char) c;
            current_position = MOD_WINDOW( current_position + 1 );
            if ( look_ahead_bytes )
                match_length = AddString( current_position, &match_position );
        }
    };
    output.OutputBit( 0 );
    output.OutputBits( (unsigned long) END_OF_STREAM_LZZ, INDEX_BIT_COUNT );
}
/*
Asta e Decompresia pt LZSS. Nu tb decat sa citeasca bitii flag si in 
functie de ei decide daca caracterul e de sine stator sau daca e 
index si tb sa il ia din dictionar
*/
void cAlgorithmLZSS::ExpandFile(cBitStreamSoup &input,std::fstream & output )
{
	int i;
	int current_position;
	int c;
	int match_length;
	int match_position;
	current_position = 1;
	while ( 1 )
	{
		if ( input.InputBit() ) 
		{
			c = (int) input.InputBits( 8 );
			output.put( (char)c );
			window[ current_position ] = (unsigned char) c;
			current_position = MOD_WINDOW( current_position + 1 );
		}
		else 
		{
			match_position = (int) input.InputBits(  INDEX_BIT_COUNT );
			if ( match_position == END_OF_STREAM_LZZ )
				break;
			match_length = (int) input.InputBits( LENGTH_BIT_COUNT );
			match_length += BREAK_EVEN;
			for ( i = 0 ; i <= match_length ; i++ ) 
			{
				c = window[ MOD_WINDOW( match_position + i ) ];
				output.put((char) c);
				window[ current_position ] = (unsigned char) c;
				current_position = MOD_WINDOW( current_position + 1 );
			}
		}
	}
}
