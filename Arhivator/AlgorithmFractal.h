#ifndef M_FRACTALANALGO
#define M_FRACTALANALGO
#include "Algorithm.h"
#include "BitStreamSoup.h"
#include "ABitmap.h"
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
	public:
		cAlgorithmFractal();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmFractal();
	private:
		cimg_library::CImg<unsigned char> * m_inputImage;
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
	struct domain_info 
	{
		int pos_bits; /* Number of bits required to encode a domain position */
		int x_domains; /* Number of domains in x (horizontal) dimension */
	} dom_info[MAX_BITS+1];
	typedef struct domain_struct 
	{
		int x; /* horizontal position */
		int y; /* vertical position */
		float d_sum; /* sum of all values in the domain */
		float d_sum2; /* sum of all squared values in the domain */
		struct domain_struct *next; /* next domain in same class */
	} domain_data;
	cBitStreamSoup *frac_file;
	domain_data *domain_head[NCLASSES][MAX_BITS+1];
	typedef struct range_struct 
	{
		int x; 
		int y; 
		int s_log; 
		double r_sum; 
		double r_sum2;
	} range_data;
	typedef struct map_struct 
	{
		int contrast; 
		int brightness; 
		double error2;
	} affine_map;
		void CompressFile( std::fstream &input, cBitStreamSoup &output);
		void ExpandFile( cBitStreamSoup &input, std::fstream &output );

		/*Pentru Compresie*/
		void CompressInit(int x_size, int y_size,std::fstream &t,int channel); 
		void CompressCleanup (int y_size);
		void ClassifyDomains (int x_size, int y_size, int s);
		int FindClass (int x, int y, int size);
		void CompressRange(int x, int y, int s_log);
		void FindMap (range_data *rangep, domain_data *dom, affine_map *map);
		
		/*Pentru Decompresie*/
		void DecompressRange (int x, int y, int s_log);
		void RefineImage (void);
		void AverageBoundaries (void);

		/*Functii comune compresie-decompresie*/
	
		void TraverseImage (int x, int y, int x_size, int y_size,int whatToDo);
		int Quantize (double value, double max, int imax);
		void DominfoInit (int x_size, int y_size, int density);
		void *xalloc (unsigned size);
		void **allocate (int rows, int columns, int elem_size);
		void FreeArray (void **array, int rows);	
		int BitLength (unsigned long val);
		
		
		CBitmapRaster  m_Bitmap;

};
#endif