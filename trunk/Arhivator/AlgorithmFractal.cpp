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
cAlgorithmFractal::sNonShareData::sNonShareData()
{
	range = NULL;
	domain = NULL;
	cum_range = NULL;
	cum_range2 = NULL;
	cum_domain2 = NULL;
	dom_density = 0;
	max_error2 = 0;

	for(unsigned int i = 0 ;i< MAX_BITS+1 ; i++)
	{
		dom_info[i].pos_bits = 0;
		dom_info[i].x_domains = 0;
	}
	int s; /* marimea indexului pentru domenii */
	int classa; /* numarul de clase */

	map_head = NULL; /*primul element din lista inlantuita*/
	for (s = MIN_BITS; s <= MAX_BITS; s++)
		for (classa = 0; classa < NCLASSES; classa++)
				domain_head[classa][s] = NULL;
			
}
void cAlgorithmFractal::sNonShareData::Cleanup(unsigned int y_size)
{
	int s; /* marimea indexului pentru domenii */
	int classa; /* numarul de clase */
	domain_data *dom, *next; /* pointeri la domenii*/
	cAlgorithmFractal::FreeArray((void**)range, y_size);
	cAlgorithmFractal::FreeArray((void**)domain, y_size/2);
	cAlgorithmFractal::FreeArray((void**)cum_range, y_size/2 + 1);
	cAlgorithmFractal::FreeArray((void**)cum_range2, y_size/2 + 1);
	cAlgorithmFractal::FreeArray((void**)cum_domain2, y_size/2 + 1);
	for (s = MIN_BITS; s <= MAX_BITS; s++)
		for (classa = 0; classa < NCLASSES; classa++)
			for (dom = domain_head[classa][s]; dom != NULL; dom = next)
			{
				if(dom)
				{
					next = dom->next;
					free(dom);
					dom = NULL;
					domain_head[classa][s] = NULL;
				}
			}
	range = NULL;
	domain = NULL;
	cum_range = NULL;
	cum_range2 = NULL;
	dom_density = NULL;
	max_error2 = NULL;
	cum_domain2 = NULL;
	for(unsigned int i = 0 ;i< MAX_BITS+1 ; i++)
	{
		dom_info[i].pos_bits = 0;
		dom_info[i].x_domains = 0;
	}

			map_head = NULL;
}
cAlgorithmFractal::cAlgorithmFractal():cAlgorithm()
{
	m_Name="Algorithm Fractal";
	m_Ext=".frac";
	m_isLossy = true;
		 
}
void	cAlgorithmFractal::Compress(std::string filenameInput,std::string filenameOutput)
{
	filenameOutput+=m_Ext;
	GetFileSize(filenameInput);
	std::fstream input(filenameInput.c_str(),std::ios::in|std::ios::binary);
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
	ExpandFile(input,filenameOutput);
	m_CompressionProgress=0;
}
void cAlgorithmFractal::CompressFile(std::fstream &input,cBitStreamSoup &output)
{
	cBitStreamSoup *frac_file;
	sNonShareData   *ns_data;

	frac_file = &output;

	int x_size = m_inputImage->_width; /* marimea orizontala a  imaginii */
	int y_size = m_inputImage->_height; /* marimea verticala a imaginii */


	if (x_size % 4 != 0 || y_size % 4 != 0) 
	{
		assert(0);// ("Marimea imaginilor trebuie sa fie multiplu de 4\n");
	}
	// facem asta pe toate canalele r g b


	frac_file->OutputBits((unsigned long)'F', 8);
	frac_file->OutputBits((unsigned long)x_size, 16);
	frac_file->OutputBits((unsigned long )y_size,16);
	
	
	cBitStreamSoup r_stream("", "out-buffer");
	cBitStreamSoup g_stream("", "out-buffer");
	cBitStreamSoup b_stream("", "out-buffer");

	rgb_files[0] = &r_stream;
	rgb_files[1] = &g_stream;
	rgb_files[2] = &b_stream;

	rgb_data[0]	= new sNonShareData;
	rgb_data[1]	= new sNonShareData;
	rgb_data[2] = new sNonShareData;

#pragma omp parallel for private(frac_file), private(ns_data)
	for (	int i =0 ; i<3 ; i++)
	{
		frac_file = rgb_files[i];
		ns_data	  = rgb_data[i];
		
		double quality = 2.0; /* factor de calitate */
		int s; /* indexul de marime pentru domenii este 1<<(s+1)  adica 2^(s+1) */

		/* Aloca si initializeaza datele imaginii si a imaginii cumulative */
		CompressInit(x_size, y_size, input,i,ns_data);
		
		if (ns_data->dom_density < 0 || ns_data->dom_density > 2) 
		{
			assert(0);//("Domeniu de densitate incorect.\n");
		}
		/* Initializeaza marimea domeniului ca in decomprimare*/
		DominfoInit(x_size, y_size, ns_data->dom_density,ns_data);
		/* Clasifica domeniile: */
		for (s = MIN_BITS; s <= MAX_BITS; s++)
		{
			ClassifyDomains(x_size, y_size, s,ns_data);
		}
		/*Scriem headerul fisierului comprimat. */

		frac_file->OutputBits((unsigned long)ns_data->dom_density,2);
		/* Comprimam imaginea recursiv */
		ns_data->max_error2 = quality*quality;
		TraverseImage(0, 0, x_size, y_size,COMPRESS_RANGE,ns_data,frac_file);
		/* Eliberam memoria: */
		//CompressCleanup(y_size);
		ns_data->Cleanup(y_size);
	}

	delete rgb_data[0];
	delete rgb_data[1];
	delete rgb_data[2];

	rgb_data[0] = NULL;
	rgb_data[1] = NULL;
	rgb_data[2] = NULL;

	frac_file = &output;
 	for(int i = 0 ; i< 3; i++)
	{
		rgb_files[i]->AppendToFile(frac_file);
	}

}
/* ================================================================
Elibereaza toate structurile alocate dinamic pentru compresie.
*/
void cAlgorithmFractal::CompressCleanup(int y_size)
{


}
void cAlgorithmFractal::CompressInit(int x_size,int y_size, std::fstream &image_file,int channel,sNonShareData   *ns_data)
{
	int x, y; /* indicii pe orizontala si pe verticala*/
	unsigned long r_sum; /*data pentru partitia cumulativa si domeniu*/
	double r_sum2; /* data pentru data partitiei cumulative sub radical*/
	double d_sum2; /* data pentru domeniu sub radical */
	ns_data->range = (unsigned char**)allocate(y_size, x_size,
		sizeof(unsigned char));
	ns_data->domain = (unsigned**)allocate(y_size/2, x_size/2,
		sizeof(unsigned));
	ns_data->cum_range = (unsigned long**)allocate(y_size/2+1, x_size/2+1,
		sizeof(unsigned long));
	ns_data->cum_range2 = (float**)allocate(y_size/2+1, x_size/2+1,
		sizeof(float));
	ns_data->cum_domain2 = (float**)allocate(y_size/2+1, x_size/2+1,
		sizeof(float));
	/* Citim imaginile de input: */
	CImg<unsigned char> _chanel=m_inputImage->get_channel(channel);
	cimg_forY(_chanel,y)
		cimg_forX(_chanel,x)
	{
		ns_data->range[x][y] = _chanel(x,y);
	}

	/*Calculam imaginea domeniului din cea a partitiei. Fiecare pixel in imaginea domeniu este suma
	a 4 pixeli din imaginea partitiei. Nu facem media (adica nu impartim la 4) ca sa nu pierdem precizie	
	*/
	for (y=0; y < y_size/2; y++)
		for (x=0; x < x_size/2; x++) 
		{
			ns_data->domain[y][x] = (unsigned)ns_data->range[y<<1][x<<1] + ns_data->range[y<<1]
			[(x<<1)+1] + ns_data->range[(y<<1)+1][x<<1] + ns_data->range[(y<<1)+1][(x<<1)+1];
		}
		/* Calculam data cumulativa ceea ce va evita computari repetate pentru acelasi lucru mai tarziu
		( cu functia region_sum() )*/
		for (x=0; x <= x_size/2; x++) 
		{
			ns_data->cum_range[0][x] = 0;
			ns_data->cum_range2[0][x] = ns_data->cum_domain2[0][x] = 0.0;
		}
		for (y=0; y < y_size/2; y++) 
		{
			d_sum2 = r_sum2 = 0.0;
			r_sum = ns_data->cum_range[y+1][0] = 0;
			ns_data->cum_range2[y+1][0] = ns_data->cum_domain2[y+1][0] = 0.0;
			for (x=0; x < x_size/2; x++) 
			{
				r_sum += ns_data->domain[y][x];
				ns_data->cum_range[y+1][x+1] = ns_data->cum_range[y][x+1] + r_sum;
				d_sum2 += (double)square(ns_data->domain[y][x]);
				ns_data->cum_domain2[y+1][x+1] = ns_data->cum_domain2[y][x+1] + (float)d_sum2;
				r_sum2 += (double) (square(ns_data->range[y<<1][x<<1])
					+ square(ns_data->range[y<<1][(x<<1)+1])
					+ square(ns_data->range[(y<<1)+1][x<<1])
					+ square(ns_data->range[(y<<1)+1][(x<<1)+1]));
				ns_data->cum_range2[y+1][x+1] = ns_data->cum_range2[y][x+1] + (float)r_sum2;
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
void cAlgorithmFractal::ClassifyDomains(int x_size,int y_size,int s,sNonShareData   *ns_data)
{
	domain_data *dom = NULL; /* pointer catre domeniul nou*/
	int x, y; /* pozitiile domeniului pe orizontala si pe verticala */
	int classa; /* clasa domeniului */
	int dom_size = 1<<(s+1); /* marimea domeniului */
	int dom_dist = dom_size >> ns_data->dom_density; /* distanta dintre domenii*/
	/* Initializeaza listele domeniului ca fiind goale: */
	for (classa = 0; classa < NCLASSES; classa++)
	{
		ns_data->domain_head[classa][s] = NULL;
	}
	/* Clasifica toate domeniiile de aceasta marime */
	for (y = 0; y <= y_size - dom_size; y += dom_dist)
		for (x = 0; x <= x_size - dom_size; x += dom_dist) 
		{
			dom = (domain_data *)xalloc(sizeof(domain_data));
			dom->x = x;
			dom->y = y;
			dom->d_sum =(float)( 0.25 *(double)region_sum(ns_data->cum_range, x, y,dom_size));
			dom->d_sum2 = (float)(0.0625*(double)region_sum(ns_data->cum_domain2, x, y,dom_size));
			classa = FindClass(x, y, dom_size,ns_data);
			dom->next = ns_data->domain_head[classa][s];
			ns_data->domain_head[classa][s] = dom;
		}
		/* Verifica ca fiecare clasa de domeniiu contine cel putin un domeniu . Daca clasa  este goala
		ne comportam ca si cand contine ultimee domenii create (ceea ce reprezinta de fapt alta clasa).
		*/
		for (classa = 0; classa < NCLASSES; classa++)
		{
			if (ns_data->domain_head[classa][s] == NULL) {
				domain_data *dom2 = (domain_data *)xalloc(sizeof(domain_data));
				*dom2 = *dom;
				dom2->next = NULL;
				ns_data->domain_head[classa][s] = dom2;
			}
		}
}

void cAlgorithmFractal::ExpandFile( cBitStreamSoup &input, std::string &outputSTR)
{

	int x_size; /* marimea imaginii pe orizontala */
	int y_size; /* marimea imaginii pe verticala */
	int x_dsize; /* marimea imaginii decomprimata pe orizontala */
	int y_dsize; /* marimea imaginii decomprimata pe verticala*/
	/*Citim headerul fisierului fractal:*/
	cBitStreamSoup *frac_file = &input;
	
	rgb_data[0]	= new sNonShareData;
	rgb_data[1]	= new sNonShareData;
	rgb_data[2] = new sNonShareData;


	if (frac_file->InputBits( 8) != 'F') 
	{
		assert(0);///"Format gresit\n";
	}
	x_size = (int)frac_file->InputBits( 16);
	y_size = (int)frac_file->InputBits( 16);

	// facem asta pe toate canalele r g b
	CImg<unsigned char> out(x_size,y_size,1,3);

	for (unsigned int c =0 ; c< 3 ; c++)
	{	
		sNonShareData   *ns_data = rgb_data[c];

		int iterations = 16; /* numarul de iteratii*/
		//CImg<unsigned char> &chan = out.channel(i);
		ns_data->dom_density = (int)frac_file->InputBits( 2);
		/* Alocam imaginea scalata: */
		x_dsize = x_size * image_scale;
		y_dsize = y_size * image_scale;
		ns_data->range = (unsigned char**)allocate(y_dsize, x_dsize, sizeof(unsigned char));

		/*Initializam informatia domeniului ca in compresor*/
		DominfoInit(x_size, y_size, ns_data->dom_density,ns_data);
		/*Citim toate maparile afine folosind aceeasi traversare recursiva a imaginii ca in compresor */
		TraverseImage(0, 0, x_size, y_size, DECOMPRESS_RANGE,ns_data,frac_file);
		for(int i=0;i< x_dsize ;i++)
			for(int j=0 ; j< y_size ; j++)
				ns_data->range[i][j] = 255;
		/*Iteram toate maparile afine pe imaginea initiala (care e random) Astfel ca 
		fiindca toate maparile afine sunt contractive, procesul converge*/
		while (iterations-- > 0) 
			RefineImage(ns_data);
		/* Smoothuim tranzitiile intre marginile partitiilor vecine:*/
		AverageBoundaries(ns_data);

		cimg_forY(out,y)
			cimg_forX(out,x)
		{
			out(x,y,0,c) =ns_data->range[x][y];
		}
		CompressCleanup(y_dsize);
		//FreeArray((void**)ns_data->range, y_dsize);
		ns_data->dom_density = 0;
		
		//ns_data->Cleanup(y_size);
		//delete(ns_data);
		//ns_data = NULL;
	}
	std::string filename = outputSTR.append( ".bmp");
	
	
	delete rgb_data[0];
	delete rgb_data[1];
	delete rgb_data[2];

	rgb_data[0] = NULL;
	rgb_data[1] = NULL;
	rgb_data[2] = NULL;
	
	out.save((const char*)filename.c_str() );
	
	/* Scriem fisierul decomprimat: */



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
int cAlgorithmFractal::FindClass(int x, int y,int size, sNonShareData   *ns_data)
{
	int classa = 0; /* clasa rezultata*/
	int i,j; /* indicii cadranelor */
	unsigned long sum[4]; /* sumele pentru fiecare cadran*/
	static int delta[3] = {6, 2, 1}; /*tabel folosit pentru calculul numarului de clasa*/
	int size1 = size >> 1;
	/*Ia valorile cumulative pentru fiecare cadran
	*/
	sum[0] = region_sum(ns_data->cum_range, x, y, size1);
	sum[1] = region_sum(ns_data->cum_range, x, y+size1, size1);
	sum[2] = region_sum(ns_data->cum_range, x+size1, y+size1, size1);
	sum[3] = region_sum(ns_data->cum_range, x+size1, y, size1);
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
ca ns_data->max_error2

* In plus: MIN_BITS <= s_log <= MAX_BITS
*/
void cAlgorithmFractal::CompressRange(int x,int y, int s_log, sNonShareData   *ns_data,cBitStreamSoup * frac_file)
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
	classa = FindClass(x, y, r_size,ns_data);
	range.r_sum = (double)region_sum(ns_data->cum_range, x, y, r_size);
	range.r_sum2 = (double)region_sum(ns_data->cum_range2, x, y, r_size);
	range.x = x;
	range.y = y;
	range.s_log = s_log;
	/* Cautarea prin toate clasele poate imbunatatii calitatea imaginii rezultate dar incetineste foarte
	mult procesul de compresie. (Pentru teste putem definii COMPLETE_SEARCH 
	*/
#ifdef COMPLETE_SEARCH
	for (classa = 0; classa < NCLASSES; classa++)
#endif

		for (dom = ns_data->domain_head[classa][s_log]; dom != NULL; dom =dom->next) 
		{
			/*Gaseste maparea optima de la partitie la domeniu */
			FindMap(&range, dom, &map,ns_data);
			if (best_dom == NULL || map.error2 < best_map.error2) 
			{
				best_map = map;
				best_dom = dom;
			}
		}
		/*Scrie cea mai buna mapare afina daca eroarea cu domeniul cel mai bun e mai mica de ns_data->max_error2, sau 
		daca nu mai e posibil sa impartim partitia in partii mai mici fiindca e prea mica 
		*/
		if (s_log == MIN_BITS || best_map.error2 <= ns_data->max_error2*((long)r_size*r_size)) 
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
			dom_number = (unsigned long)best_dom->y * ns_data->dom_info[s_log].x_domains+ (unsigned long)best_dom->x;
			/* Distanta dintre doua domenii este marimea domeniului 1<<(s_log+1) shiftata la stanga de densitatea domeniului astfel incat este o putere a lui 2
			Pozitiile domeniului x si y au (s_log + 1 - ns_data->dom_density) numar de biti zero , pe care nu treubie sa ii transmitem*/
			frac_file->OutputBits( dom_number >> (s_log + 1 - ns_data->dom_density),ns_data->dom_info[s_log].pos_bits);
		} 
		else 
		{
			/*Spune decompresorului ca nu urmeaza nici o mapare afina fiindca partitia a fost splituita
			*/
			frac_file->OutputBit( 0);
			/* Impartim partitia in 4 patrate si le procesam recursiv: */
			CompressRange(x, y, s_log-1,ns_data,frac_file);
			CompressRange(x+r_size/2, y, s_log-1,ns_data,frac_file);
			CompressRange(x, y+r_size/2, s_log-1,ns_data,frac_file);
			CompressRange(x+r_size/2, y+r_size/2, s_log-1,ns_data,frac_file);
		}
}
/* ==================================================================
* Gaseste cea mai buna mapare afina de la o partitie la un domeniu.
Aste e facuta minimizand suma sub radical a erorilor ca o functie de contrast si luminozitate
data de
* radical din(contrast*ns_data->domain[di] + brightness - ns_data->range[ri])
si rezolvam ecuatiile rezultate pentru a gasi contrastul si luminozitatea
*/
void cAlgorithmFractal::FindMap(range_data *rangep,domain_data * dom,affine_map * map,sNonShareData   *ns_data)
{
	int ry; /* pozitia verticala in partitie*/
	int dy = dom->y >> 1; /* pozitia verticala in domeniu*/
	unsigned long rd = 0; /* suma ns_data->range*ns_data->domain values (scalata cu  4) */
	double rd_sum; /* suma valorilor ns_data->range*ns_data->domain  (normalizata) */
	double contrast; /* contrast optimal intre partitie si domeniu*/
	double brightness; /*offset de luminozitate optima intre partitie si domeniu*/
	double qbrightness;/*luminozitate dupa cuantizare*/
	double max_scaled; /* maximum scaled value = contrast*MAX_GREY */
	int r_size = 1 << rangep->s_log; /* marimea partitiei*/
	double pixels = (double)((long)r_size*r_size); /*numarul total de pixeli*/
	for (ry = rangep->y; ry < rangep->y + r_size; ry++, dy++)
	{
		register unsigned char *r = &ns_data->range[ry][rangep->x];
		register unsigned *d = &ns_data->domain[dy][dom->x >> 1];
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


/* Citeste maparea afina pentru partitie sau imparte partitia daca compresorul a facut asta in compress_range()
*/
void cAlgorithmFractal::DecompressRange(int x,int y,int s_log,sNonShareData   *ns_data,cBitStreamSoup * frac_file)
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
		map->next = ns_data->map_head;
		ns_data->map_head = map;
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
			dom_number = frac_file->InputBits( ns_data->dom_info[s_log].pos_bits);
			map->dom_x = (int)(dom_number % ns_data->dom_info[s_log].x_domains)<< (s_log + 1 - ns_data->dom_density);
			map->dom_y = (int)(dom_number / ns_data->dom_info[s_log].x_domains)<< (s_log + 1 - ns_data->dom_density);
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
		DecompressRange(x, y, s_log-1,ns_data,frac_file);
		DecompressRange(x+r_size/2, y, s_log-1,ns_data,frac_file);
		DecompressRange(x, y+r_size/2, s_log-1,ns_data,frac_file);
		DecompressRange(x+r_size/2, y+r_size/2, s_log-1,ns_data,frac_file);
	}
}
/* ===================================================================
* generam imaginea aplicand o runda de toate maparile afine pe imagine.Metoda 
"pura" ar necesa computarea unei imagini noi separate si apoi copierea ei peste
cea originala. Insa convergenta catre imaginea finala se intampla rapid si din acest motiv
daca rescriem aceeasi imagine si cu maparile afine aplicate obtinem acelasi rezultat optim
dpdv al memoriei*/
void cAlgorithmFractal::RefineImage(sNonShareData  *ns_data)
{
	map_info *map; /*pointer la maparea afina curenta*/
	long brightness; /*offsetul de luminozitate al maparii scalat cu 65536 */
	long val; /* marime noua pentru pixel */
	int y; /* pozitie verticala in ns_data->range */
	int dom_y; /* pozitie verticala in domeniu */
	int j;
	for (map = ns_data->map_head; map != NULL; map = map->next)
	{
		/* map->brightness e scalata cu 128, trebuie scalata din nou cu  512 pentru a obtine factorul de scalare 65536:*/
		brightness = (long)map->brightness << 9;
		dom_y = map->dom_y;
		for (y = map->y; y < map->y + map->size; y++) 
		{
			/* Urmatorul loop consuma cel mai mult timp astfel ca mutam niste calcule de adresa
			in afara lui*/
			unsigned char *r = &ns_data->range[y][map->x];
			unsigned char *d = &ns_data->range[dom_y++][map->dom_x];
			unsigned char *d1 = &ns_data->range[dom_y++][map->dom_x];
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
void cAlgorithmFractal::AverageBoundaries(	sNonShareData  *ns_data)
{
	map_info *map; /* pointer la maparea curenta afina*/
	unsigned val; /* suma valorilor de pixeli pentru partitiile curente si cele vecine*/
	int x; /*pozitia orizontala in partitia curenta*/
	int y; /*pozitia verticala in partitia curenta*/
	for (map = ns_data->map_head; map != NULL; map = map->next) 
	{
		if (map->size == (1<<MIN_BITS))
			continue; /* partitie prea mica*/
		if (map->x > 1) 
		{
			/* Smooth marginea din stanga a partitie si cea din dreapta a partitiei vecine */
			for (y = map->y; y < map->y + map->size; y++) 
			{
				val = ns_data->range[y][map->x - 1] + ns_data->range[y][map->x];
				ns_data->range[y][map->x - 1] =(unsigned char)((ns_data->range[y][map->x - 2] + val)/3);
				ns_data->range[y][map->x] =(unsigned char)((val + ns_data->range[y][map->x + 1])/3);
			}
		}
		if (map->y > 1) 
		{
			/* Smooth marginea de sus a partitiei si de jos a parittiei vecine
			*/
			for (x = map->x; x < map->x + map->size; x++)
			{
				val = ns_data->range[map->y - 1][x] + ns_data->range[map->y][x];
				ns_data->range[map->y - 1][x] =(unsigned char)((ns_data->range[map->y - 2][x] + val)/3);
				ns_data->range[map->y][x] =(unsigned char)((val + ns_data->range[map->y + 1][x])/3);
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
void cAlgorithmFractal::TraverseImage(int x, int y,int  x_size,int  y_size,int process,	sNonShareData  *ns_data,cBitStreamSoup *frac_file)
{
	int s_size; /* marimea patratului s_size = 1<<s_log */
	int s_log; /* log base 2 a marimii*/
	s_log = BitLength(x_size < y_size ? (unsigned long)x_size :(unsigned long)y_size)-1;
	s_size = 1 << s_log;
	/* Cum x_size si y_size sunt >=4  s_log >= MIN_BITS */
	/* Imparte patratul recursiv daca este prea mare pentru partitie:*/
	if (s_log > MAX_BITS) 
	{
		TraverseImage(x, y, s_size/2, s_size/2,process,ns_data,frac_file);
		TraverseImage(x+s_size/2, y, s_size/2, s_size/2,process,ns_data,frac_file);
		TraverseImage(x, y+s_size/2, s_size/2, s_size/2,process,ns_data,frac_file);
		TraverseImage(x+s_size/2, y+s_size/2, s_size/2, s_size/2,process,ns_data,frac_file);
	} 
	else 
	{
		/* Comprima sau decomprima patratul ca in partitie: */
		switch (process)
		{
		case COMPRESS_RANGE:
			CompressRange(x, y, s_log,ns_data,frac_file);
			break;
		case DECOMPRESS_RANGE:
			DecompressRange(x, y, s_log,ns_data,frac_file);
			break;
		}

	}
	/* Traverseaza dreptunghiul din dreapta patratului: */
	if (x_size > s_size) 
	{
		TraverseImage(x + s_size, y, x_size - s_size, y_size,process,ns_data,frac_file);
		/* Cum x_size si s_size sunt multiplii de 4, x + s_size si
		* x_size - s_size sunt de asemena multiplii de 4.
		*/
	}
	/* Traverseaza dreptunghiul din josul patratului:*/
	if (y_size > s_size) 
	{
		TraverseImage(x, y + s_size, s_size, y_size - s_size,process,ns_data,frac_file);
	}
}
/* =================================================================
*Initializaeaza informatia din domeniu. Asta trebuie facuta in aceeasi manierea
ca in compresor si decompresor.
*/
void cAlgorithmFractal::DominfoInit(int x_size,int y_size,int density,sNonShareData  *ns_data)
{
	int s; /* marimea domeniilor 1<<(s+1) */
	for (s = MIN_BITS; s <= MAX_BITS; s++)
	{
		int y_domains; /* numarul domeniilor pe verticala*/
		int dom_size = 1<<(s+1); /* marimea domeniului */
		/* distanta intre doua domenii este marimea domeniului  1<<(s+1)
		* shiftata la dreapta cu densitatea domeniului.*/
		ns_data->dom_info[s].x_domains = ((x_size - dom_size)>>(s + 1 - density)) + 1;
		y_domains = ((y_size - dom_size)>>(s + 1 -density)) + 1;
		/* Numarul de biti necesar pentru a encoda pozitiile domeniului: */
		ns_data->dom_info[s].pos_bits = BitLength((unsigned long)ns_data->dom_info[s].x_domains * y_domains - 1);
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
		assert(0&&"insuficient memory");//("insufficient memory\n");
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
	if(array)
	{
		int row;
		for (row = 0; row < rows; row++)
		{
			free(array[row]);
		}
		array = NULL;
	}
}
