#ifndef RUNTIMEINFO_H
#define	RUNTIMEINFO_H

#include <string>
#include "../util/DataInput.h"
#include "../util/DataOutput.h"

//Runtime information (output) is stored in this class.

class RuntimeInfo {
public:
   RuntimeInfo(unsigned int simulatorVersion, unsigned int simulationDate, unsigned int simulationTimer, std::string computerName, std::string implementationName, unsigned long initialState, unsigned long finalState);
   ~RuntimeInfo();
   static RuntimeInfo* GenerateRuntimeInfo();
   void UpdateSimlationTimer();
   void SetInitialState(unsigned long initialState);
   void SetFinalState(unsigned long finalState);
   void Write(DataOutput* output);
   static RuntimeInfo* Read(DataInput* input, unsigned int majorVersion);
   //The simulator version
   unsigned int simulatorVersion;
   //The datetime the simulation was started (seconds since epoch, January 1, 1970)
   unsigned int simulationDate;
   //The amount of time the simulation took in seconds
   unsigned int simulationTimer;
   //The name of the computer (to compare performance across different machines)
   std::string computerName;
   //The name of the implementation (to compare performance across different implementation)
   std::string implementationName;
   //The RuntimeInfo.finalState field of the simulation which was resumed
   unsigned long initialState;
   //This is intended to be a (reasonably) unique "hash" of the entire simulation trajectory
   unsigned long finalState;
};

#endif	/* RUNTIMEINFO_H */

