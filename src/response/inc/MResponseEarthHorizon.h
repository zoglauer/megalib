/*
 * MResponseEarthHorizon.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEarthHorizon__
#define __MResponseEarthHorizon__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEarthHorizon : public MResponseBase
{
  // public interface:
 public:
  MResponseEarthHorizon();
  virtual ~MResponseEarthHorizon();

  //! Set and verify the revan configuration file name
  bool SetMimrecConfigurationFileName(const MString FileName);
  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();


  // protected methods:
 protected:
  //MResponseEarthHorizon() {};
  //MResponseEarthHorizon(const MResponseEarthHorizon& ResponseEarthHorizon) {};

  //! Load the simulation file:
  virtual bool OpenSimulationFile();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Mimrec configuration file name
  MString m_MimrecCfgFileName;
  //! Revan configuration file name
  MString m_RevanCfgFileName;



#ifdef ___CINT___
 public:
  ClassDef(MResponseEarthHorizon, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
