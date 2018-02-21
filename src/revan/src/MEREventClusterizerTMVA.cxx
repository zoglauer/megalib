/*
 * MEREventClusterizerTMVA.cxx
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
// MEREventClusterizerTMVA
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEREventClusterizerTMVA.h"

// Standard libs:
#include "MStreams.h"
#include "MRESE.h"
#include "MRawEventIncarnations.h"

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEREventClusterizerTMVA)
#endif


////////////////////////////////////////////////////////////////////////////////


MEREventClusterizerTMVA::MEREventClusterizerTMVA() : MEREventClusterizer()
{
  // Construct an instance of MEREventClusterizerTMVA
}


////////////////////////////////////////////////////////////////////////////////


MEREventClusterizerTMVA::~MEREventClusterizerTMVA()
{
  // Delete this instance of MEREventClusterizerTMVA
}


////////////////////////////////////////////////////////////////////////////////


//! Set the file name for TMVA analysis
bool MEREventClusterizerTMVA::SetTMVAFileNameAndMethod(MString FileName, MERCSRTMVAMethods Methods)
{  
  m_FileName = FileName;
  m_Methods = Methods;
  
  // Read the steering file
  MParser SteeringFile;
  if (SteeringFile.Open(m_FileName, MFile::c_Read) == false) {
    mgui<<"Unable to open TMVA steering file: "<<FileName<<error;
    return false;
  }
  
  MString BaseDirectory = m_FileName;
  BaseDirectory.ReplaceAllInPlace(".eventclusterizer.tmva", "");
  
  for (unsigned int l = 0; l < SteeringFile.GetNLines(); ++l) {
    MTokenizer* T = SteeringFile.GetTokenizerAt(l);
    if (T->GetNTokens() <= 1) continue;
    
    if (T->IsTokenAt(0, "MH") == true) {
      if (T->GetNTokens() != 2) {
        mgui<<"MH needs two keywords, e.g. MH 15"<<error;
        return false;
      }
      m_MaxNHits = T->GetTokenAtAsUnsignedInt(1);
    }
    
    else if (T->IsTokenAt(0, "MG") == true) {
      if (T->GetNTokens() != 2) {
        mgui<<"MG needs two keywords, e.g. MG 3"<<error;
        return false;
      }
      m_MaxNGroups = T->GetTokenAtAsUnsignedInt(1);
    }
    
    else if (T->IsTokenAt(0, "TA") == true) {
      vector<MERCSRTMVAMethod> AvailableMethods;
      MString MethodsString = T->GetTokenAtAsString(1);
      vector<MString> Methods = MethodsString.Tokenize(",");
      for (MString M: Methods) {
        AvailableMethods.push_back(m_Methods.GetMethod(M));
        cout<<"Available: "<<M<<endl;
      }
      // Verify that only available methods are UsedMethods
      vector<MERCSRTMVAMethod> UsedMethods = m_Methods.GetUsedMethods();
      for (MERCSRTMVAMethod M: UsedMethods) {
        cout<<"Used: "<<m_Methods.GetString(M)<<endl;
        /*
        if (find(AvailableMethods.begin(), AvailableMethods.end(), M) == AvailableMethods.end()) {
          mgui<<"Method "<<m_Methods.GetString(M)<<" is not available."<<error;
          m_Methods.UnsetUsedMethod(M);
          //return false;
        }
        */
      }
    }
  }
  
  
  // Create the data sets - must be identical to what's in the response creator
  for (unsigned int h = 2; h <= m_MaxNHits; ++h) {
    m_DS.push_back(new MEREventClusterizerDataSet());
    m_DS.back()->Initialize(h, m_MaxNGroups);
    // Initialize the TMVA readers
    m_Readers.push_back(m_DS.back()->CreateReader());
  }
  
  // Book the methods
  vector<MERCSRTMVAMethod> UsedMethods = m_Methods.GetUsedMethods();
  m_MethodNames.resize(UsedMethods.size());
  for (unsigned int r = 0; r < m_Readers.size(); ++r) {
    for (unsigned int m = 0; m < UsedMethods.size(); ++m) {
      m_MethodNames[m] = m_Methods.GetString(UsedMethods[m]) + " method";
      MString WeightsFile = BaseDirectory + "/N" + (r+2) + "/weights/TMVARegression_" + m_Methods.GetString(UsedMethods[m]) + ".weights.xml";
      MFile::ExpandFileName(WeightsFile);
      if (MFile::Exists(WeightsFile) == false) {
        mgui<<"Unable to open file: "<<WeightsFile<<endl;
        return false;
      }
      m_Readers[r]->BookMVA(m_MethodNames[m].Data(), WeightsFile.Data());
    }  
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do the analysis - this will add events to the whole incarnation list
bool MEREventClusterizerTMVA::Analyze(MRawEventIncarnationList* List)
{
  unsigned int r_max = List->Size();
  for (unsigned int r = 0; r < r_max; ++r) {
    MRawEventIncarnations* REI = List->Get(r);
    if (REI->GetNRawEvents() != 1) {
      merr<<"Event clustering: Only one raw event incarnation can be present at this stage of the analysis"<<endl;
      //RE->SetRejectionReason(MRERawEvent::c_RejectionTooManyEventIncarnations);  // This automatically marks the event invalid for further analysis...
      continue; 
    }
    
    MRERawEvent* RE = REI->GetRawEventAt(0);
    
    unsigned int NRESEs = RE->GetNRESEs();
    
    // If we have just one there is nothing to clusterize - thus continue;
    if (NRESEs <= 1) continue;
    
    if (NRESEs > m_MaxNHits) {
      merr<<"Event clustering: The event has more hits than we have trained for -- skipping it and setting it to bad"<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringTooManyHits);  // This automatically marks the event invalid for further analysis...
      continue;
    }
    
    vector<MRESE*> RESEs;
    for (unsigned int r = 0; r < NRESEs; ++r) {
      RESEs.push_back(RE->GetRESEAt(r));
    }
    
    m_DS[NRESEs-2]->FillEventData(RE->GetEventID(), RESEs);
    vector<Float_t> IDs =  m_Readers[NRESEs-2]->EvaluateRegression(m_MethodNames[0].Data());  
   
    // Now check how many events we have
    
    // (0) Group the data correctly
    vector<vector<Float_t>> Origins(NRESEs, vector<Float_t>(m_MaxNGroups+1, 0));
    
    unsigned int RunningIndex = 0;
    for (unsigned int h = 0; h < NRESEs; ++h) {
      for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
        Origins[h][g] = IDs[RunningIndex];
        ++RunningIndex;
      }
    }
    
    // Print it for diagnostics:
    for (unsigned int h = 0; h < NRESEs; ++h) {
      cout<<"RESE with "<<RESEs[h]->GetEnergy()<<" keV: "<<endl;
      cout<<"   --> ";
      for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
        cout<<Origins[h][g]<<" ";
      }
      cout<<endl;
    }
    
    
    // (1) Check if some hits belong to different gamma rays, then we reject the events
    bool AllOriginsGood = true;
    for (unsigned int h = 0; h < NRESEs; ++h) {
      int NHitOrigins = 0;
      for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
        if (Origins[h][g] > 0.5) {
          ++NHitOrigins;
        }
      }
      if (NHitOrigins != 1) {
        cout<<"ERROR: Hit "<<h<<" has not exactly one origin"<<endl; 
        AllOriginsGood = false;
      }
    }
    if (AllOriginsGood == false) {
      merr<<"Event clustering: The event has a hit with more than 1 origin"<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringUnresolvedHits);  // This automatically marks the event invalid for further analysis...
      continue;      
    }
    
    
    // (2) Check how many origins we have
    unsigned int NOrigins = 0; 
    for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
      for (unsigned int h = 0; h < NRESEs; ++h) {
        if (Origins[h][g] > 0.5) {
          ++NOrigins;
          break;
        }
      }
    }
    cout<<"Number of origins: "<<NOrigins<<endl;
    
    if (NOrigins == 1) continue; // Perfect, one event
    
    if (NOrigins == 0) {
      merr<<"Event clustering: The event has no origins -- something might have gone wronf during machine learning..."<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringNoOrigins);  // This automatically marks the event invalid for further analysis...
      continue;      
    }
    
    // (3) Split the event up:
    
    vector<MRERawEvent*> REs;
    REs.push_back(RE);
    REs.back()->RemoveAllAndCompress(); // So that we just copy the additional information
    for (unsigned int o = 0; o < NOrigins; ++o) {
      REs.push_back(RE->Duplicate());
    }

    
    // Now add back the correct events:
    unsigned int REIIndex = 0;
    for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
      int NHitOrigins = 0; 
      for (unsigned int h = 0; h < NRESEs; ++h) {
        if (Origins[h][g] > 0.5) {
          ++NHitOrigins;
          break;
        }
      }
      if (NHitOrigins > 0) {
        // Since it it the first remove all RESEs not belonging to 
        for (unsigned int h = 0; h < NRESEs; ++h) {
          if (Origins[h][g] > 0.5) {
             REs[REIIndex]->AddRESE(RESEs[h]);
          }
        }
        ++REIIndex;
      }
    }
    
    // (4) Create a new incarnation list for the new gamma rays
    for (unsigned int i = 1; i < REs.size(); ++i) {
      MRawEventIncarnations* New = new MRawEventIncarnations();
      New->AddRawEvent(REs[i]);
      List->Add(New);
    }
    
    // Done!
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MEREventClusterizerTMVA::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:
  
  ostringstream out;
  
  out<<"# Event clustering options:"<<endl;
  out<<"# "<<endl;
  out<<"# TMVA file:    "<<m_FileName<<endl;
  out<<"# TMVA methods: "<<m_Methods.GetUsedMethodsString()<<endl; 
  out<<"# "<<endl;
  
  return out.str().c_str();
}



// MEREventClusterizerTMVA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
