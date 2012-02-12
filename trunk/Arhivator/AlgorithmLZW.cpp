#include "stdafx.h"
#include "AlgorithmLZW.h"
#include "AlgorithmsManager.h"
cAlgorithmLZW m_globalTemplate;
cAlgorithmLZW::cAlgorithmLZW():cAlgorithm()
{
	m_Name="Algorithm LZW";
}
void cAlgorithmLZW::Compress(std::string filenameInput,std::string filenameOutput)
{
	GetFileSize(filenameInput);
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");
	CompressFile(input,output);
	input.close();
	m_CompressionProgress=0;
}
void cAlgorithmLZW::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);	
	ExpandFile(input,output);
	output.close();
}
cAlgorithmLZW::~cAlgorithmLZW()//strangem dupa noi
{

}
void cAlgorithmLZW::InitializeDictionary( void )
{
	unsigned int i;
	for ( i = 0 ; i < TABLE_SIZE ; i++ )
		DICT(i).code_value = UNUSED;
	next_code = FIRST_CODE;
	current_code_bits = 9;
	next_bump_code = 511;
}
/*
Alocam dictionarul. facem treaba cu impartitul in bucati mai mici... 
se putea aloca ca o variabila imensa... dar prefer asa momentan, poate
ma razgandesc
*/
void cAlgorithmLZW::InitializeStorage( void )
{
	int i;
	for ( i = 0 ; i < TABLE_BANKS ; i++ )
	{
		dict[ i ] =  new dictionary[256]; 
		MEMORY_MANAGER->RegisterVariable(dict[i],MMW_DICTIONARY);	
		memset(dict[i],0,256);

	}
}
/*
Compresorul este scurt si simplu. Citeste simbolurile unu cate unu
din input. Verifica daca combinatia simbolului curent si a codului curent
exista definite in dictionar. Daca nu exista sunt adaugate si o luam de la inceput
cu un nou simbol cod. Daca sunt atnci codul cimbinatiei devine noul cod.
Nota: Este versiunea imbunatatita de LZW, encoderul tb sa verifice codul
pentru marginire*/
void cAlgorithmLZW::CompressFile(std::fstream  & input,cBitStreamSoup & output )
{

	int character;
	int string_code;
	unsigned int index;
	InitializeStorage();
	InitializeDictionary();
	string_code=input.get();
	if ( string_code == EOF )
		string_code = END_OF_STREAM;
	character=input.get();
	m_CompressionProgress++;
	while (  character  != EOF ) 
	{	
		index = FindChildNode( string_code, character );
		if ( DICT( index ).code_value != -1)
			string_code = DICT( index ).code_value;
		else 
		{
			DICT( index ).code_value = next_code++;
			DICT( index ).parent_code = string_code;
			DICT( index ).character = (char) character;
			output.OutputBits( (unsigned long) string_code, current_code_bits );
			string_code = character;
			if ( next_code > MAX_CODE )
			{
				output.OutputBits((unsigned long) FLUSH_CODE, current_code_bits );
				InitializeDictionary();
			} 
			else if ( next_code > next_bump_code ) 
			{
				output.OutputBits( (unsigned long) BUMP_CODE, current_code_bits );
				current_code_bits++;
				next_bump_code <<= 1;
				next_bump_code |= 1;
			}
		}
		character=input.get();
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
		if(character==EOF)
			break;//inutil? :))
	}
	output.OutputBits(  (unsigned long) string_code, current_code_bits );
	output.OutputBits(  (unsigned long) END_OF_STREAM, current_code_bits);
	ReleaseMemory();
}
/*
Decomprimatorul e asemanator comprimatorului.Trebuie sa citeasca coduri, si apoi
sa le converteasca in stringuri de caractere. SIngura chestie este ca toata operatia
se petrece cand intralnim CHAR+STRING+CHAR+STRING+CHAR. Cand asta se intampla
encoderul da un cod care nu e definit in tabel.Toate codurile speciale sunt
abordate in modurile specifice*/
void cAlgorithmLZW::ExpandFile(cBitStreamSoup & input,std::fstream &output )
{
	unsigned int new_code;
	unsigned int old_code;
	int character;
	unsigned int count;
	InitializeStorage();
	while( true ) 
	{
		InitializeDictionary();
		old_code = (unsigned int) input.InputBits(  current_code_bits );
		if ( old_code == END_OF_STREAM )
			return;
		character = old_code;
		output.put( (char)old_code);
		while(true ) 
		{
			new_code = (unsigned int) input.InputBits( current_code_bits );
			if ( new_code == END_OF_STREAM )
			{
				ReleaseMemory();
				return;
			}
			if ( new_code == FLUSH_CODE )
				break;
			if ( new_code == BUMP_CODE )
			{
				current_code_bits++;
				continue;
			}
			if ( new_code >= next_code ) 
			{
				decode_stack[ 0 ] = (char) character;
				count = DecodeString( 1, old_code );
			}
			else
				count = DecodeString( 0, new_code );
			character = decode_stack[ count - 1 ];
			while ( count > 0 )
				output.put( (char)decode_stack[ --count ]);
			DICT( next_code ).parent_code = old_code;
			DICT( next_code ).character = (char) character;
			next_code++;
			old_code = new_code;
		}
	}
	ReleaseMemory();
}
/*
functia  asta de hashing este responsabila pentrui gasirea locatiei din tabel
pentruj combinatia string/character. Indexul tabelului este creat folosind 
o combinatie de XOR pe prefix si caracter. Acest cod trebuie sa si verifice
pentru coliziuni si sa le repare sarind ca broasca prin tabel
*/
unsigned int cAlgorithmLZW::FindChildNode(int parent_code, int child_character )
{
	unsigned int index;
	unsigned int offset;
	index = ( child_character << ( BITS - 8 ) ) ^ parent_code;
	if ( index == 0 )
		offset = 1;
	else
		offset = TABLE_SIZE -index;
	while (true )
	{
		if ( DICT( index ).code_value == UNUSED )
			return( (unsigned int) index );
		if ( DICT( index ).parent_code == parent_code &&
			DICT( index ). character == (char) child_character )
			return( index );
		if ( index >= offset )
			index -= offset;
		else
			index += TABLE_SIZE - offset;
	}
}
/*
Functia asta decodeaza stringul din dictionar si il stocheaza in decode_stack
Returneaza countul (cate caractere am bagat in stack .. stiva adica :)) )
*/
unsigned int cAlgorithmLZW::DecodeString(unsigned int count,unsigned int code )
{
	while ( code > 255 ) 
	{
		decode_stack[ count++ ] = DICT( code ).character;
		code = DICT( code ).parent_code;
	}
	decode_stack[ count++ ] = (char) code;
	return  count ;
}
void cAlgorithmLZW::ReleaseMemory()// curatenie, strangem jucariile
{
	for (int i = 0 ; i < TABLE_BANKS ; i++ )
	{
		dictionary * p=dict[i];
		MEMORY_MANAGER->UnregisterVariable(dict[i]);
		delete []p ;
	}
}
