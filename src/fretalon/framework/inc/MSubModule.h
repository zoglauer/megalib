/*
 * MSubModule.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModule__
#define __MSubModule__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlNode.h"
#include "MReadOutAssembly.h"


////////////////////////////////////////////////////////////////////////////////

//! This is the base class for all sub modules. It provides
//! + Read-out assembly handling
//! + and general analysis task management features
class MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModule();

  //! No copy constructor
  MSubModule(const MSubModule&) = delete;
  //! No copy assignment
  MSubModule& operator=(const MSubModule&) = delete;
  //! No move constructors
  MSubModule(MSubModule&&) = delete;
  //! No move MSubModule
  MSubModule& operator=(MSubModule&&) = delete;

  //! Default destructor
  virtual ~MSubModule();
  //! Initialize the module
  //! When overwritten, the base-class'es Initialize() has to be called *at the very end* of the 
  //! Initilize() function of the derived class, since in multi-threaded mode it 
  //! starts the thraeds
  virtual bool Initialize();

  //! Clear event data
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  //! Has to be overwritten in derived class
  virtual bool AnalyzeEvent(MReadOutAssembly* Event) = 0;

  //! Finalize the module
  //! When overwritten, the base-class'es Finalize() has to be called *at the very beginning* of the
  //! Finalize() function of the derived class, since in multi-threaded mode it
  //! ends the threads
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);
  
  // protected methods:
 protected:
  

  // private methods:
 private:

  
  // protected members:
 protected:
  //! A name of the module
  MString m_Name;

  
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MSubModule, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
