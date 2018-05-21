/*
 * MFretalonRegistry.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFretalonRegistry__
#define __MFretalonRegistry__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The registry containing all registered read-out elements and read-out data
class MFretalonRegistry
{
  // public interface:
 public:
  //! Return the single instance of this class
  static MFretalonRegistry& Instance();

  //! Add another read-out element type
  void Register(const MReadOutElement& ROE);
  //! Add another read-out data type
  void Register(const MReadOutData& ROD);
  
  //! Return true if this kind of read-out element is registered
  bool IsReadOutElementRegistered(const MString& Name);
  //! Return true if this kind of read-out data is registered
  bool IsReadOutDataRegistered(const MString& Name);
  
  //! Return the new instance of this read-out element
  //! Return zero if it doesn't exist, it must be deleted
  MReadOutElement* GetReadOutElement(const MString& Name);
  
  //! Return the new instance of this read-out data
  //! Return zero if it doesn't exist, it must be deleted
  MReadOutData* GetReadOutData(const MString& Name);
  
  
  // protected methods:
 protected:
  //! Default constructor
  MFretalonRegistry();
  //! Default destuctor 
  virtual ~MFretalonRegistry();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Instances of all existing read-out elements
  vector<MReadOutElement*> m_ROEs;
  //! Instances of all existing read-out datas
  vector<MReadOutData*> m_RODs;


#ifdef ___CLING___
 public:
  ClassDef(MFretalonRegistry, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
