#include "stdafx.h"
#include "AlgorithmRLE.h"
#include "AlgorithmArithmetic.h"
cAlgorithmArithmetic m_globalTemplate;
cAlgorithmArithmetic::cAlgorithmArithmetic():cAlgorithm()
{
	m_Name="Algorithm Arithmetic";
	m_Ext=".ath";
}


cAlgorithmArithmetic::~cAlgorithmArithmetic()
{
}
/*
Partea asta e destul de simpla , face rost de statistici si initializeaza
encoderul aritmetic.Apoi ecodeaza toate caracterele in fisier si la sfarsit
adauga EOF. Streamul de output este adaugat la sfarsit si se iese. 
Nota: Se baga in output 2 bytes in plus. Cand decodam streamul aritmetic
trebuie sa citim biti in plus. Procesul de decodare are loc in MSB 
(most significant bit) a inturilor de high si low astfel incat cand
decodam ultimul bit o sa avem inca cel putin 15 biti complet inutili
(de tras apa dupa ei) de incarcat in registrii. Bytesii extra sunt
exact pentru asta
*/
void cAlgorithmArithmetic:: Compress(std::string filenameInput,std::string filenameOutput)
{
	m_CompressionProgress=0;
	filenameOutput+=m_Ext;
	GetFileSize(filenameInput);
	int c;
	SYMBOL s;
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");
	BuildModel( input, output.m_File );
	InitializeArithmeticEncoder();
	c = input.get();m_CompressionProgress++;
	while ( c != EOF ) 
	{
		ConvertIntToSymbol( c, s );
		EncodeSymbol( output, s );
		c = input.get();
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
	}
	ConvertIntToSymbol( END_OF_STREAM, s );
	EncodeSymbol( output, s );
	FlushArithmeticEncoder( output );
	output.OutputBits(  0L, 16 );
	input.close();
	m_CompressionProgress=0;

}
/*
Si asta de decompresi e destul de logica. Citeste modelul , initializeaza
decodorul , apoti sta in bucla citind caractere. Cand decodam END_OF_STREAM 
inseamna ca putem inchide fisierul si sa iesim.
Nota: Decodificarea unui singur caracter este un proces in 3 pasi :
1- Determinam care este scala pentru simbolul curent (uitandu-ne la diferenta dintre low si high)
2- Vedem unde ar tb sa fie input valuesurile (in functie de range)
3- Ne uitam in vectorul totals ca sa descoperim ce simbol este
La sfarsim trebuie sa scoate simbolul din decodor..
*/
void cAlgorithmArithmetic::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	m_CompressionProgress=0;
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);
	SYMBOL s;
	int c;
	int count;

	InputCounts( input.m_File );
	InitializeArithmeticDecoder( input );
	while(TRUE)
	{
		GetSymbolScale( s );
		count = GetCurrentCount( s );
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
		c = ConvertSymbolToInt( count, s );
		if ( c == END_OF_STREAM )
			break;
		RemoveSymbolFromStream( input, s );
		output.put((char)c);
	}
	output.close();
	m_CompressionProgress=0;
}
/*
Asta scaneaza input fileul , scale counturile , face vectorul totals si 
baga in output counturile scalate
*/
void cAlgorithmArithmetic::BuildModel(std::fstream & input,std::fstream & output )
{
	unsigned long counts[ 256 ];
	unsigned char scaled_counts[ 256 ];
	CountBytes( input, counts );
	ScaleCounts( counts, scaled_counts );
	OutputCounts( output, scaled_counts );
	BuildTotals( scaled_counts );
}
/*
Mergem prin fisier si numaram aparitiile fiecarui caracter
*/

