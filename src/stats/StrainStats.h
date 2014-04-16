#ifndef STRAINSTATS_H
#define	STRAINSTATS_H

#include <string>

#define NULL_PARENT 0xFFFFFFFF

class StrainStats {
public:

   StrainStats() {
      rna = "";
      protein = "";
      id = nextStrainID++;
      parentID = NULL_PARENT;
      infected = 0;
      incidence = 0;
      mutations = 0;
      age = 0;
   }

   std::string rna;
   std::string protein;
   unsigned int id;
   unsigned int parentID;
   unsigned int infected;
   unsigned int incidence;
   unsigned int mutations;
   unsigned int age;
   static unsigned int nextStrainID;
};

#endif	/* STRAINSTATS_H */

