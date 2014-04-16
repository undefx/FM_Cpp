#include <stdio.h>
#include <string.h>
#include "SimulationInfo.h"
#include "../fergusonmodel/Main.h"
#include "../util/ByteArrayDataInput.h"
#include "../util/ByteArrayDataOutput.h"
#include "../util/base64.h"
#define MINIZ_HEADER_FILE_ONLY
#include "../util/miniz.c"

SimulationInfo::SimulationInfo() {
}

SimulationInfo::SimulationInfo(double theta0, double theta1, double nt, double omega, double tau, long randomSeed, double seasonalityMultiplier, double neighborhoodRadius, double mutationProbability, int numEpitopes, int codonsPerEpitope, int numPatches, int hostsPerPatch, int hostLifespan, double R0_local, double R0_patch, double R0_global, int numDays, int minCarriers, Algorithm infectionAlgorithm, Algorithm mutationAlgorithm, Algorithm patchStatsAlgorithm, Algorithm connectivityAlgorithm, bool saveState) {
   this->theta0 = theta0;
   this->theta1 = theta1;
   this->nt = nt;
   this->omega = omega;
   this->tau = tau;
   this->randomSeed = randomSeed;
   this->seasonalityMultiplier = seasonalityMultiplier;
   this->neighborhoodRadius = neighborhoodRadius;
   this->mutationProbability = mutationProbability;
   this->numEpitopes = numEpitopes;
   this->codonsPerEpitope = codonsPerEpitope;
   this->numPatches = numPatches;
   this->hostsPerPatch = hostsPerPatch;
   this->hostLifespan = hostLifespan;
   this->R0_local = R0_local;
   this->R0_patch = R0_patch;
   this->R0_global = R0_global;
   this->numDays = numDays;
   this->minCarriers = minCarriers;
   this->infectionAlgorithm = infectionAlgorithm;
   this->mutationAlgorithm = mutationAlgorithm;
   this->patchStatsAlgorithm = patchStatsAlgorithm;
   this->connectivityAlgorithm = connectivityAlgorithm;
   this->saveState = saveState;
}

SimulationInfo::~SimulationInfo() {
}

void SimulationInfo::Write(DataOutput* output) {
   output->WriteDouble(theta0);
   output->WriteDouble(theta1);
   output->WriteDouble(nt);
   output->WriteDouble(omega);
   output->WriteDouble(tau);
   output->WriteDouble(seasonalityMultiplier);
   output->WriteDouble(neighborhoodRadius);
   output->WriteDouble(mutationProbability);
   output->WriteInt(numEpitopes);
   output->WriteInt(codonsPerEpitope);
   output->WriteInt(numPatches);
   output->WriteInt(hostsPerPatch);
   output->WriteInt(hostLifespan);
   output->WriteDouble(R0_local);
   output->WriteDouble(R0_patch);
   output->WriteDouble(R0_global);
   output->WriteLong(randomSeed);
   output->WriteInt(numDays);
   output->WriteInt(minCarriers);
   output->WriteByte(infectionAlgorithm);
   output->WriteByte(mutationAlgorithm);
   output->WriteByte(patchStatsAlgorithm);
   output->WriteByte(connectivityAlgorithm);
   output->WriteBoolean(saveState);
}

SimulationInfo* SimulationInfo::Read(DataInput* input, int majorVersion) {
   if (majorVersion != Main::VERSION_MAJOR) {
      return NULL;
   }
   double theta0 = input->ReadDouble();
   double theta1 = input->ReadDouble();
   double nt = input->ReadDouble();
   double omega = input->ReadDouble();
   double tau = input->ReadDouble();
   double seasonalityMultiplier = input->ReadDouble();
   double neighborhoodRadius = input->ReadDouble();
   double mutationProbability = input->ReadDouble();
   int numEpitopes = input->ReadInt();
   int codonsPerEpitope = input->ReadInt();
   int numPatches = input->ReadInt();
   int hostsPerPatch = input->ReadInt();
   int hostLifespan = input->ReadInt();
   double R0_local = input->ReadDouble();
   double R0_patch = input->ReadDouble();
   double R0_global = input->ReadDouble();
   long randomSeed = input->ReadLong();
   int numDays = input->ReadInt();
   int minCarriers = input->ReadInt();
   SimulationInfo::Algorithm infectionAlgorithm = (SimulationInfo::Algorithm)input->ReadByte();
   SimulationInfo::Algorithm mutationAlgorithm = (SimulationInfo::Algorithm)input->ReadByte();
   SimulationInfo::Algorithm patchStatsAlgorithm = (SimulationInfo::Algorithm)input->ReadByte();
   SimulationInfo::Algorithm connectivityAlgorithm = (SimulationInfo::Algorithm)input->ReadByte();
   bool saveState = input->ReadBoolean();
   return new SimulationInfo(theta0, theta1, nt, omega, tau, randomSeed, seasonalityMultiplier, neighborhoodRadius, mutationProbability, numEpitopes, codonsPerEpitope, numPatches, hostsPerPatch, hostLifespan, R0_local, R0_patch, R0_global, numDays, minCarriers, infectionAlgorithm, mutationAlgorithm, patchStatsAlgorithm, connectivityAlgorithm, saveState);
}

