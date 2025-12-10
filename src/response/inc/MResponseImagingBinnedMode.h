/*
 * MResponseImagingBinnedMode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingBinnedMode__
#define __MResponseImagingBinnedMode__


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


//! Create a binned-mode imaging reponse
class MResponseImagingBinnedMode : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseImagingBinnedMode();
  //! Default destructor
  virtual ~MResponseImagingBinnedMode();
  
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
  //! Use relative coordinate parametrization
  MString m_ParametrizationMode;
  //! Number of epsilon bins
  unsigned int m_EpsilonNBins;
  //! The mode how to do the epsilon binning: lin or log
  MString m_EpsilonBinMode;
  //! Minimum epsilon range
  double m_EpsilonMinimum;
  //! Maximum epsilon range
  double m_EpsilonMaximum;
  //! Epsilon bin edges
  vector<double> m_EpsilonBinEdges;
  //! Phi bin edges
  vector<double> m_PhiBinEdges;
  //! Theta bin edges
  vector<double> m_ThetaBinEdges;
  //! Zeta bin edges
  vector<double> m_ZetaBinEdges;
  //! The bin width of the angles near the equator for the gamma rays
  double m_AngleBinWidth;
  //! The mode on how to do the spherical coordinates binning: fisbel or healpix
  MString m_AngleBinMode;
  //! The bin width of the angles near the equator for the recoil electron (360 for no electron tracking)
  double m_AngleBinWidthElectron;
  //! Number of energy bins
  unsigned int m_EnergyNBins;
  //! The mode how to do the spherical coordinates binning: lin or log
  MString m_EnergyBinMode;
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
  
  //! The imaging response 
  MResponseMatrixON m_ImagingResponse;
  //! The exposure
  MResponseMatrixON m_Exposure;
  //! The 4D energy response 
  MResponseMatrixON m_EnergyResponse4D;
  //! The 2D energy response 
  MResponseMatrixON m_EnergyResponse2D;
  
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
  ClassDef(MResponseImagingBinnedMode, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
