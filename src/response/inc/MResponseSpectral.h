/*
 * MResponseSpectral.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseSpectral__
#define __MResponseSpectral__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseSpectral : public MResponseBase
{
  // public interface:
 public:
  MResponseSpectral();
  virtual ~MResponseSpectral();

  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseSpectral() {};
  //MResponseSpectral(const MResponseSpectral& ResponseSpectral) {};

  //! Load the simulation file:
  virtual bool OpenSimulationFile();

  // private methods:
 private:



  // protected members:
 protected:
  //! Revan configuration file name
  MString m_RevanCfgFileName;
  
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseSpectral, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
