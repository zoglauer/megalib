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


//! This exception is thrown when a parmeter is out of range and 
//! the error cannot be reconvered gracefully
class MExceptionParameterOutOfRange : public exception
{
public:
  //! Default constructor
  MExceptionParameterOutOfRange() : m_IsEmpty(true), m_Value(0), m_Minimum(0), m_Maximum(0), m_Name("") {
    abort();
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionParameterOutOfRange(double Value, double Min, double Max, MString Name) {
    abort();
    SetMinSizeIndex(Value, Min, Max, Name);
  }
  //! Default destructor
  ~MExceptionParameterOutOfRange() throw() {}
  //! Set the data minimum array index, its size, and the accessing index to the array
  void SetMinSizeIndex(double Value, double Min, double Max, MString Name) {
    m_Value = Value; m_Minimum = Min; m_Maximum = Max; m_Name = Name; m_IsEmpty = false;
  }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      stream<<"Parameter \""<<m_Name<<"\" out of range - allowed: ["<<m_Minimum<<".."<<m_Maximum<<"] - you have: "<<m_Value<<endl;
      return stream.str().c_str();
    } else {
      return "Index out of bounds!"; 
    }
  }

private:
  //! True if SetValueMinMaxName() has never been called
  bool m_IsEmpty;
  //! The current value
  double m_Value;
  //! The minimum value
  double m_Minimum;
  //! The maximum value
  double m_Maximum;
  //! The name of the parameter
  MString m_Name;
};


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
  //! Set the data minimum array index, its size, and the accessing index to the array
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
  //! Set the data minimum array index, its size, and the accessing index in the array
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
  MExceptionUnknownMode() : m_IsEmpty(true), m_Type(""), m_Mode("") {
    abort();
  }
  //! Standard constructor
  MExceptionUnknownMode(const MString& Mode) : m_IsEmpty(true), m_Type(""), m_Mode(Mode) {
    abort();
  }
  //! Standard constructor
  MExceptionUnknownMode(const MString& Type, int i) : m_IsEmpty(true), m_Type(Type) {
    m_Mode += i;
    abort();
  }
  //! Default destructor
  ~MExceptionUnknownMode() throw() {}
  //! Set a name for the unknown mode
  void SetName(const MString& Mode) { m_Mode = Mode; }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      if (m_Type == "") {
        stream<<"Unknown mode "<<m_Mode<<"!"<<endl;
      } else {
        stream<<"Unknown "<<m_Type<<" mode "<<m_Mode<<"!"<<endl;        
      }
      return stream.str().c_str();
    } else {
      return "Unknown mode!"; 
    }
  }

private:
  //! True if SetName() has never been called
  bool m_IsEmpty;
  //! The type of the mode
  MString m_Type;
  //! The mode which is unknown
  MString m_Mode;
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionNeverReachThatLineOfCode : public exception
{
public:
  //! Default constructor
  MExceptionNeverReachThatLineOfCode() : m_IsEmpty(true) {
    abort();
  }
  //! Standard constructor
  MExceptionNeverReachThatLineOfCode(const MString& Description) : m_IsEmpty(false), m_Description(Description) {
    abort();
  }
  //! Default destructor
  ~MExceptionNeverReachThatLineOfCode() throw() {}
  //! Set a name for the unknown mode
  void SetDescription(const MString& Description) { m_Description = Description; m_IsEmpty = false; }
  //! The error message
  virtual const char* what() const throw() {
    if (m_IsEmpty == false) {
      ostringstream stream;
      if (m_Description == "") {
        stream<<"We should have never reached that line of code: "<<endl;
        stream<<m_Description<<endl;
      } else {
        stream<<"We should have never reached that line of code!"<<endl;        
      }
      return stream.str().c_str();
    } else {
      return "We should have never reached that line of code!"; 
    }
  }

private:
  //! True if SetName() has never been called
  bool m_IsEmpty;
  //! The description of what went wrong
  MString m_Description;

};


#endif


////////////////////////////////////////////////////////////////////////////////
