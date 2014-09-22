/*
 * MModuleLoaderRoa.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderRoa__
#define __MModuleLoaderRoa__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileReadOuts.h"
#include "MModule.h"
#include "MModuleInterfaceFileName.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleLoaderRoa : public MModule, public MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderRoa();
  //! Default destructor
  virtual ~MModuleLoaderRoa();

  //! Initialize the module
  virtual bool Initialize();

  //! Initialize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:
  //! Reads one event from file - return zero in case of no more events present or an Error occured
  bool ReadNextEvent(MReadOutAssembly* Event);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out file
  MFileReadOuts m_RoaFile;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MModuleLoaderRoa, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
