#ifndef FILEDATAIO_H
#define	FILEDATAIO_H

#include <string>
#include <iostream>
#include <fstream>
#include "DataOutput.h"
#include "DataInput.h"

//Inspired by java.io.RandomAccessFile

class FileDataIO : public DataOutput, public DataInput {
public:
   FileDataIO(std::string filename);
   ~FileDataIO();
   //File operations
   void Flush();
   unsigned long GetReadPosition();
   unsigned long GetWritePosition();
   void SeekRead(unsigned long pos);
   void SeekWrite(unsigned long pos);
   void Close();
   //DataOutput interface
   void WriteBoolean(const bool x);
   void WriteByte(const unsigned char x);
   void WriteShort(const unsigned short x);
   void WriteInt(const unsigned int x);
   void WriteLong(const unsigned long x);
   void WriteDouble(const double x);
   void WriteString(const std::string x);
   void Write(const unsigned char* buffer, unsigned int offset, unsigned int length);
   //DataInput interface
   bool ReadBoolean();
   unsigned char ReadByte();
   unsigned short ReadShort();
   unsigned int ReadInt();
   unsigned long ReadLong();
   double ReadDouble();
   std::string ReadString();
   void Read(unsigned char* buffer, unsigned int offset, unsigned int length);
protected:
   std::fstream* file;
};

#endif	/* FILEDATAIO_H */

