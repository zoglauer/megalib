/*
 * MResponseClusteringDSS.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseClusteringDSS__
#define __MResponseClusteringDSS__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseClusteringDSS : public MResponseBase
{
  // public interface:
 public:
  MResponseClusteringDSS();
  virtual ~MResponseClusteringDSS();

  //! Set and verify the revan configuration file name
  bool SetRevanConfigurationFileName(const MString FileName);

  //! Do all the response creation
  virtual bool CreateResponse();


  // protected methods:
 protected:
  //MResponseClusteringDSS() {};
  //MResponseClusteringDSS(const MResponseClusteringDSS& ResponseClusteringDSS) {};

  //! Load the simulation file:
  virtual bool OpenSimulationFile();

  //! Check if the first ID has the second one as ancestor
  bool HasAsAncestor(int ID, int AncestorID);
  
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
  ClassDef(MResponseClusteringDSS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
