#ifndef PATCHSTATS_H
#define	PATCHSTATS_H

class PatchStats {
public:

   PatchStats() {
      Initialize(0, 0, 0, 0, 0);
   }

   PatchStats(unsigned int naive, unsigned int exposed, unsigned int infectious, unsigned int recovered, unsigned int incidence) {
      Initialize(naive, exposed, infectious, recovered, incidence);
   }

   ~PatchStats() {
   }

   void Initialize(unsigned int naive, unsigned int exposed, unsigned int infectious, unsigned int recovered, unsigned int incidence) {
      this->naive = naive;
      this->exposed = exposed;
      this->infectious = infectious;
      this->recovered = recovered;
      this->incidence = incidence;
   }
   unsigned int naive;
   unsigned int exposed;
   unsigned int infectious;
   unsigned int recovered;
   unsigned int incidence;
};

#endif	/* PATCHSTATS_H */

