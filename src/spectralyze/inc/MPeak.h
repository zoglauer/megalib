/*
 * MPeak.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPeak__
#define __MPeak__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MQualifiedIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPeak
{
  // public interface:
 public:
  //! Default constructor
  MPeak();
  //! Default destructor
  virtual ~MPeak();


  //! Set the energy of the peak
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  //! Return the peak energy
  double GetEnergy() const { return m_Energy; }

  //! Set the sigma of peak from Gaussian fit value
  void SetEnergySigma(const double EnergySigma) { m_EnergySigma = EnergySigma; }
  //! return the sigma of peak
  double GetEnergySigma() const { return m_EnergySigma; }
  
  //! Set the number of counts in the peak
  void SetPeakCounts(const double PeakCounts) { m_PeakCounts = PeakCounts; }
  //! Return the number of counts the peak
  double GetPeakCounts() const { return m_PeakCounts; }
  
  //! Set the number of counts in the background below the peak
  void SetBackgroundCounts(const double BackgroundCounts) { m_BackgroundCounts = BackgroundCounts; }
  //! Return the number of counts in the background below the peak
  double GetBackgroundCounts() const { return m_BackgroundCounts; }
  
  //! Get the signal to background ratio
  double GetSignalToBackgroundRatio() const { if (m_BackgroundCounts <= 0) return m_PeakCounts; else return m_PeakCounts/m_BackgroundCounts; }
  //! Get the significance: signal to sqrt(background)
  double GetSignificance() const { if (m_BackgroundCounts <= 0) return m_PeakCounts; else return m_PeakCounts/sqrt(m_BackgroundCounts); }
  
  //! Add candidate isotope:
  void AddIsotope(MQualifiedIsotope* I, unsigned int LineID);
  //! Get the number of stored isotope:
  unsigned int GetNIsotopes() const { return m_Isotopes.size(); }
  //! Get the isotope
  MQualifiedIsotope* GetIsotope(unsigned int i) const;
  //! Contains the isotope
  bool ContainsIsotope(MQualifiedIsotope* I) const;
  //! Remove the isotope
  void RemoveIsotope(MQualifiedIsotope* I);
  //! Get the isotope line ID
  unsigned int GetIsotopeLineID(unsigned int i) const;
  
  // protected methods:
 protected:
  //MPeak() {};
  //MPeak(const MPeak& Peak) {};

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! The peak energy
  double m_Energy;
  //! Sigma of peak from Gaussian fit
  double m_EnergySigma;
  //! Get the source counts below the peak
  double m_PeakCounts;
  //! Get the background counts below the peak
  double m_BackgroundCounts;

  //! A list of candidate isotopes
  vector<MQualifiedIsotope*> m_Isotopes;
  //! And their candidate line IDs
  vector<unsigned int> m_IsotopeLineIDs;
  
#ifdef ___CLING___
 public:
  ClassDef(MPeak, 0) // no description
#endif

};

//! Stream a peak
ostream& operator<<(ostream& os, const MPeak& Peak);

#endif


////////////////////////////////////////////////////////////////////////////////
