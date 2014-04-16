#ifndef PATCH_H
#define	PATCH_H

#include "Host.h"
#include "../stats/PatchStats.h"
#include "../stats/SimulationInfo.h"
#include "../util/DataInput.h"
#include "../util/DataOutput.h"
#include "../util/LinkedHashMap.h"

class Patch {
public:
   Patch(unsigned int id, Host* refHosts);
   ~Patch();
   void SaveState(DataOutput* out);
   void LoadState(DataInput* in, LinkedHashMap<Hashable::Integer, Strain>* strains);
   Host* hosts;
   PatchStats stats;
protected:
   void Load(DataInput* dis);
};

#endif	/* PATCH_H */
