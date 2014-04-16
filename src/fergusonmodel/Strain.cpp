#include <sstream>
#include <string.h>
#include "Strain.h"
#include "Main.h"

Strain::Strain(unsigned int date, unsigned char* rna) {
   firstSeenDate = date;
   unsigned int aminoAcids = Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope;
   epitopes = new unsigned char[aminoAcids];
   this->rna = rna;
   unsigned int nucleicAcid = 0;
   for (unsigned int aminoAcid = 0; aminoAcid < aminoAcids; aminoAcid++) {
      unsigned int na1 = (rna[nucleicAcid / 4] >> ((nucleicAcid % 4) * 2)) & 0x03;
      ++nucleicAcid;
      unsigned int na2 = (rna[nucleicAcid / 4] >> ((nucleicAcid % 4) * 2)) & 0x03;
      ++nucleicAcid;
      unsigned int na3 = (rna[nucleicAcid / 4] >> ((nucleicAcid % 4) * 2)) & 0x03;
      ++nucleicAcid;
      epitopes[aminoAcid] = codonToAA[GET_CODON(na1, na2, na3)];
   }
   stats.rna = GetRNAString();
   stats.protein = GetProtinString();
   referenceCount = 0;
}

Strain::~Strain() {
   delete [] epitopes;
   delete [] rna;
}

std::string Strain::GetRNAString() {
   unsigned int nucleicAcids = Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope * 3;
   std::ostringstream buffer;
   for (unsigned int spot = 0; spot < nucleicAcids; ++spot) {
      unsigned int arrayIndex = spot / 4;
      unsigned int shift = (spot % 4) * 2;
      unsigned char na1 = (rna[arrayIndex] >> shift) & 0x03;
      switch (na1) {
         case U:
            buffer << "U";
            break;
         case C:
            buffer << "C";
            break;
         case A:
            buffer << "A";
            break;
         case G:
            buffer << "G";
            break;
      }
   }
   return buffer.str();
}

std::string Strain::GetProtinString() {
   std::ostringstream sb;
   for (int i = 0; i < Main::simulationInfo->numEpitopes; i++) {
      for (int j = 0; j < Main::simulationInfo->codonsPerEpitope; j++) {
         sb << aaIntToStr[epitopes[i * Main::simulationInfo->codonsPerEpitope + j] & 0xFF];
      }
   }
   return sb.str();
}

unsigned char* Strain::DecodeGenotype(std::string rnaString) {
   unsigned char* rna = new unsigned char[RNA_LENGTH];
   memset(rna, 0, RNA_LENGTH);
   unsigned int nucleicAcid = 0;
   for (int stringIndex = 0; stringIndex < rnaString.length(); stringIndex++) {
      char c = rnaString[stringIndex];
      unsigned char na = 0;
      switch (c) {
         case 'A':
            na = A;
            break;
         case 'U':
            na = U;
            break;
         case 'G':
            na = G;
            break;
         case 'C':
            na = C;
            break;
      }
      rna[nucleicAcid / 4] |= na << ((nucleicAcid % 4) * 2);
      ++nucleicAcid;
   }
   return rna;
}

