#ifndef MAIN_H
#define	MAIN_H

#include <cstdarg>
#include <string>
#include "../util/MersenneTwister.h"
#include "../stats/SimulationInfo.h"
#include "../stats/RuntimeInfo.h"

#define CDF_DAYS 365
#define CDF_NUM 20
#define NUM_EPITOPES 4
#define CODONS_PER_EPITOPE 3

class Main {
public:
   static const unsigned int VERSION_MAJOR = 1;
   static const unsigned int VERSION_MINOR = 1;
   static const unsigned int VERSION = ((VERSION_MAJOR << 16) | VERSION_MINOR);
   static void Print(const char* format, ...);
   static unsigned int DrawFromDistribution(double* cdf, double rand);
   static RuntimeInfo* Run(SimulationInfo* simulationInfo, std::string* oldFilename);
   static void Setup();
   static void Destroy();
   static void InitializeProbabilities();
   //The random number generator
   static MersenneTwister* RANDOM;
   //Simulation parameters are loaded at runtime
   static SimulationInfo* simulationInfo;
   //Intrinsic parameters
   const static double AVERAGE_NEIGHBORS = 12.554;
   static double LOCAL_INFECTION_PROBABILITY;
   //Precalculated tables
   static double POISSON_GLOBAL[CDF_DAYS][CDF_NUM];
   static double POISSON_PATCH[CDF_DAYS][CDF_NUM];
   static double POISSON_LOCAL[CDF_DAYS][CDF_NUM];
   static double BINOMIAL_STRAIN[NUM_EPITOPES * CODONS_PER_EPITOPE * 3 + 1];
   static double INFECTION_LOCAL[CDF_DAYS];
   //Record keeping
   static std::string statsFile;
   static double PI;
   static double E;
private:
   //These are the exact IEEE 754 floating-point "double format" bit layouts for Java's Math.PI and Math.E
   const static long PI_BITS = 0x400921fb54442d18L;
   const static long E_BITS = 0x4005bf0a8b145769L;
   static double Choose(unsigned int n, unsigned int k);
   static double Factorial(unsigned int a);
};

#endif	/* MAIN_H */
