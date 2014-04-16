#ifndef BYTEARRAYDATAINPUT_H
#define	BYTEARRAYDATAINPUT_H

#include <vector>
#include "DataInput.h"

//Inspired by Java's DataInputStream and ByteArrayInputStream

class ByteArrayDataInput : public DataInput {
public:
   ByteArrayDataInput(const unsigned char* buffer);
   ~ByteArrayDataInput();
   bool ReadBoolean();
   unsigned char ReadByte();
   unsigned short ReadShort();
   unsigned int ReadInt();
   unsigned long ReadLong();
   double ReadDouble();
   std::string ReadString();
   void Read(unsigned char* buffer, unsigned int offset, unsigned int length);
protected:
   const unsigned char* buffer;
   unsigned int position;
};

#endif	/* BYTEARRAYDATAINPUT_H */

