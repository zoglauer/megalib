/*
 * MInterfaceMelinator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceMelinator__
#define __MInterfaceMelinator__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MInterface.h"
#include "MDGeometryQuest.h"
#include "MMelinator.h"

// Forwards declarations:
class MGUIMainMelinator;
class MSettingsMelinator;


////////////////////////////////////////////////////////////////////////////////


class MInterfaceMelinator : public MInterface
{
  // Public Interface:
 public:
  //! Default constructor
  MInterfaceMelinator();
  //! Default destructor
  virtual ~MInterfaceMelinator();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  bool ParseCommandLine(int argc, char** argv);

  //! Set the geometry file name and initialize the geometry
  //! Returns true if successful
  virtual bool SetGeometry(MString FileName, bool UpdateGui = true);

  //! Load the configuration file
  virtual bool LoadConfiguration(MString FileName);
  //! Save the configuration file
  virtual bool SaveConfiguration(MString FileName);

  //! Load all data
  bool Load();
  
  //! exit the program
  bool Exit();
  
  // protected methods:
 protected:
 

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The graphical usr interface
  MGUIMainMelinator* m_Gui;

  //! The store for all user data of the GUI:
  MSettingsMelinator* m_Data;

  //! The geometry
  MDGeometryQuest* m_Geometry;

  //! The melinator
  MMelinator m_Melinator;
  
#ifdef ___CLING___
 public:
  ClassDef(MInterfaceMelinator, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
