========================================================================
  Arhivator Project Overview
========================================================================
In principal poti sa scrii cod cum vrei doar sa mearga, insa ca sa nu modific eu
si sa imi faceti viata mai usoara.. ca sa am timp sa mai ies si in oras , 
sa mai beau o cana cu apa .. sa mai vorbesc cu mama , cu tata, sunteti niste
extraordinari daca respectati urmatoarele chestii:
(inainte sa fii rebel/rebela si sa faci cum vrei,citeste si motivul.. ): 

1. Minimizam numarul variabilelor globale.. (de fapt nu treubie sa fie niciuna)
	Motiv: -> probleme cu modificarea lor fara sa vrei.. respectiv eliberarea lor 
				din memorie
2. Vom utiliza denumiri intelgente ... (fara variabile numite v , s ,r etc...
	poate doar la parcurgeri i... in general se abordeaza urmatoarea solutie:
	variabilele care sunt intr-o clasa vor avea urmatoarea structura de nume:
	m_InitialeTipDataNUMEVARIABILA
	spre exemplu: int numarElevi va fi: int m_iNumarElevi
				  unsigned int numarElevi va fi: int m_uiNumarElevi
	(cuvintele se vor separa prin utilizarea Literelor mari, de preferat simbolurilor
	tip underline.. adica sa nu avem numar_elevi ci numarElevi)
	.. in interiorul functiilor voastre puteti sa dati cum vreti voi numele 
	(dar sa reflecte ce face variabila
	Motiv:-> in Visual e o chestie numita Intelli Sense , cand esti in .cpp si 
			nu vrei sa te intorci in .h sa vezi cum se numea o variabila 
			scrii m_ (astepti putin sau apesi CTRL+SPACE) si o sa iti afiseze
			toate variabilele din clasa respectiva...De asemenea daca ai tipul
			in numele variabilei , esti constient tot timpul de ce poti sa faci
			cu ea
3. O clasa va fi denumita cNumeClasa , iar fisierul in care se va regasi va fi NumeClasa.h, NumeClasa.cpp
	Clasa nu se va defini in .h ci in .cpp (exceptie facand functiile extra banale
	de tip get set sau constructorii foarte simplii)
	Motiv:-> timp mult mai mic de compilare si 0 erori la linkare
4. Fisierele include .. si de preferat toata sursa se va regasi in folderul Arhivator/Arhivator
	ca sa nu ne facem probleme, (separarea se va face doar din filterele din visual .. 
	adica click dreapta Add new filter .. arata ca niste directoare da nu sunt fizice)
	Motivul:->sa nu avem includerui complicate in stilul #include "..\bla\bsdrfaes12\dsewq\twer22.h"
	si sa nu mai stim cum sa ajungem de la un include la altul
5. De preferat sa fie cat mai portabil codul... desi nu tb sa ne strofocam cu asta momentan .. tinand cont
ca interfata grafica este facuta (deocamdata) cu chestii de windows..
6. Nu lasati warninguri aiurea in cod
	Motiv:-> sunt deranjante la compilare , prefer sa vad erorile.. nu warninguri inutile
7. Pana sa imi trimiteti ... as prefera daca ar functiona.. sau sa fie testat
	Motiv:-> e naspa cand nu merge.. ca trebuie reparat :) 
8. Folositi pentru fisiere fstream si nu FILE * (daca nu stiti cum , sunt exemple suficiente in codul existent)
	in general ca regula.. folositi cat mai putin c ... cat mai mult c++
	Motiv:-> cat mai putine accesari de memorie ilegale, erori mult mai inteligibile
9. Daca folositi pointeri la anumite clase .. nu tb neaparat sa includeti fisierul unde e definita clasa in .h
scrieti acolo deasupra doar class Numeclasa; si dupaia includeti in .cpp 
	Motiv:-> timpi mai rapizi de compilare
10. Ar tb sa inlocuiesc defineurile de nume cu const...dar deocamdata lasam asa
	Motiv:-> type checking.. (dezavantaj , ocupa memorie)
11. Variablele care nu sunt accesate in alte parti decat in clasa voastra faceti-le private..
	Motiv:-> Compilare mai rapida, se pastreaza integritatea logica , nu lasa omul care nu a facut codul
			sa faca tampenii (adica de exemplu eu .. fiindca mie imi place sa fac tampenii.. as face daca
			mi-ar permite codul vostru)
12. Daca aveti variabile globale... desi sper sa nu , acestea le denumiti cu g_ in fata
	Motiv:->Imi sar in ochi Putem sa le vedem pe toate cu un Find g_ , respectiv g_ (intellisense)
13. Pentru alocari folositi new/delete... NU FOLOSITI malloc/free
	->Motiv: 1.e setat pe acest proiect un memory debugger care iti spune la incheierea
				rularii memory leakurile (adica memoria care a fost alocata dar nu a fost
				eliberata). Asta nu merge calumea decat pt new/delete
			 2.Clasele nu se aloca cu malloc NEVER EVER, pt ca malloc nu stie de clase 
				(chiar daca va merge pt anumite clase asta nu inseamna nimic)
14.Bagati un enter inainte de { sau de } 
	Motiv: ca sa putem indenta codul din visual calumea cu ALT+F8
15.Indentati codul (cu alt+f8 macar) :)
	Motiv: sa poata sa inteleaga lumea ce scrieti acolo.. eu inteleg greu si daca e indentat..:))
			asa ca tb sa ma ajuti
