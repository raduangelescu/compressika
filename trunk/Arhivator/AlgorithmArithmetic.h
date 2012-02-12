#ifndef M_ARITHMETICALGO
#define M_ARITHMETICALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"
// imi place sa il tot definesc pe asta .. uite asa
 #define END_OF_STREAM 256
/*
Asta e structura de definire a unui simbol in codarea aritmetica.
Un simbol e definit prin Rageul dintre 0 si 1. Fiindca folosim 
format de virgul fixa(de fapt nici macar atat.. folosim intregi
sa simulam virgula fixa). Low_count si high_count definesc unde
ar tb sa fie simbolul fata de rangel falls in the range.
*/
struct SYMBOL 
{
	unsigned short int low_count;
	unsigned short int high_count;
	unsigned short int scale;
} ;
// doua vorbe despre Arithmetic coding
// vorbe despre Arithmetic, vorbe despre Arithmetic 
// In principal rezolva problema lui Huffman ca un cod nu poate sa fie reprezentat decat
//	pe un numar intreg de biti, nu mai folosim copac.. ceea ce e suuuper ok :)) fiindca deja
// ma plictisisem de greselile pe care le faceam  :)) .
// si ca sa nu mai scriu aici : http://en.wikipedia.org/wiki/Arithmetic_coding (cred ca mai bine de acum dau linkuri)
// castigul se simte in cazul fisierelor care au diferente majore de statistici
// comprima spre exemplu un fisier in care ai 111111111111111111111111111111111111 (asta inseamna  90% pt 1 si restul pt EOF si EOS)
// si atat .. baga atati unu cat sa aiba 3 mega... pe huffman/shannon comprima la 300-400kb
// asta il face 35-40 de kb... la alte tipuri de fisiere insa se poate huffman sa iasa putin mai bine
// (doar putin) (depinde de statistici).. la majoritatea arithemticu castiga fata huffman/shannon/huffman adaptiv
// e putin mai incet
class cAlgorithmArithmetic: cAlgorithm
{
	public:
		cAlgorithmArithmetic();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmArithmetic();
private:
	void BuildModel( std::fstream &input, std::fstream &output );
	void ScaleCounts( unsigned long counts[],unsigned char scaled_counts[] );
	void BuildTotals( unsigned char scaled_counts[] );
	void CountBytes( std::fstream& input, unsigned long counts[] );
	void OutputCounts( std::fstream& output, unsigned char scaled_counts[] );
	void InputCounts( std::fstream& stream );
	void ConvertIntToSymbol( int symbol, SYMBOL &s );
	void GetSymbolScale( SYMBOL &s );
	int ConvertSymbolToInt( int count, SYMBOL &s );
	void InitializeArithmeticEncoder(  );
	void EncodeSymbol( cBitStreamSoup& stream, SYMBOL &s );
	void FlushArithmeticEncoder( cBitStreamSoup& stream );
	short int GetCurrentCount( SYMBOL &s );
	void InitializeArithmeticDecoder( cBitStreamSoup& stream );
	void RemoveSymbolFromStream( cBitStreamSoup& stream, SYMBOL & s );
private:
	short int m_siTotals[ 258 ]; /* totalurile cumulative */
	/*Variabilele 4 urmatoare definesc starea curenta a comprimatorului/decomprimatorului 
	Se presupune ca sunt 16 biti.
	Nota1: Se presupune.. pe majoritatea compilatoarelor sunt (teoretic si in visual.. nu am testat)
	Nota2: Depinde si de sistemul de operare
	*/
	unsigned short int m_siCode;/* Valoarea input codeului curent */
	unsigned short int m_siLow; /* Inceputul rangeului codului curent */
	unsigned short int m_siHigh;/* Sfarsitul rangeului codului curent */
	long m_lUnderflowBits; /* Numarul de biti underflow  */
};
#endif