#ifndef STATSWRITER_H
#define	STATSWRITER_H

#include <string>
#include "PatchStats.h"
#include "StrainStats.h"
#include "SimulationInfo.h"
#include "RuntimeInfo.h"
#include "../util/DataOutput.h"
#include "../util/FileDataIO.h"

class StatsWriter {
public:
   StatsWriter(std::string filename, SimulationInfo* simulationInfo, RuntimeInfo* runtimeInfo);
   ~StatsWriter();
   void Initialize();
   void SaveDay(int day, PatchStats** &patchStats, StrainStats** &strainStats, unsigned int numStrains);
   DataOutput* GetSaveStateOutput();
   void Close();
protected:
   std::string filename;
   FileDataIO* file;
   unsigned int* dayOffsetsTable;
   SimulationInfo* simulationInfo;
   RuntimeInfo* runtimeInfo;
   unsigned long runtimeInfoOffset;
   unsigned long saveStatePointerOffset;
};

#endif	/* STATSWRITER_H */
