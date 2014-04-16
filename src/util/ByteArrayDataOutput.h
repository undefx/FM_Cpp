#ifndef BYTEARRAYDATAOUTPUT_H
#define	BYTEARRAYDATAOUTPUT_H

#include <vector>
#include "DataOutput.h"

//Inspired by Java's DataOutputStream and ByteArrayOutputStream

class ByteArrayDataOutput : public DataOutput {
public:
   ByteArrayDataOutput();
   ~ByteArrayDataOutput();
   void WriteBoolean(const bool x);
   void WriteByte(const unsigned char x);
   void WriteShort(const unsigned short x);
   void WriteInt(const unsigned int x);
   void WriteLong(const unsigned long x);
   void WriteDouble(const double x);
   void WriteString(const std::string x);
   void Write(const unsigned char* buffer, unsigned int offset, unsigned int length);
   unsigned int GetSize();
   unsigned char* GetByteArray();
protected:
   std::vector<unsigned char> buffer;
};

#endif	/* BYTEARRAYDATAOUTPUT_H */

