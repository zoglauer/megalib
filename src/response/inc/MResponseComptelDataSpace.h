/*
 * MResponseComptelDataSpace.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseComptelDataSpace__
#define __MResponseComptelDataSpace__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixON.h"
#include "MAtmosphericAbsorption.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseComptelDataSpace : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseComptelDataSpace();
  //! Default destructor
  virtual ~MResponseComptelDataSpace();
  
  //! Return a brief description of this response class
  static MString Description();
  //! Return information on the parsable options for this response class
  static MString Options();
  //! Parse the options
  virtual bool ParseOptions(const MString& Options);
  
  
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
  
  // private methods:
 private:

  //! Initialize MRotations to rotate source position to (0,0) for all det coords
  bool InitializeSourcePositions();
	//! Round double to precision P
	double RoundF(double N, int P);



  // protected members:
 protected:
  //! The size of the regions in the Comptel data space
  double m_CDS_ARM;
  //! Number of energy bins
  unsigned int m_EnergyNBins;
  //! Minimum energy range
  double m_EnergyMinimum;
  //! Maximum energy range
  double m_EnergyMaximum;
  //! name of background probability file for physical background
  MString m_BkgProbabilityFileName;
  //! use probability file: i.e. change sample from flat spectrum
  bool m_UseBkgProbability;

  //! The Comptel data space response 
  MResponseMatrixON m_CDSResponse;
  //! Ling probability response
  MResponseMatrixON m_LingProbability;

  //! List of rotations for all source positions
  vector<MRotation> m_RTo00;
  //! MRotation to rotate cone into plane
  MRotation m_R2;
 
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseComptelDataSpace, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
