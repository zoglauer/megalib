/*
 * MInterfaceGeomega.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceGeomega__
#define __MInterfaceGeomega__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include "MVector.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsGeomega.h"
#include "MDGeometryQuest.h"
#include "MSystem.h"

class MGUIGeomegaMain;

////////////////////////////////////////////////////////////////////////////////


class MInterfaceGeomega : public MInterface
{
  // Public Interface:
 public:
  MInterfaceGeomega();
  ~MInterfaceGeomega();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  bool ParseCommandLine(int argc, char** argv);

  //! Set the geometry file name and initialize the geometry
  //! Returns true if successful
  virtual bool SetGeometry(MString FileName, bool UpdateGui = true) { return false; }

  //! Load the configuration file
  virtual bool LoadConfiguration(MString FileName);
  //! Save the configuration file
  virtual bool SaveConfiguration(MString FileName);
  
  bool ReadGeometry();
  void ViewGeometry();
  void TestIntersections();
  void CalculateMasses();
  void GetResolutions();
  void DumpInformation();
  void WriteGeant3Files();
  void WriteMGeantFiles();
  void TestOnly();
  void FindVolume(MVector Pos);
  void CreateCrossSections();

  // protected methods:
 protected:
 

  // private methods:
 private:


  // protected members:
 protected:
  MSettingsGeomega* m_Data;                   // All the information of the GUI


  // private members:
 private:
  MSystem m_System;


#ifdef ___CINT___
 public:
  ClassDef(MInterfaceGeomega, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
