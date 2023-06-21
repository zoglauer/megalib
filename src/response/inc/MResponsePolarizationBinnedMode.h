/*
 * MResponsePolarizationBinnedMode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponsePolarizationBinnedMode__
#define __MResponsePolarizationBinnedMode__


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


class MResponsePolarizationBinnedMode : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponsePolarizationBinnedMode();
  //! Default destructor
  virtual ~MResponsePolarizationBinnedMode();
  
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
  //! The bin width of the angles near the equator for the gamma rays
  double m_AngleBinWidth;
  //! The mode with which to bin the sky coordiantes: fisbel or healpix
  MString m_AngleBinMode;
  //! The bin width of the angles near the equator for the recoil electron (360 for no electron tracking)
  double m_AngleBinWidthElectron;
  //! Number of energy bins
  unsigned int m_EnergyNBins;
  //! Minimum energy range
  double m_EnergyMinimum;
  //! Maximum energy range
  double m_EnergyMaximum;
  //! Energy bin edges
  vector<double> m_EnergyBinEdges;
  //! Number of distance bins
  unsigned int m_DistanceNBins;
  //! Minimum distance range
  double m_DistanceMinimum;
  //! Maximum energy range
  double m_DistanceMaximum;
  //! Number of distance bins
  unsigned int m_PolarizationAngleNBins;
  
  //! The Polarization response 
  MResponseMatrixON m_PolarizationResponse;
  
  //! Use an absorption factor
  bool m_UseAtmosphericAbsorption;
  //! An atmospheric absorption factor
  MAtmosphericAbsorption m_AtmosphericAbsorption;
  //! The name of the atmospheric absorption data file
  MString m_AtmosphericAbsorptionFileName;
  //! The altitude used for the atmospheric absorption
  double m_Altitude;
    
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponsePolarizationBinnedMode, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
