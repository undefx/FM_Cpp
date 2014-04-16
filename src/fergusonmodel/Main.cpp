#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include "Main.h"
#include "Strain.h"
#include "Host.h"
#include "Patch.h"
#include "World.h"
#include "../stats/SimulationInfo.h"
#include "../stats/RuntimeInfo.h"
#include "../stats/StatsWriter.h"
#include "../stats/StatsReader.h"
#include "../util/FileDataIO.h"
#include "../util/LinkedHashMap.h"
#include "../util/LinkedHashSet.h"

int main(int argc, char** argv) {
   std::string datetime = /* datetime( */ ("2013-10-03 20:24:24") /* ) */;
   int buildNumber = (int) /* increment( */ (1017L) /* ) */;
   printf("Version: %d.%d [Build #%d, %s]\n", Main::VERSION_MAJOR, Main::VERSION_MINOR, buildNumber, datetime.c_str());
   if (argc < 3) {
      printf("Usage:\n");
      printf("  fergusonmodel_native -new <base64 settings string>\n");
      printf("  fergusonmodel_native -resume <filename> <settings>\n");
      printf("  fergusonmodel_native -patch <random seed> <num hosts>\n");
      return 0;
   }
   if (strcmp(argv[1], "-new") == 0) {
      SimulationInfo* simulationInfo = SimulationInfo::ImportBase64String(std::string(argv[2]));
      delete Main::Run(simulationInfo, NULL);
      delete simulationInfo;
   } else if (strcmp(argv[1], "-resume") == 0) {
      std::string filename(argv[2]);
      StatsReader sr(filename);
      sr.Initialize();
      SimulationInfo* oldInfo = sr.simulationInfo;
      SimulationInfo* newInfo = SimulationInfo::ImportBase64String(std::string(argv[3]));
      //Make sure the new parameter set is compatible with the previous simulation
      bool valid = true;
      if (newInfo->codonsPerEpitope != oldInfo->codonsPerEpitope) {
         Main::Print("codonsPerEpitope mismatch");
         valid = false;
      }
      if (newInfo->numEpitopes != oldInfo->numEpitopes) {
         Main::Print("numEpitopes mismatch");
         valid = false;
      }
      if (newInfo->hostsPerPatch != oldInfo->hostsPerPatch) {
         Main::Print("hostsPerPatch mismatch");
         valid = false;
      }
      if (newInfo->numPatches != oldInfo->numPatches) {
         Main::Print("numPatches mismatch");
         valid = false;
      }
      if (newInfo->neighborhoodRadius != oldInfo->neighborhoodRadius) {
         Main::Print("neighborhoodRadius mismatch");
         valid = false;
      }
      sr.Close();
      if (valid) {
         //Start a new simulation where the previous one left off
         delete Main::Run(newInfo, &filename);
      }
      delete newInfo;
   } else if (strcmp(argv[1], "-patch") == 0) {
      Main::Print("PatchGenerator has not been implemented, please use the Java version.");
   } else {
      Main::Print("Unknown flag [%s]. Run with no arguments to see usage.", argv[1]);
   }
   return 0;
}

void Main::Print(const char* format, ...) {
   va_list args;
   time_t globalTime;
   struct tm* localTime;
   time(&globalTime);
   localTime = localtime(&globalTime);
   char date[128];
   char message[2048];
   sprintf(date, "%04d/%02d/%02d %02d:%02d:%02d:%03d", 1900 + localTime->tm_year, 1 + localTime->tm_mon, localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec, 0);
   va_start(args, format);
   vsprintf(message, format, args);
   va_end(args);
   printf("[%s] %s\n", date, message);
   fflush(stdout);
}