void cAlgorithmArithmetic::CountBytes( std::fstream &input,unsigned long counts[] )
{
	int i;
	int c;
	for ( i = 0 ; i < 256; i++ )
		counts[ i ] = 0;
	c=input.get();
	while ( c != EOF )
	{
		counts[ c ]++;
		c=input.get();
	}
	input.clear();
	input.seekg(0, std::ios::beg);
}
/*
Asta scaleaza counturile. Sunt doua tipuri de scalare ce trebuie facute.
1- Counturile trebuie sa fie scalate ca sa poata incape fiecare intr-un
unsigned char (adica 255)
2-Counturile trebuie rescalate astfel incat totalul tuturor counturilor
sa fie mai mic decat 16384 pt ca am considerat doar 16 biti
*/
void  cAlgorithmArithmetic::ScaleCounts(unsigned long counts[], unsigned char scaled_counts[] )
{
	int i;
	unsigned long max_count;
	unsigned int total;
	unsigned long scale;
	/*
	partea asta face ca:
	un count  sa incapa intr-un singur byte
	*/
	max_count = 0;
	for ( i = 0 ; i < 256 ; i++ )
		if ( counts[ i ] > max_count )
			max_count = counts[ i ];
	scale = max_count / 256;
	scale = scale + 1;
	for ( i = 0 ; i < 256 ; i++ ) 
	{
		scaled_counts[ i ] = (unsigned char ) ( counts[ i ] / scale );
		if ( scaled_counts[ i ] == 0 && counts[ i ] != 0 )
			scaled_counts[ i ] = 1;
	}
	/*
	Partea asta se asigura ca totalul este mai mic decat 16384
	Initializez totalul cu 1 in loc de 0 pentru ca o sa mai fie un 1 adaugat
	pentru eternul END_OF_STREAM
	*/
	total = 1;
	for ( i = 0 ; i < 256 ; i++ )
		total += scaled_counts[ i ];
	if ( total > ( 32767 - 256 ) )
		scale = 4;
	else if ( total > 16383 )
		scale = 2;
	else
		return;
	for ( i = 0 ; i < 256 ; i++ )
		scaled_counts[ i ] =(unsigned char)((unsigned long)scaled_counts[ i ] / scale);// am scris asa mare ca sa nu avem warninguri.. 
	// puteam sa scriu evident scaled_counts[i]/=scale;... dar da warning.. 
	// va rog sa nu bagati warninguri in program :).. si daca exista discutam sa le scoatem
	// si cum sa le scoatem fiindca daca aici faceam scaled_counts[i]/=(unsigned char)scale.. disparea
	// warningul ... dar era gresit ..fiindca facea truncherea inainte sa imparta
	
}
/*
Functia asta e folosita si de compresie si de decompresie pentru a
construi tabelul de totaluri cumulative. Counturile pentru caracterele
din fisiere sun in vectorul counts, si stim ca o sa fie o singura instanta
a lui EOF

*/
void cAlgorithmArithmetic::BuildTotals( unsigned char scaled_counts[] )
{
	int i;
	m_siTotals[ 0 ] = 0;
	for ( i = 0 ; i < END_OF_STREAM ; i++ )
		m_siTotals[ i + 1 ] = m_siTotals[ i ] + scaled_counts[ i ];
	m_siTotals[ END_OF_STREAM + 1 ] = m_siTotals[ END_OF_STREAM ] + 1;
}
/*
Compresorul trebuie sa construiasca acelasi model , inseamna ca trebuie
sa tinem counturile simbolurilor in fisierul comprimat ca decomprimatorul
sa poata sa le citeasca. Pentru a salva spatiu nu salvam toate 256 simboluri
neconditionat (exact ca la SHannon-Fanno)., Formatul este:

start,stop,counts,start,stop,counts,.....0

Ca sa fie eficient tb sa identificam counturile diferite de 0. Datorita
formatului asta nu vreau sa opresc treaba daca doar 1 sau 2 sunt zerouri
asa ca stam in bucla si ne uitam dupa streamuri de trei sau mai multe 
zerouri 
Desi e un concept usor.. de kko :) functia arata nasol.. Daca faceam 
mai taraneste , sa scriu direct tot vectorul se simtea la compresia
fisierelor mici.
*/
void cAlgorithmArithmetic::OutputCounts(std::fstream & output, unsigned char scaled_counts[] )
{
	int first;
	int last;
	int next;
	int i;
	first = 0;
	while ( first < 255 && scaled_counts[ first ] == 0 )
		first++;

	for ( ; first < 256 ; first = next ) 
	{
		last = first + 1;
		while ( TRUE ) 
		{
			for ( ; last < 256 ; last++ )
				if ( scaled_counts[ last ] == 0 )
					break;
			last --;
			for ( next = last + 1; next < 256 ; next++ )
				if ( scaled_counts[ next ] != 0 )
					break;
			if ( next > 255 )
				break;
			if ( ( next - last ) > 3 )
				break;
			last = next;
		}

		output.put( first);
		output.put(last);
		for ( i = first ; i <= last ; i++ )
		{
			output.put( scaled_counts[ i ]);
		}
	}
	output.put(0);
}
/*
La decompresie trebuie sa citesc counturile scrise cu functia de mai sus
si uite ca asta si fac aici :)
*/
void cAlgorithmArithmetic::InputCounts(std::fstream &input )
{
	int first;
	int last;
	int i;
	int c;
	unsigned char scaled_counts[ 256 ];
	for ( i = 0 ; i < 256 ; i++ )
		scaled_counts[ i ] = 0;
	first =input.get() ;
	last = input.get() ;
	while (TRUE) 
	{
		for ( i = first ; i <= last ; i++ )
		{
			c = input.get();
			scaled_counts[ i ] = (unsigned int) c;
		}
		 first =input.get();

		if ( first == 0 )
			break;
		last=input.get();
	}
	BuildTotals( scaled_counts );
}
/*
Pana acum au fost functii destul de standard .. majoritatea sa gasesc si in
SHannon-Fanno (de altfel ma gandesc sa facem o clasa separata pentru astea
fiindca le folosim destul de des... aproape la fiecare algoritm
////////////////////////////////////////////////////////////////////
De aici incolo vine smecheroshenia cu comprimatorul Aritmetic
________________________________________________________________
*/
/*

/*
Initializam... high=111111111111111....1111111111unununununun... infinit
*/
void cAlgorithmArithmetic::InitializeArithmeticEncoder()
{
    m_siLow = 0;
    m_siHigh = 0xffff;
    m_lUnderflowBits = 0;
}
/*
La sfarsitul encodari procesului , e un numar semnificativ de biti ramasi
in high si low. Scriem 2 biti plus cati biti de underflow este nevoie
*/
void cAlgorithmArithmetic::FlushArithmeticEncoder(cBitStreamSoup & stream )
{

	stream.OutputBit(  m_siLow & 0x4000 );
    m_lUnderflowBits++;
    while ( m_lUnderflowBits-- > 0 )
       stream.OutputBit( ~m_siLow & 0x4000 );
}
/*
Gasim countul pt low, si pt high si scalam pt un simbol.. e destul de usor
deoarece  totalurile sunt stocate ca vector. Cam singura chestie misto
din implementarea cu totaluri asa..
*/
void cAlgorithmArithmetic::ConvertIntToSymbol(int  c, SYMBOL &s )
{
	s.scale = m_siTotals[ END_OF_STREAM + 1];
	s.low_count = m_siTotals[ c ];
	s.high_count = m_siTotals[ c + 1 ];
}
/*
* Scala contextului curent
*/
void cAlgorithmArithmetic::GetSymbolScale(SYMBOL & s )
{
	s.scale = m_siTotals[ END_OF_STREAM + 1 ];
}
int cAlgorithmArithmetic::ConvertSymbolToInt( int count, SYMBOL &s )
{
	int c;
	for ( c = END_OF_STREAM ; count < m_siTotals[ c ] ; c-- );
		s.high_count = m_siTotals[ c + 1 ];
		s.low_count = m_siTotals[ c ];
	
	return( c );
}
/*
Encodeaza simbolul. Simbolul este trimis in structura SYMBOL ca un low count
high count si range. Encodarea are 2 pasi:
1--Valorile lui hi si a lui low sunt updatate sa tina cont de restrictia rangeului
creata de noul simbol
2--Cat mai multi biti sunt shiftati in output stream
3--High si low sunt stabilizati , iesim afara.. bem o cana cu apa , ne jucam cu prietena
*/
void cAlgorithmArithmetic::EncodeSymbol(cBitStreamSoup& stream,SYMBOL & s )
{
	  long range;
/*
 *Rescalam high si low pentru simbolul nou
 */
    range = (long) ( m_siHigh-m_siLow ) + 1;
    m_siHigh = m_siLow + (unsigned short int )
                 (( range * s.high_count ) / s.scale - 1 );
    m_siLow  = m_siLow  + (unsigned short int )
                 (( range * s.low_count ) / s.scale );
/*
 * bagam biti pana se stabilizeaza (adica high si low sunt suficient de departati)
 */
    while(TRUE)
    {
/*
Daca e cum zice in iful asta , inseamna ca MSD (most significant digit)
se potrivesc si putem trimite in output
 */
        if ( ( m_siHigh & 0x8000 ) == ( m_siLow  & 0x8000 ) )
        {
			stream.OutputBit(  m_siHigh & 0x8000 );
            while ( m_lUnderflowBits > 0 )
            {
               stream.OutputBit( ~m_siHigh & 0x8000 );
                m_lUnderflowBits--;
            }
        }
/*
 * Daca e cum zice if-ul astalalt atunci e super primejdie de underflow
 * nu se potrivesc MSD-urile inseamna ca digiturile sunt diferite doar cu 1
 */
        else if ( ( m_siLow  & 0x4000 ) && !( m_siHigh & 0x4000 ))
        {
            m_lUnderflowBits += 1;
            m_siLow  &= 0x3fff;
            m_siHigh |= 0x4000;
        }
        else
            return ;
        m_siLow  <<= 1;
        m_siHigh <<= 1;
        m_siHigh |= 1;
    }
}
/*
Cand decodam chemam prostia asta sa vedem ce simbol urmeaza sa fie decodat
cod= count/scala_simbol
*/
short int cAlgorithmArithmetic::GetCurrentCount( SYMBOL &s )
{
    long range;
    short int count;

    range = (long) ( m_siHigh - m_siLow  ) + 1;
	count = (short int)
            ((((long) ( m_siCode - m_siLow  ) + 1 ) * s.scale-1 ) / range );
    return( count );
}
/*
Initializam decodorul
high low bla bla.. mai interesant este ca citim primii 16 biti in 
valoarea de cod
*/
void cAlgorithmArithmetic::InitializeArithmeticDecoder(cBitStreamSoup& stream )
{
    int i;

    m_siCode = 0;
    for ( i = 0 ; i < 16 ; i++ )
    {
        m_siCode <<= 1;
		int ex=0;
		m_siCode += stream.InputBit( );
    }
    m_siLow  = 0;
    m_siHigh = 0xffff;
}
/*
Dupa decodare tb sa scoatem caracterul din input
*/
void cAlgorithmArithmetic::RemoveSymbolFromStream(cBitStreamSoup& stream,SYMBOL & s )
{
	   long range;

/*
 * Modificam rangeul pt ca se modifica daca scoatem un caracter.. duh.. evident
 */
    range = (long)( m_siHigh - m_siLow  ) + 1;
    m_siHigh = m_siLow  + (unsigned short int)
                 (( range * s.high_count ) / s.scale - 1 );
    m_siLow  = m_siLow + (unsigned short int)
                 (( range * s.low_count ) / s.scale );

	while(TRUE)
    {

        if ( ( m_siHigh & 0x8000 ) == ( m_siLow  & 0x8000 ) )
        {
        }

        else if ((m_siLow  & 0x4000) == 0x4000  && (m_siHigh & 0x4000) == 0 )
        {
            m_siCode ^= 0x4000;
            m_siLow    &= 0x3fff;
            m_siHigh  |= 0x4000;
        }
     else
            return;
        m_siLow  <<= 1;
        m_siHigh <<= 1;
        m_siHigh |= 1;
        m_siCode <<= 1;
		int ex=0;
        m_siCode += stream.InputBit(  );
    }
}
/* Ma gandeam sa bagam si simbolul #define END_OF_CAT... 
   
            \ \
             \ \
             / /
            / /
           _\ \_/\/\
          /  *  \@@ =
         |       |Y/
         |       |~
          \ /_\ /
           \\ //
            |||
           _|||_
          ( / \ )
		  
	Daca ai ajuns pana aici dau o bere
*/