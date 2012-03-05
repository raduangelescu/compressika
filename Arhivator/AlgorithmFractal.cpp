#include "stdafx.h"
#include "AlgorithmFractal.h"
#include "AlgorithmsManager.h"
#include <assert.h>
#include <math.h>
#define COMPRESS_RANGE 0
#define DECOMPRESS_RANGE 1

using namespace cimg_library ;

cAlgorithmFractal m_globalTemplate;
/*
Factorul de scalare pentru decompresie (marimea decomprimata impartita la marimea originala)
Suportam doar int pentru a simplifica implementarea
*/
int image_scale = 1;
cAlgorithmFractal::cAlgorithmFractal():cAlgorithm(),dom_density(0)
{
	m_Name="Algorithm Fractal";
	m_Ext=".frac";
}
void	cAlgorithmFractal::Compress(std::string filenameInput,std::string filenameOutput)
{
	filenameOutput+=m_Ext;
	GetFileSize(filenameInput);
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
	//m_Bitmap.LoadBMP( (char*)filenameInput.c_str());
	CImg<unsigned char> img(filenameInput.c_str());
	m_inputImage = &img ;
	cBitStreamSoup output(filenameOutput,"out");

	CompressFile(input,output);
	input.close();
	m_CompressionProgress=0;
	
}
void cAlgorithmFractal::DeCompress(std::string filenameInput,std::string filenameOutput)
{
	cBitStreamSoup input(filenameInput,"in");
	std::fstream output(filenameOutput.c_str(),std::ios::out|std::ios::binary);	
	ExpandFile(input,output);
	output.close();
	m_CompressionProgress=0;
}
void cAlgorithmFractal::CompressFile(std::fstream &input,cBitStreamSoup &output)
{
	frac_file= &output;
	double quality = 2.0; /* factor de calitate */
	int s; /* indexul de marime pentru domenii este 1<<(s+1)  adica 2^(s+1) */
	
	int x_size = m_inputImage->_width; /* marimea orizontala a  imaginii */
	int y_size = m_inputImage->_height; /* marimea verticala a imaginii */

	if (dom_density < 0 || dom_density > 2) 
	{
		assert(0);//("Domeniu de densitate incorect.\n");
	}
	if (x_size % 4 != 0 || y_size % 4 != 0) 
	{
		assert(0);// ("Marimea imaginilor trebuie sa fie multiplu de 4\n");
	}
	/* Aloca si initializeaza datele imaginii si a imaginii cumulative */
	CompressInit(x_size, y_size, input);
	/* Initializeaza marimea domeniului ca in decomprimare*/
	DominfoInit(x_size, y_size, dom_density);
	/* Clasifica domeniile: */
	for (s = MIN_BITS; s <= MAX_BITS; s++)
	{
		ClassifyDomains(x_size, y_size, s);
	}
	/*Scriem headerul fisierului comprimat. */
	output.OutputBits((unsigned long)'F', 8);
	output.OutputBits((unsigned long)x_size, 16);
	output.OutputBits((unsigned long )y_size,16);
	output.OutputBits((unsigned long)dom_density,2);
	/* Comprimam imaginea recursiv */
	max_error2 = quality*quality;
	TraverseImage(0, 0, x_size, y_size,COMPRESS_RANGE);
	/* Eliberam memoria: */
	CompressCleanup(y_size);
}
/* ================================================================
Elibereaza toate structurile alocate dinamic pentru compresie.
*/
void cAlgorithmFractal::CompressCleanup(int y_size)
{
	int s; /* marimea indexului pentru domenii */
	int classa; /* numarul de clase */
	domain_data *dom, *next; /* pointeri la domenii*/
	FreeArray((void**)range, y_size);
	FreeArray((void**)domain, y_size/2);
	FreeArray((void**)cum_range, y_size/2 + 1);
	FreeArray((void**)cum_range2, y_size/2 + 1);
	FreeArray((void**)cum_domain2, y_size/2 + 1);
	for (s = MIN_BITS; s <= MAX_BITS; s++)
		for (classa = 0; classa < NCLASSES; classa++)
			for (dom = domain_head[classa][s]; dom != NULL; dom = next)
			{
				next = dom->next;
				free(dom);
			}
}
void cAlgorithmFractal::CompressInit(int x_size,int y_size, std::fstream &image_file)
{
	int x, y; /* indicii pe orizontala si pe verticala*/
	unsigned long r_sum; /*data pentru partitia cumulativa si domeniu*/
	double r_sum2; /* data pentru data partitiei cumulative sub radical*/
	double d_sum2; /* data pentru domeniu sub radical */
	range = (unsigned char**)allocate(y_size, x_size,
		sizeof(unsigned char));
	domain = (unsigned**)allocate(y_size/2, x_size/2,
		sizeof(unsigned));
	cum_range = (unsigned long**)allocate(y_size/2+1, x_size/2+1,
		sizeof(unsigned long));
	cum_range2 = (float**)allocate(y_size/2+1, x_size/2+1,
		sizeof(float));
	cum_domain2 = (float**)allocate(y_size/2+1, x_size/2+1,
		sizeof(float));
	/* Citim imaginile de input: */
	CImg<unsigned char> red=m_inputImage->get_channel(0);
	
	cimg_forY(*m_inputImage,y)
		cimg_forX(*m_inputImage,x)
	{
		range[x][y] = red(x,y);
	}
	//for (y = 0; y < y_size; y++) 
	//{
	//	m_inputImage
	//	for(x = 0 ; x < x_size ; x++)
	//		range[x][y]=(m_Bitmap.Palette[m_Bitmap.Raster[y*m_Bitmap.getWidth()+x]].rgbRed+m_Bitmap.Palette[m_Bitmap.Raster[y*m_Bitmap.getWidth()+x]].rgbBlue+m_Bitmap.Palette[m_Bitmap.Raster[y*m_Bitmap.getWidth()+x]].rgbGreen)/3;
		
	//}
	/*Calculam imaginea domeniului din cea a partitiei. Fiecare pixel in imaginea domeniu este suma
	a 4 pixeli din imaginea partitiei. Nu facem media (adica nu impartim la 4) ca sa nu pierdem precizie	
	*/
	for (y=0; y < y_size/2; y++)
		for (x=0; x < x_size/2; x++) 
		{
			domain[y][x] = (unsigned)range[y<<1][x<<1] + range[y<<1]
			[(x<<1)+1] + range[(y<<1)+1][x<<1] + range[(y<<1)+1][(x<<1)+1];
		}
		/* Calculam data cumulativa ceea ce va evita computari repetate pentru acelasi lucru mai tarziu
		( cu functia region_sum() )*/
		for (x=0; x <= x_size/2; x++) 
		{
			cum_range[0][x] = 0;
			cum_range2[0][x] = cum_domain2[0][x] = 0.0;
		}
		for (y=0; y < y_size/2; y++) 
		{
			d_sum2 = r_sum2 = 0.0;
			r_sum = cum_range[y+1][0] = 0;
			cum_range2[y+1][0] = cum_domain2[y+1][0] = 0.0;
			for (x=0; x < x_size/2; x++) 
			{
				r_sum += domain[y][x];
				cum_range[y+1][x+1] = cum_range[y][x+1] + r_sum;
				d_sum2 += (double)square(domain[y][x]);
				cum_domain2[y+1][x+1] = cum_domain2[y][x+1] + (float)d_sum2;
				r_sum2 += (double) (square(range[y<<1][x<<1])
					+ square(range[y<<1][(x<<1)+1])
					+ square(range[(y<<1)+1][x<<1])
					+ square(range[(y<<1)+1][(x<<1)+1]));
				cum_range2[y+1][x+1] = cum_range2[y][x+1] + (float)r_sum2;
			}
		}
}
/* ==================================================================
* Calculeaza suma valorilor pixelilor sau a radicalului de pixeli din partitie
sau domeniu de la (x,y) pana la (x+size-1, y+size-1) inclusiv.
Pentru un domeniu, valoarea returnata este scalata cu 4 sau 16 respectiv 
x,y si marimea trebuie sa fie toate pare.
*/
#define region_sum(cum,x,y,size) \
(cum[((y)+(size))>>1][((x)+(size))>>1] - cum[(y)>>1][((x)+(size))>>1] \
- cum[((y)+(size))>>1][(x)>>1] + cum[(y)>>1][(x)>>1])
/* =================================================================
*	Clasifica toate domeniile de o anumita marime. Acest lucru e facut o singura data
sa salveze computatii mai tarziu. Fiecare domeniu este inserrat intr-o lista inlantuita
corespunzator clasei ei si marimii .
*/
void cAlgorithmFractal::ClassifyDomains(int x_size,int y_size,int s)
{
	domain_data *dom = NULL; /* pointer catre domeniul nou*/
	int x, y; /* pozitiile domeniului pe orizontala si pe verticala */
	int classa; /* clasa domeniului */
	int dom_size = 1<<(s+1); /* marimea domeniului */
	int dom_dist = dom_size >> dom_density; /* distanta dintre domenii*/
	/* Initializeaza listele domeniului ca fiind goale: */
	for (classa = 0; classa < NCLASSES; classa++)
	{
		domain_head[classa][s] = NULL;
	}
	/* Clasifica toate domeniiile de aceasta marime */
	for (y = 0; y <= y_size - dom_size; y += dom_dist)
		for (x = 0; x <= x_size - dom_size; x += dom_dist) 
		{
			dom = (domain_data *)xalloc(sizeof(domain_data));
			dom->x = x;
			dom->y = y;
			dom->d_sum =(float)( 0.25 *(double)region_sum(cum_range, x, y,dom_size));
			dom->d_sum2 = (float)(0.0625*(double)region_sum(cum_domain2, x, y,dom_size));
			classa = FindClass(x, y, dom_size);
			dom->next = domain_head[classa][s];
			domain_head[classa][s] = dom;
		}
		/* Verifica ca fiecare clasa de domeniiu contine cel putin un domeniu . Daca clasa  este goala
		ne comportam ca si cand contine ultimee domenii create (ceea ce reprezinta de fapt alta clasa).
		*/
		for (classa = 0; classa < NCLASSES; classa++)
		{
			if (domain_head[classa][s] == NULL) {
				domain_data *dom2 = (domain_data *)xalloc(sizeof(domain_data));
				*dom2 = *dom;
				dom2->next = NULL;
				domain_head[classa][s] = dom2;
			}
		}
}
void cAlgorithmFractal::ExpandFile( cBitStreamSoup &input, std::fstream &output )
{
	int x_size; /* marimea imaginii pe orizontala */
	int y_size; /* marimea imaginii pe verticala */
	int x_dsize; /* marimea imaginii decomprimata pe orizontala */
	int y_dsize; /* marimea imaginii decomprimata pe verticala*/
	int iterations = 16; /* numarul de iteratii*/
	int y; /* randul curent care este inscris pe hddisc */
	/*Citim headerul fisierului fractal:*/
	frac_file = &input;
	if (frac_file->InputBits( 8) != 'F') 
	{
		assert(0);///"Format gresit\n";
	}
	x_size = (int)frac_file->InputBits( 16);
	y_size = (int)frac_file->InputBits( 16);
	dom_density = (int)frac_file->InputBits( 2);
	/* Alocam imaginea scalata: */
	x_dsize = x_size * image_scale;
	y_dsize = y_size * image_scale;
	range = (unsigned char**)allocate(y_dsize, x_dsize, sizeof(unsigned char));

	/*Initializam informatia domeniului ca in compresor*/
	DominfoInit(x_size, y_size, dom_density);
	/*Citim toate maparile afine folosind aceeasi traversare recursiva a imaginii ca in compresor */
	TraverseImage(0, 0, x_size, y_size, DECOMPRESS_RANGE);
	for(int i=0;i< x_dsize ;i++)
		for(int j=0 ; j< y_size ; j++)
			range[i][j] = 255;
	/*Iteram toate maparile afine pe imaginea initiala (care e random) Astfel ca 
	fiindca toate maparile afine sunt contractive, procesul converge*/
	while (iterations-- > 0) 
		RefineImage();
	/* Smoothuim tranzitiile intre marginile partitiilor vecine:*/
	AverageBoundaries();
	/* Scriem fisierul decomprimat: */

		//Create a new file for writing

	FILE *pFile = fopen("unpack.bmp", "wb");
	if(pFile == NULL)
		{
		assert(0);	
	}
		BITMAPINFOHEADER BMIH;
		BMIH.biSize = sizeof(BITMAPINFOHEADER);
		BMIH.biBitCount = 24;
		BMIH.biPlanes = 1;
		BMIH.biCompression = BI_RGB;
		BMIH.biWidth = x_size;
		BMIH.biHeight = y_size;
		BMIH.biSizeImage = ((((BMIH.biWidth * BMIH.biBitCount) + 31) & ~31) >> 3) * BMIH.biHeight;
		BITMAPFILEHEADER bmfh;
		int nBitsOffset = sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);
		LONG lImageSize = BMIH.biSizeImage; 
		LONG lFileSize = nBitsOffset + lImageSize;
		bmfh.bfType = 'B'+('M'<<8);
		bmfh.bfOffBits = nBitsOffset;
		bmfh.bfSize = lFileSize;
		bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		//Write the bitmap file header

		UINT nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);
		//And then the bitmap info header

		UINT nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(BITMAPINFOHEADER), pFile);
		//Finally, write the image data itself 

		//-- the data represents our drawing

	
	
	
	for (y = y_dsize-1; y >=0 ; y--)
	{
		output.write((const char*)range[y],x_dsize);
		for(int x=0 ; x< x_dsize;x++)
		{
			UINT nWrittenDIBDataSize = fwrite(&range[x][y], 1,1, pFile);
			nWrittenDIBDataSize = fwrite(&range[x][y], 1,1, pFile);
			nWrittenDIBDataSize = fwrite(&range[x][y], 1,1, pFile);
		}

	}
		fclose(pFile);
	/*Curatam memoria: */
	FreeArray((void**)range, y_dsize);
}
cAlgorithmFractal::~cAlgorithmFractal()
{
}
/* =================================================================
Clasificam o partitie sau un domeniu. Clasa este determinata prin
ordonarea luminositatii in cele 4 cadrane ale partitiei sau domeniului.
Pentru fiecare cadran computam numarul de cuadrane mai luminoase; aceasta
abordare este suficienta ca sa determinam in mod unic clasa. clasa 0 are
cadranele in ordinea descrescatoare a luminii; clasa 23 are cadranele
in ordinea crescatoare a luminii.
In plus: x , y si marimile sunt toate multiplii de 4
*/
int cAlgorithmFractal::FindClass(int x, int y,int size)
{
	int classa = 0; /* clasa rezultata*/
	int i,j; /* indicii cadranelor */
	unsigned long sum[4]; /* sumele pentru fiecare cadran*/
	static int delta[3] = {6, 2, 1}; /*tabel folosit pentru calculul numarului de clasa*/
	int size1 = size >> 1;
	/*Ia valorile cumulative pentru fiecare cadran
	*/
	sum[0] = region_sum(cum_range, x, y, size1);
	sum[1] = region_sum(cum_range, x, y+size1, size1);
	sum[2] = region_sum(cum_range, x+size1, y+size1, size1);
	sum[3] = region_sum(cum_range, x+size1, y, size1);
	/* Calculeaza clasa din ordonarea acestor valori */
	for (i = 0; i <= 2; i++)
		for (j = i+1; j <= 3; j++) {
			if (sum[i] < sum[j]) classa += delta[i];
		}
		return classa;
}
/* =================================================================
Comprima o partitie cautand un matchcu toate domeniile din aceeasi clasa
Imparte partitia daca eroarea cu domaniul cel mai apropiat este mai mare
ca max_error2

* In plus: MIN_BITS <= s_log <= MAX_BITS
*/
void cAlgorithmFractal::CompressRange(int x,int y, int s_log)
{
	int r_size = 1<<s_log; /*marimea partitiei*/
	int classa; /* clasa partitiei */
	domain_data *dom; /*folosit pentru a itera prin toate domeniile acestei clase*/
	domain_data *best_dom = NULL; /*pointer catre cel mai bun domeniu */
	range_data range; /* informatie pentru partitie */
	affine_map map; /* mapare afina pentru domeniul curent */
	affine_map best_map; /* cea mai buna mapare pentru partitie */
	unsigned long dom_number; /* numarul domeniului */
	/* Calculeaza clasa rangeului si sumele cumulative: */
	classa = FindClass(x, y, r_size);
	range.r_sum = (double)region_sum(cum_range, x, y, r_size);
	range.r_sum2 = (double)region_sum(cum_range2, x, y, r_size);
	range.x = x;
	range.y = y;
	range.s_log = s_log;
	/* Cautarea prin toate clasele poate imbunatatii calitatea imaginii rezultate dar incetineste foarte
	mult procesul de compresie. (Pentru teste putem definii COMPLETE_SEARCH 
	*/
#ifdef COMPLETE_SEARCH
	for (classa = 0; classa < NCLASSES; classa++)
#endif
		for (dom = domain_head[classa][s_log]; dom != NULL; dom =dom->next) 
		{
			/*Gaseste maparea optima de la partitie la domeniu */
			FindMap(&range, dom, &map);
			if (best_dom == NULL || map.error2 < best_map.error2) 
			{
				best_map = map;
				best_dom = dom;
			}
		}
		/*Scrie cea mai buna mapare afina daca eroarea cu domeniul cel mai bun e mai mica de max_error2, sau 
		daca nu mai e posibil sa impartim partitia in partii mai mici fiindca e prea mica 
		*/
		if (s_log == MIN_BITS || best_map.error2 <= max_error2*((long)r_size*r_size)) 
		{
				/*Daca partitia e prea mica sa mai fie impartitia decompresorul trebuie sa stie asta
				altfel trebuie sa indicam ca partitia nu a mai fost impartita:
				*/
				if (s_log != MIN_BITS) 
				{
					frac_file->OutputBit( 1); /* urmeaza maparea afina*/
				}
				frac_file->OutputBits( (unsigned long)best_map.contrast,CONTRAST_BITS);
				frac_file->OutputBits( (unsigned long)best_map.brightness,BRIGHTNESS_BITS);
				/*Cand contrastul e null , decompresorul nu trebuie sa stie ce domeniu a fost selectat: */
				if (best_map.contrast == 0) 
					return;
				dom_number = (unsigned long)best_dom->y * dom_info[s_log].x_domains+ (unsigned long)best_dom->x;
				/* Distanta dintre doua domenii este marimea domeniului 1<<(s_log+1) shiftata la stanga de densitatea domeniului astfel incat este o putere a lui 2
				Pozitiile domeniului x si y au (s_log + 1 - dom_density) numar de biti zero , pe care nu treubie sa ii transmitem*/
				frac_file->OutputBits( dom_number >> (s_log + 1 - dom_density),dom_info[s_log].pos_bits);
		} 
		else 
		{
			/*Spune decompresorului ca nu urmeaza nici o mapare afina fiindca partitia a fost splituita
			*/
			frac_file->OutputBit( 0);
			/* Impartim partitia in 4 patrate si le procesam recursiv: */
			CompressRange(x, y, s_log-1);
			CompressRange(x+r_size/2, y, s_log-1);
			CompressRange(x, y+r_size/2, s_log-1);
			CompressRange(x+r_size/2, y+r_size/2, s_log-1);
		}
}
/* ==================================================================
* Gaseste cea mai buna mapare afina de la o partitie la un domeniu.
Aste e facuta minimizand suma sub radical a erorilor ca o functie de contrast si luminozitate
data de
* radical din(contrast*domain[di] + brightness - range[ri])
si rezolvam ecuatiile rezultate pentru a gasi contrastul si luminozitatea
*/
void cAlgorithmFractal::FindMap(range_data *rangep,domain_data * dom,affine_map * map)
{
	int ry; /* pozitia verticala in partitie*/
	int dy = dom->y >> 1; /* pozitia verticala in domeniu*/
	unsigned long rd = 0; /* suma range*domain values (scalata cu  4) */
	double rd_sum; /* suma valorilor range*domain  (normalizata) */
	double contrast; /* contrast optimal intre partitie si domeniu*/
	double brightness; /*offset de luminozitate optima intre partitie si domeniu*/
	double qbrightness;/*luminozitate dupa cuantizare*/
	double max_scaled; /* maximum scaled value = contrast*MAX_GREY */
	int r_size = 1 << rangep->s_log; /* marimea partitiei*/
	double pixels = (double)((long)r_size*r_size); /*numarul total de pixeli*/
	for (ry = rangep->y; ry < rangep->y + r_size; ry++, dy++)
	{
		register unsigned char *r = &range[ry][rangep->x];
		register unsigned *d = &domain[dy][dom->x >> 1];
		int i = r_size >> 2;
		/* Urmatorul loop este partea cea mai consumnatoare de timp din tot
		programul , si din acest motiv este unrolluit putin (sunt scrise instructiuni in plus
		de iteratie). Ne bazam pe faptul ca r_size este multiplu de 4 (partitiile mai mici de 4
		nu au sens fiindca rezulta compresie foarte proasta) . rd nu poate avea overflow cu 
		pe un unsigned de 32 de biti fiindca MAX_BITS <= 7 ceea ce implica r_size <=128*/
		do 
		{
			rd += (unsigned long)(*r++)*(*d++);
			rd += (unsigned long)(*r++)*(*d++);
			rd += (unsigned long)(*r++)*(*d++);
			rd += (unsigned long)(*r++)*(*d++);
		}
		while (--i != 0);
	}
	rd_sum = 0.25*rd;
	/* Calculaeaza si cuantizeaza contrastul*/
	contrast = pixels * dom->d_sum2 - dom->d_sum * dom->d_sum;
	if (contrast != 0.0) {
		contrast = (pixels*rd_sum - rangep->r_sum*dom->d_sum)/contrast;
	}
	map->contrast = Quantize(contrast, MAX_CONTRAST, MAX_QCONTRAST);
	/* Recalculeaza contrastul ca in decompresor:*/
	contrast = dequantize(map->contrast, MAX_CONTRAST, MAX_QCONTRAST);
	/* Calculeaza si cuantizeaza luminozitatea. Cuantizam de fapt valoarea
	* (brightness + 255*contrast) pentru a avea o valoare pozitiva:
	* -contrast*255 <= brightness <= 255
	* astfel 0 <= brightness + 255*contrast <= 255 + contrast*255
	*/
	brightness = (rangep->r_sum - contrast*dom->d_sum)/pixels;
	max_scaled = contrast*MAX_GREY;
	map->brightness = Quantize(brightness + max_scaled,
		max_scaled + MAX_GREY, MAX_QBRIGHTNESS);
	/*Recalculam valoarea cuantizata a luminozitatii ca in decompresor: */
	qbrightness = dequantize(map->brightness, max_scaled + MAX_GREY,
		MAX_QBRIGHTNESS) - max_scaled;
	/*Calculam suma erorilor care este cantitatea pe care incercam sa o minimizam
	*/
	map->error2 = contrast*(contrast*dom->d_sum2 -
		2.0*rd_sum) +
		rangep->r_sum2 + qbrightness*pixels*(qbrightness - 2.0*brightness);
}
/************************************/
/* Functii pentru decompresie */
/************************************/

