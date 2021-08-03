/*
 * MBinaryStore.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MBinaryStore.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinaryStore)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinaryStore::MBinaryStore()
{
  // Sanity checks:
  //float 4 bit IEEE
  //double 8 bit IEEE.
  
  m_Position = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinaryStore::~MBinaryStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 8-bit int
void MBinaryStore::AddInt8(int8_t Value) 
{ 
  m_Array.push_back(Value & 0xFF); 
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 8-bit int
int8_t MBinaryStore::GetInt8() 
{ 
  if (m_Position+0 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+0);
    return 0;
  }
  
  int8_t R = 0;
  R |= m_Array[m_Position];
  m_Position += 1;
  return R; 
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 8-bit unsigned int
void MBinaryStore::AddUInt8(uint8_t Value) 
{ 
  m_Array.push_back(Value); 
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 8-bit unsigned int
uint8_t MBinaryStore::GetUInt8() 
{ 
  if (m_Position+0 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+0);
    return 0;
  }
  
  uint8_t R = m_Array[m_Position];
  m_Position += 1;
  return R; 
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add an 16-bit int
void MBinaryStore::AddInt16(int16_t Value) 
{ 
  union { int16_t V; uint16_t I; } U;
  U.V = Value;
  
  for (unsigned int i = 0; i < 2; ++i) {
    m_Array.push_back((U.I >> i*8) & 0x00ff);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 16-bit int
int16_t MBinaryStore::GetInt16() 
{ 
  if (m_Position+1 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+1);
    return 0;
  }
  
  union { int16_t R; uint16_t I; } U;
  U.I = 0;
  for (unsigned int i = 0; i < 2; ++i) {
    U.I |= (uint16_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 2;
  
  return U.R; 
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 16-bit unsigned int
void MBinaryStore::AddUInt16(uint16_t Value) 
{ 
  for (unsigned int i = 0; i < 2; ++i) {
    m_Array.push_back((Value >> i*8) & 0x00ff);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 16-bit unsigned int
uint16_t MBinaryStore::GetUInt16() 
{ 
  if (m_Position+1 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+1);
    return 0;
  }
  
  uint16_t R = 0;
  for (unsigned int i = 0; i < 2; ++i) {
    R |= (uint16_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 2;
  return R; 
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add an 32-bit int
void MBinaryStore::AddInt32(int32_t Value) 
{ 
  union { int32_t V; uint32_t I; } U;
  U.V = Value;
  
  for (unsigned int i = 0; i < 4; ++i) {
    m_Array.push_back((U.I >> i*8) & 0x000000ff);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 32-bit int
int32_t MBinaryStore::GetInt32() 
{ 
  if (m_Position+3 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+3);
    return 0;
  }
  
  union { int32_t R; uint32_t I; } U;
  U.I = 0;
  for (unsigned int i = 0; i < 4; ++i) {
    U.I |= (uint32_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 4;
  
  return U.R; 
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 32-bit unsigned int
void MBinaryStore::AddUInt32(uint32_t Value) 
{ 
  for (unsigned int i = 0; i < 4; ++i) {
    m_Array.push_back((Value >> i*8) & 0x000000ff);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 32-bit unsigned int
uint32_t MBinaryStore::GetUInt32() 
{ 
  if (m_Position+3 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+3);
    return 0;
  }
  
  uint32_t R = 0;
  for (unsigned int i = 0; i < 4; ++i) {
    R |= (uint32_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 4;
  return R; 
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add an 64-bit int
void MBinaryStore::AddInt64(int64_t Value) 
{ 
  union { int64_t V; uint64_t I; } U;
  U.V = Value;
  
  for (unsigned int i = 0; i < 8; ++i) {
    m_Array.push_back((U.I >> i*8) & 0x00000000000000fful);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 64-bit int
int64_t MBinaryStore::GetInt64() 
{ 
  if (m_Position+7 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+7);
    return 0;
  }
  
  union { int64_t R; uint64_t I; } U;
  U.I = 0;
  for (unsigned int i = 0; i < 8; ++i) {
    U.I |= (uint64_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 8;
  
  return U.R; 
}


////////////////////////////////////////////////////////////////////////////////


//! Add an 64-bit unsigned int
void MBinaryStore::AddUInt64(uint64_t Value) 
{ 
  for (unsigned int i = 0; i < 8; ++i) {
    m_Array.push_back((Value >> i*8) & 0x00000000000000fful);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get an 64-bit unsigned int
uint64_t MBinaryStore::GetUInt64() 
{ 
  if (m_Position+7 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+7);
    return 0;
  }
  
  uint64_t R = 0;
  for (unsigned int i = 0; i < 8; ++i) {
    R |= (uint64_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 8;
  return R; 
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add a float
void MBinaryStore::AddFloat(float Value) 
{ 
  union { float V; uint32_t I; } U;
  U.V = Value;
  
  for (unsigned int i = 0; i < 4; ++i) {
    m_Array.push_back((U.I >> i*8) & 0x000000ff);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a float
float MBinaryStore::GetFloat() 
{
  if (m_Position+3 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+3);
    return 0;
  }
    
  union { float V; uint32_t I; } U;
  U.I = 0;
  for (unsigned int i = 0; i < 4; ++i) {
    U.I |= (uint32_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 4;

  return U.V;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a double
void MBinaryStore::AddDouble(double Value) 
{ 
  union { double V; uint64_t I; } U;
  U.V = Value;
  
  for (uint64_t i = 0; i < 8; ++i) {
    m_Array.push_back((U.I >> i*8) & 0x00000000000000fful);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a double
double MBinaryStore::GetDouble() 
{ 
  if (m_Position+7 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+7);
    return 0;
  }
  
  union { double V; uint64_t I; } U;
  U.I = 0;
  for (unsigned int i = 0; i < 8; ++i) {
    U.I |= (uint64_t) m_Array[m_Position + i] << i*8;
  }
  m_Position += 8;
  
  return U.V;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add a MVector as 3 floats
void MBinaryStore::AddNormalizedVectorInt16(const MVector& V)
{
  if (V[0] > 1 || V[0] < -1) {
    throw MExceptionValueOutOfBounds(-1, 1, V[0]);
    return;
  }
  if (V[1] > 1 || V[1] < -1) {
    throw MExceptionValueOutOfBounds(-1, 1, V[1]);
    return;
  }
  if (V[2] > 1 || V[2] < -1) {
    throw MExceptionValueOutOfBounds(-1, 1, V[2]);
    return;
  }
  
  AddInt16(10000*V[0]);
  AddInt16(10000*V[1]);
  AddInt16(10000*V[2]);
}


////////////////////////////////////////////////////////////////////////////////


//! Get a MVector from 3 floats
MVector MBinaryStore::GetNormalizedVectorInt16()
{
  if (m_Position+5 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+5);
    return 0;
  }
  
  // The execution orser is not guaraanteed to be left to right, therefore this is not possible"
  //return MVector(GetFloat(), GetFloat(), GetFloat());
  
  double X = GetInt16()/10000.0;
  double Y = GetInt16()/10000.0;
  double Z = GetInt16()/10000.0;
  
  return MVector(X, Y, Z);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a MVector as 3 floats
void MBinaryStore::AddVectorFloat(const MVector& V)
{
  AddFloat(V[0]);
  AddFloat(V[1]);
  AddFloat(V[2]);
}


////////////////////////////////////////////////////////////////////////////////


//! Get a MVector from 3 floats
MVector MBinaryStore::GetVectorFloat()
{
  if (m_Position+11 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+11);
    return 0;
  }

  // The execution orser is not guaraanteed to be left to right, therefore this is not possible"
  //return MVector(GetFloat(), GetFloat(), GetFloat());
  
  double X = GetFloat();
  double Y = GetFloat();
  double Z = GetFloat();
  
  return MVector(X, Y, Z);
}


////////////////////////////////////////////////////////////////////////////////


//! Add a MVector as 3 doubles 
void MBinaryStore::AddVectorDouble(const MVector& V)
{
  AddDouble(V[0]);
  AddDouble(V[1]);
  AddDouble(V[2]);  
}


////////////////////////////////////////////////////////////////////////////////


//! Get a MVector from 3 doubles
MVector MBinaryStore::GetVectorDouble()
{
  if (m_Position+23 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+23);
    return 0; 
  }
  
  // The execution orser is not guaraanteed to be left to right, therefore this is not possible"
  // return MVector(GetDouble(), GetDouble(), GetDouble());
    
  double X = GetDouble();
  double Y = GetDouble();
  double Z = GetDouble();
  
  return MVector(X, Y, Z);
  
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add time
void MBinaryStore::AddTime(const MTime& Time) 
{ 
  AddInt64(Time.GetInternalSeconds()); 
  AddInt32(Time.GetInternalNanoSeconds()); 
}


////////////////////////////////////////////////////////////////////////////////


//! Add time
MTime MBinaryStore::GetTime() 
{ 
  if (m_Position+11 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+11);
    return 0; 
  }
  
  int64_t Seconds = GetInt64();
  int64_t NanoSeconds = GetInt32(); 
  
  return MTime(Seconds, NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


//! Add time
void MBinaryStore::AddTimeUInt64(const MTime& Time) 
{ 
  if ((uint64_t) Time.GetInternalSeconds() >= numeric_limits<uint64_t>::max() / 1000000000) {
    throw MExceptionArbitrary("MTime is too large to fit into an uint64_t");
    return;
  }
  
  uint64_t NanoSeconds = Time.GetInternalSeconds()*1000000000 + Time.GetInternalNanoSeconds();
  
  AddInt64(NanoSeconds); 
}


////////////////////////////////////////////////////////////////////////////////


//! Add time
MTime MBinaryStore::GetTimeUInt64() 
{ 
  if (m_Position+7 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+7);
    return 0; 
  }
  
  int64_t NanoSeconds = GetInt64(); 
  
  return MTime(NanoSeconds / 1000000000, NanoSeconds % 1000000000);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Add a string
void MBinaryStore::AddString(MString Value, unsigned int NumberOfCharacters) 
{ 
  for (unsigned int i = 0; i < Value.Length(); ++i) { 
    AddChar(Value[i]); 
    NumberOfCharacters--; 
    if (NumberOfCharacters == 0) break; 
  }  
  for (unsigned int i = 0; i < NumberOfCharacters; ++i) { 
    AddChar('\0'); 
  } 
}


////////////////////////////////////////////////////////////////////////////////


//! Get a string
MString MBinaryStore::GetString(unsigned int NumberOfCharacters)
{
  if (NumberOfCharacters == 0) return MString("");
  
  if (m_Position+NumberOfCharacters-1 >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+NumberOfCharacters-1);
    return 0; 
  }
  
  
  MString R;
  for (unsigned int i = 0; i < NumberOfCharacters; ++i) {
    R += GetChar(); 
  }
  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a character
void MBinaryStore::AddChar(char Value) 
{ 
  union { char V; uint8_t I; } U;
  U.V = Value;
  
  m_Array.push_back(U.I);
}


////////////////////////////////////////////////////////////////////////////////


//! Add an array of characters
void MBinaryStore::AddChars(char* Value, unsigned int NumberOfCharacters)
{
  for (unsigned int i = 0; i < NumberOfCharacters; ++i) {
    AddChar(Value[i]); 
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get a character
char MBinaryStore::GetChar()
{
  if (m_Position >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position);
    return 0;
  }
  
  union { char V; uint8_t I; } U;
  U.I = m_Array[m_Position];
  m_Position += 1;
  
  return U.V; 
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//! Move the reading position pointer
void MBinaryStore::ProgressPosition(long Position)
{
  if (m_Position+Position >= m_Array.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+Position);
    return;
  }
  if (m_Position+Position < 0) {
    throw MExceptionIndexOutOfBounds(0, m_Array.size(), m_Position+Position);
    return;
  }
  
  m_Position += Position;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const MBinaryStore& Store)
{
  for (unsigned int i = 0; i < Store.GetArraySize(); ++i) {
    os<<Store.GetArrayValue(i)<<" ";
  }
  return os;
}


// MBinaryStore.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
