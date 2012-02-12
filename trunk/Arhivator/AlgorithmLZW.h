#ifndef M_LZWALGO
#define M_LZWALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"
/*Constantele ar fi cam asa:
BITS defineste numarul maxim de biti care pot fi folositi in codarea de output
TABLE_SIZE defineste marimea tabelei dictionar
TABLE_BANKS e numarul de pafini de 256 de elemente din tabla dictionar,necesare
*/
#define BITS 15
#define MAX_CODE ( ( 1 << BITS ) - 1 )
#define TABLE_SIZE 35023L
#define TABLE_BANKS ( ( TABLE_SIZE >> 8 ) + 1 )
#define END_OF_STREAM 256
#define BUMP_CODE 257
#define FLUSH_CODE 258
#define FIRST_CODE 259
#define UNUSED -1
#define DICT( i ) dict[ i >> 8 ][ i & 0xff ]
class cAlgorithmLZW: cAlgorithm
{
public:
	cAlgorithmLZW();
	void Compress(std::string filenameInput,std::string filenameOutput);
	void DeCompress(std::string filenameInput,std::string filenameOutput);
	virtual ~cAlgorithmLZW();
	unsigned int FindChildNode( int parent_code, int child_character );
	unsigned int DecodeString( unsigned int offset, unsigned int code );
private:
		/*
	Structura asta de date defineste dictionarul.Fiecare inscriere in dictionar
	are o valoare a codului.Aceasta este codul emis de compresor. Fiecare cod este de fapt
	facut din doua bucati: un cod parinte (parent_code) si un caracter. Valorile codului
	mai mmici decat 256 sunt de fapt coduri banale de text
	Nota: Daca am vrea sa ne descurcam cu compilatoare care genereaza cod pe 16 biti segemen
	tat (alea vechi de MS-DOS sau nuj.. poate pic-uri sau microcontrolerel varza.. desi deja aberez :)) )
	impartim dictionarul intr-un tabel de pointeri de dictionar mai mici.
	Astfel fiecare referinta la dictionar a fost inlocuita de un macro care face o dereferentieree
	a pointerilor mai intai. Spargand indexul dupa marginile byteului ar tb sa fie cat se poate de
	eficienta treaba.*/
	struct dictionary
	{
		int code_value;
		int parent_code;
		char character;
	} *dict[ TABLE_BANKS ];
	char decode_stack[ TABLE_SIZE ];//folosim ca sa inversam striungurile care ies din copac in timpul decodarii
	unsigned int next_code;//este urmatorul cod de adaugat in dictionar
	int current_code_bits;//defineste cati biti sunt in mod curent folositi pentru output
	unsigned int next_bump_code;//defineste codul care os a declanseze urmatoarea variatie in marimea cuvantului
	/*
	Initializam dictionarul , si la compresi si la decompresie, si cand tb sa facem
	flush (primim cod de flush).
	Nota: Desi decompresorul seteaza code_value de la elemente la UNUSED nu trebuie
	neaparat sa faca treaba asta*/
	void InitializeDictionary( );
	void InitializeStorage();
	void CompressFile(std::fstream  & input,cBitStreamSoup & output );
	void ExpandFile(cBitStreamSoup & input,std::fstream &output );
	void ReleaseMemory();

};
#endif