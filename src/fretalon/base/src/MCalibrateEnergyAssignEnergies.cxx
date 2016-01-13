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
  Match() {};
  Match(unsigned int ROG, unsigned int Point,  unsigned int Isotope, unsigned int Line) {
    m_ROG = ROG;
    m_Point = Point;
    m_Isotope = Isotope;
    m_Line = Line;
  }
    
  Match(unsigned int ROG, unsigned int Point, double Peak, unsigned int Isotope, unsigned int Line, double Energy) {
    m_ROG = ROG;
    m_Point = Point;
    m_Peak = Peak;
    m_Isotope = Isotope;
    m_Line = Line;
    m_Energy = Energy;
  }
    
  MString ToString() {
    MString Out;
    Out += "(";
    Out += m_Peak;
    Out += " rou --> ";
    Out += m_Energy;
    Out += "keV)"; 
    return Out;
  }
  
  unsigned int m_ROG;
  unsigned int m_Point;
  unsigned int m_Isotope;
  unsigned int m_Line;
  double m_Conversion;
  double m_QualityFactor;
  
  double m_Peak;
  double m_Energy;
};

class SetOfMatches 
{
public:
  SetOfMatches() {};
  bool IsotopeLineUsed(unsigned int ROG, unsigned int Isotope, unsigned int Line) {
    for (auto M: m_Matches) {
      if (M.m_ROG == ROG && M.m_Isotope == Isotope && M.m_Line == Line) return true;
    }
    return false;
  }
  // Return false if the newly added point would result in a negative incline 
  void Add(Match& M) { m_Matches.push_back(M); }

  // Return false if the newly added point would result in a negative incline 
  bool AddWithTest(Match& M) {
    bool AllIncreasing = true;
    double Energy1 = M.m_Energy;
    double Peak1 = M.m_Peak;
    for (unsigned int m = 0; m < m_Matches.size(); ++m) {
      double Energy2 = m_Matches[m].m_Energy;
      double Peak2 = m_Matches[m].m_Peak;
      if (Energy1 > Energy2) { // We do not care about equal because in case they are the same line there will be differences
        if (Peak1 < Peak2) {
          AllIncreasing = false;
          break;
        }
      } else if (Energy1 < Energy2) { 
        if (Peak1 > Peak2) {
          AllIncreasing = false;
          break;
        }
      }             
    }
    if (AllIncreasing == false) {
      return false;
    }
    
    m_Matches.push_back(M); 
    return true;
  }
  
  MString ToString() {
    MString Out;
    for (auto M: m_Matches) {
      Out += M.ToString() + " ";
    }
    return Out;
  }
  
  vector<Match> m_Matches;
  double m_QualityFactor;
  
  // as in y = m*x + t;
  double m_t;
  double m_m;
};


