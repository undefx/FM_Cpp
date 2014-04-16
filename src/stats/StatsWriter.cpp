#include "StatsWriter.h"
#include "../fergusonmodel/Main.h"

StatsWriter::StatsWriter(std::string filename, SimulationInfo* simulationInfo, RuntimeInfo* runtimeInfo) {
   this->filename = filename;
   this->simulationInfo = simulationInfo;
   this->runtimeInfo = runtimeInfo;
}

StatsWriter::~StatsWriter() {
}

void StatsWriter::Initialize() {
   dayOffsetsTable = new unsigned int[simulationInfo->numDays];
   file = new FileDataIO(filename);
   file->SeekWrite(0);
   file->WriteInt(Main::VERSION);
   //Write the simulation info
   simulationInfo->Write(file);
   file->Flush();
   //Write the runtime info here to allocate space in the file. This will
   //eventually be re-written, but first we need to know the location and size
   //of the data structure.
   runtimeInfoOffset = file->GetWritePosition();
   runtimeInfo->Write(file);
   for (int i = 0; i < simulationInfo->numDays; i++) {
      dayOffsetsTable[i] = (unsigned int) file->GetWritePosition();
      file->WriteLong(~(0L));
   }
   saveStatePointerOffset = file->GetWritePosition();
   file->WriteLong(~(0L));
   file->Flush();
}

void StatsWriter::SaveDay(int day, PatchStats** &patchStats, StrainStats** &strainStats, unsigned int numStrains) {
   long filePointer = file->GetWritePosition();
   file->SeekWrite(dayOffsetsTable[day]);
   file->WriteLong(filePointer);
   file->Flush();
   file->SeekWrite(filePointer);
   for (unsigned int i = 0; i < simulationInfo->numPatches; ++i) {
      PatchStats stats = *(patchStats[i]);
      file->WriteInt(stats.naive);
      file->WriteInt(stats.exposed);
      file->WriteInt(stats.infectious);
      file->WriteInt(stats.recovered);
      file->WriteInt(stats.incidence);
   }
   file->WriteInt(numStrains);
   for (unsigned int i = 0; i < numStrains; ++i) {
      StrainStats stats = *(strainStats[i]);
      file->WriteString(stats.rna);
      file->WriteString(stats.protein);
      file->WriteInt(stats.id);
      file->WriteInt(stats.parentID);
      file->WriteInt(stats.infected);
      file->WriteInt(stats.incidence);
      file->WriteInt(stats.mutations);
      file->WriteInt(stats.age);
   }
   file->Flush();
}

DataOutput* StatsWriter::GetSaveStateOutput() {
   //Update the save state file offset in the header
   unsigned long filePointer = file->GetWritePosition();
   file->SeekWrite(saveStatePointerOffset);
   file->WriteLong(filePointer);
   file->SeekWrite(filePointer);
   //Return the output stream so the other classes can save their states
   return file;
}

void StatsWriter::Close() {
   //Seek to the runtime info file location, update the simulation timer, and
   //(re)write the runtime info data structure
   file->SeekWrite(runtimeInfoOffset);
   runtimeInfo->UpdateSimlationTimer();
   runtimeInfo->Write(file);
   file->Flush();
   //All done, close the file
   file->Close();
   delete file;
   delete [] dayOffsetsTable;
}
