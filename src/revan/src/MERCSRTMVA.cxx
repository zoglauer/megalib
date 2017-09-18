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
  
  m_QualityFactorMin = -numeric_limits<double>::max();
  m_QualityFactorMax = +numeric_limits<double>::max();
  
  
  // Read the steering file
  MParser SteeringFile;
  if (SteeringFile.Open(FileName, MFile::c_Read) == false) {
    merr<<"Unable to open TMVA steering file: "<<FileName<<endl;  
  }
  
  MString BaseDirectory = FileName;
  BaseDirectory.ReplaceAllInPlace(".tmva", "");
  
  for (unsigned int l = 0; l < SteeringFile.GetNLines(); ++l) {
    MTokenizer* T = SteeringFile.GetTokenizerAt(l);
    if (T->GetNTokens() <= 1) continue;
    
    if (T->IsTokenAt(0, "SL") == true) {
      int MaxNInteractions = 0;
      vector<int> NIAs = T->GetTokenAtAsIntVector(1);
      for (int x: NIAs) {
        if (x > MaxNInteractions) {
          MaxNInteractions = x;
        }
      }
      if (MaxNInteractions < m_MaxNInteractions) {
        m_MaxNInteractions = MaxNInteractions;
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
  
  cout<<"Max N Interactions: "<<m_MaxNInteractions<<" vs. "<<m_Readers.size()<<endl;
  
  // Book the methods
  for (unsigned int r = 0; r < m_Readers.size(); ++r) {
    for (unsigned int m = 0; m < m_UsedMethods.size(); ++m) {
      MString MethodName = m_UsedMethods[m] + " method";
      MString WeightsFile = BaseDirectory + "/N" + (r+2) + "/weights/TMVAClassification_" + m_UsedMethods[m] + ".weights.xml";
      MFile::ExpandFileName(WeightsFile);
      m_Readers[r]->BookMVA(MethodName.Data(), WeightsFile.Data());
    }  
  }
  
  // Build permutation matrix:
  m_Permutator.resize(m_MaxNInteractions+1);
  for (unsigned int i = 2; i <= (unsigned int) m_MaxNInteractions; ++i) {
    vector<vector<unsigned int>> Permutations;
    vector<unsigned int> Indices;
    for (unsigned int j = 0; j < i; ++j) {
      Indices.push_back(j);
    }
    vector<unsigned int>::iterator Begin = Indices.begin();
    vector<unsigned int>::iterator End = Indices.end();
    do {
      Permutations.push_back(Indices);
    } while (next_permutation(Begin, End));
    m_Permutator[i] = Permutations;
  }
  //   // Verify:
  //   for (unsigned int i = 2; i < m_Permutator.size(); ++i) {
  //     cout<<"Size: "<<i<<endl;
  //     for (unsigned int j = 0; j < m_Permutator[i].size(); ++j) {
  //       cout<<"  Permutation "<<j<<"/"<<m_Permutator[i].size()<<": "<<endl;
  //       for (unsigned int k = 0; k < m_Permutator[i][j].size(); ++k) {
  //         cout<<m_Permutator[i][j][k]<<" ";
  //       }
  //       cout<<endl;
  //     }
  //   }    
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MERCSRTMVA::ComputeAllQualityFactors(MRERawEvent* RE)
{
  // This function computes all quality factors
  // At the end of this function call, the m_QualityFactors map has to be 
  // filled with permutations and their quality!
  
  vector<MRESE*> RESEs(RE->GetNRESEs());
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    RESEs[i] = RE->GetRESEAt(i);
  }
  
  unsigned int SequenceLength = RESEs.size();
  
  if (SequenceLength <= 1) {
    mdebug<<"MERCSRTMVA: Not enough hits: "<<RESEs.size()<<endl;
    return c_CSRFailed;
  }
  if (SequenceLength > RESEs.size()) {
    mdebug<<"MERCSRTMVA: Too many hits: "<<RESEs.size()<<endl;
    return c_CSRFailed;
  }  
  // Now we are in some programming trouble:
  // We have to evaluate all possible permutations of first degree 
  // sub elements in raw event:
  // For 4 hits there are 24 permutations which can be evaluated via this
  // shifting algorithm:
  // 4 times do: 432->32->2->32->2->32->2->
  // ("->" means: evaluate; 4 left shift the first 4 elements, 
  // 3 left shift 3... and so on.)
  
  // First find all different permutations:
  vector<vector<MRESE*> > Permutations;
  FindPermutations(RESEs, RESEs.size(), Permutations);
  
  // Calculate quality factor:
  int NGoodPermutations = 0;
  double QualityFactor = c_CSRFailed;
  m_QualityFactors.clear();
  for (unsigned int c = 0; c < Permutations.size(); ++c) {
    m_DS.Fill(RE->GetEventID(), Permutations[c], m_Geometry);
    
    QualityFactor = -m_Readers[SequenceLength-2]->EvaluateMVA("BDTD method");  
    
    if (QualityFactor != c_CSRFailed) {
      m_QualityFactors.insert(map<double, vector<MRESE*>, less_equal<double> >::value_type(QualityFactor, Permutations[c]));
      NGoodPermutations++;
    }
  }
  
  return NGoodPermutations;
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
