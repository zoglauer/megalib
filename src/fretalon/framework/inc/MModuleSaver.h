/*
 * MModuleSaver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleSaver__
#define __MModuleSaver__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MModuleInterfaceFileName.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleSaver : public MModule, public MModuleInterfaceFileName
{
  // public interface:
 public:
  //! Default constructor
  MModuleSaver();
  //! Default destructor
  virtual ~MModuleSaver();

  //! Create a new object of this class 
  virtual MModuleSaver* Clone()  { return new MModuleSaver(); }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  static const unsigned int c_RoaFile  = 0;
  static const unsigned int c_DatFile  = 1;
  static const unsigned int c_EvtaFile = 2;
  static const unsigned int c_SimFile  = 3;
  
  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The operation mode
  unsigned int m_Mode;
  
  //! Output stream for dat file
  ofstream m_Out;

  
#ifdef ___CINT___
 public:
  ClassDef(MModuleSaver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
