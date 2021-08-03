/*
 * MBinaryStore.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBinaryStore__
#define __MBinaryStore__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A BinaryStore definition of a class
class MBinaryStore
{
  // public interface:
 public:
  //! Default constructor
  MBinaryStore();
  //! Default destuctor 
  virtual ~MBinaryStore();
  
  //! Add an 8-bit int
  void AddInt8(int8_t Value);
  //! Get an 8-bit int
  int8_t GetInt8();
  //! Add an 8-bit unsigned int
  void AddUInt8(uint8_t Value);
  //! Get an 8-bit unsigned int
  uint8_t GetUInt8();
  
  //! Add an 16-bit int
  void AddInt16(int16_t Value);
  //! Get an 16-bit int
  int16_t GetInt16();
  //! Add an 16-bit unsigned int
  void AddUInt16(uint16_t Value);
  //! Get an 16-bit unsigned int
  uint16_t GetUInt16();
  
  //! Add an 32-bit int
  void AddInt32(int32_t Value);
  //! Get an 32-bit int
  int32_t GetInt32();
  //! Add an 32-bit unsigned int
  void AddUInt32(uint32_t Value);
  //! Get an 32-bit unsigned int
  uint32_t GetUInt32();
  
  //! Add an 64-bit int
  void AddInt64(int64_t Value);
  //! Get an 64-bit int
  int64_t GetInt64();
  //! Add an 64-bit unsigned int
  void AddUInt64(uint64_t Value);
  //! Get an 64-bit unsigned int
  uint64_t GetUInt64();
  
  //! Add a float
  void AddFloat(float Value);
  //! Get a float
  float GetFloat();
  //! Add a double
  void AddDouble(double Value);
  //! Get a double
  double GetDouble();
 
  //! Add a MVector as 3 floats
  void AddNormalizedVectorInt16(const MVector& V);
  //! Get a MVector from 3 floats
  MVector GetNormalizedVectorInt16();
  //! Add a MVector as 3 floats
  void AddVectorFloat(const MVector& V);
  //! Get a MVector from 3 floats
  MVector GetVectorFloat();
  //! Add a MVector as 3 doubles 
  void AddVectorDouble(const MVector& V);
  //! Get a MVector from 3 doubles
  MVector GetVectorDouble();
  
  //! Add 2 floats
  void AddFloats(float Value1, float Value2) { AddFloat(Value1); AddFloat(Value2); }
  //! Add 2 doubles
  void AddDoubles(double Value1, double Value2) { AddDouble(Value1); AddDouble(Value2); }
  
  
  //! Add time
  void AddTime(const MTime& Time);
  //! Get time
  MTime GetTime();
  //! Add time
  void AddTimeUInt64(const MTime& Time);
  //! Get time
  MTime GetTimeUInt64();
  
  //! Add a string
  void AddString(MString Value, unsigned int NumberOfCharacters);
  //! Get a string
  MString GetString(unsigned int NumberOfCharacters);
  //! Add a character
  void AddChar(char Value);
  //! Add an array of characters
  void AddChars(char* Value, unsigned int NumberOfCharacters);
  //! Get a character
  char GetChar();
  
  //! Return a reference of the array
  //vector<uint8_t>& GetArray() { return m_Array; } 
  //! Return the size of the array
  unsigned long GetArraySize() const { return m_Array.size(); }
  //! Return the content of hte arrat
  uint8_t GetArrayValue(unsigned int i) const { return m_Array.at(i); }
  
  //! Return the unread array size
  unsigned long GetArraySizeUnread() const { return m_Array.size() - m_Position; }
  
  //! Trunctate the array
  void Truncate() { m_Array.erase(m_Array.begin(), m_Array.begin()+m_Position); m_Position = 0; }
  
  //! Move the reading position pointer
  void ProgressPosition(long m_Position);
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The underlying data array
  vector<uint8_t> m_Array;
  //! Reading position start
  unsigned long m_Position;
  

#ifdef ___CLING___
 public:
  ClassDef(MBinaryStore, 1)
#endif

};

//! Stream the content of this class
std::ostream& operator<<(std::ostream& os, const MBinaryStore& Store);

#endif


////////////////////////////////////////////////////////////////////////////////
