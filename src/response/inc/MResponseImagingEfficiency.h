/*
 * MResponseImagingEfficiency.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingEfficiency__
#define __MResponseImagingEfficiency__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingEfficiency : public MResponseBase
{
  // public interface:
 public:
  MResponseImagingEfficiency();
  virtual ~MResponseImagingEfficiency();

  //! Set and verify the revan configuration file name
  bool SetMimrecConfigurationFileName(const MString FileName);
  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingEfficiency() {};
  //MResponseImagingEfficiency(const MResponseImagingEfficiency& ResponseImagingEfficiency) {};

  //! Load the simulation file:
  virtual bool OpenSimulationFile();

  // private methods:
 private:



  // protected members:
 protected:


  //! Mimrec configuration file name
  MString m_MimrecCfgFileName;
  //! Revan configuration file name
  MString m_RevanCfgFileName;
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseImagingEfficiency, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
