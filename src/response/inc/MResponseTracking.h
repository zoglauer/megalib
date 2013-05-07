/*
 * MResponseTracking.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseTracking__
#define __MResponseTracking__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseTracking : public MResponseMultipleCompton
{
  // public interface:
 public:
  MResponseTracking();
  virtual ~MResponseTracking();

  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();


  // protected methods:
 protected:
  //MResponseTracking() {};
  //MResponseTracking(const MResponseTracking& ResponseTracking) {};

  //! Load the simulation file:
  virtual bool OpenSimulationFile();

  double CalculateAngleIn(MRESE& Start, MRESE& Central);
  double CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Revan configuration file name
  MString m_RevanCfgFileName;



#ifdef ___CINT___
 public:
  ClassDef(MResponseTracking, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
