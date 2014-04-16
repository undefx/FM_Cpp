#ifndef DATAOUTPUT_H
#define	DATAOUTPUT_H

#include <string>

//Inspired by java.io.DataOutput

class DataOutput {
public:
   virtual void WriteBoolean(const bool x) = 0;
   virtual void WriteByte(const unsigned char x) = 0;
   virtual void WriteShort(const unsigned short x) = 0;
   virtual void WriteInt(const unsigned int x) = 0;
   virtual void WriteLong(const unsigned long x) = 0;
   virtual void WriteDouble(const double x) = 0;
   virtual void WriteString(const std::string x) = 0;
   virtual void Write(const unsigned char* buffer, unsigned int offset, unsigned int length) = 0;
};

#endif	/* DATAOUTPUT_H */
