#ifndef C_BITSTREAMSOUP
#define C_BITSTREAMSOUP
#include "Utile.h"
enum FILEMODE
{
	INPUT_FILE,
	OUTPUT_FILE,
	OUTPUT_BUFFER
};
/*
clasa care se ocupa cu scrierea in fisier binar a bitilor elementul masca este initializat
cu  0x80 la deschiderea fisierului.La output prima scriere in fisier va seta sau sterge acel
bit, si maska se va shifta la urmatorul.Odata ce maska s-a shiftat atat de mult incat bitii din 
m_Rack au fost toti setati sau anulati,m_Rack este scris in fisier si initializam un nou byte de
m_Rack
*/
class cBitStreamSoup
{
public:
	int m_Mod;// modul de deschidere a fisierului (input sau output).. 
				// poate fi folosit cu FILEMODE
	//MSB este primul bit, LSB e ultimul
    int m_Rack; // contine byteul current care asteapta sa fie scris in fisier
				//(in cazul fisierelor de iesire) sau care e citit din fisier
				// (pentru fisierele de intrare)
	unsigned char m_Mask; //contine masca unui singur bit folosita pentru a pune sau a
						  // sterge bitul curent de output , sau masca pentru bitul
						 // curent de input
    unsigned int m_PacifierCounter; // variabila asta se foloseste in general sa vedem progresul
							// se incrementeaza odata la fiecare scriere/citire de 1 byte
							// in final ar trebuii sa contina numarul de bytes al fisierului
							//NOTA: poate fi folosit si ca verificare
	unsigned int m_FileSizeInBytes;
	unsigned int m_LeftFileSizeInBytes;
	
	unsigned char *m_Buffer;
	unsigned int   m_BufferSize;
	unsigned int m_crtBuffEl;
public:
	std::fstream m_File;
	cBitStreamSoup(std::string filename,std::string mod,int bufferSize = FILE_BUFFER_SIZE);
	void OutputBit( int bit ); 
	void OutputBits(unsigned long code, int count); 
	void OutputBytes(unsigned char *bytes,int count);
	void GetFileSize(std::string filename);
	int  InputBit(); 
	unsigned long   InputBits(int bit_count );

	void  AppendToFile(cBitStreamSoup *fileBS);
	void  ForceWrite();
	void  FilePrintBinary( unsigned int code, int bits); 
 
	~ cBitStreamSoup();
};
#endif