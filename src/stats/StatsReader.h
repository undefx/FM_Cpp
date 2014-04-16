#ifndef STATSREADER_H
#define	STATSREADER_H

#include <string>
#include "../util/FileDataIO.h"
#include "PatchStats.h"
#include "StrainStats.h"
#include "SimulationInfo.h"
#include "RuntimeInfo.h"

class StatsReader {
public:
   StatsReader(std::string filename);
   ~StatsReader();
   void Initialize();
   void ReadDay(int day, PatchStats* &patchStats, StrainStats* &strainStats, unsigned int &numStrains);
   DataInput* GetSaveStateInput();
   void Close();
   SimulationInfo* simulationInfo;
   RuntimeInfo* runtimeInfo;
protected:
   std::string filename;
   FileDataIO* file;
   unsigned long* dayOffsets;
   unsigned long saveStateOffset;
   DataInput* input;
};

#endif	/* STATSREADER_H */

