#include <string.h>

#include "ByteArrayDataInput.h"

ByteArrayDataInput::ByteArrayDataInput(const unsigned char* buffer) {
   this->buffer = buffer;
   position = 0;
}

ByteArrayDataInput::~ByteArrayDataInput() {
}

bool ByteArrayDataInput::ReadBoolean() {
   return ReadByte() != 0;
}

unsigned char ByteArrayDataInput::ReadByte() {
   return buffer[position++];
}

unsigned short ByteArrayDataInput::ReadShort() {
   unsigned short x = buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   return x;
}

unsigned int ByteArrayDataInput::ReadInt() {
   unsigned int x = buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   return x;
}

unsigned long ByteArrayDataInput::ReadLong() {
   unsigned long x = buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   x <<= 8;
   x |= buffer[position++];
   return x;
}

double ByteArrayDataInput::ReadDouble() {
   union {
      unsigned long l;
      double d;
   } u;
   u.l = ReadLong();
   return u.d;
}

std::string ByteArrayDataInput::ReadString() {
   unsigned short length = ReadShort();
   position += length;
   return std::string((const char*) (buffer + (position - length)), (size_t) length);
}

void ByteArrayDataInput::Read(unsigned char* buffer, unsigned int offset, unsigned int length) {
   memcpy(buffer + offset, this->buffer + position, length);
   position += length;
}
