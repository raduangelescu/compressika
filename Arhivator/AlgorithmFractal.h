#ifndef M_FRACTALANALGO
#define M_FRACTALANALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"
// maximum de gri dintr-o imagine
#define MAX_GREY 255
/*
Numarul claselor. Fiecare clasa corespunde unei ordonari specifice a
brightnesului din imagine in cele patru cadrane ale unui range sau domeniu
* Sunt 4*3*2 = 24 clase.
*/
#define NCLASSES 24

/*
Numarul minim si maxim de biti pentru marginea unei partitii. Marimea reala
a rangeului este intre 1<<MIN_BITS (adica 2 la puterea) si 1<<MAX_BITS
Pentru a simplifica implementarea si a evita partitii ridicol de mici
MIN_BITS trebuie sa fie >=2
Implementarea asta are nevoie ca MAX_BITS <= 7.
*/
#define MIN_BITS 2
#define MAX_BITS 4
/*
* Maximum contrast factor in a range to domain mapping.
*/
#define MAX_CONTRAST 1.0
/*
* numarul de biti pe care se face encodarea contrastului si a brightnesului in functia afina
Daca folosim marimi mai mici atunci obtinem compresie mai buna dar calitatea imaginii are de suferit*/
#define CONTRAST_BITS 4
#define BRIGHTNESS_BITS 6
#define MAX_QCONTRAST ((1<<CONTRAST_BITS)-1) /* max quantized contrast */
#define MAX_QBRIGHTNESS ((1<<BRIGHTNESS_BITS)-1) /* max quantized brightness */
/*
De-quantize an integer value in the range 0 .. imax to the range 0.0 .. max
while preserving the mapping 0 -> 0.0 and imax -> max.
*/
#define dequantize(value, max, imax) ((double)(value)*(max)/(double)imax)
/*
Compute the square of a pixel value and return the result as unsigned
long
*/
#define square(pixel) (unsigned long)(pixel)*(pixel)
template <class T>
struct cimg_library::CImg;
// Algoritmul fractal
class cAlgorithmFractal: cAlgorithm
{
private:
	// structures
	struct domain_info 
	{
		int pos_bits; /* Number of bits required to encode a domain position */
		int x_domains; /* Number of domains in x (horizontal) dimension */
	};
	struct domain_data
	{
		int x; /* horizontal position */
		int y; /* vertical position */
		float d_sum; /* sum of all values in the domain */
		float d_sum2; /* sum of all squared values in the domain */
		domain_data *next; /* next domain in same class */
	} ;
	struct range_data
	{
		int x; 
		int y; 
		int s_log; 
		double r_sum; 
		double r_sum2;
	} ;
	struct affine_map
	{
		int contrast; 
		int brightness; 
		double error2;
	};
	/*
	O mapare afina este descrisa de contrast, offset luminozitate, o partitie si un domeniu.
	COntrastul si luminozitatea sunt tinute ca int pentru a face rapida decompresia pe procesoare
	cu floating point incet.
	*/
	struct map_info
	{
		int contrast; /*contrastul e scalat cu 16384 (pentru a pastra precizia)*/
		int brightness; /*offsetul de luminozitate e scalat cu 128*/
		int x; /*pozitie orizontala a partitiei horizontal position of the range */
		int y;/*pozitie verticala a partitiei horizontal position of the range */
		int size; /* marime partitie*/
		int dom_x; /* pozitia oriziontala a domeniului*/
		int dom_y; /* pozitia verticala a domeniului*/
		struct map_info *next; /*urmatoarea mapare*/
	} ;
	struct sNonShareData
	{
		/* data din partitie : range[i][j] este luminozitatea la randul i si coloana j*/
		unsigned char **range;
		/* Data domeniului este sumata peste 4 pixeli: domain[i][j] este suma valorilor pixelilor 
		* de la (2j, 2i), (2j+1, 2i), (2j, 2i+1) and (2j+1, 2i+1)*/
		unsigned **domain;
		/*Datele cumulative pentru partitii sunt pastrate doar pt pixeli de la coordonate pare.
		* cum_range[i][j] este suma tuturor valorilor de pixeli strict deasupra si in stanga pixelului
		* (2j, 2i). In particular, cum_range[y_size/2][x_size/2] este
		* suma tuturor valorilor de pixeli din imagine.Acest tabel este folosit si pentru cumulative domain data.*/
		unsigned long **cum_range;
		/* similar cu cum_range1*/
		float **cum_range2;
		float **cum_domain2;
		int dom_density;
		double max_error2;
		domain_info dom_info[MAX_BITS+1];

		domain_data *domain_head[NCLASSES][MAX_BITS+1];
		map_info *map_head; /*primul element din lista inlantuita*/
		
		sNonShareData();
		void Cleanup(unsigned int y_size);
	};

public:
	cAlgorithmFractal();
	void Compress(std::string filenameInput,std::string filenameOutput);
	void DeCompress(std::string filenameInput,std::string filenameOutput);
	virtual ~cAlgorithmFractal();

private:
	cimg_library::CImg<unsigned char> * m_inputImage;

	// we need copys for each channel
	sNonShareData  *rgb_data[3];
	cBitStreamSoup *rgb_files[3];

private:
	void CompressFile( std::fstream &input, cBitStreamSoup &output);
	void ExpandFile( cBitStreamSoup &input, std::string &output);

	/*Pentru Compresie*/
	void CompressInit(int x_size, int y_size,std::fstream &t,int channel,sNonShareData   *ns_data); 
	void CompressCleanup (int y_size);
	void ClassifyDomains (int x_size, int y_size, int s,sNonShareData   *ns_data);
	int FindClass (int x, int y, int size, sNonShareData   *ns_data);
	void CompressRange(int x, int y, int s_log, sNonShareData   *ns_data,cBitStreamSoup * frac_file);
	void FindMap (range_data *rangep, domain_data *dom, affine_map *map,sNonShareData   *ns_data);

	/*Pentru Decompresie*/
	void DecompressRange (int x, int y, int s_log,sNonShareData   *ns_data,cBitStreamSoup *frac_file);
	void RefineImage (sNonShareData  *ns_data);
	void AverageBoundaries (	sNonShareData  *ns_data);

	/*Functii comune compresie-decompresie*/

	void TraverseImage (int x, int y, int x_size, int y_size,int whatToDo,	sNonShareData  *ns_data,cBitStreamSoup *frac_file);
	
	int inline fasterfloor( const float x ) { return x < 0 ? (int) x == x ? (int) x : (int) x -1 : (int) x; }
	/* ==============================================================
	Cuantizeaza o valoare din rangeul 0-> max in rangeul 0->imax si se asigura
	ca 0.0 este codata ca 0 si max ca imax
	*/
	inline int Quantize (float value, float max, int imax)
	{	
		int ival = fasterfloor((value/max)*(imax+1));
		if (ival < 0) return 0;
		if (ival > imax) return imax;
		return ival;
	}
	void DominfoInit (int x_size, int y_size, int density,sNonShareData  *ns_data);
	void *xalloc (unsigned size);
	void **allocate (int rows, int columns, int elem_size);
	static void FreeArray (void **array, int rows);

};
#endif