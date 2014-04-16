#ifndef DATAINPUT_H
#define	DATAINPUT_H

#include <string>

//Inspired by java.io.DataInput

class DataInput {
public:
   virtual bool ReadBoolean() = 0;
   virtual unsigned char ReadByte() = 0;
   virtual unsigned short ReadShort() = 0;
   virtual unsigned int ReadInt() = 0;
   virtual unsigned long ReadLong() = 0;
   virtual double ReadDouble() = 0;
   virtual std::string ReadString() = 0;
   virtual void Read(unsigned char* buffer, unsigned int offset, unsigned int length) = 0;
};

#endif	/* DATAINPUT_H */
