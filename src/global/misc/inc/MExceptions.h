/*
 * MExceptions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MExceptions__
#define __MExceptions__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include <exception>
#include <iostream>
#include <sstream>
#include <cstdlib>
using namespace std;

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionIndexOutOfBounds : public exception
{
public:
  //! Default constructor
  MExceptionIndexOutOfBounds() : m_IsEmpty(true), m_Min(0), m_Size(0), m_Index(0) {
    abort();
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionIndexOutOfBounds(unsigned int Min, unsigned int Size, unsigned int Index) {
    abort();
    SetMinSizeIndex(Min, Size, Index);
  }
  //! Default destructor
  ~MExceptionIndexOutOfBounds() throw() {}
  //! Set the data minium array index, its size, and the accessing index to the array
  void SetMinSizeIndex(unsigned int Min, unsigned int Size, unsigned int Index) {
    m_Min = Min; m_Size = Size; m_Index = Index; m_IsEmpty = false;
  }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      stream<<"Index out of bounds - allowed: ["<<m_Min<<".."<<m_Size<<"[ - you are trying to access with index: "<<m_Index<<endl; 
      return stream.str().c_str();
    } else {
      return "Index out of bounds!"; 
    }
  }

private:
  //! True if SetMinSizeIndex() has never been called
  bool m_IsEmpty;
  //! The minimum index
  unsigned int m_Min;
  //! The size of the array, vector, etc.
  unsigned int m_Size;
  //! The index with which we wanted to access the array, vector, etc.
  unsigned int m_Index;
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionObjectDoesNotExist : public exception
{
public:
  //! Default constructor
  MExceptionObjectDoesNotExist() : m_IsEmpty(true), m_Name("") {
    abort();
  }
  //! Standard constructor
  MExceptionObjectDoesNotExist(const MString& Name) : m_IsEmpty(true), m_Name(Name) {
    abort();
  }
  //! Default destructor
  ~MExceptionObjectDoesNotExist() throw() {}
  //! Set the data minium array index, its size, and the accessing index in the array
  void SetName(const MString& Name) { m_Name = Name; }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      stream<<"Object "<<m_Name<<" not found in the list/vector/array!"<<endl; 
      return stream.str().c_str();
    } else {
      return "Object not found in the list/vector/array!"; 
    }
  }

private:
  //! True if SetName() has never been called
  bool m_IsEmpty;
  //! The Name of the not found object
  MString m_Name;
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionUnknownMode : public exception
{
public:
  //! Default constructor
  MExceptionUnknownMode() : m_IsEmpty(true), m_Mode("") {
    abort();
  }
  //! Standard constructor
  MExceptionUnknownMode(const MString& Mode) : m_IsEmpty(true), m_Mode(Mode) {
    abort();
  }
  //! Default destructor
  ~MExceptionUnknownMode() throw() {}
  //! Set the data minium array index, its size, and the accessing index in the array
  void SetName(const MString& Mode) { m_Mode = Mode; }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      stream<<"Unknown mode "<<m_Mode<<"!"<<endl; 
      return stream.str().c_str();
    } else {
      return "Unknown mode!"; 
    }
  }

private:
  //! True if SetName() has never been called
  bool m_IsEmpty;
  //! The mode which is unknown
  MString m_Mode;
};


#endif


////////////////////////////////////////////////////////////////////////////////
