/*
 * MModuleInterfaceFileName.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleInterfaceFileName__
#define __MModuleInterfaceFileName__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Interface providing a file name for various modules
class MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleInterfaceFileName();
  //! Default destructor
  virtual ~MModuleInterfaceFileName();

  //! Create a new object of this class 
  virtual MModuleInterfaceFileName* Clone() = 0;

  //! Get the file name
  MString GetFileName() const { return m_FileName; }
  //! Set the file name
  void SetFileName(const MString& Name) { m_FileName = Name; }  


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The file name
  MString m_FileName;

  // private members:
 private:

  
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleInterfaceFileName, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
