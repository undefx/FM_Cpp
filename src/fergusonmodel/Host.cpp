#include <string.h>
#include "Host.h"
#include "Main.h"

Host::Host() {
   numNeighbors = 0;
   lastInfectionDate = DEFAULT_INFECTION_DATE;
   lastInfectionStrain = NULL;
   Reset();
}

Host::~Host() {
   if (neighborList != NULL) {
      delete [] neighborList;
      neighborList = NULL;
   }
   Strain::SafeDelete(lastInfectionStrain);
}

void Host::Reset() {
   Strain::SafeDelete(lastInfectionStrain);
   lastInfectionStrain = NULL;
   lastInfectionDate = DEFAULT_INFECTION_DATE;
   memset(immuneHistory, 0, IMMUNE_HISTORY_LENGTH);
}

void Host::Infect(unsigned int date, Strain* strain) {
   Strain::SafeDelete(lastInfectionStrain);
   lastInfectionStrain = strain;
   ++lastInfectionStrain->referenceCount;
   lastInfectionDate = date;
   //Update the immune history
   unsigned int aminoAcids = Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope;
   for (unsigned int i = 0; i < aminoAcids; i++) {
      unsigned int bitIndex = i * 20 + strain->epitopes[i];
      immuneHistory[bitIndex / 8] |= (1 << (bitIndex % 8));
   }
}

unsigned int Host::GetImmunityDistance(Strain* strain) {
   unsigned int sum = 0;
   unsigned int aminoAcids = Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope;
   for (unsigned int i = 0; i < aminoAcids; i++) {
      unsigned int bitIndex = i * 20 + strain->epitopes[i];
      if ((immuneHistory[bitIndex / 8] & (1 << (bitIndex % 8))) == 0) {
         ++sum;
      }
   }
   return sum;
}

void Host::SaveState(DataOutput* out) {
   out->WriteInt(lastInfectionDate);
   out->WriteBoolean(lastInfectionStrain != NULL);
   if (lastInfectionStrain != NULL) {
      out->WriteInt(lastInfectionStrain->stats.id);
   }
   out->Write(immuneHistory, 0, IMMUNE_HISTORY_LENGTH);
}

void Host::LoadState(DataInput* in, LinkedHashMap<Hashable::Integer, Strain >* strains) {
   lastInfectionDate = in->ReadInt();
   if (in->ReadBoolean()) {
      Hashable::Integer* strainID = new Hashable::Integer(in->ReadInt());
      lastInfectionStrain = strains->Get(strainID);
      ++lastInfectionStrain->referenceCount;
      delete strainID;
   }
   in->Read(immuneHistory, 0, IMMUNE_HISTORY_LENGTH);
}

//This emulates a Java static initialization block
#ifndef _STATIC_HOST_H
#define	_STATIC_HOST_H
unsigned int* Host::neighborList = NULL;
#endif	/* _STATIC_HOST_H */