/*
O mapare afina este descrisa de contrast, offset luminozitate, o partitie si un domeniu.
COntrastul si luminozitatea sunt tinute ca int pentru a face rapida decompresia pe procesoare
cu floating point incet.
*/
typedef struct map_info_struct 
{
	int contrast; /*contrastul e scalat cu 16384 (pentru a pastra precizia)*/
	int brightness; /*offsetul de luminozitate e scalat cu 128*/
	int x; /*pozitie orizontala a partitiei horizontal position of the range */
	int y;/*pozitie verticala a partitiei horizontal position of the range */
	int size; /* marime partitie*/
	int dom_x; /* pozitia oriziontala a domeniului*/
	int dom_y; /* pozitia verticala a domeniului*/
	struct map_info_struct *next; /*urmatoarea mapare*/
} map_info;

map_info *map_head = NULL; /*primul element din lista inlantuita*/
/* Citeste maparea afina pentru partitie sau imparte partitia daca compresorul a facut asta in compress_range()
*/
void cAlgorithmFractal::DecompressRange(int x,int y,int s_log)
{
	int r_size = 1<<s_log; /* marime partitie*/
	map_info *map; /* pointer la informatia de mapare afina*/
	double contrast; /* contrastul dintre partitie si domeniu */
	double brightness; /* offset luminozitate intre partitie si domeniu*/
	double max_scaled; /* maximum scaled value = contrast*MAX_GREY */
	unsigned long dom_number; /* numarul domeniului*/
	/*Citestee maparea afina daca compresorul a scris una */
	if (s_log == MIN_BITS || frac_file->InputBit()) 
	{
		map = (map_info *)xalloc(sizeof(map_info));
		map->next = map_head;
		map_head = map;
		map->x = x;
		map->y = y;
		map->size = r_size;
		map->contrast = (int)frac_file->InputBits(CONTRAST_BITS);
		map->brightness = (int)frac_file->InputBits(BRIGHTNESS_BITS);
		contrast = dequantize(map->contrast, MAX_CONTRAST,
			MAX_QCONTRAST);
		max_scaled = contrast*MAX_GREY;
		brightness = dequantize(map->brightness, max_scaled +MAX_GREY,MAX_QBRIGHTNESS) - max_scaled;
		/*Scaleaza luminozitatea cu 128 ca sa mentina precizia mai tarziu , in acelasi timp evitam overflow
		aritmetic cu 16 biti  -255<= ~contrast *255 <= luminozitate<= 255 astfel -32767 < luminozitate *128<32767*/
		map->brightness = (int)(brightness*128.0);
		/*Cand contrastul este nul , compresorul nu a encodat numarul domeniului: */
		if (map->contrast != 0) 
		{
			/* Scaleaza contrasxtul cu 16384 pentru a mentine precizia mai incolo.
			* 0.0 <= contrast <= 1.0 so 0 <= contrast*16384 <= 16384 */
			map->contrast = (int)(contrast*16384.0);
			/* Citim numarul domeniului si adaugam bitii de zero netransmissi de compresor:*/
			dom_number = frac_file->InputBits( dom_info[s_log].pos_bits);
			map->dom_x = (int)(dom_number % dom_info[s_log].x_domains)<< (s_log + 1 - dom_density);
			map->dom_y = (int)(dom_number / dom_info[s_log].x_domains)<< (s_log + 1 - dom_density);
		} 
		else
		{
			/*Pentru un contrast nul folosim un domeniu arbitrar: */
			map->dom_x = map->dom_y = 0;
		}
		/* Scalam partitia si domeniul daca este necesar. Aceasta implementare foloseste doar scalare pe integer
		ca sane asiguram ca reuniunea tuturor partitiilor este excat imaginea scalata si ca partitiile nu se suprapun 
		si ca toate partiiile sunt de marime para
		*/
		if (image_scale != 1) 
		{
			map->x *= image_scale;
			map->y *= image_scale;
			map->size *= image_scale;
			map->dom_x *= image_scale;
			map->dom_y *= image_scale;
		}
	}
	else 
	{
		/*Impartim partitia in 4 patrate si  le procesam recursiv ca la compresie:*/
		DecompressRange(x, y, s_log-1);
		DecompressRange(x+r_size/2, y, s_log-1);
		DecompressRange(x, y+r_size/2, s_log-1);
		DecompressRange(x+r_size/2, y+r_size/2, s_log-1);
	}
}
/* ===================================================================
* generam imaginea aplicand o runda de toate maparile afine pe imagine.Metoda 
"pura" ar necesa computarea unei imagini noi separate si apoi copierea ei peste
cea originala. Insa convergenta catre imaginea finala se intampla rapid si din acest motiv
daca rescriem aceeasi imagine si cu maparile afine aplicate obtinem acelasi rezultat optim
dpdv al memoriei*/
void cAlgorithmFractal::RefineImage()
{
	map_info *map; /*pointer la maparea afina curenta*/
	long brightness; /*offsetul de luminozitate al maparii scalat cu 65536 */
	long val; /* marime noua pentru pixel */
	int y; /* pozitie verticala in range */
	int dom_y; /* pozitie verticala in domeniu */
	int j;
	for (map = map_head; map != NULL; map = map->next)
	{
		/* map->brightness e scalata cu 128, trebuie scalata din nou cu  512 pentru a obtine factorul de scalare 65536:*/
		brightness = (long)map->brightness << 9;
		dom_y = map->dom_y;
		for (y = map->y; y < map->y + map->size; y++) 
		{
			/* Urmatorul loop consuma cel mai mult timp astfel ca mutam niste calcule de adresa
			in afara lui*/
			unsigned char *r = &range[y][map->x];
			unsigned char *d = &range[dom_y++][map->dom_x];
			unsigned char *d1 = &range[dom_y++][map->dom_x];
			j = map->size;
			do 
			{
				val = *d++ + *d1++;
				val += *d++ + *d1++;
				/* val este scalata cu 4 si map->contrast e scalat cu 16384,
				* astfel val * map->contrast va fi scalat cu 65536.*/
				val = val * map->contrast + brightness;
				if (val < 0)
					val = 0;
				val >>= 16; /*renuntam la scalarea 65536 */
				if (val >= MAX_GREY) 
					val = MAX_GREY;
				*r++ = (unsigned char)val;
			} 
			while (--j != 0);
		}
	}
}
/* =================================================================
* Mergem prin toate partitiile ca sa smoothuim tranzitiile dintre ele mai putin intre cele foarte mici
*/
void cAlgorithmFractal::AverageBoundaries()
{
	map_info *map; /* pointer la maparea curenta afina*/
	unsigned val; /* suma valorilor de pixeli pentru partitiile curente si cele vecine*/
	int x; /*pozitia orizontala in partitia curenta*/
	int y; /*pozitia verticala in partitia curenta*/
	for (map = map_head; map != NULL; map = map->next) 
	{
		if (map->size == (1<<MIN_BITS))
			continue; /* partitie prea mica*/
		if (map->x > 1) 
		{
			/* Smooth marginea din stanga a partitie si cea din dreapta a partitiei vecine */
			for (y = map->y; y < map->y + map->size; y++) 
			{
				val = range[y][map->x - 1] + range[y][map->x];
				range[y][map->x - 1] =(unsigned char)((range[y][map->x - 2] + val)/3);
				range[y][map->x] =(unsigned char)((val + range[y][map->x + 1])/3);
			}
		}
		if (map->y > 1) 
		{
			/* Smooth marginea de sus a partitiei si de jos a parittiei vecine
			*/
			for (x = map->x; x < map->x + map->size; x++)
			{
				val = range[map->y - 1][x] + range[map->y][x];
				range[map->y - 1][x] =(unsigned char)((range[map->y - 2][x] + val)/3);
				range[map->y][x] =(unsigned char)((val + range[map->y + 1][x])/3);
			}
		}
	}
}

