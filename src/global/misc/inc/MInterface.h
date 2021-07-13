/*
 * MInterface.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterface__
#define __MInterface__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "TF1.h"

// Forward declarations:
class MDGeometryQuest;
class MSettings;
class MGUIMain;


////////////////////////////////////////////////////////////////////////////////


class MInterface
{
  // public interface:
 public:
  //! Default constructor of the interface ABC
  MInterface();
  //! Default destructor of the interface ABC
  virtual ~MInterface();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  virtual bool ParseCommandLine(int argc, char** argv) = 0;

  //! True if we use or intend to use th UI
  bool UseUI() const { return m_UseGui; }
  
  //! Set the geometry file name and initialize the geometry
  //! Returns true if successful
  virtual bool SetGeometry(MString FileName, bool UpdateGui = true) = 0;
  //! Returns the geometry or zero if there is none
  virtual MDGeometryQuest* GetGeometry();

  //! Sets the Gui Data
  // virtual void SetGuiData(MGUIData* BasicGuiData = 0) = 0; // tbd.

  //! Load the configuration file
  virtual bool LoadConfiguration(MString FileName = g_StringNotDefined) = 0;
  //! Save the configuration file
  virtual bool SaveConfiguration(MString FileName = g_StringNotDefined) = 0;

  // Some other auxiliary functions used by many other interfaces
  
  //! Calculate optimum axes given a set of vectors, which include all positions
  //! Return values are the axis-dimensions
  static bool DetermineAxis(double& xMin, double& xMax, 
                            double& yMin, double& yMax, 
                            double& zMin, double& zMax, 
                            const vector<MVector>& Positions);

  //! Return an array usable for ROOT histograms
  //! The user must delete the array!
  static double* CreateAxisBins(double Min, double Max, int Bins, bool IsLog);

  static double GetFWHM(TF1* Function, double Min = 0, double Max = 0);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The geometry
  MDGeometryQuest* m_Geometry;
 
  //! True, if the GUI should be used
  bool m_UseGui;

  // The GUI itself
  MGUIMain* m_Gui;

  //! Basic GUI data (including geometry)
  MSettings* m_BasicGuiData;

  // private members:
 private:
  

#ifdef ___CLING___
 public:
  ClassDef(MInterface, 0) // the interface: connection class between the GUI and the library (i.e. the real MEGAlib)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
