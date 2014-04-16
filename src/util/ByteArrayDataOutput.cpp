#include <algorithm>
#include "ByteArrayDataOutput.h"

ByteArrayDataOutput::ByteArrayDataOutput() {
}

ByteArrayDataOutput::~ByteArrayDataOutput() {
   buffer.clear();
}

void ByteArrayDataOutput::WriteBoolean(const bool x) {
   WriteByte(x ? 1 : 0);
}

void ByteArrayDataOutput::WriteByte(const unsigned char x) {
   buffer.push_back(x);
}

void ByteArrayDataOutput::WriteShort(const unsigned short x) {
   buffer.push_back((x >> 8) & 0xFF);
   buffer.push_back(x & 0xFF);
}

void ByteArrayDataOutput::WriteInt(const unsigned int x) {
   buffer.push_back((x >> 24) & 0xFF);
   buffer.push_back((x >> 16) & 0xFF);
   buffer.push_back((x >> 8) & 0xFF);
   buffer.push_back(x & 0xFF);
}

void ByteArrayDataOutput::WriteLong(const unsigned long x) {
   buffer.push_back((x >> 56) & 0xFF);
   buffer.push_back((x >> 48) & 0xFF);
   buffer.push_back((x >> 40) & 0xFF);
   buffer.push_back((x >> 32) & 0xFF);
   buffer.push_back((x >> 24) & 0xFF);
   buffer.push_back((x >> 16) & 0xFF);
   buffer.push_back((x >> 8) & 0xFF);
   buffer.push_back(x & 0xFF);
}

void ByteArrayDataOutput::WriteDouble(const double x) {
   union {
      double d;
      unsigned long l;
   } u;
   u.d = x;
   WriteLong(u.l);
}

void ByteArrayDataOutput::WriteString(const std::string x) {
   unsigned short length = x.length();
   WriteShort(length);
   for (int i = 0; i < length; i++) {
      buffer.push_back(x[i]);
   }
}

void ByteArrayDataOutput::Write(const unsigned char* buffer, unsigned int offset, unsigned int length) {
   for(unsigned int i = 0; i < length; i++) {
      this->buffer.push_back(buffer[offset + i]);
   }
}

unsigned int ByteArrayDataOutput::GetSize() {
   return buffer.size();
}

unsigned char* ByteArrayDataOutput::GetByteArray() {
   int length = buffer.size();
   unsigned char* array = new unsigned char[length];
   std::copy(buffer.begin(), buffer.end(), array);
   return array;
}
