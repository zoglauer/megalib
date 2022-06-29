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

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
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
  //! Default contructor
  MInterfaceGeomega();
  //! standard destructor
  virtual ~MInterfaceGeomega();

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
  
  //! Read the geoemtry
  bool ReadGeometry();
  
  //! Display the geometry
  void ViewGeometry();
  
  //! Ray-trace the geoometry
  void RaytraceGeometry();
  
  //! Test for intersections 
  void TestIntersections();
  
  //! The for intersections 
  //! Returns true if there are intersections or if the test failed in some other way
  //! Diagnostics output is stored in the stream
  bool HasIntersections(ostringstream& Diagnostics);
  
  //! Calculate the masses of the geometry
  void CalculateMasses();
  
  //! Calculate the resolutions of the detectors
  void GetResolutions();
  
  //! Dump information about the geometrey to the screen
  void DumpInformation();
  
  //! Determine the volume at the given position
  void FindVolume(MVector Pos);
  
  //! Create all crossection files
  void CreateCrossSections();
  
  //! Given to positions give the path length in the material
  void GetPathLengths(const MVector& Start, const MVector& Stop);

  //! Some dummy tests - old - don't use
  void TestOnly();
  
  // protected methods:
 protected:
  //! Return true if we need to (re-) load the geometry
  bool ReloadRequired();

  // private methods:
 private:


  // protected members:
 protected:
  MSettingsGeomega* m_Data;                   // All the information of the GUI


  // private members:
 private:
  MSystem m_System;


#ifdef ___CLING___
 public:
  ClassDef(MInterfaceGeomega, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
