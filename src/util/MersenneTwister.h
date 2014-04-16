#ifndef MT19937AR_H
#define	MT19937AR_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "DataInput.h"
#include "DataOutput.h"

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

class MersenneTwister {
public:
   //Constructors
   MersenneTwister();
   MersenneTwister(unsigned long seed);
   MersenneTwister(unsigned long init_key[], int key_length);
   //Destroctor
   ~MersenneTwister();
   //Interface for printing original test vectors
   static void PrintTestVectors();
   //Functions from java.util.Random
   bool NextBoolean();
   double NextDouble();
   float NextFloat();
   int NextInt();
   int NextInt(int n);
   long NextLong();
   //Custom functions
    void SaveState(DataOutput* out);
    void LoadState(DataInput* in);
protected:
   //Original Implementation
   void init_genrand(unsigned long s);
   void init_by_array(unsigned long init_key[], int key_length);
   unsigned long genrand_int32();
   long genrand_int31();
   double genrand_real1();
   double genrand_real2();
   double genrand_real3();
   double genrand_res53();
   static int main();
   unsigned long mt[N]; /* the array for the state vector  */
   int mti; /* mti==N+1 means mt[N] is not initialized */
};

#endif	/* MT19937AR_H */

