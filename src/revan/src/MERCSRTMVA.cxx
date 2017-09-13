/*
 * MERCSRTMVA.cxx
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
// MERCSRTMVA
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRTMVA.h"

// Standard libs:
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MFile.h"
#include "MParser.h"
#include "MTokenizer.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"
#include "MERCSRBayesian.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERCSRTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCSRTMVA::MERCSRTMVA() : MERCSR()
{
  // Construct an instance of MERCSRTMVA
}


////////////////////////////////////////////////////////////////////////////////


MERCSRTMVA::~MERCSRTMVA()
{
  // Delete this instance of MERCSRTMVA
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRTMVA::SetParameters(MString FileName, 
                               MGeometryRevan* Geometry, 
                               double QualityFactorMin, 
                               double QualityFactorMax, 
                               int MaxNInteractions,
                               bool GuaranteeStartD1,
                               bool CreateOnlyPermutations)
{
  if (MERCSR::SetParameters(Geometry, 
                            QualityFactorMin, 
                            QualityFactorMax, 
                            MaxNInteractions,
                            GuaranteeStartD1,
                            CreateOnlyPermutations) == false) return false;

  m_FileName = FileName;
  m_UsedMethods.push_back("BDTD");
  
  // Read the steering file
  MParser SteeringFile;
  if (SteeringFile.Open(FileName) == false) {
    merr<<"Unable to open TMVA steering file: "<<FileName<<endl;  
  }
  
  MString BaseDirectory = FileName;
  BaseDirectory.ReplaceAllInPlace(".tmva", "");
  
  for (unsigned int l = 0; SteeringFile.GetNLines(); ++l) {
    MTokenizer* T = SteeringFile.GetTokenizerAt(l);
    if (T->GetNTokens() <= 1) continue;
    
    if (T->IsTokenAt(0, "SL") == true) {
      int MaxNIAs = T->GetTokenAtAsInt(1);
      if (MaxNIAs < m_MaxNInteractions) {
        m_MaxNInteractions = MaxNIAs;
      }
    }
    
    else if (T->IsTokenAt(0, "TA") == true) {
      vector<MString> Methods = T->GetTokenAtAsStringVector(1);
      for (MString M: Methods) {
        m_AvailableMethods[M] = 1;
      }
    }
  }
  
  // Create the data sets - must be identical to what's in the response creator
  m_DS.Initialize(m_MaxNInteractions);
  
  // Initialize the TMVA readers
  m_DS.CreateReaders(m_Readers);
  
  // Book the methods
  for (unsigned int r = 0; r < m_Readers.size(); ++r) {
    for (unsigned int m = 0; m < m_UsedMethods.size(); ++m) {
      MString MethodName = m_UsedMethods[m] + " method";
      MString WeightsFile = BaseDirectory + "/N" + (r+2) + "/weights/TMVAClassification_" + m_UsedMethods[m] + ".weights.xml";
      m_Readers[m]->BookMVA(MethodName.Data(), WeightsFile.Data());
    }  
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MERCSRTMVA::ComputeAllQualityFactors(MRERawEvent* RE)
{
  // In contrast to the classic & Bayesian approaches, we are not looping
  // over all different permutations
  // Therefore we have only one or no permutation as result ans all analysis
  // is performed in this function instead of "ComputeQualityFactor"

  double QualityFactor = c_CSRFailed;
  m_QualityFactors.clear();

  RE->Shuffle();

  vector<MRESE*> RESEs(RE->GetNRESEs());
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    RESEs[i] = RE->GetRESEAt(i);
  }

  if (RESEs.size() <= 1) {
    mdebug<<"MERCSRTMVA: Not enough hits: "<<RESEs.size()<<endl;
    return 0;
  }

  mdebug<<RE->ToString()<<endl;

  // Apply the TMVA data to all permutations and find the best one

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSRTMVA::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# CSR - TMVA options:"<<endl;
  out<<"# "<<endl;
  out<<"# Filename:                        "<<m_FileName<<endl;
  out<<MERCSR::ToString(true);
  out<<"# "<<endl;
  
  return out.str().c_str();
}

// MERCSRTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
