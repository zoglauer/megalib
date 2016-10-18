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
#include "MResponseBuilder.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Build a response for clustering of double-sided strip detectors
class MResponseClusteringDSS : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseClusteringDSS();
  //! Default destructor
  virtual ~MResponseClusteringDSS();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();


  // protected methods:
 protected:
  //! Save the response matrices
  virtual bool Save();

  //! Check if the first ID has the second one as ancestor
  bool HasAsAncestor(int ID, int AncestorID);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! Times a cluster with these parameters is separable
  MResponseMatrixO2 m_SeparableYes;
  //! Times a cluster with these parameters is NOT separable
  MResponseMatrixO2 m_SeparableNo;
  //! Times a cluster with two hits and these parameters is separable
  MResponseMatrixO3 m_DualSeparableYes;
  //! Times a cluster with two hits and these parameters is NOT separable
  MResponseMatrixO3 m_DualSeparableNo;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseClusteringDSS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