16.Daca vedeti vreo greseala o raportati 
			
* Dar Radu .... vreau sa implementez un algoritm!!!11111111ununununu .. cum fac?
 Pai stimate programator , daca vrei sa implementezi un algoritm si sa nu rescrii
 tot ce am facut pana acum.. faci in felul urmator:
 1.Faci un .h (click dreapta add new file)in filtrul Header Files\Structure\Algoritmi "AlgorithmNumeAlgorithm.h" in care scrii(dai copy paste la):
	#ifndef M_NUMEALGORITM
	#define M_NUMEALGORITM
	#include "Algorithm.h"
	class cAlgorithmNumeAlgorithm: cAlgorithm
	{
	public:
		cAlgorithmNumeAlgorithm();
		void Compress(std::string filenameInput,std::string filenameOutput);
		void DeCompress(std::string filenameInput,std::string filenameOutput);
		virtual ~cAlgorithmNumeAlgorithm();

	};
	#endif 
 2.Faci un .cpp  (click dreapta add new file)in filtrul Source Files\Structure\Algoritmi  "AlgorithmNumeAlgorithm.cpp"  in care scrii(dai copy paste la):
	#include "stdafx.h"
	#include "AlgorithmNumeAlgorithm.h"
	#include "AlgorithmsManager.h"
	cAlgorithmNumeAlgorithm m_globalTemplate;
	cAlgorithmNumeAlgorithm::cAlgorithmNumeAlgorithm():cAlgorithm()	
	{
		m_Name="Algorithm Nume Algoritm";
	}
	void cAlgorithmNumeAlgorithm::Compress(std::string filenameInput,std::string filenameOutput)
	{
	}
	void cAlgorithmNumeAlgorithm::DeCompress(std::string filenameInput,std::string filenameOutput)
	{
	}
	cAlgorithmNumeAlgorithm::~cAlgorithmNumeAlgorithm()
	{
	}
3. Scrii codul de compresie ca un zeu si il bagi in functia compress
4. Scrii codul de decompresie ca un zeu si il bagi in functia decompress
5. Compilezi fara de eroare, si cand vei rula programul vei ramane stupefiat
ca iti apare algoritmul dupa nume in lista de algoritmi.. si funcitioneaza cu tot
cu butonul de Compress respectiv Decompress

Daca iti plac algoritmii de compresie nici nu tb sa iti bati capul cu altceva
decat cu ei :)

Implementarea de CRC , nu va fi ca un algoritm de compresie... aia va avea clasa ei
separata, deocamdata nu sunt 100% in tema cu CRC-ul..



Inainte sa fiti rebeli... ganditi-va la faptul ca am un velociraptor
                               ___......__             _
                           _.-'           ~-_       _.=a~~-_
   --=====-.-.-_----------~   .--.       _   -.__.-~ ( ___==>
                 '''--...__  (    \ \\\ { )       _.-~
                           =_ ~_  \\-~~~//~~~~-=-~
                            |-=-~_ \\   \\
                            |_/   =. )   ~}
                            |}      ||
                           //       ||
                         _//        {{
                      '='~'          \\_
                                      ~~'
                                      si nu mi-e frica sa-l folosesc
                                      (sper sa nu scriu codul singur :( )