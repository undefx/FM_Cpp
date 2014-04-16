#include <vector>
#include "math.h"
#include "World.h"

World::World() {
   //Initial variable values
   date = 0;
   patches = new Patch*[Main::simulationInfo->numPatches];
   //Initialize the patches
   for (unsigned int i = 0; i < Main::simulationInfo->numPatches; i++) {
      if (i == 0) {
         //Allocate and load a new neighborhood map
         patches[i] = new Patch(i, NULL);
      } else {
         //Resuse the existing neighborhood map
         patches[i] = new Patch(i, patches[0]->hosts);
      }
   }
}

World::~World() {
   for (unsigned int i = 0; i < Main::simulationInfo->numPatches; i++) {
      delete patches[i];
   }
   delete [] patches;
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      Strain::SafeDelete(element->key);
      delete element->value;
   }
}

unsigned int World::GetDate() {
   return date;
}

std::string World::GetDateString() {
   char buffer[32];
   sprintf(buffer, "%04d-%02d-%02d", date / 365, (date % 365) / 31 + 1, (date % 365 % 31) + 1);
   return std::string(buffer);
}

double World::GetCrossProtection(double d) {
   if (d >= Main::simulationInfo->nt) {
      return Main::simulationInfo->theta1 + (Main::simulationInfo->theta0 - Main::simulationInfo->theta1) * (d - Main::simulationInfo->nt) / (Main::simulationInfo->numEpitopes * Main::simulationInfo->codonsPerEpitope - Main::simulationInfo->nt);
   } else if (d > 0) {
      return Main::simulationInfo->theta1;
   } else {
      return 1;
   }
}

double World::GetInfectionProbability(Host* host, Strain* strain) {
   double a = 1;
   double b = 1;
   if (host->lastInfectionDate != DEFAULT_INFECTION_DATE) {
      a = 1 - Main::simulationInfo->omega * exp(-((date - host->lastInfectionDate) / Main::simulationInfo->tau));
      b = 1 - GetCrossProtection(host->GetImmunityDistance(strain));
   }
   return a * b;
}

void World::UpdateStats(StatsWriter* statsWriter, int pauseDate) {
   PatchStats** patchStats = new PatchStats*[Main::simulationInfo->numPatches];
   for (int i = 0; i < Main::simulationInfo->numPatches; i++) {
      patchStats[i] = &(patches[i]->stats);
   }
   StrainStats** strainStats = new StrainStats*[knownStrains.GetSize()];
   int index = 0;
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      strainStats[index++] = &(element->key->stats);
   }
   statsWriter->SaveDay(date - pauseDate, patchStats, strainStats, knownStrains.GetSize());
   delete [] patchStats;
   delete [] strainStats;
//   Main::Print("%d-%d %08x", date, pauseDate, Main::RANDOM->NextInt());
}

unsigned long World::GetTotalSick() {
   unsigned long count = 0;
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      count += knownStrains.Get(element->key)->GetSize();
   }
   return count;
}

