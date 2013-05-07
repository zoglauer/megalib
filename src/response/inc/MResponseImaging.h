/*
 * MResponseImaging.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImaging__
#define __MResponseImaging__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImaging : public MResponseBase
{
  // public interface:
 public:
  MResponseImaging();
  virtual ~MResponseImaging();

  //! Set and verify the revan configuration file name
  bool SetMimrecConfigurationFileName(const MString FileName);
  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImaging() {};
  //MResponseImaging(const MResponseImaging& ResponseImaging) {};

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
  ClassDef(MResponseImaging, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
