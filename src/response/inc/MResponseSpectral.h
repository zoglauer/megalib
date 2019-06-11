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
#include "MResponseBuilder.h"
#include "MResponseMatrixON.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseSpectral : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseSpectral();
  //! Default destructor
  virtual ~MResponseSpectral();
  
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



  // protected members:
 protected:
   //! Underflow energy minimum
   double m_EnergyUnderflow;
   //! Minimum energy range
   double m_EnergyMinimum;
   //! Maximum energy range
   double m_EnergyMaximum;
   //! Overflow energy maxmium
   double m_EnergyOverflow;
   //! Number of bins excluding under- and overflow
   unsigned int m_EnergyNumberOfBins;
   //! The bin edges
   vector<double> m_EnergyBinEdges;
   //! Number of sky bins
   unsigned int m_EnergyNumberOfSkyBins;
   
   //! The ARM cut range
   double m_ARMCut;
   //! The acceptance radius around the ARM cut bin center from which the photons are allowed to have started
   double m_ARMCutOriginAcceptanceRadius;
   //! The ARM cut sky bins
   unsigned int m_ARMCutNumberOfSkyBins;
   
   
  MResponseMatrixON m_EnergyBeforeER;
  MResponseMatrixON m_EnergyUnselected;
  MResponseMatrixON m_EnergySelected;
 
  MResponseMatrixON m_EnergyRatioBeforeER;
  MResponseMatrixON m_EnergyRatioUnselected;
  MResponseMatrixON m_EnergyRatioSelected;
  
  MResponseMatrixON m_EnergySelectedARMCut;
  MResponseMatrixON m_EnergySelectedARMCutOriginRestricted; 
 
  //! The bin centers for the ARM cut
  vector<MVector> m_BinCenters;
  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseSpectral, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
