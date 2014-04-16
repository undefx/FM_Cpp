#ifndef WORLD_H
#define	WORLD_H

#include <string>
#include "Host.h"
#include "Strain.h"
#include "Patch.h"
#include "../stats/StatsWriter.h"
#include "../util/LinkedHashMap.h"
#include "../util/LinkedHashSet.h"
#include "../util/DataInput.h"
#include "../util/DataOutput.h"

class World {
public:
   World();
   ~World();
   std::string GetDateString();
   unsigned int GetDate();
   void Update();
   void UpdateStats(StatsWriter* statsWriter, int pauseDate);
   unsigned long GetTotalSick();
   void SaveState(DataOutput* out);
   void LoadState(DataInput* in);
   Patch** patches;
   LinkedHashMap<Strain, LinkedHashSet<Host> > knownStrains;
   int date; //Needs to be signed for date math to work
protected:
   static double GetCrossProtection(double d);
   double GetInfectionProbability(Host* host, Strain* strain);
};

#endif	/* WORLD_H */
