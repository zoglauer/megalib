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
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Exception base class
class MException : public exception
{
public:
  // Default constructor
  MException() : m_Description("No description of the exception available.") {
    if (m_Abort == true) {
      cout<<what()<<endl<<flush;
      abort();
    }
  }
  //! Default destructor
  virtual ~MException() throw() {}
  
  //! Abort instead of throwing anexception
  static void UseAbort(bool Abort = true) { m_Abort = Abort; }
  
  //! The default error message
  virtual const char* what() const noexcept {      
    return m_Description.data();
  }
  
protected:
  //! Flag storing the global variable if exception should abort the program instead of being thrown
  static bool m_Abort;
  //! The description of the error message
  string m_Description;
  
#ifdef ___CLING___
public:
  ClassDef(MException, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when a parmeter is out of range and 
//! the error cannot be reconvered gracefully
class MExceptionTestFailed : public MException
{
public:
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionTestFailed(MString Description, double Value1, MString Test, double Value2) : MException() {
    ostringstream out;
    out<<"Test failed: "<<Description<<": "<<Value1<<" "<<Test<<" "<<Value2<<endl;
    m_Description = out.str();
  }
  /*
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionTestFailed(MString Description, int Value1, int Value2, MString Test) : MException() {
    ostringstream out;
    out<<"Test failed: "<<Description<<": "<<Value1<<" "<<Test<<" "<<Value2<<endl;
    m_Description = out.str();
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionTestFailed(MString Description, unsigned int Value1, unsigned int Value2, MString Test) : MException() {
    ostringstream out;
    out<<"Test failed: "<<Description<<": "<<Value1<<" "<<Test<<" "<<Value2<<endl;
    m_Description = out.str();
  }
  */
  //! Default destructor
  virtual ~MExceptionTestFailed() throw() {}

  
private:
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionTestFailed, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when a parameter is out of range and
//! the error cannot be recovered gracefully
class MExceptionParameterOutOfRange : public MException
{
public:
  //! Default constructor
  MExceptionParameterOutOfRange() : MException() {
    m_Description = "Index out of bounds!"; 
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionParameterOutOfRange(double Value, double Min, double Max, MString Name) : MException() {
    SetMinSizeIndex(Value, Min, Max, Name);
  }
  //! Default destructor
  virtual ~MExceptionParameterOutOfRange() throw() {}
  //! Set the data minimum array index, its size, and the accessing index to the array
  void SetMinSizeIndex(double Value, double Min, double Max, MString Name) {
    ostringstream stream;
    stream<<"Parameter \""<<Name<<"\" out of range - allowed: ["<<Min<<".."<<Max<<"] - you have: "<<Value<<endl;
    m_Description = stream.str();
  }
  
private:
  
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionParameterOutOfRange, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionIndexOutOfBounds : public MException
{
public:
  //! Default constructor
  MExceptionIndexOutOfBounds() : MException() {
    m_Description = "Index out of bounds!"; 
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionIndexOutOfBounds(unsigned int Min, unsigned int Size, unsigned int Index) : MException() {
    SetMinSizeIndex(Min, Size, Index);
  }
  //! Default destructor
  virtual ~MExceptionIndexOutOfBounds() throw() {}
  //! Set the data minimum array index, its size, and the accessing index to the array
  void SetMinSizeIndex(unsigned int Min, unsigned int Size, unsigned int Index) {
    ostringstream stream;
    stream<<"Index out of bounds - allowed: ["<<Min<<".."<<Size<<"[ - you are trying to access with index: "<<Index<<endl; 
    m_Description = stream.str();
  }
  
private:
  
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionIndexOutOfBounds, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an value cannot be found (e.g. in a map or vector) and
//! the error cannot be recovered gracefully
//! This class would ideal for using templeates, but ROOT does not allow that
class MExceptionValueNotFound : public MException
{
public:
  //! Default constructor
  MExceptionValueNotFound() : MException() {
    m_Description = "Value not found!";
  }
  //! Constructor giving the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  MExceptionValueNotFound(double& Value, const MString& Where) : MException() {
    Set(Value, Where);
  }
  //! Constructor giving the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  MExceptionValueNotFound(int& Value, const MString& Where) : MException() {
    Set(Value, Where);
  }
  //! Constructor giving the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  MExceptionValueNotFound(unsigned int& Value, const MString& Where) : MException() {
    Set(Value, Where);
  }
  //! Constructor giving the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  MExceptionValueNotFound(MString& Value, const MString& Where) : MException() {
    Set(Value, Where);
  }
  //! Default destructor
  virtual ~MExceptionValueNotFound() throw() {}
  //! Set the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  void Set(double& Value, const MString& Where) {
    ostringstream stream;
    stream<<"The value \'"<<Value<<"\' cannot be found in "<<Where<<"."<<endl;
    m_Description = stream.str();
  }
  //! Set the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  void Set(unsigned int& Value, const MString& Where) {
    ostringstream stream;
    stream<<"The value \'"<<Value<<"\' cannot be found in "<<Where<<"."<<endl;
    m_Description = stream.str();
  }
  //! Set the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  void Set(int& Value, const MString& Where) {
    ostringstream stream;
    stream<<"The value \'"<<Value<<"\' cannot be found in "<<Where<<"."<<endl;
    m_Description = stream.str();
  }
  //! Set the not found value and a descriptor where the value cannot be found, e.g. "vector of strip IDs"
  void Set(MString& Value, const MString& Where) {
    ostringstream stream;
    stream<<"The value \'"<<Value<<"\' cannot be found in "<<Where<<"."<<endl;
    m_Description = stream.str();
  }

private:


#ifdef ___CLING___
public:
  ClassDef(MExceptionValueNotFound, 1)
#endif
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionEmptyArray : public MException
{
public:
  //! Default constructor
  MExceptionEmptyArray() : MException() {
    m_Description = "The array has no elements!"; 
  }
  //! Constructor giving the minium array index, its size, and the accessing index to the array 
  MExceptionEmptyArray(MString Name) : MException() {
    ostringstream stream;
    stream<<"The array \""<<Name<<"\" has no elements."<<endl; 
    m_Description = stream.str();
  }
  //! Default destructor
  virtual ~MExceptionEmptyArray() throw() {}
  
private:

  
#ifdef ___CLING___
public:
  ClassDef(MExceptionEmptyArray, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an value is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionValueOutOfBounds : public MException
{
public:
  //! Default constructor
  MExceptionValueOutOfBounds() : MException() {
    m_Description = "Value out of bounds!"; 
  }
  //! Constructor giving the minimum and maximum and the given value 
  MExceptionValueOutOfBounds(double Value) : MException() {
    m_Description = "Value out of bounds: ";
    m_Description += Value;
  }
  //! Constructor giving the minimum and maximum and the given value 
  MExceptionValueOutOfBounds(double Min, double Max, double Value) : MException() {
    ostringstream stream;
    stream<<"Value out of bounds - allowed: ["<<Min<<".."<<Max<<"] - your's: "<<Value<<endl; 
    m_Description = stream.str();
  }
  //! Default destructor
  virtual ~MExceptionValueOutOfBounds() throw() {}
  
private:

  
#ifdef ___CLING___
public:
  ClassDef(MExceptionValueOutOfBounds, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when a division by zero would have occured
class MExceptionDivisionByZero : public MException
{
public:
  //! Default constructor
  MExceptionDivisionByZero() : MException() {
    m_Description = "Division by zero!";
  }
  //! Default destructor
  virtual ~MExceptionDivisionByZero() throw() {}
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionDivisionByZero, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when a number is not finite, i.e. nan or inf
class MExceptionNumberNotFinite : public MException
{
public:
  //! Default constructor
  MExceptionNumberNotFinite() : MException() {
    m_Description = "Number not finite!";
  }
  //! Default destructor
  virtual ~MExceptionNumberNotFinite() throw() {}
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionNumberNotFinite, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionObjectDoesNotExist : public MException
{
public:
  //! Default constructor
  MExceptionObjectDoesNotExist() : MException() {
    m_Description = "Object/Key not found in the list/vector/array/etc. !"; 
  }
  //! Standard constructor
  MExceptionObjectDoesNotExist(const MString& Name) : MException() {
    SetName(Name);
  }
  //! Default destructor
  virtual ~MExceptionObjectDoesNotExist() throw() {}
  
  //! Set the name of the object which does not exists
  void SetName(const MString& Name) { 
    ostringstream stream;
    stream<<"Object/Key "<<Name<<" not found in the list/vector/array/etc. !"<<endl; 
    m_Description = stream.str();
  }
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionObjectDoesNotExist, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionObjectsNotIdentical : public MException
{
public:
  //! Default constructor
  MExceptionObjectsNotIdentical() : MException() {
    m_Description = "Objects not identical!";
  }
  //! Standard constructor
  MExceptionObjectsNotIdentical(const MString& Name1, const MString& Name2) : MException() {
    if (Name1 != "" && Name2 != "") {
      ostringstream stream;
      stream<<"The object "<<Name1<<" and "<<Name2<<" are not identical!"<<endl; 
      m_Description = stream.str();
    } else {
      m_Description = "Objects not identical!";
    }
  }
  //! Default destructor
  virtual ~MExceptionObjectsNotIdentical() throw() {}
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionObjectsNotIdentical, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionUnknownMode : public MException
{
public:
  //! Default constructor
  MExceptionUnknownMode() : MException() {
    m_Description = "Unknown mode!"; ;
  }
  //! Standard constructor
  MExceptionUnknownMode(const MString& Mode) : MException() {
    ostringstream stream;
    stream<<"Unknown mode "<<Mode<<"!"<<endl;
    m_Description = stream.str();
  }
  //! Standard constructor
  MExceptionUnknownMode(const MString& Type, int i) : MException() {
    ostringstream stream;
    stream<<"Unknown "<<Type<<" mode "<<i<<"!"<<endl;     
    m_Description = stream.str();
  }
  //! Default destructor
  virtual ~MExceptionUnknownMode() throw() {}
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionUnknownMode, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionNeverReachThatLineOfCode : public MException
{
public:
  //! Default constructor
  MExceptionNeverReachThatLineOfCode() : MException() {
    m_Description = "We should have never reached that line of code!"; 
  }
  //! Standard constructor
  MExceptionNeverReachThatLineOfCode(const MString& Description) : MException() {
    ostringstream stream;
    if (Description == "") {
      stream<<"We should have never reached that line of code: "<<endl;
      stream<<Description<<endl;
    } else {
      stream<<"We should have never reached that line of code!"<<endl;        
    }
    m_Description = stream.str();
  }
  //! Default destructor
  virtual ~MExceptionNeverReachThatLineOfCode() throw() {}

  
#ifdef ___CLING___
public:
  ClassDef(MExceptionNeverReachThatLineOfCode, 1)
#endif  
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when a pointer is zero and
//! the error cannot be reconvered gracefully
class MExceptionPointerIsInvalid : public MException
{
public:
  //! Constructor giving the minium array index, its size, and the accessing index to the array
  MExceptionPointerIsInvalid(MString PointerName, void* Pointer) : MException() {
    ostringstream out;
    out<<"The pointer \""<<PointerName<<"\" is invalid: "<<reinterpret_cast<std::size_t>(Pointer)<<endl;
    m_Description = out.str();
  }
  //! Default destructor
  virtual ~MExceptionPointerIsInvalid() throw() {}


private:

#ifdef ___CLING___
public:
  ClassDef(MExceptionPointerIsInvalid, 1)
#endif
};


////////////////////////////////////////////////////////////////////////////////


//! This exception is thrown when an index is out-of-bounds and 
//! the error cannot be recovered gracefully
class MExceptionArbitrary : public MException
{
public:
  //! Default constructor
  MExceptionArbitrary() : MException() {
    m_Description = "An exception was triggered!"; 
  }
  //! Standard constructor
  MExceptionArbitrary(const MString& Description) : MException() {
    SetDescription(Description);
  }
  //! Default destructor
  virtual ~MExceptionArbitrary() throw() {}
  //! Set a name for the unknown mode
  void SetDescription(const MString& Description) { 
    if (Description != "") {
      ostringstream stream;
      stream<<"An exception was triggered: "<<endl;
      stream<<m_Description<<endl;
      m_Description = stream.str();
    }
  }
  
private:
  
#ifdef ___CLING___
public:
  ClassDef(MExceptionArbitrary, 1)
#endif  
};


#endif


////////////////////////////////////////////////////////////////////////////////
