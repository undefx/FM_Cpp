#ifndef HASHABLE_H
#define	HASHABLE_H

class Hashable {
public:
   class Integer;
   virtual unsigned int GetHash() = 0;
};

class Hashable::Integer : public Hashable {
public:

   Integer(unsigned int value) {
      this->value = value;
   }

   inline unsigned int GetHash() {
      return value;
   }
   unsigned int value;
};

#endif	/* HASHABLE_H */

