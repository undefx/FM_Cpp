#ifndef SIMULATIONINFO_H
#define	SIMULATIONINFO_H

#include <string>
#include "../util/DataOutput.h"
#include "../util/DataInput.h"

//Simulation information (input) is stored in this class.

class SimulationInfo {
public:
   //The enumeration of all algorithm choices

   enum Algorithm {
      //Select the number of hosts to infect from a binomial, with replacement
      //This was David's first implementation
      Infection_Approximate1 = 1,
      //Select the number of hosts to infect from the expected value, with replacement
      //This is a copy of Anuroop's first implementation
      Infection_Approximate2 = 2,
      //Select the number of hosts to infect from the expected value, without replacement
      //This is a copy of Anuroop's latest implementation (and this is how FRED works)
      Infection_Approximate3 = 3,
      //Select the hosts to infect individually (no replacement because it's not sampling)
      //This is David's latest implementation
      Infection_Exact = 4,
      //Select at most one nucleic acid mutation per replication
      //This shouldn't be used because it's a loss of accuracy for very little performance gain
      Mutation_Approximate = 5,
      //Select the number of nucleic acids to mutate from a binomial, without replacement
      //This should pretty much always be used because it's more correct,
      //and performance is about the same
      Mutation_Exact = 6,
      //Never allow the virus to mutate
      Mutation_None = 7,
      //Only track incidence for each patch
      //Very fast, simply incrementing a counter whenever there is an infection
      PatchStats_Incidence = 8,
      //Track all stats (naive, exposed, infectious, recovered, incidence) for each patch
      //Can be slow, requires looping over the entire population every day
      PatchStats_All = 9,
      //Patches are split into nothern and southern hemispheres (1 row each)
      //Every patch has some nonzero contact rate with every other patch
      Connectivity_Flat = 10,
      //Patches are split into nothern and southern hemispheres (2 rows each)
      //Patches have a nonzero contact rate only with immediate neighbors (3 or 4)
      Connectivity_Cylindrical = 11,
      //Patches are split into 3 demes: north, tropics, south (4:5:1 ratio)
      //Patches have the Connectivity_Flat contact rate
      Connectivity_Tropics = 12
   };

   static const char* GetAlgorithmName(Algorithm algorithm) {
      switch (algorithm) {
         case 1: return "Infection_Approximate1";
         case 2: return "Infection_Approximate2";
         case 3: return "Infection_Approximate3";
         case 4: return "Infection_Exact";
         case 5: return "Mutation_Approximate";
         case 6: return "Mutation_Exact";
         case 7: return "Mutation_None";
         case 8: return "PatchStats_Incidence";
         case 9: return "PatchStats_All";
         case 10: return "Connectivity_Flat";
         case 11: return "Connectivity_Cylindrical";
         case 12: return "Connectivity_Tropics";
      }
      return "<Invalid Algorithm!>";
   }
   SimulationInfo();
   SimulationInfo(double theta0, double theta1, double nt, double omega, double tau, long randomSeed, double seasonalityMultiplier, double neighborhoodRadius, double mutationProbability, int numEpitopes, int codonsPerEpitope, int numPatches, int hostsPerPatch, int hostLifespan, double R0_local, double R0_patch, double R0_global, int numDays, int minCarriers, Algorithm infectionAlgorithm, Algorithm mutationAlgorithm, Algorithm patchStatsAlgorithm, Algorithm connectivityAlgorithm, bool saveState);
   ~SimulationInfo();
   void Write(DataOutput* output);
   static SimulationInfo* Read(DataInput* input, int majorVersion);
   void Print();
   std::string ExportBase64String();
   static SimulationInfo* ImportBase64String(std::string str);
   static SimulationInfo* GetDefault();
   //========== Ferguson Parameters ==========
   //Long-term, specific immunity
   double theta0;
   double theta1;
   double nt;
   //Short-term, general immunity
   double omega;
   double tau;
   //The seasonality multiplier (called "ep" in the paper)
   double seasonalityMultiplier;
   //The radius that defines a neighborood (called "R" in the paper)
   double neighborhoodRadius;
   //Probability of mutation per nucleotide (called "delta" in the paper)
   double mutationProbability;
   //Numer of epitopes on the viral protein(s) (called "A" in the paper)
   unsigned int numEpitopes;
   //Number of codons making up a single epitope (called "C" in the paper)
   unsigned int codonsPerEpitope;
   //Number of patches (called "M" in the paper)
   unsigned int numPatches;
   //Number of hosts per patch (in the paper, "N" = [num patches] * [hosts per patch])
   unsigned int hostsPerPatch;
   //How long each host lives in days (called "L" in the paper)
   unsigned int hostLifespan;
   //R0 of the virus
   double R0_local;
   double R0_patch;
   double R0_global;
   //========== My Nontrivial Parameters ==========
   //Seed of the RNG
   unsigned long randomSeed;
   //Number of days simulated
   unsigned int numDays;
   //The number of hosts to keep infected to prevent extinction
   unsigned int minCarriers;
   //The algorithm to use when choosing local infections
   Algorithm infectionAlgorithm;
   //The algorithm to use when mutating the virus
   Algorithm mutationAlgorithm;
   //The connectivity of the patches
   Algorithm connectivityAlgorithm;
   //========== My Trivial Parameters ==========
   //The algorithm to use when calculating daily patch stats
   Algorithm patchStatsAlgorithm;
   //Whether or not to save the simulator state at the end of the simulation
   bool saveState;
};

#endif	/* SIMULATIONINFO_H */
