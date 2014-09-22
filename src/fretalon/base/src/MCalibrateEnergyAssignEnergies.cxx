/*
 * MCalibrateEnergyAssignEnergies.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MCalibrateEnergyAssignEnergies.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TF1.h"
#include "TMath.h"
#include "TBox.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MCalibrationSpectralPoint.h"
#include "MReadOutDataADCValue.h"
#include "MCalibrationFit.h"
#include "MCalibrationFitGaussian.h"
#include "MCalibrationFitGaussLandau.h"
#include "MCalibrationModel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrateEnergyAssignEnergies)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateEnergyAssignEnergies::MCalibrateEnergyAssignEnergies() : MCalibrateEnergy()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrateEnergyAssignEnergies::~MCalibrateEnergyAssignEnergies()
{
}


////////////////////////////////////////////////////////////////////////////////


class Match 
{
public:
  Match(unsigned int ROG, unsigned int Point,  unsigned int Isotope, unsigned int Line) {
    m_ROG = ROG;
    m_Point = Point;
    m_Isotope = Isotope;
    m_Line = Line;
  }
  unsigned int m_ROG;
  unsigned int m_Point;
  unsigned int m_Isotope;
  unsigned int m_Line;
  double m_Conversion;
  double m_QualityFactor;
};


//! Perform the calibration
bool MCalibrateEnergyAssignEnergies::Calibrate()
{ 
  vector<Match> Matches;
    
  // Create all combinations
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
        for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
          Match M(r, i, p, l);
          M.m_Conversion = m_Isotopes[r][i].GetLineEnergy(l)/m_Results.GetSpectralPoint(r, p).GetPeak();
          Matches.push_back(M);
        }
      }
    }
  }
  
  if (Matches.size() == 0) return false;
  
  // Calculate the quality factor:
  for (unsigned int m = 0; m < Matches.size(); ++m) {
    double QualityFactor = 0.0;
    int QualityFactorCounter = 0.;
    for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
      for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
        if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
        double Energy = m_Results.GetSpectralPoint(r, p).GetPeak() * Matches[m].m_Conversion;
        
        // Find closest line:
        unsigned int closest_i = 0;
        unsigned int closest_l = 0;
        double Difference = numeric_limits<double>::max();
        for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
          for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
            if (fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l)) < Difference) {
              Difference = fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l));
              closest_i = i;
              closest_l = l;
            }
          }
        }
        
        QualityFactor += 
          0.001/(m_Isotopes[r][closest_i].GetLineEnergy(closest_l) * m_Isotopes[r][closest_i].GetLineBranchingRatio(closest_l)) + fabs(Energy - m_Isotopes[r][closest_i].GetLineEnergy(closest_l))/m_Isotopes[r][closest_i].GetLineEnergy(closest_l);
        QualityFactorCounter++;
        //cout<<m_Isotopes[r][closest_i].GetLineEnergy(closest_l)<<": "<<QualityFactor<<endl;
      }
    }
    Matches[m].m_QualityFactor = QualityFactor/QualityFactorCounter;
  }
    
    
    
    
  // Find the best (=smallest) quality factor
  unsigned int BestMatch = 0;
  double BestMatchQualityFactor = Matches[BestMatch].m_QualityFactor;
  for (unsigned int m = 1; m < Matches.size(); ++m) {
    if (Matches[m].m_QualityFactor < BestMatchQualityFactor) {
      BestMatchQualityFactor = Matches[m].m_QualityFactor;
      BestMatch = m;
    }
  }
    
  // Finally do the assignment:  
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      double Energy = m_Results.GetSpectralPoint(r, p).GetPeak() * Matches[BestMatch].m_Conversion;
        
      // Find closest line:
      unsigned int closest_i = 0;
      unsigned int closest_l = 0;
      double Difference = numeric_limits<double>::max();
      for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
        for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
          if (fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l)) < Difference) {
            Difference = fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l));
            closest_i = i;
            closest_l = l;
          }
        }
      }
      m_Results.GetSpectralPoint(r, p).SetIsotope(m_Isotopes[r][closest_i]);
      m_Results.GetSpectralPoint(r, p).SetEnergy(m_Isotopes[r][closest_i].GetLineEnergy(closest_l)); 
      m_Results.GetSpectralPoint(r, p).IsGood(!m_Isotopes[r][closest_i].GetLineExcludeFlag(closest_l));
    }
  }

  // If one peak is used twice - throw out the one with lower count rate
  //bool FoundNotGood = false;
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      double Energy = m_Results.GetSpectralPoint(r, p).GetEnergy();
      for (unsigned int q = p+1; q < m_Results.GetNumberOfSpectralPoints(r); ++q) {
        if (m_Results.GetSpectralPoint(r, q).IsGood() == false) continue;
        if (m_Results.GetSpectralPoint(r, q).GetEnergy() == Energy) {
          if (m_Results.GetSpectralPoint(r, p).GetCounts() < m_Results.GetSpectralPoint(r, q).GetEnergy()) {
            m_Results.GetSpectralPoint(r, p).IsGood(false);
            if (g_Verbosity >= c_Info) cout<<"Bad: "<<m_Results.GetSpectralPoint(r, p)<<endl;
            //FoundNotGood = true;
            break;
          } else {
            m_Results.GetSpectralPoint(r, q).IsGood(false);  
            if (g_Verbosity >= c_Info) cout<<"Bad: "<<m_Results.GetSpectralPoint(r, q)<<endl;
            //FoundNotGood = true;
          }
        }
      }
    }
  }

  return true;
}


// MCalibrateEnergyAssignEnergies.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
