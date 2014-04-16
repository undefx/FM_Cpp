#include "FileDataIO.h"

FileDataIO::FileDataIO(std::string filename) {
   //Create the file if it doesn't exist (but don't truncate it if it does exist)
   std::ofstream temp(filename.c_str(), std::ios::app | std::ios::out | std::ios::binary);
   //Close the write-only file
   temp.close();
   //Open the file for reading and writing
   file = new std::fstream(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
}

FileDataIO::~FileDataIO() {
   delete file;
}

//File operations

void FileDataIO::Flush() {
   file->flush();
}

unsigned long FileDataIO::GetReadPosition() {
   return (unsigned long) file->tellg();
}

unsigned long FileDataIO::GetWritePosition() {
   return (unsigned long) file->tellp();
}

void FileDataIO::SeekRead(unsigned long pos) {
   file->seekg(pos, std::ios::beg);
}

void FileDataIO::SeekWrite(unsigned long pos) {
   file->seekp(pos, std::ios::beg);
}

void FileDataIO::Close() {
   file->close();
}

//DataOutput interface

void FileDataIO::WriteBoolean(const bool x) {
   WriteByte(x ? 1 : 0);
}

void FileDataIO::WriteByte(const unsigned char x) {
   char buffer[1];
   buffer[0] = x;
   file->write(buffer, 1);
}

void FileDataIO::WriteShort(const unsigned short x) {
   char buffer[2];
   buffer[0] = (x >> 8) & 0xFF;
   buffer[1] = x & 0xFF;
   file->write(buffer, 2);
}

void FileDataIO::WriteInt(const unsigned int x) {
   char buffer[4];
   buffer[0] = (x >> 24) & 0xFF;
   buffer[1] = (x >> 16) & 0xFF;
   buffer[2] = (x >> 8) & 0xFF;
   buffer[3] = x & 0xFF;
   file->write(buffer, 4);
}

void FileDataIO::WriteLong(const unsigned long x) {
   char buffer[8];
   buffer[0] = (x >> 56) & 0xFF;
   buffer[1] = (x >> 48) & 0xFF;
   buffer[2] = (x >> 40) & 0xFF;
   buffer[3] = (x >> 32) & 0xFF;
   buffer[4] = (x >> 24) & 0xFF;
   buffer[5] = (x >> 16) & 0xFF;
   buffer[6] = (x >> 8) & 0xFF;
   buffer[7] = x & 0xFF;
   file->write(buffer, 8);
}

void FileDataIO::WriteDouble(const double x) {

   union {
      double d;
      unsigned long l;
   } u;
   u.d = x;
   WriteLong(u.l);
}

void FileDataIO::WriteString(const std::string x) {
   unsigned short length = x.length();
   WriteShort(length);
   file->write(x.c_str(), length);
}

void FileDataIO::Write(const unsigned char* buffer, unsigned int offset, unsigned int length) {
   file->write((const char*)(buffer + offset), length);
}

//DataInput interface

bool FileDataIO::ReadBoolean() {
   return ReadByte() != 0;
}

unsigned char FileDataIO::ReadByte() {
   char buffer[1];
   file->read(buffer, 1);
   return buffer[0] & 0xFF;
}

unsigned short FileDataIO::ReadShort() {
   char buffer[2];
   file->read(buffer, 2);
   unsigned short x = buffer[0] & 0xFF;
   x <<= 8;
   x |= buffer[1] & 0xFF;
   return x;
}

unsigned int FileDataIO::ReadInt() {
   char buffer[4];
   file->read(buffer, 4);
   unsigned int x = buffer[0] & 0xFF;
   x <<= 8;
   x |= buffer[1] & 0xFF;
   x <<= 8;
   x |= buffer[2] & 0xFF;
   x <<= 8;
   x |= buffer[3] & 0xFF;
   return x;
}

unsigned long FileDataIO::ReadLong() {
   char buffer[8];
   file->read(buffer, 8);
   unsigned long x = buffer[0] & 0xFFL;
   x <<= 8;
   x |= buffer[1] & 0xFFL;
   x <<= 8;
   x |= buffer[2] & 0xFFL;
   x <<= 8;
   x |= buffer[3] & 0xFFL;
   x <<= 8;
   x |= buffer[4] & 0xFFL;
   x <<= 8;
   x |= buffer[5] & 0xFFL;
   x <<= 8;
   x |= buffer[6] & 0xFFL;
   x <<= 8;
   x |= buffer[7] & 0xFFL;
   return x;
}

double FileDataIO::ReadDouble() {

   union {
      unsigned long l;
      double d;
   } u;
   u.l = ReadLong();
   return u.d;
}

std::string FileDataIO::ReadString() {
   unsigned short length = ReadShort();
   char* buffer = new char[length];
   file->read(buffer, length);
   std::string str(buffer, (size_t) length);
   delete [] buffer;
   return str;
}

void FileDataIO::Read(unsigned char* buffer, unsigned int offset, unsigned int length) {
   file->read((char*)(buffer + offset), length);
}
