#include <unistd.h>
#include <time.h>
#include "RuntimeInfo.h"
#include "../fergusonmodel/Main.h"

RuntimeInfo::RuntimeInfo(unsigned int simulatorVersion, unsigned int simulationDate, unsigned int simulationTimer, std::string computerName, std::string implementationName, unsigned long initialState, unsigned long finalState) {
   this->simulatorVersion = simulatorVersion;
   this->simulationDate = simulationDate;
   this->simulationTimer = simulationTimer;
   this->computerName = computerName;
   this->implementationName = implementationName;
   this->initialState = initialState;
   this->finalState = finalState;
}

RuntimeInfo::~RuntimeInfo() {
}

RuntimeInfo* RuntimeInfo::GenerateRuntimeInfo() {
   std::string implementationName("c++");
   char buffer[256];
   gethostname(buffer, 256);
   std::string computerName(buffer);
   return new RuntimeInfo(Main::VERSION, time(NULL), 0, computerName, implementationName, 0, 0);
}

void RuntimeInfo::UpdateSimlationTimer() {
   simulationTimer = (unsigned int) time(NULL) - simulationDate;
}

void RuntimeInfo::SetInitialState(unsigned long initialState) {
   this->initialState = initialState;
}

void RuntimeInfo::SetFinalState(unsigned long finalState) {
   this->finalState = finalState;
}

void RuntimeInfo::Write(DataOutput* output) {
   output->WriteInt(simulatorVersion);
   output->WriteInt(simulationDate);
   output->WriteInt(simulationTimer);
   output->WriteString(computerName);
   output->WriteString(implementationName);
   output->WriteLong(initialState);
   output->WriteLong(finalState);
}

RuntimeInfo* RuntimeInfo::Read(DataInput* input, unsigned int majorVersion) {
   unsigned int simulatorVersion = input->ReadInt();
   unsigned int simulationDate = input->ReadInt();
   unsigned int simulationTimer = input->ReadInt();
   std::string computerName = input->ReadString();
   std::string implementationName = input->ReadString();
   unsigned long initialState = input->ReadLong();
   unsigned long finalState = input->ReadLong();
   return new RuntimeInfo(simulatorVersion, simulationDate, simulationTimer, computerName, implementationName, initialState, finalState);
}
