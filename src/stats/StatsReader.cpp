#include "StatsReader.h"
#include "../fergusonmodel/Main.h"

StatsReader::StatsReader(std::string filename) {
   this->filename = filename;
}

StatsReader::~StatsReader() {
}

void StatsReader::Initialize() {
   file = new FileDataIO(filename);
   file->SeekRead(0);
   unsigned int version = file->ReadInt();
   unsigned int majorVersion = (version >> 16) & 0xFFFF;
   //unsigned int minorVersion = version & 0xFFFF;
//   if (majorVersion != Main::VERSION_MAJOR) {
//      //This is an old file, but it may still be readable (backwards compatibility)
//      if (majorVersion >= 1 && majorVersion < Main::VERSION_MAJOR) {
//         //warning
//      } else {
//         //error
//      }
//   }
   simulationInfo = SimulationInfo::Read(file, majorVersion);
   runtimeInfo = RuntimeInfo::Read(file, majorVersion);
   dayOffsets = new unsigned long[simulationInfo->numDays];
   for (int i = 0; i < simulationInfo->numDays; i++) {
      dayOffsets[i] = file->ReadLong();
      //Check to see if the offset is defined. It's ok if it's not because it
      //is nice to be able to analyze the stats file while the simulation is
      //still running. If this happens on a completed stats file, then
      //something is terribly wrong.
      if (dayOffsets[i] == ~(0L)) {
         //Assume that the data is truncated somewhere in the middle of the previous day
         simulationInfo->numDays = i - 1;
         Main::Print("Warning: The stats file is corrupted starting at (or before) day " + i);
         break;
      }
   }
   saveStateOffset = file->ReadLong();
}

void StatsReader::ReadDay(int day, PatchStats* &patchStats, StrainStats* &strainStats, unsigned int &numStrains) {
   file->SeekRead(dayOffsets[day]);
   patchStats = new PatchStats[simulationInfo->numPatches];
   for (int i = 0; i < simulationInfo->numPatches; i++) {
      unsigned int naive = file->ReadInt();
      unsigned int exposed = file->ReadInt();
      unsigned int infectious = file->ReadInt();
      unsigned int recovered = file->ReadInt();
      unsigned int incidence = file->ReadInt();
      patchStats[i].Initialize(naive, exposed, infectious, recovered, incidence);
   }
   numStrains = file->ReadInt();
   strainStats = new StrainStats[numStrains];
   for (int i = 0; i < numStrains; i++) {
      strainStats[i].rna = file->ReadString();
      strainStats[i].protein = file->ReadString();
      strainStats[i].id = file->ReadInt();
      strainStats[i].parentID = file->ReadInt();
      strainStats[i].infected = file->ReadInt();
      strainStats[i].incidence = file->ReadInt();
      strainStats[i].mutations = file->ReadInt();
      strainStats[i].age = file->ReadInt();
   }
}

DataInput* StatsReader::GetSaveStateInput() {
   //Update the save state file offset in the header
   file->SeekRead(saveStateOffset);
   //Return the output stream so the other classes can save their states
   return file;
}

void StatsReader::Close() {
   file->Close();
   delete file;
   delete [] dayOffsets;
   if (simulationInfo != NULL) {
      delete simulationInfo;
   }
   if (runtimeInfo != NULL) {
      delete runtimeInfo;
   }
}
