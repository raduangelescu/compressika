#include "stdafx.h"
#include "BitStreamSoup.h"
#include "Progress.h"
cBitStreamSoup::cBitStreamSoup(std::string filename, std::string mod,int bufferSize)
{
	m_Mod=-1;
	m_BufferSize = bufferSize;
	m_Rack = 0; 
	m_Mask = 0x80;
	m_PacifierCounter=0;
	m_crtBuffEl = 0;

	if(mod.compare("in")==0)
	{
		m_File.open(filename.c_str(),std::ios::in|std::ios::binary);
		GetFileSize(filename);
		m_LeftFileSizeInBytes  = m_FileSizeInBytes;
		// Citeste tot fisierul in memorie
		if(bufferSize == -1)
		{
			bufferSize = m_LeftFileSizeInBytes;
		}
		m_Mod=INPUT_FILE;
	}
	if(mod.compare("out")==0)
	{
		m_File.open(filename.c_str(),std::ios::out|std::ios::binary);
		m_Mod=OUTPUT_FILE;
	}
	if(mod.compare("out-buffer")==0)
	{
		m_Mod = OUTPUT_BUFFER;
	}
	if(m_Mod != OUTPUT_BUFFER)
	{
		if(m_File.is_open())
		{
			std::string log="File ";
			log+=filename;
			log+=" Opened";
			LOG(log);// logam ceva sa fim siguri ca s-a deschis fisieru
		
			m_Buffer = NULL;
			if(m_BufferSize > 0 )
			{
				m_Buffer = new unsigned char[m_BufferSize];
			}

			if(m_Mod == INPUT_FILE)
				m_File.read((char*)m_Buffer,m_BufferSize);
		}
		else
		{
			std::string log="Could not open file: ";
			log+=filename;
			LOG(log);
		}
	}
	else
	{
		
		if(m_BufferSize > 0 )
		{
			m_Buffer = new unsigned char[m_BufferSize];
		}
	}

}
void cBitStreamSoup::OutputBit(int bit)
{
   if ( bit ) 
       m_Rack |= m_Mask; // exemplu 1101000 | 0000100=1101100 .. masca va contine tot timpul un 1 si numai 1
   m_Mask >>= 1; // o shiftam
   if ( m_Mask == 0 )
   {
	   if(!m_Buffer)
	   {
		   m_File.put( m_Rack); //scriem byteul fiindca inseamna ca am shiftat pana am completat tot byteul  
	   }
	   else
	   {
		   m_Buffer[m_crtBuffEl] = m_Rack;
		   m_crtBuffEl++;
		   if(m_crtBuffEl >= m_BufferSize)
		   {
			   if(m_Mod != OUTPUT_BUFFER)
			   {
				   m_File.write((char*)m_Buffer,m_BufferSize);
				   m_crtBuffEl = 0;
			   }
			   else // reallocate Buffer
			   {
				 unsigned char *temp = m_Buffer;
				 m_BufferSize = m_BufferSize*2;
				 m_Buffer = new unsigned char[m_BufferSize];
				 memcpy(m_Buffer,temp,m_BufferSize/2);
				 delete []temp;
			   }
		   }
	   }
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
		if(!m_Buffer)
		{
			m_File.put( m_Rack ) ;
		}
		else
		{
			m_Buffer[m_crtBuffEl] = m_Rack;
			m_crtBuffEl++;
			if(m_crtBuffEl >= m_BufferSize)
			{
				if(m_Mod != OUTPUT_BUFFER)
				{
					m_File.write((char*)m_Buffer,m_BufferSize);
					m_crtBuffEl = 0;
				}
				else // reallocate Buffer
				{
					unsigned char *temp = m_Buffer;
					m_BufferSize = m_BufferSize*2;
					m_Buffer = new unsigned char[m_BufferSize];
					memcpy(m_Buffer,temp,m_BufferSize/2);
					delete []temp;
				}
			}
		}
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
		 if(!m_Buffer)
		 {
			 m_Rack = m_File.get(); 
		 }
		 else
		 {
			if(m_crtBuffEl < m_BufferSize)
			{
				m_Rack = m_Buffer[m_crtBuffEl];
				m_crtBuffEl++;
			}
			else
			{
				m_crtBuffEl = 0;
				if(m_LeftFileSizeInBytes > 0)
				{
					m_BufferSize = (m_LeftFileSizeInBytes - m_BufferSize) < m_BufferSize ?
									(m_LeftFileSizeInBytes - m_BufferSize) : m_BufferSize;
					m_File.read((char*)m_Buffer,m_BufferSize);
					m_LeftFileSizeInBytes -= m_BufferSize;
					
					m_Rack = m_Buffer[m_crtBuffEl];
					m_crtBuffEl++;
		
				}
				else
				{
					assert(0 && "PROBLEM WITH INPUTING FROM ALGO!!");
				}
			}
		 }
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
			 if(!m_Buffer)
			 {
				 m_Rack = m_File.get(); 
			 }
			 else
			 {
				 if(m_crtBuffEl < m_BufferSize)
				 {
					 m_Rack = m_Buffer[m_crtBuffEl];
					 m_crtBuffEl++;
				 }
				 else
				 {
					 m_crtBuffEl = 0;
					 if(m_LeftFileSizeInBytes > 0)
					 {
						 m_BufferSize = (m_LeftFileSizeInBytes - m_BufferSize) < m_BufferSize ?
							 (m_LeftFileSizeInBytes - m_BufferSize) : m_BufferSize;
						 m_File.read((char*)m_Buffer,m_BufferSize);
						 m_LeftFileSizeInBytes -= m_BufferSize;

						 m_Rack = m_Buffer[m_crtBuffEl];
						 m_crtBuffEl++;

					 }
					 else
					 {
						 assert(0 && "PROBLEM WITH INPUTING FROM ALGO!!");
					 }
				 }
			 }
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

void OutputBytes(unsigned char *bytes,int count)
{

}
 //appendez bufferul curent la un fisier
 void cBitStreamSoup::AppendToFile(cBitStreamSoup *fileBS)
 {
	 for(unsigned int i = 0 ;i< m_crtBuffEl ; i++)
	 {
		fileBS->OutputBits(m_Buffer[i],8);
	 }

	 if (m_Mask != 0x80) 
	 {
		 int writeBits = BitLength(m_Mask); 
		fileBS->OutputBits(m_Rack,writeBits);
	 }

 }
 // force write buffer
 void cBitStreamSoup::ForceWrite()
 {
	 if(m_Mod==OUTPUT_FILE)
	 {
		 if (m_Buffer)
		 {
			 if(m_crtBuffEl < m_BufferSize)
			 {
				 m_File.write((char*)m_Buffer,m_crtBuffEl);
			 }
		 }
		 if ( m_Mask != 0x80 ) 
			 m_File.put(m_Rack); 
	 }

 }
// ne-am jucat si acum strangem dupa noi 
cBitStreamSoup::~cBitStreamSoup()
{
	ForceWrite();
	
	if(m_File.is_open())
		m_File.close();  
	
	if(m_Buffer)
		delete [] m_Buffer;
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