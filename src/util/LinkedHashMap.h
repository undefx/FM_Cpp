#ifndef LINKEDHASHMAP_H
#define	LINKEDHASHMAP_H

#include <list>
#include <cstddef>
#include "Hashable.h"

template <class Key, class Value> class LinkedHashMap {
public:

   class MapElement {
   public:

      MapElement(Key* key, Value* value, MapElement* next) {
         this->key = key;
         this->value = value;
         this->prev = NULL;
         this->next = next;
      }
      Key* key;
      Value* value;
      MapElement* prev;
      MapElement* next;
   };
   const static unsigned int DEFAULT_BINS = 53;

   LinkedHashMap() {
      numBins = DEFAULT_BINS;
      bins = new std::list<MapElement*>[numBins];
      root = NULL;
      size = 0;
      _KeyTypeTest();
   }

   ~LinkedHashMap() {
      delete [] bins;
      MapElement* e = root;
      while (e != NULL) {
         MapElement* next = e->next;
         delete e;
         e = next;
      }
   }

   void Put(Key* key, Value* value) {
      Remove(key);
      MapElement* e = new MapElement(key, value, root);
      if (root != NULL) {
         root->prev = e;
      }
      root = e;
      bins[((Hashable*) key)->GetHash() % numBins].push_back(e);
      ++size;
      if (size > numBins * .8) {
         Resize(numBins * 4 + 1);
      }
   }

   bool Contains(Key* key) {
      unsigned int id = ((Hashable*) key)->GetHash();
      for (typename std::list<MapElement*>::const_iterator iter = bins[id % numBins].begin(), end = bins[id % numBins].end(); iter != end; ++iter) {
         if (((Hashable*) ((*iter)->key))->GetHash() == id) {
            return true;
         }
      }
      return false;
   }

   Value* Get(Key* key) {
      unsigned int id = ((Hashable*) key)->GetHash();
      for (typename std::list<MapElement*>::const_iterator iter = bins[id % numBins].begin(), end = bins[id % numBins].end(); iter != end; ++iter) {
         if (((Hashable*) ((*iter)->key))->GetHash() == id) {
            return (*iter)->value;
         }
      }
      return NULL;
   }

   void Remove(Key* key) {
      unsigned int id = ((Hashable*) key)->GetHash();
      for (typename std::list<MapElement*>::iterator iter = bins[id % numBins].begin(), end = bins[id % numBins].end(); iter != end; ++iter) {
         MapElement* e = *iter;
         if (((Hashable*) (e->key))->GetHash() == id) {
            bins[id % numBins].erase(iter);
            if (e->prev != NULL) {
               e->prev->next = e->next;
            }
            if (e->next != NULL) {
               e->next->prev = e->prev;
            }
            if (e == root) {
               root = e->next;
            }
            delete e;
            --size;
            if (size < numBins * .05 && numBins > DEFAULT_BINS * 5) {
               int temp = numBins / 5;
               Resize(temp - (temp % 2 == 0 ? 1 : 0));
            }
            return;
         }
      }
   }

   void Resize(int numBins) {
      this->numBins = numBins;
      delete [] bins;
      bins = new std::list<MapElement*>[numBins];
      for (MapElement* e = root; e != NULL; e = e->next) {
         bins[((Hashable*) (e->key))->GetHash() % numBins].push_back(e);
      }
   }

   void Reverse() {
      MapElement* node = root;
      while (node != NULL) {
         root = node;
         MapElement* next = node->next;
         node->next = node->prev;
         node = node->prev = next;
      }
   }

   MapElement* GetRoot() {
      return root;
   }

   unsigned int GetSize() {
      return size;
   }

protected:
   unsigned int numBins;
   std::list<MapElement*>* bins;
   MapElement* root;
   unsigned int size;
   
private:

   void _KeyTypeTest() {
      //At compile time, this ensures that Key is a subclass of Hashable
      //At runtime, this is essentially a no-op
      if (root != NULL) {
         static_cast<Hashable*> (root->key);
      }
   }
};

#endif	/* LINKEDHASHMAP_H */
