#include "stdafx.h"
#include "BitStreamSoup.h"
#include "Progress.h"
cBitStreamSoup::cBitStreamSoup(std::string filename, std::string mod)
{
	m_Mod=-1;
	
	if(mod.compare("in")==0)
	{
		m_File.open(filename.c_str(),std::ios::in|std::ios::binary);
		GetFileSize(filename);
		m_Mod=INPUT_FILE;
	}
	if(mod.compare("out")==0)
	{
		m_File.open(filename.c_str(),std::ios::out|std::ios::binary);
		m_Mod=OUTPUT_FILE;
	}
	if(m_File.is_open())
	{
		std::string log="File ";
		log+=filename;
		log+=" Opened";
		LOG(log);// logam ceva sa fim siguri ca s-a deschis fisieru
		m_Rack = 0; 
		m_Mask = 0x80;
		m_PacifierCounter=0;
	}
	else
	{
		std::string log="Could not open file: ";
		log+=filename;
		LOG(log);
	}
		
}
void cBitStreamSoup::OutputBit(int bit)
{
   if ( bit ) 
       m_Rack |= m_Mask; // exemplu 1101000 | 0000100=1101100 .. masca va contine tot timpul un 1 si numai 1
   m_Mask >>= 1; // o shiftam
   if ( m_Mask == 0 )
   {
		m_File.put( m_Rack); //scriem byteul fiindca inseamna ca am shiftat pana am completat tot byteul  
		m_PacifierCounter++ ;// aici e progresul..
   }
   if(m_Mask==0)
   {
	 m_Rack = 0; 
     m_Mask = 0x80;
   }
}
void cBitStreamSoup::OutputBits(unsigned long code, int count) 
{ 
	// in principal e cam la fel ca OutputBit...
  unsigned long mask;  
  mask = 1 << ( count - 1 ); //(1L adica 1 long) :)
  while ( mask != 0) 
  { 
       if ( mask & code ) 
            m_Rack |= m_Mask; 
	m_Mask >>= 1; 
	if (m_Mask == 0 ) 
	{
		m_File.put( m_Rack ) ;
		m_PacifierCounter++;
		m_Rack = 0; 
        m_Mask = 0x80;  
	}
        
     mask >>= 1; 
   } 
} 
 int cBitStreamSoup::InputBit( ) 
{ 
     int value; 
     if ( m_Mask == 0x80 ) 
	 { 
          m_Rack = m_File.get(); 
          if ( m_Rack == EOF ) 
		  {
               LOG( "am terminat fisieru.. ar cam tb schimbat sistemul asta" );
		  }
		  m_PacifierCounter++; 
		  if(m_Mod==INPUT_FILE)
			  UpdateFileProgressBar(m_PacifierCounter,m_FileSizeInBytes);	
		  
     } 
     value = m_Rack & m_Mask; 
     m_Mask >>= 1; 
     if ( m_Mask ==0 )        
		 m_Mask = 0x80; 
     return ( value ? 1 : 0 ); 
} 
 
 unsigned long cBitStreamSoup::InputBits(int bit_count ) 
{ 
 
     unsigned long mask; 
     unsigned long return_value; 
 
     mask = 1L << ( bit_count - 1 ); 
     return_value = 0; 
     while ( mask != 0)
	 { 
          if ( m_Mask == 0x80 )
		  { 
              m_Rack = m_File.get(); 
               if ( m_Rack == EOF ) 
                    LOG( "Fatal error in InputBit!" ); 
				m_PacifierCounter++ ;
         if(m_Mod==INPUT_FILE)
			  UpdateFileProgressBar(m_PacifierCounter,m_FileSizeInBytes);
          } 
          if ( m_Rack & m_Mask ) 
               return_value |=mask; 
          mask >>= 1; 
          m_Mask >>= 1; 
          if (m_Mask == 0 ) 
               m_Mask = 0x80; 
     } 
     return( return_value ); 
} 
 //scrie direct codul :|
 void cBitStreamSoup::FilePrintBinary(unsigned int code, int bits)
{ 
     unsigned int mask; 
     mask = 1 << ( bits - 1 );
     while ( mask != 0 ){ 
         if ( code & mask ) 
             m_File.put( '1'); 
         else 
             m_File.put( '0'); 
         mask >>= 1; 
     } 
}
 // ne-am jucat si acum strangem dupa noi 
cBitStreamSoup::~cBitStreamSoup()
{
	if(m_Mod==1)
	{
		if ( m_Mask != 0x80 ) 
			m_File.put(m_Rack); 
	}
	m_File.close();  
}
// Functia asta nu e neaparat exacta.. adica poate arata mai marf fisierele
// decat sunt , si mai are dezavantajul ca maxim o sa putem pe unsigned int
// ceea ce inseamna fisiere de 4 giga... asta se poate schimba inlocuind totul
// cu __int64 (dar dupaia nu merge decat pe visual), deocamdata o lasam asa
// si daca e modificam dupa , oricum orice ai face la getfilesize.. o sa 
// existe intotdeauna fisiere atat de mari incat sa nu returneze cum tb valoarea
void cBitStreamSoup::GetFileSize(std::string filename)
{
  std::ifstream f;
  f.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
  if (!f.good() || f.eof() || !f.is_open()) { return ; }
  f.seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = f.tellg();
  f.seekg(0, std::ios_base::end);
  m_FileSizeInBytes =static_cast<int>(f.tellg() - begin_pos);
  f.close();
}