void CreateSubSetOfMatches(vector<Match> FirstMatches, vector<Match> SecondMatches, SetOfMatches& CurrentMatches, vector<SetOfMatches>& FinalMatches)
{
  if (FirstMatches.size() == 0 || SecondMatches.size() == 0) {
    FinalMatches.push_back(CurrentMatches);
    return;
  }
  
  vector<Match> NewFirstMatches = FirstMatches;
  NewFirstMatches.erase(NewFirstMatches.begin());
  
  for (unsigned int l2 = 0; l2 < SecondMatches.size(); ++l2) {
    // Fill the initial new element -- and take care of the swap
    Match Start;
    Start.m_ROG = (FirstMatches[0].m_ROG != numeric_limits<unsigned int>::max() ? FirstMatches[0].m_ROG : SecondMatches[l2].m_ROG);
    Start.m_Point = (FirstMatches[0].m_Point != numeric_limits<unsigned int>::max() ? FirstMatches[0].m_Point : SecondMatches[l2].m_Point);
    Start.m_Peak = (FirstMatches[0].m_Peak != numeric_limits<double>::max() ? FirstMatches[0].m_Peak : SecondMatches[l2].m_Peak);
    Start.m_Isotope = (SecondMatches[l2].m_Isotope != numeric_limits<unsigned int>::max() ? SecondMatches[l2].m_Isotope : FirstMatches[0].m_Isotope);
    Start.m_Line = (SecondMatches[l2].m_Line != numeric_limits<unsigned int>::max() ? SecondMatches[l2].m_Line : FirstMatches[0].m_Line);
    Start.m_Energy = (SecondMatches[l2].m_Energy != numeric_limits<double>::max() ? SecondMatches[l2].m_Energy : FirstMatches[0].m_Energy);
    
    SetOfMatches NewCurrentMatches = CurrentMatches;
    NewCurrentMatches.Add(Start);
    
    vector<Match> NewSecondMatches = SecondMatches;
    NewSecondMatches.erase(NewSecondMatches.begin() + l2);
    
    CreateSubSetOfMatches(NewFirstMatches, NewSecondMatches, NewCurrentMatches, FinalMatches);
  }
}


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
  if (g_Verbosity >= c_Info) cout<<endl<<"Assigning energies via linear method"<<endl;
  
  // Inventory:
  if (g_Verbosity >= c_Info) {
    cout<<"Current line inventory: "<<endl;
    for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
      for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
        if (m_Results.GetSpectralPoint(r, p).IsGood() == false) {
          cout<<"Excluded: "<<m_Results.GetSpectralPoint(r, p)<<endl;
        } else {
          cout<<"Included: "<<m_Results.GetSpectralPoint(r, p)<<endl;
        }
      }
    }
  }
      
      
  // Create a list of spectral points
  vector<SetOfMatches> SetsOfMatches;
  
  // For each calibration file group
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    // (1) Create a subset of matches
    vector<Match> FirstMatches;
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      Match M(r, p, m_Results.GetSpectralPoint(r, p).GetPeak(), numeric_limits<unsigned int>::max(), numeric_limits<unsigned int>::max(), numeric_limits<double>::max());
      FirstMatches.push_back(M);
      //cout<<"First match "<<p<<": "<<m_Results.GetSpectralPoint(r, p).GetPeak()<<" rou's"<<endl;
    }
    vector<Match> SecondMatches;  
    for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
      for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
        Match M(numeric_limits<unsigned int>::max(), numeric_limits<unsigned int>::max(), numeric_limits<double>::max(), i, l, m_Isotopes[r][i].GetLineEnergy(l));
        SecondMatches.push_back(M);
        //cout<<"Second match: "<<m_Isotopes[r][i].GetLineEnergy(l)<<" keV"<<endl;
      }
    }
  
    vector<SetOfMatches> SubSetsOfMatches;
    SetOfMatches CurrentSetOfMatches;
    
    if (FirstMatches.size() < SecondMatches.size()) {
      CreateSubSetOfMatches(FirstMatches, SecondMatches, CurrentSetOfMatches, SubSetsOfMatches);
    } else {
      CreateSubSetOfMatches(SecondMatches, FirstMatches, CurrentSetOfMatches, SubSetsOfMatches);
    }
    
    //cout<<"ROG "<<r<<": SubSet of matches: "<<endl;
    //for (auto S: SubSetsOfMatches) cout<<S.ToString()<<endl;
    
    
    // (2) Merge the subset of matches in, if all point fit into the increasing ADC vs energy plot theme:
    if (SetsOfMatches.size() > 0) {
      vector<SetOfMatches> NewSetsOfMatches;
      for (auto OldSet: SetsOfMatches) {
        for (auto SubSet: SubSetsOfMatches) {
          SetOfMatches NewSet = OldSet;
          bool AllIncreasing = true;
          for (unsigned int m = 0; m < SubSet.m_Matches.size(); ++m) {
            if ((AllIncreasing = NewSet.AddWithTest(SubSet.m_Matches[m])) == false) {
               break;
            }
          }
          if (AllIncreasing == true) {
            NewSetsOfMatches.push_back(NewSet);
            //cout<<"Increasing: "<<OldSet.ToString()<<" with "<<SubSet.ToString()<<endl; 
          } else {
            //cout<<"Not increasing: "<<OldSet.ToString()<<" with "<<SubSet.ToString()<<endl; 
          }
        }
      }
      if (NewSetsOfMatches.size() != 0) {
        SetsOfMatches = NewSetsOfMatches;
      } else {
        cout<<"Error something went wrong with the latest sets of matches since we would now have less matches (old: "<<SetsOfMatches.size()<<"  new: "<<NewSetsOfMatches.size()<<") ... Excluding a ton of lines"<<endl; 
      }
    } else {
      for (auto SubSet: SubSetsOfMatches) {
        SetOfMatches NewSet;
        bool AllIncreasing = true;
        for (unsigned int m = 0; m < SubSet.m_Matches.size(); ++m) {
          if ((AllIncreasing = NewSet.AddWithTest(SubSet.m_Matches[m])) == false) break;
        }
        if (AllIncreasing == true) {
          SetsOfMatches.push_back(NewSet);
        }
      }
    }
    
    //cout<<"ROG "<<r<<": Matches: "<<endl;
    //for (auto S: SetsOfMatches) cout<<S.ToString()<<endl;
  }
  
  
  if (g_Verbosity >= c_Info) {
    cout<<"Investigating "<<SetsOfMatches.size()<<" peak-line combos..."<<endl;
    for (auto S: SetsOfMatches) cout<<S.ToString()<<endl;
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
    
    if (g_Verbosity >= c_Info) {
      for (auto P: Points) cout<<P.GetPeak()<<" -> "<<P.GetEnergy()<<"  ";
      cout<<" ---------> "<<QualityFactor<<endl;
    }
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
  
  // (a) Set all to false, so that we ignore lines not included in the above matching
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      m_Results.GetSpectralPoint(r, p).IsGood(false);
    }
  }
  
  
  // (b)
  for (unsigned int m = 0; m < SetsOfMatches[BestMatch].m_Matches.size(); ++m) {
    unsigned int r = SetsOfMatches[BestMatch].m_Matches[m].m_ROG;
    unsigned int p = SetsOfMatches[BestMatch].m_Matches[m].m_Point;

    unsigned int i = SetsOfMatches[BestMatch].m_Matches[m].m_Isotope;
    unsigned int l = SetsOfMatches[BestMatch].m_Matches[m].m_Line;
    
    m_Results.GetSpectralPoint(r, p).SetIsotope(m_Isotopes[r][i]);
    m_Results.GetSpectralPoint(r, p).SetEnergy(m_Isotopes[r][i].GetLineEnergy(l)); 
    m_Results.GetSpectralPoint(r, p).IsGood(!m_Isotopes[r][i].GetLineExcludeFlag(l));
    if (g_Verbosity >= c_Info) {
      cout<<"Assign energies: "<<m_Isotopes[r][i].GetLineEnergy(l)<<" keV == "<<m_Results.GetSpectralPoint(r, p).GetPeak()<<" rou's"<<endl;
      if (m_Isotopes[r][i].GetLineExcludeFlag(l) == true) {
        cout<<"Line excluded due to exclude flag: "<<m_Isotopes[r][i].GetLineEnergy(l)<<" keV"<<endl;  
      }
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
