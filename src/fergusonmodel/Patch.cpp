#include "Patch.h"
#include "../util/FileDataIO.h"

Patch::Patch(unsigned int id, Host* refHosts) {
   //All hosts are naive
   stats.naive = Main::simulationInfo->hostsPerPatch;
   //Allocate hosts
   hosts = new Host[Main::simulationInfo->hostsPerPatch];
   //Create hosts
   if (refHosts != NULL) {
      //Reuse the existing neighborhood map
      for (unsigned int i = 0; i < Main::simulationInfo->hostsPerPatch; i++) {
         hosts[i].id = (id << 24) | i;
         hosts[i].neighborIndex = refHosts[i].neighborIndex;
         hosts[i].numNeighbors = refHosts[i].numNeighbors;
      }
      return;
   } else {
      //Create the hosts now, and create/load the neighborhood later
      for (unsigned int i = 0; i < Main::simulationInfo->hostsPerPatch; i++) {
         hosts[i].id = (id << 24) | i;
      }
   }
   //Try to read a saved patch file - the host locations and neighborhood map are stored in this file
   char filename[100];
   sprintf(filename, "patch-%d.bin", Main::simulationInfo->hostsPerPatch);
   FileDataIO* file = new FileDataIO(std::string(filename));
   Load(file);
   file->Close();
   delete file;
}

Patch::~Patch() {
   delete [] hosts;
}

void Patch::Load(DataInput* dis) {
   unsigned int numHosts = dis->ReadInt();
   //Make sure the number oh hosts in the file matched the expected number
   if (numHosts != Main::simulationInfo->hostsPerPatch) {
      Main::Print("File has %d hosts, Patch has %d hosts", numHosts, Main::simulationInfo->hostsPerPatch);
      return;
   }
   unsigned arrayLength = dis->ReadInt();
   unsigned int offset = 0;
   Host::neighborList = new unsigned int[arrayLength];
   for (unsigned int i = 0; i < Main::simulationInfo->hostsPerPatch; i++) {
      hosts[i].numNeighbors = dis->ReadByte();
      hosts[i].neighborIndex = offset;
      offset += hosts[i].numNeighbors;
   }
   for (unsigned int i = 0; i < arrayLength; i++) {
      Host::neighborList[i] = dis->ReadInt();
   }
}

void Patch::SaveState(DataOutput* out) {
   for (unsigned int i = 0; i < Main::simulationInfo->hostsPerPatch; i++) {
      hosts[i].SaveState(out);
   }
}

void Patch::LoadState(DataInput* in, LinkedHashMap<Hashable::Integer, Strain>* strains) {
   for (unsigned int i = 0; i < Main::simulationInfo->hostsPerPatch; i++) {
      hosts[i].LoadState(in, strains);
   }
}