Strain* Strain::Replicate(unsigned int date) {
   int numMutations = 0;
   switch (Main::simulationInfo->mutationAlgorithm) {
      case SimulationInfo::Mutation_Approximate:
         if (Main::RANDOM->NextDouble() < Main::simulationInfo->mutationProbability * Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope * 3) {
            numMutations = 1;
         }
         break;
      case SimulationInfo::Mutation_Exact:
         numMutations = Main::DrawFromDistribution(Main::BINOMIAL_STRAIN, Main::RANDOM->NextDouble());
         break;
      case SimulationInfo::Mutation_None:
         //Leave numMutations set to 0
         break;
      default:
         Main::Print("Invalid mutation algorithm: %d", Main::simulationInfo->mutationAlgorithm);
         return this;
   }
   if (numMutations > 0) {
      unsigned char* newRNA = new unsigned char[RNA_LENGTH];
      memcpy(newRNA, rna, RNA_LENGTH);
      std::deque<unsigned int> possibleSites;
      if (numMutations > 1) {
         for (int i = 0; i < Main::simulationInfo->numEpitopes; i++) {
            for (int j = 0; j < Main::simulationInfo->codonsPerEpitope; j++) {
               for (int k = 0; k < 3; k++) {
                  possibleSites.push_back((i << 16) | (j << 8) | k);
               }
            }
         }
         Strain::Shuffle(possibleSites, Main::RANDOM);
      }
      for (int mutationCounter = 0; mutationCounter < numMutations; mutationCounter++) {
         int epitope;
         int codon;
         int naIndex;
         if (numMutations == 1) {
            epitope = Main::RANDOM->NextInt(Main::simulationInfo->numEpitopes);
            codon = Main::RANDOM->NextInt(Main::simulationInfo->codonsPerEpitope);
            naIndex = Main::RANDOM->NextInt(3);
         } else {
            int temp = possibleSites[0];
            possibleSites.pop_front();
            epitope = (temp >> 16) & 0xFF;
            codon = (temp >> 8) & 0xFF;
            naIndex = temp & 0xFF;
         }
         int nucleicAcid = epitope * Main::simulationInfo->codonsPerEpitope * 3 + codon * 3 + naIndex;
         unsigned char oldNA = (rna[nucleicAcid / 4] >> ((nucleicAcid % 4) * 2)) & 0x03;
         unsigned char newNA = Main::RANDOM->NextInt(3);
         if (newNA >= oldNA) {
            ++newNA;
         }
         //Clear out the previous NA
         newRNA[nucleicAcid / 4] &= ~(0x03 << ((nucleicAcid % 4) * 2));
         //Write the new NA
         newRNA[nucleicAcid / 4] |= newNA << ((nucleicAcid % 4) * 2);
      }
      //Create the new Strain
      Strain* strain = new Strain(date, newRNA);
      if (strain->stats.protein.find(".") != std::string::npos) {
         //The mutation introduced a stop codon, so it's not valid
         delete strain;
         return this;
      }
      //Record lineage
      strain->stats.parentID = stats.id;
      //Increment the mutation counter
      strain->stats.mutations = stats.mutations + numMutations;
      return strain;
   } else {
      //No mutations
      return this;
   }
}

void Strain::SaveState(DataOutput* out) {
   out->WriteInt(stats.age);
   out->WriteInt(stats.id);
   out->WriteInt(stats.incidence);
   out->WriteInt(stats.infected);
   out->WriteInt(stats.mutations);
   out->WriteInt(stats.parentID);
   out->WriteInt(firstSeenDate);
   out->Write(rna, 0, RNA_LENGTH);
}

Strain* Strain::LoadState(DataInput* in) {
   unsigned int age = in->ReadInt();
   unsigned int id = in->ReadInt();
   unsigned int incidence = in->ReadInt();
   unsigned int infected = in->ReadInt();
   unsigned int mutations = in->ReadInt();
   unsigned int parentID = in->ReadInt();
   unsigned int firstSeenDate = in->ReadInt();
   unsigned char* rna = new unsigned char[RNA_LENGTH];
   in->Read(rna, 0, RNA_LENGTH);
   Strain* strain = new Strain(firstSeenDate, rna);
   strain->stats.id = id;
   strain->stats.age = age;
   strain->stats.incidence = incidence;
   strain->stats.infected = infected;
   strain->stats.mutations = mutations;
   strain->stats.parentID = parentID;
   return strain;
}

void Strain::Shuffle(std::deque<unsigned int> &sites, MersenneTwister* random) {
   for (unsigned int i = sites.size(); i > 1; i--) {
      unsigned int j = random->NextInt(i);
      unsigned int temp = sites[i - 1];
      sites[i - 1] = sites[j];
      sites[j] = temp;
   }
}

//This emulates a Java static initialization block
#ifndef _STATIC_STRAIN_H
#define	_STATIC_STRAIN_H
unsigned int Strain::codonToAA[64];
std::string Strain::aaIntToStr[21];
unsigned char Strain::staticInitialization = Strain::Initialize();
#endif	/* _STATIC_STRAIN_H */