void SimulationInfo::Print() {
   Main::Print("===== Simulation Parameters =====");
   Main::Print("theta0=%.5f", theta0);
   Main::Print("theta1=%.5f", theta1);
   Main::Print("nt=%d", (int) nt);
   Main::Print("omega=%.5f", omega);
   Main::Print("tau=%d", (int) tau);
   Main::Print("seasonalityMultiplier=%.5f", seasonalityMultiplier);
   Main::Print("mutationProbability=%.7f", mutationProbability);
   Main::Print("numEpitopes=%d", numEpitopes);
   Main::Print("codonsPerEpitope=%d", codonsPerEpitope);
   Main::Print("numPatches=%d", numPatches);
   Main::Print("hostsPerPatch=%d", hostsPerPatch);
   Main::Print("neighborhoodRadius=%.5f", neighborhoodRadius);
   Main::Print("hostLifespan=%d", hostLifespan);
   Main::Print("R0_local=%.5f", R0_local);
   Main::Print("R0_patch=%.5f", R0_patch);
   Main::Print("R0_global=%.5f", R0_global);
   Main::Print("randomSeed=%08x%08x", (int) (randomSeed >> 32), (int) (randomSeed & 0xFFFFFFFF));
   Main::Print("numDays=%d", numDays);
   Main::Print("minCarriers=%d", minCarriers);
   Main::Print("infectionAlgorithm=%s", GetAlgorithmName(infectionAlgorithm));
   Main::Print("mutationAlgorithm=%s", GetAlgorithmName(mutationAlgorithm));
   Main::Print("patchStatsAlgorithm=%s", GetAlgorithmName(patchStatsAlgorithm));
   Main::Print("connectivityAlgorithm=%s", GetAlgorithmName(connectivityAlgorithm));
   Main::Print("saveState=%s", saveState ? "true" : "false");
   Main::Print("=====-----------------------=====");
}

std::string SimulationInfo::ExportBase64String() {
   //Write the raw data
   ByteArrayDataOutput out1;
   Write(&out1);
   unsigned char* rawBytes = out1.GetByteArray();
   //XOR with an expected set of simulation parameters
   ByteArrayDataOutput out2;
   SimulationInfo* defaultInfo = GetDefault();
   defaultInfo->Write(&out2);
   delete defaultInfo;
   unsigned char* refBytes = out2.GetByteArray();
   for (int i = 0; i < out2.GetSize(); i++) {
      rawBytes[i] = (unsigned char) ((refBytes[i] & 0xFF) ^ (rawBytes[i] & 0xFF));
   }
   //Prepend the simulator's major version number as a data format specifier
   ByteArrayDataOutput out3;
   out3.WriteByte(Main::VERSION_MAJOR);
   out3.Write(rawBytes, 0, out1.GetSize());
   unsigned char* xorBytes = out3.GetByteArray();
   //Compress the data
   unsigned long decompressedLength = out3.GetSize();
   unsigned char compressedBytes[1024];
   unsigned long compressedLength = sizeof (compressedBytes);
   int status = mz_compress2(compressedBytes, &compressedLength, xorBytes, decompressedLength, MZ_BEST_COMPRESSION);
   delete [] rawBytes;
   delete [] refBytes;
   delete [] xorBytes;
   if (status != MZ_OK) {
      return NULL;
   }
   return base64_encode(compressedBytes, compressedLength);
}

SimulationInfo* SimulationInfo::ImportBase64String(std::string str) {
   //Decompress the data
   std::string decoded = base64_decode(str);
   if (decoded.length() == 0) {
      return NULL;
   }
   const unsigned char* compressedBytes = reinterpret_cast<const unsigned char*> (decoded.c_str());
   unsigned long compressedLength = decoded.length();
   unsigned char decompressedBytes[1024];
   unsigned long decompressedLength = sizeof (decompressedBytes);
   int status = mz_uncompress(decompressedBytes, &decompressedLength, compressedBytes, compressedLength);
   if (status != MZ_OK) {
      return NULL;
   }
   ByteArrayDataInput in1(decompressedBytes);
   //Get the data format version
   unsigned int majorVersion = in1.ReadByte() & 0xFF;
   //Undo the XOR operation (with another XOR)
   ByteArrayDataOutput out;
   SimulationInfo* defaultInfo = GetDefault();
   defaultInfo->Write(&out);
   delete defaultInfo;
   unsigned char* refBytes = out.GetByteArray();
   unsigned char* rawBytes = new unsigned char[out.GetSize()];
   in1.Read(rawBytes, 0, out.GetSize());
   for (int i = 0; i < out.GetSize(); i++) {
      rawBytes[i] = (unsigned char) ((refBytes[i] & 0xFF) ^ (rawBytes[i] & 0xFF));
   }
   //Read the raw data
   ByteArrayDataInput in2(rawBytes);
   SimulationInfo* simulationInfo = Read(&in2, majorVersion);
   delete [] refBytes;
   delete [] rawBytes;
   return simulationInfo;
}

SimulationInfo* SimulationInfo::GetDefault() {
   return new SimulationInfo(.25, .99, 2, 1, 270, 0, .25, 4, 1e-6, 4, 3, 20, 5000000, 60 * 365, 5, .4, .02, 100 * 365, 1, Infection_Exact, Mutation_Exact, PatchStats_Incidence, Connectivity_Flat, false);
}