/* ===================================================================
* Imparte un dreputinghi intr-un patrat si  doua dreptunghiuri, apoi imparte patratul in dreptunghiuri recursiv daca este necesar.
Pentru a simplifica algoritmul, marimea patratului este aleasa ca o putere a lui 2.
Daca patratul este suficient de mic ca partitie , cheama metoda de compresie sau de decompresie
* stim ca: x, y, x_size si y_size sunt multiplii 4.
*/
void cAlgorithmFractal::TraverseImage(int x, int y,int  x_size,int  y_size,int process)
{
	int s_size; /* marimea patratului s_size = 1<<s_log */
	int s_log; /* log base 2 a marimii*/
	s_log = BitLength(x_size < y_size ? (unsigned long)x_size :(unsigned long)y_size)-1;
	s_size = 1 << s_log;
	/* Cum x_size si y_size sunt >=4  s_log >= MIN_BITS */
	/* Imparte patratul recursiv daca este prea mare pentru partitie:*/
	if (s_log > MAX_BITS) 
	{
		TraverseImage(x, y, s_size/2, s_size/2,process);
		TraverseImage(x+s_size/2, y, s_size/2, s_size/2,process);
		TraverseImage(x, y+s_size/2, s_size/2, s_size/2,process);
		TraverseImage(x+s_size/2, y+s_size/2, s_size/2, s_size/2,process);
	} 
	else 
	{
		/* Comprima sau decomprima patratul ca in partitie: */
		switch (process)
		{
			case COMPRESS_RANGE:
				CompressRange(x, y, s_log);
				break;
			case DECOMPRESS_RANGE:
				DecompressRange(x, y, s_log);
				break;
		}
				
	}
	/* Traverseaza dreptunghiul din dreapta patratului: */
	if (x_size > s_size) 
	{
		TraverseImage(x + s_size, y, x_size - s_size, y_size,process);
		/* Cum x_size si s_size sunt multiplii de 4, x + s_size si
		* x_size - s_size sunt de asemena multiplii de 4.
		*/
	}
	/* Traverseaza dreptunghiul din josul patratului:*/
	if (y_size > s_size) 
	{
		TraverseImage(x, y + s_size, s_size, y_size - s_size,process);
	}
}
/* =================================================================
*Initializaeaza informatia din domeniu. Asta trebuie facuta in aceeasi manierea
ca in compresor si decompresor.
*/
void cAlgorithmFractal::DominfoInit(int x_size,int y_size,int density)
{
	int s; /* marimea domeniilor 1<<(s+1) */
	for (s = MIN_BITS; s <= MAX_BITS; s++)
	{
		int y_domains; /* numarul domeniilor pe verticala*/
		int dom_size = 1<<(s+1); /* marimea domeniului */
		/* distanta intre doua domenii este marimea domeniului  1<<(s+1)
		* shiftata la dreapta cu densitatea domeniului.*/
		dom_info[s].x_domains = ((x_size - dom_size)>>(s + 1 - density)) + 1;
		y_domains = ((y_size - dom_size)>>(s + 1 -density)) + 1;
		/* Numarul de biti necesar pentru a encoda pozitiile domeniului: */
		dom_info[s].pos_bits = BitLength((unsigned long)dom_info[s].x_domains * y_domains - 1);
	}
}
/* ==============================================================
Cuantizeaza o valoare din rangeul 0-> max in rangeul 0->imax si se asigura
ca 0.0 este codata ca 0 si max ca imax
*/
int cAlgorithmFractal::Quantize(double value,double max,int imax)
{
	int ival = (int) floor((value/max)*(double)(imax+1));
	if (ival < 0) return 0;
	if (ival > imax) return imax;
	return ival;
}
/* ==============================================================
* Aloca memorie si verifica daca a alaocat memorie cum trebuie
*/
void *cAlgorithmFractal::xalloc(unsigned size)
{
	void *p = malloc(size);
	if (p == NULL)
	{
		assert(0);//("insufficient memory\n");
	}
	return p;
}
/* ==============================================================
*Aloca o matrice.
*/
void **cAlgorithmFractal::allocate(int rows,int columns,int elem_size)
{
	int row;
	void **array = (void**)xalloc(rows * sizeof(void *));
	for (row = 0; row < rows; row++) {
		array[row] = (void*)xalloc(columns * elem_size);
	}
	return array;
}
/* ==========================================================
*Elibereaza o matrice 
*/
void cAlgorithmFractal::FreeArray(void **array,int rows)
{
	int row;
	for (row = 0; row < rows; row++)
	{
		free(array[row]);
	}
}
/* =============================================================
* Da numarul de biti necesar reprezentarii unui intreg:
* 0 to 1 -> 1,
* 2 to 3 -> 2,
* 3 to 7 -> 3, etc...
* Se poate mai rapid cu tabel de lookup.
*/
int cAlgorithmFractal::BitLength(unsigned long val)
{
int bits = 1;
if (val > 0xffff)
{
	bits += 16;
	val >>= 16;
}	

if (val > 0xff)
{
	bits += 8;
	val >>= 8;
}
if (val > 0xf)
{
	bits += 4;
	val >>= 4;
}
if (val > 0x3)
{
	bits += 2;
	val >>= 2;
}
if (val > 0x1) 
	bits += 1;
return bits;
}