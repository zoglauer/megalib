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
  m_Mode = MCalibrateEnergyAssignEnergyModes::e_Linear;
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

class SetOfMatches 
{
public:
  SetOfMatches() {};
  bool IsotopeLineUsed(unsigned int m_Isotope, unsigned int m_Line) {
    for (auto M: m_Matches) {
      if (M.m_Isotope == m_Isotope && M.m_Line == m_Line) return true;
    }
    return false;
  }
  void Add(Match& M) { m_Matches.push_back(M); }

  vector<Match> m_Matches;
  double m_QualityFactor;
  
  // as in y = m*x + t;
  double m_t;
  double m_m;
};


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyAssignEnergies::Calibrate()
{ 
  if (m_Mode == MCalibrateEnergyAssignEnergyModes::e_LinearZeroCrossing) {
    return CalibrateLinearZeroCrossing();
  } else {
    return CalibrateLinear();  
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyAssignEnergies::CalibrateLinear()
{ 
  // Create a list of spectral points
  vector<SetOfMatches> SetsOfMatches;
  
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      vector<Match> ListOfMatches;
      for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
        for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
          ListOfMatches.push_back(Match(r, p, i, l));
        }
      }
      vector<SetOfMatches> NewSetsOfMatches;
      if (SetsOfMatches.size() > 0) {
        for (unsigned int s = 0; s < SetsOfMatches.size(); ++s) {
          for (unsigned int l = 0; l < ListOfMatches.size(); ++l) {
            if (SetsOfMatches[s].IsotopeLineUsed(ListOfMatches[l].m_Isotope, ListOfMatches[l].m_Line) == false) {
              SetOfMatches NewSet = SetsOfMatches[s];
              NewSet.Add(ListOfMatches[l]);
              NewSetsOfMatches.push_back(NewSet);
            }
          }
        }
      } else {
        for (unsigned int l = 0; l < ListOfMatches.size(); ++l) {
          SetOfMatches NewSet;
          NewSet.Add(ListOfMatches[l]);
          NewSetsOfMatches.push_back(NewSet);
        }
      }
      SetsOfMatches = NewSetsOfMatches;
    }
  }
  
  if (SetsOfMatches.size() == 0) return false;
  
  
  // Calculate the quality factor:
  for (unsigned int s = 0; s < SetsOfMatches.size(); ++s) {
    double QualityFactor = 0.0;
    
   // Create a set of MCalibrationSpectralPoint's
    vector<MCalibrationSpectralPoint> Points;
    for (unsigned int m = 0; m < SetsOfMatches[s].m_Matches.size(); ++m) {
      unsigned int r = SetsOfMatches[s].m_Matches[m].m_ROG;
      unsigned int p = SetsOfMatches[s].m_Matches[m].m_Point;

      unsigned int i = SetsOfMatches[s].m_Matches[m].m_Isotope;
      unsigned int l = SetsOfMatches[s].m_Matches[m].m_Line;

      
      MCalibrationSpectralPoint P = m_Results.GetSpectralPoint(r, p);
      P.SetEnergy(m_Isotopes[r][i].GetLineEnergy(l));
    
      Points.push_back(P);
    }
    
    // Do the fitting
    MCalibrationModelPoly1 Model;
    QualityFactor = Model.Fit(Points);
    
    SetsOfMatches[s].m_QualityFactor = QualityFactor;
    SetsOfMatches[s].m_t = Model.GetParameter(0);
    SetsOfMatches[s].m_m = Model.GetParameter(1);
    
    //for (auto P: Points) cout<<P.GetPeak()<<" -> "<<P.GetEnergy()<<"  ";
    //cout<<" ---------> "<<QualityFactor<<endl;
    
  }
    
    
  // Find the best (=smallest) quality factor
  unsigned int BestMatch = 0;
  double BestMatchQualityFactor = SetsOfMatches[BestMatch].m_QualityFactor;
  for (unsigned int m = 1; m < SetsOfMatches.size(); ++m) {
    if (SetsOfMatches[m].m_QualityFactor < BestMatchQualityFactor) {
      BestMatchQualityFactor = SetsOfMatches[m].m_QualityFactor;
      BestMatch = m;
    }
  }
    
  // Finally do the assignment:
  for (unsigned int m = 0; m < SetsOfMatches[BestMatch].m_Matches.size(); ++m) {
    unsigned int r = SetsOfMatches[BestMatch].m_Matches[m].m_ROG;
    unsigned int p = SetsOfMatches[BestMatch].m_Matches[m].m_Point;

    unsigned int i = SetsOfMatches[BestMatch].m_Matches[m].m_Isotope;
    unsigned int l = SetsOfMatches[BestMatch].m_Matches[m].m_Line;
    
    m_Results.GetSpectralPoint(r, p).SetIsotope(m_Isotopes[r][i]);
    m_Results.GetSpectralPoint(r, p).SetEnergy(m_Isotopes[r][i].GetLineEnergy(l)); 
    m_Results.GetSpectralPoint(r, p).IsGood(!m_Isotopes[r][i].GetLineExcludeFlag(l));
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
            if (g_Verbosity >= c_Info) cout<<"Assign energies: Rejecting peak because it was found twice: "<<m_Results.GetSpectralPoint(r, p)<<endl;
            //FoundNotGood = true;
            break;
          } else {
            m_Results.GetSpectralPoint(r, q).IsGood(false);  
            if (g_Verbosity >= c_Info) cout<<"Assign energies: Rejecting peak because it was found twice: "<<m_Results.GetSpectralPoint(r, q)<<endl;
            //FoundNotGood = true;
          }
        }
      }
    }
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyAssignEnergies::CalibrateLinearZeroCrossing()
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
            if (g_Verbosity >= c_Info) cout<<"Assign energies: Rejecting peak because it was found twice: "<<m_Results.GetSpectralPoint(r, p)<<endl;
            //FoundNotGood = true;
            break;
          } else {
            m_Results.GetSpectralPoint(r, q).IsGood(false);  
            if (g_Verbosity >= c_Info) cout<<"Assign energies: Rejecting peak because it was found twice: "<<m_Results.GetSpectralPoint(r, q)<<endl;
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
