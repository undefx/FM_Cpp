#ifndef STRAIN_H
#define	STRAIN_H

#include <string>
#include <deque>
#include "Main.h"
#include "../stats/StrainStats.h"
#include "../util/MersenneTwister.h"
#include "../util/DataInput.h"
#include "../util/DataOutput.h"
#include "../util/Hashable.h"

#define GET_CODON(a, b, c) ((a << 4) | (b << 2) | c)
#define RNA_LENGTH ((NUM_EPITOPES * CODONS_PER_EPITOPE * 3 + 3) / 4)

class Strain : public Hashable {
public:
   Strain(unsigned int date, unsigned char* rna);
   ~Strain();
   std::string GetRNAString();
   std::string GetProtinString();
   static unsigned char* DecodeGenotype(std::string rnaString);
   Strain* Replicate(unsigned int date);
   void SaveState(DataOutput* out);

   inline unsigned int GetHash() {
      return stats.id;
   }
   static Strain* LoadState(DataInput* in);

   static inline void SafeDelete(Strain* strain) {
      if (strain != NULL && --strain->referenceCount == 0) {
         delete strain;
      }
   }
   unsigned int firstSeenDate;
   StrainStats stats;
   unsigned char* epitopes;
   unsigned int referenceCount;
protected:
   void Shuffle(std::deque<unsigned int> &sites, MersenneTwister* random);
   unsigned char* rna;
private:
   const static unsigned int A = 0;
   const static unsigned int U = 1;
   const static unsigned int G = 2;
   const static unsigned int C = 3;
   const static unsigned int ALA = 0;
   const static unsigned int ARG = 1;
   const static unsigned int ASN = 2;
   const static unsigned int ASP = 3;
   const static unsigned int CYS = 4;
   const static unsigned int GLN = 5;
   const static unsigned int GLU = 6;
   const static unsigned int GLY = 7;
   const static unsigned int HIS = 8;
   const static unsigned int ILE = 9;
   const static unsigned int LEU = 10;
   const static unsigned int LYS = 11;
   const static unsigned int MET = 12;
   const static unsigned int PHE = 13;
   const static unsigned int PRO = 14;
   const static unsigned int SER = 15;
   const static unsigned int THR = 16;
   const static unsigned int TRP = 17;
   const static unsigned int TYR = 18;
   const static unsigned int VAL = 19;
   const static unsigned int STOP = 20;
   static unsigned int codonToAA[64];
   static std::string aaIntToStr[21];
   static unsigned char staticInitialization;

