#ifndef HOST_H
#define	HOST_H

#include "Main.h"
#include "Strain.h"
#include "../util/Hashable.h"
#include "../util/LinkedHashMap.h"

#define DEFAULT_INFECTION_DATE 0x80000000
#define IMMUNE_HISTORY_LENGTH ((NUM_EPITOPES * CODONS_PER_EPITOPE * 20 + 7) / 8)

class Host : public Hashable {
public:
   Host();
   ~Host();
   void Reset();

   inline bool IsIncubating(int date) {
      return lastInfectionDate != DEFAULT_INFECTION_DATE && (date - lastInfectionDate) < 2;
   }

   inline bool IsInfectious(unsigned int date) {
      return lastInfectionDate != DEFAULT_INFECTION_DATE && (date - lastInfectionDate) >= 2 && (date - lastInfectionDate) < 6;
   }

   inline bool IsSick(unsigned int date) {
      return lastInfectionDate != DEFAULT_INFECTION_DATE && (date - lastInfectionDate) < 6;
   }

   inline unsigned int GetPatchIndex() {
      return (id >> 24); // & (0x000000FF)
   }

   inline unsigned int GetHostIndex() {
      return id & (0x00FFFFFF);
   }

   inline unsigned int GetHash() {
      return id;
   }
   void Infect(unsigned int date, Strain* strain);
   static unsigned int* neighborList;
   unsigned int GetImmunityDistance(Strain* strain);
   void SaveState(DataOutput* out);
   void LoadState(DataInput* in, LinkedHashMap<Hashable::Integer, Strain>* strains);
   unsigned int id;
   unsigned char numNeighbors;
   unsigned int neighborIndex;
   int lastInfectionDate; //Needs to be signed for date math to work
   Strain* lastInfectionStrain;
protected:
   unsigned char immuneHistory[IMMUNE_HISTORY_LENGTH];
};

#endif	/* HOST_H */