RuntimeInfo* Main::Run(SimulationInfo* simulationInfo, std::string* oldFilename) {
   //Assign the static simulation info
   Main::simulationInfo = simulationInfo;
   //Setup the environment
   Setup();
   RuntimeInfo* runtimeInfo = RuntimeInfo::GenerateRuntimeInfo();
   //Calculate infection probabilities
   InitializeProbabilities();
   //Initialize the scenario
   Print("Creating World. %d patches, %d hosts per patch", simulationInfo->numPatches, simulationInfo->hostsPerPatch);
   World world;
   //Either start a new simulation or resume a previous one
   if (oldFilename != NULL) {
      //At this point everything is initialized, and there is no infection - try to load the previous state here
      Print("Loading state...");
      StatsReader statsReader(*oldFilename);
      statsReader.Initialize();
      //Check to see if save-state data is present in the file
      if (!statsReader.simulationInfo->saveState) {
         Main::Print("Can't resume the simulation because the state wasn't saved.");
         return NULL;
      }
      //Load the saved state
      DataInput* in = statsReader.GetSaveStateInput();
      world.LoadState(in);
      RANDOM->LoadState(in);
      StrainStats::nextStrainID = in->ReadInt();
      //Reinitialize the RNG if the new seed is different than the previous seed
      if (statsReader.simulationInfo->randomSeed != simulationInfo->randomSeed) {
         delete RANDOM;
         RANDOM = new MersenneTwister(simulationInfo->randomSeed);
      }
      runtimeInfo->SetInitialState(statsReader.runtimeInfo->finalState);
      statsReader.Close();
      //Update the world here because an initial stats snapshot is saved before entering the main loop
      Print("Updating world...");
      world.Update();
      Print("Done");
   } else {
      //Make a strain
      Strain* strain = new Strain(world.GetDate(), Strain::DecodeGenotype("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
      //Infect someone
      Host* indexCase = &(world.patches[RANDOM->NextInt(simulationInfo->numPatches)]->hosts[RANDOM->NextInt(simulationInfo->hostsPerPatch)]);
      indexCase->Infect(world.GetDate(), strain);
      //Housekeeping for this infection
      ++strain->referenceCount;
      world.knownStrains.Put(strain, new LinkedHashSet<Host > ());
      world.knownStrains.Get(strain)->Add(indexCase);
      world.patches[indexCase->GetPatchIndex()]->stats.exposed++;
      world.patches[indexCase->GetPatchIndex()]->stats.naive--;
   }
   //Create and initialize the stats file
   char buffer[64];
   timeval tv;
   gettimeofday(&tv, NULL);
   sprintf(buffer, "stats-%d%03d.bin", (unsigned int) tv.tv_sec, (unsigned int)((tv.tv_usec / 1000L) % 1000L));
   statsFile = std::string(buffer);
   Print("Saving stats to file [%s]", statsFile.c_str());
   StatsWriter* stats = new StatsWriter(statsFile, simulationInfo, runtimeInfo);
   stats->Initialize();
   //Note the first day of the simulation
   int worldStartDate = world.date;
   //Save the first day before calling World.Update
   world.UpdateStats(stats, worldStartDate);
   //Run the simulation
   unsigned long lastOutputTime = 0;
   int lastWorldDate = world.date;
   while (world.date - worldStartDate < simulationInfo->numDays - 1) {
      //Sanity check
      if (simulationInfo->minCarriers > 0 && world.knownStrains.GetSize() == 0) {
         Print("The virus has gone extinct! Date: %s (%d)", world.GetDateString().c_str(), world.GetDate());
         break;
      }
      //Run this day
      world.Update();
      //Record the result
      world.UpdateStats(stats, worldStartDate);
      //Periodically print to screen
      long currentTime = ((unsigned long) time(NULL)) * 1000L;
      if (currentTime >= lastOutputTime + 10000) {
         unsigned int date = world.GetDate();
         double seconds = (currentTime - lastOutputTime) / 1000.0;
         Print("Date: %s | Days/Sec: %.1f", world.GetDateString().c_str(), (date - lastWorldDate) / (seconds == 0 ? 1 : seconds));
         lastWorldDate = date;
         lastOutputTime = currentTime;
      }
   }
   //Save the internal state of the simulator so the simulation can be resumed later
   if (simulationInfo->saveState) {
      Print("Saving state...");
      DataOutput* out = stats->GetSaveStateOutput();
      world.SaveState(out);
      RANDOM->SaveState(out);
      out->WriteInt(StrainStats::nextStrainID);
      Print("Done");
   }
   //Create a "hash" to uniquely identify this trajectory
   unsigned long random = RANDOM->NextLong();
   unsigned long date = ((long) world.GetDate()) << 32;
   unsigned long sick = world.GetTotalSick();
   unsigned long finalState = random ^ (date | sick);
   Print("Final state: %08x%08x", finalState >> 32, finalState & ((1L << 32) - 1));
   //Save the final state
   runtimeInfo->SetFinalState(finalState);
   //Close the stats file
   stats->Close();
   delete stats;
   Destroy();
   return runtimeInfo;
}

void Main::Setup() {
   //Sanity checking / parameter validation
   switch (simulationInfo->infectionAlgorithm) {
      case SimulationInfo::Infection_Approximate1:
      case SimulationInfo::Infection_Approximate2:
      case SimulationInfo::Infection_Approximate3:
      case SimulationInfo::Infection_Exact:
         break;
      default:
         Print("Invalid infection algorithm: %d", simulationInfo->infectionAlgorithm);
   }
   switch (simulationInfo->mutationAlgorithm) {
      case SimulationInfo::Mutation_Approximate:
      case SimulationInfo::Mutation_Exact:
      case SimulationInfo::Mutation_None:
         break;
      default:
         Print("Invalid mutation algorithm: %d", simulationInfo->mutationAlgorithm);
   }
   switch (simulationInfo->connectivityAlgorithm) {
      case SimulationInfo::Connectivity_Flat:
         if (simulationInfo->numPatches % 2 != 0) {
            Print("With Connectivity_Flat, numPatches must be a multiple of 2: %d", simulationInfo->numPatches);
         }
         break;
      case SimulationInfo::Connectivity_Cylindrical:
         if (simulationInfo->numPatches % 4 != 0) {
            Print("With Connectivity_Cylindrical, numPatches must be a multiple of 4: %d", simulationInfo->numPatches);
         }
         break;
      case SimulationInfo::Connectivity_Tropics:
         if (simulationInfo->numPatches % 10 != 0) {
            Print("With Connectivity_Tropics, numPatches must be a multiple of 10: %d", simulationInfo->numPatches);
         }
         break;
      default:
         Print("Invalid connectivity algorithm: %d", simulationInfo->connectivityAlgorithm);
   }
   //Make sure the neighborhood radius is set to 4 (there are hardcoded values that depend on this assumption)
   if (simulationInfo->neighborhoodRadius != 4.0) {
      Print("neighborhoodRadius must be set to 4, but it is currently set to [%.5f]", simulationInfo->neighborhoodRadius);
   }
   //Make sure the number of epitopes is set to 4 (there are hardcoded values that depend on this assumption)
   if (simulationInfo->numEpitopes != 4) {
      Print("numEpitopes must be set to 4, but it is currently set to [%d]", simulationInfo->numEpitopes);
   }
   //Make sure the number of codons per epitope is set to 3 (there are hardcoded values that depend on this assumption)
   if (simulationInfo->codonsPerEpitope != 3) {
      Print("codonsPerEpitope must be set to 3, but it is currently set to [%d]", simulationInfo->codonsPerEpitope);
   }
   //Make sure the other parameters are in their acceptable ranges
   if (simulationInfo->numPatches < 0) {
      Print("numPatches can't be negative: %d", simulationInfo->numPatches);
   }
   if (simulationInfo->hostsPerPatch < 0) {
      Print("hostsPerPatch can't be negative: %d", simulationInfo->hostsPerPatch);
   }
   if (simulationInfo->hostLifespan <= 0) {
      Print("hostLifespan must be positive: %d", simulationInfo->hostLifespan);
   }
   if (simulationInfo->tau < 0) {
      Print("tau can't be negative: %.5f", simulationInfo->tau);
   }
   if (simulationInfo->nt < 0) {
      Print("nt can't be negative: %.5f", simulationInfo->nt);
   }
   if (simulationInfo->R0_local < 0) {
      Print("R0_local can't be negative: %.5f", simulationInfo->R0_local);
   }
   if (simulationInfo->R0_patch < 0) {
      Print("R0_patch can't be negative: %.5f", simulationInfo->R0_patch);
   }
   if (simulationInfo->R0_global < 0) {
      Print("R0_global can't be negative: %.5f", simulationInfo->R0_global);
   }
   if (simulationInfo->minCarriers < 0) {
      Print("minCarriers can't be negative: %d", simulationInfo->minCarriers);
   }
   if (simulationInfo->seasonalityMultiplier < 0) {
      Print("seasonalityMultiplier can't be negative: %.5f", simulationInfo->seasonalityMultiplier);
   }
   if (simulationInfo->mutationProbability < 0) {
      Print("mutationProbability can't be negative: %.7f", simulationInfo->mutationProbability);
   }
   if (simulationInfo->omega < 0 || simulationInfo->omega > 1) {
      Print("omega must be between 0 and 1: %.5f", simulationInfo->omega);
   }
   if (simulationInfo->theta0 < 0 || simulationInfo->theta0 > 1) {
      Print("theta0 must be between 0 and 1: %.5f", simulationInfo->theta0);
   }
   if (simulationInfo->theta1 < 0 || simulationInfo->theta1 > 1) {
      Print("theta1 must be between 0 and 1: %.5f", simulationInfo->theta1);
   }
   if (simulationInfo->numDays <= 0) {
      Print("numDays must be at least 1: %d", simulationInfo->numDays);
   }
   //Randomize the seed (unless otherwise specified)
   if (simulationInfo->randomSeed == 0) {
      simulationInfo->randomSeed = (unsigned long) time(NULL) * 1000L;
   }
   //Initialize the RNG
   RANDOM = new MersenneTwister(simulationInfo->randomSeed);
   //Calculate the infection probability
   LOCAL_INFECTION_PROBABILITY = simulationInfo->R0_local / AVERAGE_NEIGHBORS / 4.0; //R0 * 1/num_neighbors * 1/infection_duration
   //Reset the Strain ID counter
   StrainStats::nextStrainID = 0;
   //Print the current set of parameters just to be thorough
   simulationInfo->Print();
}

void Main::Destroy() {
   delete RANDOM;
}

unsigned int Main::DrawFromDistribution(double* cdf, double rand) {
   for (int i = 0; i < CDF_NUM; i++) {
      if (cdf[i] >= rand) {
         return i;
      }
   }
   Print("Warning: DrawFromDistribution cutoff reached.");
   Print("d=%.7f", rand);
   Print("cdf[%d]=%.7f", CDF_NUM - 1, cdf[CDF_NUM - 1]);
   return CDF_NUM;
}

void Main::InitializeProbabilities() {
   //Import the exact binary values of PI and E from Java

   union {

      struct {
         unsigned long pi;
         unsigned long e;
      } bits;

      struct {
         double pi;
         double e;
      } values;
   } u;
   u.bits.pi = Main::PI_BITS;
   u.bits.e = Main::E_BITS;
   Main::PI = u.values.pi;
   Main::E = u.values.e;
   //Poisson probability for number of hosts to infect
   double infectionDays = 4;
   //Adjusted because the paper says "R0 of 0.02 between any two patches"
   double R0_global = simulationInfo->R0_global * (simulationInfo->numPatches - 1);
   double lambda1 = simulationInfo->R0_local / infectionDays;
   double lambda2 = simulationInfo->R0_patch / infectionDays;
   double lambda3 = R0_global / infectionDays;
   for (int day = 0; day < CDF_DAYS; day++) {
      double l3_seasonality = lambda3 * (1 + simulationInfo->seasonalityMultiplier * cos((double) day / (double) CDF_DAYS * Main::PI * 2.0));
      double l2_seasonality = lambda2 * (1 + simulationInfo->seasonalityMultiplier * cos((double) day / (double) CDF_DAYS * Main::PI * 2.0));
      double l1_seasonality = lambda1 * (1 + simulationInfo->seasonalityMultiplier * cos((double) day / (double) CDF_DAYS * Main::PI * 2.0));
      POISSON_GLOBAL[day][0] = pow(Main::E, -l3_seasonality);
      POISSON_PATCH[day][0] = pow(Main::E, -l2_seasonality);
      POISSON_LOCAL[day][0] = pow(Main::E, -l1_seasonality);
      double factorial = 1;
      for (int k = 1; k < CDF_NUM; k++) {
         factorial *= k;
         POISSON_GLOBAL[day][k] = POISSON_GLOBAL[day][k - 1] + pow(l3_seasonality, k) * pow(Main::E, -l3_seasonality) / factorial;
         POISSON_PATCH[day][k] = POISSON_PATCH[day][k - 1] + pow(l2_seasonality, k) * pow(Main::E, -l2_seasonality) / factorial;
         POISSON_LOCAL[day][k] = POISSON_LOCAL[day][k - 1] + pow(l1_seasonality, k) * pow(Main::E, -l1_seasonality) / factorial;
      }
   }
   //Binomial probability for mutations in the nucleic acid sequence of the virus
   double p = simulationInfo->mutationProbability;
   int n = (NUM_EPITOPES * CODONS_PER_EPITOPE * 3 + 1) - 1;
   for (int k = 0; k <= n; k++) {
      double previous = (k == 0 ? 0 : BINOMIAL_STRAIN[k - 1]);
      double d = Choose(n, k) * pow(p, k) * pow(1 - p, n - k);
      BINOMIAL_STRAIN[k] = (previous + d < 1) ? previous + d : 1;
   }
   //Probability of infecting people in a neighborhood, mirroring Anuroop's implementation
   //Using cosine so the peak is January 1
   //Only used by algorithms Infection_Approximate2 and Infection_Approximate3
   double beta = -log(1 - LOCAL_INFECTION_PROBABILITY);
   for (int day = 0; day < CDF_DAYS; day++) {
      INFECTION_LOCAL[day] = 1 - exp(-(1 + simulationInfo->seasonalityMultiplier * cos((double) day / (double) CDF_DAYS * Main::PI * 2.0)) * beta);
   }
}

double Main::Choose(unsigned int n, unsigned int k) {
   return Factorial(n) / (Factorial(k) * Factorial(n - k));
}

double Main::Factorial(unsigned int a) {
   double b = 1;
   while (a > 0) {
      b *= a--;
   }
   return b;
}

//This emulates a Java static initialization block
#ifndef _STATIC_MAIN_H
#define	_STATIC_MAIN_H
double Main::PI = 0;
double Main::E = 0;
MersenneTwister* Main::RANDOM = NULL;
SimulationInfo* Main::simulationInfo = NULL;
double Main::LOCAL_INFECTION_PROBABILITY = 0;
double Main::POISSON_GLOBAL[CDF_DAYS][CDF_NUM];
double Main::POISSON_PATCH[CDF_DAYS][CDF_NUM];
double Main::POISSON_LOCAL[CDF_DAYS][CDF_NUM];
double Main::BINOMIAL_STRAIN[NUM_EPITOPES * CODONS_PER_EPITOPE * 3 + 1];
double Main::INFECTION_LOCAL[CDF_DAYS];
std::string Main::statsFile;
#endif	/* _STATIC_MAIN_H */