void World::Update() {
   ++date;
   //Happy Birthday
   unsigned long totalSick = GetTotalSick();
   for (int i = (date - 1) % Main::simulationInfo->hostLifespan; i < Main::simulationInfo->hostsPerPatch; i += Main::simulationInfo->hostLifespan) {
      for (unsigned int j = 0; j < Main::simulationInfo->numPatches; j++) {
         Patch* patch = patches[j];
         Host* host = &(patch->hosts[i]);
         //See if this host was sick yesterday (the sick count is up-to-date as of the end of yesterday)
         bool wasSick = host->IsSick(date - 1);
         //Don't let all sick hosts be reset
         if (!wasSick || totalSick > Main::simulationInfo->minCarriers) {
            //Pointer cleanup
            if (host->lastInfectionStrain != NULL && knownStrains.Contains(host->lastInfectionStrain)) {
               knownStrains.Get(host->lastInfectionStrain)->Remove(host);
            }
            //Update the sick count
            if (wasSick) {
               --totalSick;
            }
            host->Reset();
         }
      }
   }
   //Reset daily patch stats
   for (unsigned int j = 0; j < Main::simulationInfo->numPatches; j++) {
      patches[j]->stats.incidence = 0;
   }
   //Reset daily strain stats
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      element->key->stats.incidence = 0;
   }
   //Mutate the virus in each infected host for this day
   LinkedHashMap<Strain, LinkedHashSet<Host> > newStrains;
   std::list<Host*> oldInfections;
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      Strain* strain = element->key;
      oldInfections.clear();
      for (LinkedHashSet<Host>::MapElement* element2 = knownStrains.Get(strain)->GetRoot(); element2 != NULL; element2 = element2->next) {
         Host* infectedHost = (Host*) element2->key;
         //Replicate the virus
         Strain* newStrain = infectedHost->lastInfectionStrain->Replicate(date);
         if (newStrain != infectedHost->lastInfectionStrain) {
            //The virus has mutated
            oldInfections.push_front(infectedHost);
            infectedHost->Infect(infectedHost->lastInfectionDate, newStrain);
            if (!newStrains.Contains(newStrain)) {
               newStrains.Put(newStrain, new LinkedHashSet<Host > ());
            }
            newStrains.Get(newStrain)->Add(infectedHost);
         }
      }
      knownStrains.Get(strain)->RemoveAll(oldInfections);
   }
   //Merge new strains with existing strains
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = newStrains.GetRoot(); element != NULL; element = element->next) {
      ++element->key->referenceCount;
      knownStrains.Put(element->key, newStrains.Get(element->key));
   }
   //Infect
   std::list<Host*> lastRecoveredHosts;
   std::list<Strain*> extinctStrains;
   std::list<Host*> newInfections;
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      Strain* strain = element->key;
      newInfections.clear();
      oldInfections.clear();
      for (LinkedHashSet<Host>::MapElement* element2 = knownStrains.Get(strain)->GetRoot(); element2 != NULL; element2 = element2->next) {
         Host* infectedHost = (Host*) element2->key;
         if (!infectedHost->IsInfectious(date)) {
            if (!infectedHost->IsIncubating(date)) {
               oldInfections.push_front(infectedHost);
            }
            continue;
         }
         //Calculate how many people this person will infect today
         std::list<Host*> potentialInfections;
         //Find out where this host lives, for seasonality purposes
         int deme = 0;
         switch (Main::simulationInfo->connectivityAlgorithm) {
            case SimulationInfo::Connectivity_Flat:
            case SimulationInfo::Connectivity_Cylindrical:
               if (infectedHost->GetPatchIndex() >= Main::simulationInfo->numPatches / 2) {
                  //Southern hemisphere
                  deme = 2;
               } else {
                  //Northern hemisphere
                  deme = 0;
               }
               break;
            case SimulationInfo::Connectivity_Tropics:
               if (infectedHost->GetPatchIndex() < Main::simulationInfo->numPatches * 4 / 10) {
                  //North
                  deme = 0;
               } else if (infectedHost->GetPatchIndex() < Main::simulationInfo->numPatches * 9 / 10) {
                  //Tropics
                  deme = 1;
               } else {
                  //South
                  deme = 2;
               }
               break;
            default:
            Main::Print("Invalid connectivity algorithm: %d", Main::simulationInfo->connectivityAlgorithm);
         }
         //Get the day of the year for seasonality
         int dayOfYear;
         if (deme == 0) {
            //Standard seasonality for the north
            dayOfYear = date % 365;
         } else if (deme == 1) {
            //There is no seasonality in the tropics (as if it were always April 1, using cosine)
            dayOfYear = 365 / 4;
         } else {
            //Opposite seasonality for the south
            dayOfYear = (date + 365 / 2) % 365;
         }
         //Exposures between patches - drawing from a poisson and sampling with replacement
         unsigned int numGlobal = Main::DrawFromDistribution(Main::POISSON_GLOBAL[dayOfYear], Main::RANDOM->NextDouble());
         int patchesPerRow = Main::simulationInfo->numPatches / 4;
         for (unsigned int i = 0; i < numGlobal; i++) {
            unsigned int patchID = Main::RANDOM->NextInt(Main::simulationInfo->numPatches - 1);
            if (patchID >= infectedHost->GetPatchIndex()) {
               ++patchID;
            }
            bool allowed = false;
            switch (Main::simulationInfo->connectivityAlgorithm) {
               case SimulationInfo::Connectivity_Flat:
               case SimulationInfo::Connectivity_Tropics:
                  //Allow the infection to spread to any other patch
                  allowed = true;
                  break;
               case SimulationInfo::Connectivity_Cylindrical:
               {
                  //Only allow the infection to spread to neighboring patches
                  int row1 = infectedHost->GetPatchIndex() / patchesPerRow;
                  int col1 = infectedHost->GetPatchIndex() % patchesPerRow;
                  int row2 = patchID / patchesPerRow;
                  int col2 = patchID % patchesPerRow;
                  //See if the patches are neighbors
                  if (col1 == col2 && abs(row1 - row2) == 1) {
                     allowed = true;
                  } else if (row1 == row2) {
                     int delta = (patchesPerRow + (col1 - col2)) % patchesPerRow;
                     if (delta == 1 || delta == patchesPerRow - 1) {
                        allowed = true;
                     }
                  }
                  break;
               }
               default:
                  Main::Print("Invalid connectivity algorithm: %d", Main::simulationInfo->connectivityAlgorithm);
                  return;
            }
            if (allowed) {
               unsigned int hostID = Main::RANDOM->NextInt(Main::simulationInfo->hostsPerPatch);
               potentialInfections.push_back(&(patches[patchID]->hosts[hostID]));
            }
         }
         //Exposures within this patch - drawing from a poisson and sampling with replacement
         int numPatch = Main::DrawFromDistribution(Main::POISSON_PATCH[dayOfYear], Main::RANDOM->NextDouble());
         for (unsigned int i = 0; i < numPatch; i++) {
            unsigned int patchID = infectedHost->GetPatchIndex();
            unsigned int hostID = Main::RANDOM->NextInt(Main::simulationInfo->hostsPerPatch);
            potentialInfections.push_back(&(patches[patchID]->hosts[hostID]));
         }
         //Exposures in the local neighborhood - algorithm is determined by Main::INFECTION_ALGORITHM
         if (infectedHost->numNeighbors > 0) {
            unsigned int numLocal = 0;
            //First, calculate the number of neighbors that should be exposed
            switch (Main::simulationInfo->infectionAlgorithm) {
               case SimulationInfo::Infection_Exact:
                  //Nothing to do here
                  break;
               case SimulationInfo::Infection_Approximate1:
                  //As an optimization, just draw the number of neighbors to expose from a poisson
                  numLocal = Main::DrawFromDistribution(Main::POISSON_LOCAL[dayOfYear], Main::RANDOM->NextDouble());
                  break;
               case SimulationInfo::Infection_Approximate2:
               case SimulationInfo::Infection_Approximate3:
               {
                  //This is a copy of Anuroop's implementation
                  double temp = Main::INFECTION_LOCAL[dayOfYear] * infectedHost->numNeighbors;
                  if (Main::RANDOM->NextDouble() < temp - (unsigned int) temp) {
                     numLocal = (unsigned int) temp + 1;
                  } else {
                     numLocal = (unsigned int) temp;
                  }
                  break;
               }
               default:
                  Main::Print("Invalid infection algorithm: %d", Main::simulationInfo->infectionAlgorithm);
                  return;
            }
            //Next, sample the neighbors to expose
            SimulationInfo::Algorithm infectionAlgorithm = Main::simulationInfo->infectionAlgorithm;
            if (Main::simulationInfo->infectionAlgorithm == SimulationInfo::Infection_Approximate3 && numLocal <= 1) {
               //With just 1 (or 0) neighbor to expose, revert to sampling with replacement
               //It's more efficient, and the results are the exact same
               infectionAlgorithm = SimulationInfo::Infection_Approximate2;
            }
            switch (infectionAlgorithm) {
               case SimulationInfo::Infection_Exact:
               {
                  //Query each neighbor to see if they should be exposed
                  double probability = Main::LOCAL_INFECTION_PROBABILITY;
                  if (deme != 1) {
                     //Take seasonality into account if this host isn't in the tropics
                     //Using cosine so the peak is January 1
                     probability *= (1 + Main::simulationInfo->seasonalityMultiplier * cos((double) dayOfYear / 365.0 * Main::PI * 2.0));
                  }
                  //Try to expose all the neighbors
                  for (unsigned int i = 0; i < infectedHost->numNeighbors; i++) {
                     if (Main::RANDOM->NextDouble() < probability) {
                        unsigned int neighborID = Host::neighborList[infectedHost->neighborIndex + i];
                        potentialInfections.push_back(&(patches[infectedHost->GetPatchIndex()]->hosts[neighborID]));
                     }
                  }
                  break;
               }
               case SimulationInfo::Infection_Approximate1:
               case SimulationInfo::Infection_Approximate2:
               {
                  //Sampling with replacement (fast, but can sample the same neighbor many times)
                  for (unsigned int i = 0; i < numLocal; i++) {
                     unsigned int neighborID = Host::neighborList[infectedHost->neighborIndex + Main::RANDOM->NextInt(infectedHost->numNeighbors)];
                     potentialInfections.push_back(&(patches[infectedHost->GetPatchIndex()]->hosts[neighborID]));
                  }
                  break;
               }
               case SimulationInfo::Infection_Approximate3:
               {
                  //Sampling without replacement (using a copy of this host's neighbor list)
                  std::list<int> neighborIDs;
                  for (unsigned int i = 0; i < infectedHost->numNeighbors; i++) {
                     unsigned int neighborID = Host::neighborList[infectedHost->neighborIndex + i];
                     neighborIDs.push_back(neighborID);
                  }
                  for (int i = 0; i < numLocal; i++) {
                     unsigned int index = Main::RANDOM->NextInt(neighborIDs.size());
                     std::list<int>::iterator position = neighborIDs.begin();
                     for (int j = 0; j < index; j++) {
                        ++position;
                     }
                     potentialInfections.push_back(&(patches[infectedHost->GetPatchIndex()]->hosts[*position]));
                     neighborIDs.erase(position);
                  }
                  break;
               }
               default:
                  Main::Print("Invalid infection algorithm: %d", Main::simulationInfo->infectionAlgorithm);
                  return;
            }
         }
         //Attempt to infect everyone who was exposed above
         for (std::list<Host*>::const_iterator iter = potentialInfections.begin(), end = potentialInfections.end(); iter != end; ++iter) {
            Host* host = *iter;
            //There is a chance of immunity
            double infectionProbability = GetInfectionProbability(host, infectedHost->lastInfectionStrain);
            double d = Main::RANDOM->NextDouble();
            if (d < infectionProbability) {
               //Infected
               if (host->lastInfectionStrain != NULL && host->lastInfectionStrain != infectedHost->lastInfectionStrain) {
                  if (knownStrains.Contains(host->lastInfectionStrain)) {
                     knownStrains.Get(host->lastInfectionStrain)->Remove(host);
                  }
               }
               host->Infect(date, infectedHost->lastInfectionStrain);
               newInfections.push_front(host);
               //Update incidence statistics for patches and strains
               patches[host->GetPatchIndex()]->stats.incidence++;
               infectedHost->lastInfectionStrain->stats.incidence++;
            } else {
               //Only exposed
               if (!host->IsIncubating(date) && !host->IsInfectious(date)) {
                  if (host->lastInfectionDate != DEFAULT_INFECTION_DATE) {
                     //Boost pre-existing immune responses
                     host->lastInfectionDate = (host->lastInfectionDate > date - 6) ? host->lastInfectionDate : date - 6;
                  }
               }
            }
         }
      }
      knownStrains.Get(strain)->AddAll(newInfections);
      knownStrains.Get(strain)->RemoveAll(oldInfections);
      for (std::list<Host*>::const_iterator iter = oldInfections.begin(), end = oldInfections.end(); iter != end; ++iter) {
         lastRecoveredHosts.push_front(*iter);
      }
      if (knownStrains.Get(strain)->GetSize() == 0) {
         extinctStrains.push_front(strain);
      }
   }
   for (std::list<Strain*>::const_iterator iter = extinctStrains.begin(), end = extinctStrains.end(); iter != end; ++iter) {
      Strain* extinctStrain = *iter;
      LinkedHashSet<Host>* hostSet = knownStrains.Get(extinctStrain);
      knownStrains.Remove(extinctStrain);
      delete hostSet;
      Strain::SafeDelete(extinctStrain);
   }
   totalSick = GetTotalSick();
   if (totalSick < Main::simulationInfo->minCarriers) {
      //Roni - "Now introduce the following modification:  if the person about to recover will leave no one [in] state I (infectious) or state E (exposed), don't let them recover that day."
      //Me - Keeping a minimum viral reservoir of SimulationInfo.minCarriers
      std::vector<Host*> lastRecoveredHostsList;
      for (std::list<Host*>::const_iterator iter = lastRecoveredHosts.begin(), end = lastRecoveredHosts.end(); iter != end; ++iter) {
         lastRecoveredHostsList.push_back(*iter);
      }
      int index = 0;
      while (index < lastRecoveredHostsList.size() && totalSick < Main::simulationInfo->minCarriers) {
         ++totalSick;
         Host* host = lastRecoveredHostsList[index++];
         host->lastInfectionDate = date;
         if (!knownStrains.Contains(host->lastInfectionStrain)) {
            ++host->lastInfectionStrain->referenceCount;
            knownStrains.Put(host->lastInfectionStrain, new LinkedHashSet<Host > ());
         }
         knownStrains.Get(host->lastInfectionStrain)->Add(host);
      }
   }
   //Maybe Update Patch Stats
   if (Main::simulationInfo->patchStatsAlgorithm == SimulationInfo::PatchStats_All) {
      for (unsigned int i = 0; i < Main::simulationInfo->numPatches; i++) {
         Patch* patch = patches[i];
         patch->stats.naive = 0;
         patch->stats.exposed = 0;
         patch->stats.infectious = 0;
         patch->stats.recovered = 0;
         for (unsigned int j = 0; j < Main::simulationInfo->hostsPerPatch; j++) {
            Host* host = &(patch->hosts[j]);
            if (host->lastInfectionDate == DEFAULT_INFECTION_DATE) {
               patch->stats.naive++;
            } else if ((date - host->lastInfectionDate) < 2) {
               patch->stats.exposed++;
            } else if ((date - host->lastInfectionDate) < 6) {
               patch->stats.infectious++;
            } else {
               patch->stats.recovered++;
            }
         }
      }
   }
   //Update strain stats
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      Strain* strain = element->key;
      //Update strain stats for total infections
      strain->stats.infected = knownStrains.Get(strain)->GetSize();
      //Update strain stats for strain age
      strain->stats.age = date - strain->firstSeenDate;
   }
}

