#include "stdafx.h"
#include "AlgorithmHuffmanAdaptiv.h"
#include "AlgorithmsManager.h"



cAlgorithmHuffAdaptiv m_globalTemplate;
cAlgorithmHuffAdaptiv::cAlgorithmHuffAdaptiv():cAlgorithm()
{
	m_Name="Algorithm Huffman Adaptiv";
}
void cAlgorithmHuffAdaptiv::Compress(std::string filenameInput,std::string filenameOutput)
{
		GetFileSize(filenameInput);
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	cBitStreamSoup output(filenameOutput,"out");
	CompressFile(input,output);
	input.close();
		m_CompressionProgress=0;
}
void cAlgorithmHuffAdaptiv::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);	
	ExpandFile(input,output);
	output.close();
}
cAlgorithmHuffAdaptiv::~cAlgorithmHuffAdaptiv()
{
}
/*
 Compresia vazuta de la distanta e usoara :) , initializam arborele huffman.
cu simbolul ESCAPE si END_OF_STREAM , apoi stam in bucla codand symbolurile
si adaugandu-le la model. Cand nu mai avem caractere de trimis, codam END_OF_STREAM
(asta e pt decodor sa stie cand sa se opreasca)
*/
void cAlgorithmHuffAdaptiv::CompressFile(std::fstream &input,cBitStreamSoup& output )
{
	int c;
	InitializeTree();
	c=input.get();
	m_CompressionProgress++;
	while (c != EOF )
	{
		EncodeSymbol( c, output );
		UpdateModel( c );
		c=input.get();
		m_CompressionProgress++;
		UpdateFileProgressBar(m_CompressionProgress,m_FileSizeInBytes);
	}
	EncodeSymbol(END_OF_STREAM, output );
}
/*
* Decompresia seamana cu compresia. Mai intai initializam arborele huffman
(la fel ca la compresie) apoi stam intr-o bucla decodand caractere pana ajungem
la END_OF_STREAM
*/
void cAlgorithmHuffAdaptiv::ExpandFile(cBitStreamSoup& input,std::fstream &output)
{
	int c;
	InitializeTree();
	while ( ( c = DecodeSymbol(input ) ) != END_OF_STREAM )
	{
		output.put(c);
		//if ( output.put(c) == EOF )
		//	LOG( "Error writing character" );
		UpdateModel(c );
	}
}
/*
* La compresia adaptiva , arboreele porneste aproape gol.Singurele 2 simboluri
care sa gasesc initial in el sunt ESCAPE si END_OF_STREAM. ESCAPE are rolul
sa spuna decompresiei ca transmitem un simbol neintalnit pana atunci. 
In afara de setarea copiilor si root_nodului initializam si nodurile frunza.
ESCAPE si END_OF_STREAM sunt singurele frunze definite initial, restul sunt
setate la -1 ca sa arate ca nu exista inca in arbore.
*/
void  cAlgorithmHuffAdaptiv::InitializeTree()
{
	int i;
	m_Tree.nodes[ ROOT_NODE ].child = ROOT_NODE + 1;
	m_Tree.nodes[ ROOT_NODE ].child_is_leaf = FALSE;
	m_Tree.nodes[ ROOT_NODE ].weight = 2;
	m_Tree.nodes[ ROOT_NODE ].parent = -1;
	m_Tree.nodes[ ROOT_NODE + 1 ].child = END_OF_STREAM;
	m_Tree.nodes[ ROOT_NODE + 1 ].child_is_leaf = TRUE;
	m_Tree.nodes[ ROOT_NODE + 1 ].weight = 1;
	m_Tree.nodes[ ROOT_NODE + 1 ].parent = ROOT_NODE;
	m_Tree.leaf[ END_OF_STREAM ] = ROOT_NODE + 1;
	m_Tree.nodes[ ROOT_NODE + 2 ].child = ESCAPE;
	m_Tree.nodes[ ROOT_NODE + 2 ].child_is_leaf = TRUE;
	m_Tree.nodes[ ROOT_NODE + 2 ].weight = 1;
	m_Tree.nodes[ ROOT_NODE + 2 ].parent = ROOT_NODE;
	m_Tree.leaf[ ESCAPE ] = ROOT_NODE + 2;
	m_Tree.next_free_node = ROOT_NODE + 3;
	for ( i = 0 ; i < END_OF_STREAM ; i++ )
		m_Tree.leaf[ i ] = -1; 

}
/*
Ia simbolul si il transforma intr-o secventa desemnata de arborele
huffman.Singura complicatie este ca mergem de la frunza spre radacina
si din acest motiv in final avem bitii in ordine inversa.inseamna ca tb
sa retinem bitii intr-un intreg si sa ii trimite dupaia.In aceasta versiune
tinem bitii pe un long asa ca maximul este pus la o limita de 0x8000. 
Ar putea fi pus la  65535 da nu stiu daca e safe...poate ca punem, sa vedem
ce impact are asupra performantei
*/
void cAlgorithmHuffAdaptiv::EncodeSymbol(unsigned int c,cBitStreamSoup & output )
{
	unsigned long code;
    unsigned long current_bit;
    int code_size;
    int current_node;

    code = 0;
    current_bit = 1;
    code_size = 0;
    current_node = m_Tree.leaf[ c ];
    if ( current_node == -1 )
        current_node = m_Tree.leaf[ ESCAPE ];
    while ( current_node != ROOT_NODE )
	{
        if ( ( current_node & 1 ) == 0 )
            code |= current_bit;
        current_bit <<= 1;
        code_size++;
        current_node = m_Tree.nodes[ current_node ].parent;
    };
    output.OutputBits( code, code_size );
    if ( m_Tree.leaf[ c ] == -1 )
	{
        output.OutputBits((unsigned long) c, 8 );
        add_new_node( c );
    }
}
/*
* Decodarea e usoara .Pornim de la radacina si mergem in jos pana ajugem
la o frunza. La fiecare nod decidem pe unde sa o luam in functie de bitul
de input.Dupa ce am ajuns la frunza, vedem daca am citit ESCAPE.daca da,
atunci urmatorul simbol va fi in urmatorii 8 biti, necodat.Daca e asa
cititm si il adaugam direct*/
int  cAlgorithmHuffAdaptiv::DecodeSymbol(cBitStreamSoup &input )
{
	int current_node;
    int c;

    current_node = ROOT_NODE;
    while ( !m_Tree.nodes[ current_node ].child_is_leaf )
	{
        current_node = m_Tree.nodes[ current_node ].child;
		int ex=0;
        current_node += input.InputBit();
    }
    c = m_Tree.nodes[ current_node ].child;
    if ( c == ESCAPE )
	{
        c = (int) input.InputBits( 8 );
        add_new_node(  c );
    }
    return( c );
}
/*
UpdateModel incrementeaza countul unui anumit simbol dat ca parametru
Dupa incrementare codul trebuie sa mearga in sus prin nodurile parinte
incrementand fiecare.. pana aici e ok, dupaia putin mai complicat este ca
dupa incrementare trebuie sa vedem daca arborele si-a pastrat proprietatea
de a fii huffman... daca da e super bine daca nu tb sa il modificam (mutandu-l in sus)
*/
void cAlgorithmHuffAdaptiv::UpdateModel(int  c )
{
	 int current_node;
    int new_node;

    if ( m_Tree.nodes[ ROOT_NODE].weight == MAX_WEIGHT )
        RebuildTree( );
    current_node = m_Tree.leaf[ c ];
    while ( current_node != -1 ) {
        m_Tree.nodes[ current_node ].weight++;
        for ( new_node = current_node ; new_node > ROOT_NODE ; new_node-- )
            if ( m_Tree.nodes[ new_node - 1 ].weight >=
                 m_Tree.nodes[ current_node ].weight )
                break;
        if ( current_node != new_node ) {
            swap_nodes( current_node, new_node );
            current_node = new_node;
        }
        current_node = m_Tree.nodes[ current_node ].parent;
    }
}
/*
Reconstructia arborelui se intampla atunci cand numarul de aparitii a devenit
prea mare. Nu facem decat o impartire cu 2 (desi pentru anumite streamuri de date
pot fi alte constante mai optime). Din nefericire din cauza trunchierii (impartire
pe inturi) efectele nu pot fi prevazute si din acest motiv tb sa reanalizam arborele
si sa mutam nodurile ca atare (rehufmanizam :) )
*/
void cAlgorithmHuffAdaptiv::RebuildTree( )
{
	int i;
    int j;
    int k;
    unsigned int weight;

/*
	Incepem prin a colecta toate frunzele lui nenea Hufman si le punem la sfarsitul
	copacului. In timp ce fac chestia asta , mai si scalez (impartind numarul de 
	aparitii la 2) 
*/

    j = m_Tree.next_free_node - 1;
    for ( i = j ; i >= ROOT_NODE ; i-- ) {
        if ( m_Tree.nodes[ i ].child_is_leaf ) {
            m_Tree.nodes[ j ] = m_Tree.nodes[ i ];
            m_Tree.nodes[ j ].weight = ( m_Tree.nodes[ j ].weight + 1 ) / 2;
            j--;
        }
    }

/*
	In acest moment, j indica catre primul nod liber. Acum am frunzele definite
	si trebuie sa incep construirea nodurileor de mai sus din copac. Voi incepe
	adaugarea nodului intern de la j. De fiecare data cand adaug un nou nod intern
	la varful copacului tb sa verific cui ii apartine de fapt in copac. Ar putea sa
	spuna ca apartine in sus dar exista o sansa buna sa tb sa il mut jos. Daca trebuie
	mutat jos, folosesc memmove() sa mut toti aia mai mari cu un nod.
	Nota:: memmove() s-ar putea sa tb mutat pe alte sisteme?.. putem folosi
	memcopy() .. same stuff
 */
    for ( i = m_Tree.next_free_node - 2 ; j >= ROOT_NODE ; i -= 2, j-- ) {
        k = i + 1;
        m_Tree.nodes[ j ].weight = m_Tree.nodes[ i ].weight +
                                  m_Tree.nodes[ k ].weight;
        weight = m_Tree.nodes[ j ].weight;
        m_Tree.nodes[ j ].child_is_leaf = FALSE;
        for ( k = j + 1 ; weight < m_Tree.nodes[ k ].weight ; k++ )
            ;
        k--;
        memmove( &m_Tree.nodes[ j ], &m_Tree.nodes[ j + 1 ],
                 ( k - j ) * sizeof( struct node ) );
        m_Tree.nodes[ k ].weight = weight;
        m_Tree.nodes[ k ].child = i;
        m_Tree.nodes[ k ].child_is_leaf = FALSE;
    }
/*
Pasul final in reconstruire este sa continuam prin setarea tuturor frunzelor
si parintilor. Asta o facem usor pt ca fiecare nod e in pozitia care trebuie
 */
    for ( i = m_Tree.next_free_node - 1 ; i >= ROOT_NODE ; i-- ) 
	{
        if ( m_Tree.nodes[ i ].child_is_leaf ) 
		{
            k = m_Tree.nodes[ i ].child;
            m_Tree.leaf[ k ] = i;
        }
		else 
		{
            k = m_Tree.nodes[ i ].child;
            m_Tree.nodes[ k ].parent = m_Tree.nodes[ k + 1 ].parent = i;
        }
    }
}

