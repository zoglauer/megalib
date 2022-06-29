/*
 * MResponseImagingEfficiencyNearField.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingEfficiencyNearField__
#define __MResponseImagingEfficiencyNearField__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixON.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Create the near-field efficiencies for list-mode imaging
class MResponseImagingEfficiencyNearField : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseImagingEfficiencyNearField();
  //! Default destructor
  virtual ~MResponseImagingEfficiencyNearField();
  
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
  //! Minimum X range
  double m_XMin;
  //! Maximum X range
  double m_XMax;
  //! Number of X bins
  double m_XBins;
  
  //! Minimum Y range
  double m_YMin;
  //! Maximum Y range
  double m_YMax;
  //! Number of Y bins
  double m_YBins;
  
  //! Minimum Z range
  double m_ZMin;
  //! Maximum Z range
  double m_ZMax;
  //! Number of Z bins
  double m_ZBins;
  
  //! Number of energy bins
  unsigned int m_EnergyNBins;
  //! Minimum energy range
  double m_EnergyMinimum;
  //! Maximum energy range
  double m_EnergyMaximum;
  //! Energy bin edges
  vector<double> m_EnergyBinEdges;
 
   //! The bin width of the angles near the equator for the gamma rays
  double m_AngleBinWidth;

 
  //! The basic efficiency matrix:
  //! Probability that a gamma-ray emitted in a given image bin is detected 
  MResponseMatrixON m_ResponseEmittedXDetectedAnywhere;

  //! The partial response for list-mode imaging:
  //! Probability that a gamma-ray emitted in a given image bin is detected with that first interaction position
  MResponseMatrixON m_ResponseEmittedXDetectedY;

  //! The partial response for list-mode imaging:
  //! Probability that a gamma-ray detected with that first interaction position, has a scatter direction Z
  MResponseMatrixON m_ResponseDetectedYScatteredCDS;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseImagingEfficiencyNearField, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