void World::SaveState(DataOutput* out) {
   out->WriteInt(date);
   //Get a set of old strains
   LinkedHashSet<Strain> otherStrains;
   for (unsigned int p = 0; p < Main::simulationInfo->numPatches; p++) {
      for (unsigned int h = 0; h < Main::simulationInfo->hostsPerPatch; h++) {
         Strain* strain = patches[p]->hosts[h].lastInfectionStrain;
         if (strain != NULL && !knownStrains.Contains(strain)) {
            otherStrains.Add(strain);
         }
      }
   }
   //Save all strains here
   out->WriteInt(knownStrains.GetSize() + otherStrains.GetSize());
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* element = knownStrains.GetRoot(); element != NULL; element = element->next) {
      Strain* strain = element->key;
      strain->SaveState(out);
   }
   for (LinkedHashSet<Strain >::MapElement* element = otherStrains.GetRoot(); element != NULL; element = element->next) {
      Strain* strain = element->key;
      strain->SaveState(out);
   }
   //Save all hosts here
   for (unsigned int i = 0; i < Main::simulationInfo->numPatches; i++) {
      patches[i]->SaveState(out);
   }
   //With all strains and hosts saved, save the infection hash maps
   out->WriteInt(knownStrains.GetSize());
   for (LinkedHashMap<Strain, LinkedHashSet<Host> >::MapElement* strainElement = knownStrains.GetRoot(); strainElement != NULL; strainElement = strainElement->next) {
      Strain* strain = strainElement->key;
      LinkedHashSet<Host>* hostSet = strainElement->value;
      out->WriteInt(strain->stats.id);
      out->WriteInt(hostSet->GetSize());
      for (LinkedHashSet<Host>::MapElement* hostElement = hostSet->GetRoot(); hostElement != NULL; hostElement = hostElement->next) {
         Host* host = hostElement->key;
         out->WriteInt(host->id);
      }
   }
}