/*
Schimbul de noduri are loc atunci cand un nod a crescut foarte mult
si nu mai poate sa mai stea in locul actual. Cand schimbam nodurile
i si j , rearanjam arborele schimband copii lui i cu copii lui j
-- Nota1: un fel de schimb de mame... numai ca cu copii :))
-- Nota2: Maaaameeeee singureeeee..
*/
void cAlgorithmHuffAdaptiv::swap_nodes(int i,int j )
{
	 node temp;

    if ( m_Tree.nodes[ i ].child_is_leaf )
        m_Tree.leaf[ m_Tree.nodes[ i ].child ] = j;
    else
	{
        m_Tree.nodes[ m_Tree.nodes[ i ].child ].parent = j;
        m_Tree.nodes[ m_Tree.nodes[ i ].child + 1 ].parent = j;
    }
    if ( m_Tree.nodes[ j ].child_is_leaf )
        m_Tree.leaf[ m_Tree.nodes[ j ].child ] = i;
    else
	{
        m_Tree.nodes[ m_Tree.nodes[ j ].child ].parent = i;
        m_Tree.nodes[ m_Tree.nodes[ j ].child + 1 ].parent = i;
    }
    temp = m_Tree.nodes[ i ];
    m_Tree.nodes[ i ] = m_Tree.nodes[ j ];
    m_Tree.nodes[ i ].parent = temp.parent;
    temp.parent = m_Tree.nodes[ j ].parent;
    m_Tree.nodes[ j ] = temp;
}
/*
Adaugarea unui nod nou in arbore este destul de simpla. Trebuie doar 
sa impartim cel mai usor nod in copac (adica ala cu cel mai mare
numar de aparitii).Il impartim in 2 noduri noi unu care este cel adaugat
copacului . Punem noului nod un weight de 0, asa incat copacul nu trebuie
iarasi ajustat (phew...relief).O sa fie ajustat mai tarziu in procesul normal
de update. Codul se bazeaza pe faptul ca nodul cel mai usor are o frunza 
ca copil :).. daca nu ar fi asa inseamna ca arborele e stricat (Trebuie aruncat
la gunoi.. s-a intamplat o prostioara)*/
void cAlgorithmHuffAdaptiv::add_new_node( int c )
{
	 int lightest_node;
    int new_node;
    int zero_weight_node;

    lightest_node = m_Tree.next_free_node - 1;
    new_node = m_Tree.next_free_node;
    zero_weight_node = m_Tree.next_free_node + 1;
    m_Tree.next_free_node += 2;

    m_Tree.nodes[ new_node ] = m_Tree.nodes[ lightest_node ];
    m_Tree.nodes[ new_node ].parent = lightest_node;
    m_Tree.leaf[ m_Tree.nodes[ new_node ].child ] = new_node;

    m_Tree.nodes[ lightest_node ].child         = new_node;
    m_Tree.nodes[ lightest_node ].child_is_leaf = FALSE;

    m_Tree.nodes[ zero_weight_node ].child           = c;
    m_Tree.nodes[ zero_weight_node ].child_is_leaf   = TRUE;
    m_Tree.nodes[ zero_weight_node ].weight          = 0;
    m_Tree.nodes[ zero_weight_node ].parent          = lightest_node;
    m_Tree.leaf[ c ] = zero_weight_node;
}