   static unsigned char Initialize() {
      //Translate each codon
      codonToAA[GET_CODON(U, U, U)] = PHE;
      codonToAA[GET_CODON(U, U, C)] = PHE;
      codonToAA[GET_CODON(U, U, A)] = LEU;
      codonToAA[GET_CODON(U, U, G)] = LEU;
      codonToAA[GET_CODON(U, C, U)] = SER;
      codonToAA[GET_CODON(U, C, C)] = SER;
      codonToAA[GET_CODON(U, C, A)] = SER;
      codonToAA[GET_CODON(U, C, G)] = SER;
      codonToAA[GET_CODON(U, A, U)] = TYR;
      codonToAA[GET_CODON(U, A, C)] = TYR;
      codonToAA[GET_CODON(U, A, A)] = STOP;
      codonToAA[GET_CODON(U, A, G)] = STOP;
      codonToAA[GET_CODON(U, G, U)] = CYS;
      codonToAA[GET_CODON(U, G, C)] = CYS;
      codonToAA[GET_CODON(U, G, A)] = STOP;
      codonToAA[GET_CODON(U, G, G)] = TRP;
      codonToAA[GET_CODON(C, U, U)] = LEU;
      codonToAA[GET_CODON(C, U, C)] = LEU;
      codonToAA[GET_CODON(C, U, A)] = LEU;
      codonToAA[GET_CODON(C, U, G)] = LEU;
      codonToAA[GET_CODON(C, C, U)] = PRO;
      codonToAA[GET_CODON(C, C, C)] = PRO;
      codonToAA[GET_CODON(C, C, A)] = PRO;
      codonToAA[GET_CODON(C, C, G)] = PRO;
      codonToAA[GET_CODON(C, A, U)] = HIS;
      codonToAA[GET_CODON(C, A, C)] = HIS;
      codonToAA[GET_CODON(C, A, A)] = GLN;
      codonToAA[GET_CODON(C, A, G)] = GLN;
      codonToAA[GET_CODON(C, G, U)] = ARG;
      codonToAA[GET_CODON(C, G, C)] = ARG;
      codonToAA[GET_CODON(C, G, A)] = ARG;
      codonToAA[GET_CODON(C, G, G)] = ARG;
      codonToAA[GET_CODON(A, U, U)] = ILE;
      codonToAA[GET_CODON(A, U, C)] = ILE;
      codonToAA[GET_CODON(A, U, A)] = ILE;
      codonToAA[GET_CODON(A, U, G)] = MET;
      codonToAA[GET_CODON(A, C, U)] = THR;
      codonToAA[GET_CODON(A, C, C)] = THR;
      codonToAA[GET_CODON(A, C, A)] = THR;
      codonToAA[GET_CODON(A, C, G)] = THR;
      codonToAA[GET_CODON(A, A, U)] = ASN;
      codonToAA[GET_CODON(A, A, C)] = ASN;
      codonToAA[GET_CODON(A, A, A)] = LYS;
      codonToAA[GET_CODON(A, A, G)] = LYS;
      codonToAA[GET_CODON(A, G, U)] = SER;
      codonToAA[GET_CODON(A, G, C)] = SER;
      codonToAA[GET_CODON(A, G, A)] = ARG;
      codonToAA[GET_CODON(A, G, G)] = ARG;
      codonToAA[GET_CODON(G, U, U)] = VAL;
      codonToAA[GET_CODON(G, U, C)] = VAL;
      codonToAA[GET_CODON(G, U, A)] = VAL;
      codonToAA[GET_CODON(G, U, G)] = VAL;
      codonToAA[GET_CODON(G, C, U)] = ALA;
      codonToAA[GET_CODON(G, C, C)] = ALA;
      codonToAA[GET_CODON(G, C, A)] = ALA;
      codonToAA[GET_CODON(G, C, G)] = ALA;
      codonToAA[GET_CODON(G, A, U)] = ASP;
      codonToAA[GET_CODON(G, A, C)] = ASP;
      codonToAA[GET_CODON(G, A, A)] = GLU;
      codonToAA[GET_CODON(G, A, G)] = GLU;
      codonToAA[GET_CODON(G, G, U)] = GLY;
      codonToAA[GET_CODON(G, G, C)] = GLY;
      codonToAA[GET_CODON(G, G, A)] = GLY;
      codonToAA[GET_CODON(G, G, G)] = GLY;
      //Define a string for each amino acid
      aaIntToStr[ALA] = "A";
      aaIntToStr[ARG] = "R";
      aaIntToStr[ASN] = "N";
      aaIntToStr[ASP] = "D";
      aaIntToStr[CYS] = "C";
      aaIntToStr[GLN] = "Q";
      aaIntToStr[GLU] = "E";
      aaIntToStr[GLY] = "G";
      aaIntToStr[HIS] = "H";
      aaIntToStr[ILE] = "I";
      aaIntToStr[LEU] = "L";
      aaIntToStr[LYS] = "K";
      aaIntToStr[MET] = "M";
      aaIntToStr[PHE] = "F";
      aaIntToStr[PRO] = "P";
      aaIntToStr[SER] = "S";
      aaIntToStr[THR] = "T";
      aaIntToStr[TRP] = "W";
      aaIntToStr[TYR] = "Y";
      aaIntToStr[VAL] = "V";
      aaIntToStr[STOP] = ".";
      return 0;
   }
};

#endif	/* STRAIN_H */
