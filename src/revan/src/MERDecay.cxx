/*
 * MERDecay.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MERDecay
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERDecay.h"

// Standard libs:
#include <iostream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRERawEvent.h"
#include "MRESEIterator.h"
#include "MFileDecay.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERDecay)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MERDecay::c_AllCombinations        = 0;
const int MERDecay::c_AllButOneCombination   = 1;
const int MERDecay::c_ComptonSequenceEnd     = 2;


////////////////////////////////////////////////////////////////////////////////



MERDecay::MERDecay()
{
  // Construct an instance of MERDecay

  //m_Mode = c_AllCombinations;
  m_Mode = c_AllButOneCombination;
  //m_Mode = c_ComptonSequenceEnd;
}


////////////////////////////////////////////////////////////////////////////////


MERDecay::~MERDecay()
{
  // Delete this instance of MERDecay
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::SetParameters(MString FileName,
                             const vector<double>& Energy, 
                             const vector<double>& EnergyError)
{
  MFileDecay Decay;
  if (Decay.Open(FileName) == true) {
    m_Energy = Decay.GetEnergies();
    m_EnergyErrorSigma = Decay.GetEnergyErrors();    
  } else {
    return false;
  }
  
  massert(Energy.size() == EnergyError.size());
  
  for (unsigned int e = 0; e < Energy.size(); ++e) {
    m_Energy.push_back(Energy[e]);
    m_EnergyErrorSigma.push_back(EnergyError[e]);
  }
  
  mdebug<<"Decays: "<<FileName<<endl;
  for (unsigned int e = 0; e < m_Energy.size(); ++e) {
    mdebug<<"E = ("<<m_Energy[e]<<"+-"<<m_EnergyErrorSigma[e]<<") keV"<<endl;
  }

  m_Occupation.clear();
  m_Occupation.resize(m_Energy.size());

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::Analyze(MRawEventList* List)
{
  // Analyze the raw event...
  MERConstruction::Analyze(List);

  mout<<"Starting decay search..."<<endl;

  MRESEIterator Iter;
  MRERawEvent* RE = 0;
  double Energy = 0;
  double EnergyError = 0;

  for (int e = 0; e < m_List->GetNRawEvents(); ++e) {
    RE = m_List->GetRawEventAt(e);
    massert(RE != 0);

    if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {

      // Check all combinations - execpt the total event: 
      if (m_Mode == c_AllButOneCombination) {
        if (CheckCombinations(0, 0, 0, RE, 1) == true) {
          RE->SetDecay(true);
        }
      } 
      // Check if everything after the first hit is a typical decay product:
      else if (m_Mode == c_ComptonSequenceEnd) {
        if (RE->GetNRESEs() == 0) continue;
        if (RE->GetStartPoint() == 0) continue;
        
        Energy = RE->GetEnergy() - RE->GetStartPoint()->GetEnergy();
        EnergyError = sqrt(RE->GetEnergyResolution()*RE->GetEnergyResolution() - 
                           RE->GetStartPoint()->GetEnergy()*RE->GetStartPoint()->GetEnergy());

        if (IsDecay(Energy, EnergyError) == true) {
          RE->SetDecay(true);          
        }
      }
      else if (m_Mode == c_AllCombinations) {
        if (CheckCombinations(0, 0, 0, RE, 0) == true) {
          RE->SetDecay(true);
        }
      }     
      else {
        merr<<"Unknown decay search mode: "<<m_Mode<<endl;
      } 

    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::IsDecay(double Energy, double EnergyError)
{
  for (unsigned int i = 0; i < m_Energy.size(); ++i) {
    if (fabs(Energy - m_Energy[i]) < m_EnergyErrorSigma[i]*EnergyError) {
      m_Occupation[i]++;
      mdebug<<"Found decay #"<<m_Occupation[i]<<": measured:"<<Energy
            <<" line:"<<m_Energy[i]<<"+-"<<m_EnergyErrorSigma[i]<<"*"<<EnergyError<<endl;
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::CheckCombinations(double Energy, double EnergyError, 
                                 int Start, MRERawEvent* RE, int Level)
{
  massert(Start >= 0);
  massert(Start < RE->GetNRESEs());

  if (Level == RE->GetNRESEs()) {
    return false;
  }

  for (int i = Start; i < RE->GetNRESEs(); ++i) {
    mdebug<<"Testing decay energy: "<<Energy+RE->GetRESEAt(i)->GetEnergy()<<endl;
    if (IsDecay(Energy+RE->GetRESEAt(i)->GetEnergy(), 
                sqrt(EnergyError*EnergyError + RE->GetRESEAt(i)->GetEnergyResolution()*RE->GetRESEAt(i)->GetEnergyResolution())) == true) {
      return true;
    }
  }

  for (int i = Start; i < RE->GetNRESEs()-1; ++i) {
    if (CheckCombinations(Energy + RE->GetRESEAt(i)->GetEnergy(), 
                          sqrt(EnergyError*EnergyError + RE->GetRESEAt(i)->GetEnergyResolution()*RE->GetRESEAt(i)->GetEnergyResolution()),
                          i+1, RE, Level+1) == true) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::PreAnalysis()
{
  // Do some analysis before the real analysis starts
  
  m_Occupation.clear();
  m_Occupation.resize(m_Energy.size());

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERDecay::PostAnalysis()
{
  // Do some analysis after the real analysis

  mout<<"Decay analysis: "<<endl;
  mout<<endl;

  int Sum = 0;
  for (unsigned int i = 0; i < m_Occupation.size(); ++i) {
    Sum += m_Occupation[i];
  }

  if (Sum != 0) {
    for (unsigned int i = 0; i < m_Energy.size(); ++i) {
      if (m_Occupation[i] != 0) {
        mout<<"E: "<<setw(6)<<m_Energy[i]<<" keV: "
            <<setw(6)<<m_Occupation[i]<<" ("
            <<setprecision(4)<<100.0*m_Occupation[i]/Sum<<"%)"<<endl;
      }
    }
  } else {
    mout<<"No decays found!"<<endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERDecay::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# Decay detection options:"<<endl;
  out<<"# "<<endl;
  for (unsigned int i = 0; i < m_Energy.size(); ++i) {
    out<<"# Decay "<<m_Energy[i]<<"  "<<m_EnergyErrorSigma[i]<<endl;
  }
  out<<"# "<<endl;
  
  return out.str().c_str();
}


// MERDecay.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