void World::LoadState(DataInput* in) {
   date = in->ReadInt();
   //Load all strains here
   unsigned int numStrains = in->ReadInt();
   LinkedHashMap<Hashable::Integer, Strain>* strains = new LinkedHashMap<Hashable::Integer, Strain > ();
   for (unsigned int i = 0; i < numStrains; i++) {
      Strain* strain = Strain::LoadState(in);
      Hashable::Integer* key = new Hashable::Integer(strain->stats.id);
      //Make sure not to replace an existing strain in the hash map
      Strain* temp = strains->Get(key);
      if(temp != NULL) {
         //A strain with this id is already in the hash map and is about to be overwritten - don't leak it
         Strain::SafeDelete(temp);
      }
      //Add the new strain to the hash map, replacing the old strain if there was one
      strains->Put(key, strain);
      ++strain->referenceCount;
   }
   //Load all hosts here
   for (unsigned int i = 0; i < Main::simulationInfo->numPatches; i++) {
      patches[i]->LoadState(in, strains);
   }
   //With all strains and hosts loaded, load the infection hash maps
   unsigned int num = in->ReadInt();
   for (unsigned int i = 0; i < num; i++) {
      Hashable::Integer* strainID = new Hashable::Integer(in->ReadInt());
      Strain* strain = strains->Get(strainID);
      delete strainID;
      unsigned int numHosts = in->ReadInt();
      LinkedHashSet<Host>* hosts = new LinkedHashSet<Host > ();
      for (unsigned int j = 0; j < numHosts; j++) {
         unsigned int id = in->ReadInt();
         unsigned int patchID = (id >> 24) & 0xFF;
         unsigned int hostID = id & 0xFFFFFF;
         Host* host = &(patches[patchID]->hosts[hostID]);
         hosts->Add(host);
      }
      //Saved and loaded as a stack, so the entries all backwards now
      hosts->Reverse();
      knownStrains.Put(strain, hosts);
      ++strain->referenceCount;
   }
   //Saved and loaded as a stack, so the entries all backwards now
   knownStrains.Reverse();
   //Cleanup
   for(LinkedHashMap<Hashable::Integer, Strain>::MapElement* x = strains->GetRoot(); x != NULL; x = x->next) {
      delete x->key;
      Strain::SafeDelete(x->value);
   }
   delete strains;
}
