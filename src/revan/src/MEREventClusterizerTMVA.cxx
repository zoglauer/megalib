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


#ifdef ___CLING___
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
  
  MString Prefix = "";
  
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
    
    else if (T->IsTokenAt(0, "EB") == true) {
      m_EnergyBinEdges = T->GetTokenAtAsIntVector(1);
    }
    
    else if (T->IsTokenAt(0, "DN") == true) {
      Prefix = T->GetTokenAtAsString(1);
    }
  }
  
  // Some sabity checks:
  if (m_EnergyBinEdges.size() < 2) {
    mgui<<"No energy bin edges found in tmva file"<<error;
    return false;
  }
  
  
  
  // Create the data sets - must be identical to what's in the response creator
  
  m_DS = vector<vector<MEREventClusterizerDataSet*>>(m_MaxNHits - 1, vector<MEREventClusterizerDataSet*>(m_EnergyBinEdges.size() - 1, nullptr));
  m_Readers = vector<vector<TMVA::Reader*>>(m_MaxNHits - 1, vector<TMVA::Reader*>(m_EnergyBinEdges.size() - 1, nullptr));
  
  for (unsigned int h = 2; h <= m_MaxNHits; ++h) {
    for (unsigned int e = 0; e < m_EnergyBinEdges.size() - 1; ++e) {
      
      // Create and initialize the data set 
      m_DS[h-2][e] = new MEREventClusterizerDataSet();
      m_DS[h-2][e]->Initialize(h, m_MaxNGroups);
      
      // Initialize the TMVA readers
      m_Readers[h-2][e] = m_DS[h-2][e]->CreateReader();
    }
  }
  
  // Book the methods
  vector<MERCSRTMVAMethod> UsedMethods = m_Methods.GetUsedMethods();
  m_MethodNames.resize(UsedMethods.size());
  for (unsigned int h = 2; h <= m_MaxNHits; ++h) {
    for (unsigned int e = 0; e < m_EnergyBinEdges.size() - 1; ++e) {
      for (unsigned int m = 0; m < UsedMethods.size(); ++m) {
        m_MethodNames[m] = m_Methods.GetString(UsedMethods[m]) + " method";
        MString WeightsFile = BaseDirectory + "/" + Prefix + ".hits" + h + ".emin" + m_EnergyBinEdges[e] + ".emax" + m_EnergyBinEdges[e+1] + "/weights/TMVARegression_" + m_Methods.GetString(UsedMethods[m]) + ".weights.xml";
        cout<<"File: "<<WeightsFile<<endl;
        MFile::ExpandFileName(WeightsFile);
        if (MFile::Exists(WeightsFile) == false) {
          mgui<<"Unable to open file: "<<WeightsFile<<endl;
          return false;
        }
        m_Readers[h-2][e]->BookMVA(m_MethodNames[m].Data(), WeightsFile.Data());
      }
    }  
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do the analysis - this will add events to the whole incarnation list
bool MEREventClusterizerTMVA::Analyze(MRawEventIncarnationList* List)
{
  mdebug<<endl;
  mdebug<<"Event clustering for event: "<<List->Get(0)->GetInitialRawEvent()->GetEventID()<<endl;
  
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
      merr<<"Event clustering: The event has more hits ("<<NRESEs<<") than we have trained for ("<<m_MaxNHits<<") -- skipping it and setting it to bad"<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringTooManyHits);  // This automatically marks the event invalid for further analysis...
      continue;
    }
    
    if (RE->GetEnergy() < m_EnergyBinEdges.front() || RE->GetEnergy() > m_EnergyBinEdges.back()) {
      merr<<"Event clustering: The event energy ("<<RE->GetEnergy()<<" keV) is outside the bound of which the TMVA methods have been trained for ["<<m_EnergyBinEdges.front()<<"-"<<m_EnergyBinEdges.back()<<"]."<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringEnergyOutOfBounds);  // This automatically marks the event invalid for further analysis...
      continue;
    }
    
    // Find the energy bin - the above line made sure we are inside the bounds!
    unsigned int EnergyBin = 0;
    for (unsigned int e = 1; e < m_EnergyBinEdges.size(); ++e) {
      if (RE->GetEnergy() <= m_EnergyBinEdges[e]) {
        EnergyBin = e-1;
        break;
      }
    }
    
    
    vector<MRESE*> RESEs;
    for (unsigned int r = 0; r < NRESEs; ++r) {
      RESEs.push_back(RE->GetRESEAt(r));
    }
    
    m_DS[NRESEs-2][EnergyBin]->FillEventData(RE->GetEventID(), RESEs);
    vector<Float_t> IDs = m_Readers[NRESEs-2][EnergyBin]->EvaluateRegression(m_MethodNames[0].Data());  
   
    // Now check how many events we have
    
    // (0) Group the data correctly
    vector<vector<Float_t>> Origins(NRESEs, vector<Float_t>(m_MaxNGroups+1, 0));
    
    unsigned int RunningIndex = 0;
    for (unsigned int h = 0; h < NRESEs; ++h) {
      for (unsigned int g = 0; g < m_MaxNGroups+1; ++g) { // Plus one since we have the extra garbage bin
        Origins[h][g] = IDs[RunningIndex];
        ++RunningIndex;
      }
    }
    
    // Print it for diagnostics:
    /*
    for (unsigned int h = 0; h < NRESEs; ++h) {
      cout<<"RESE with "<<RESEs[h]->GetEnergy()<<" keV: "<<endl;
      cout<<"   --> ";
      for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
        cout<<Origins[h][g]<<" ";
      }
      cout<<endl;
    }
    */
    
    // (1) For each hit find the highest gamma ray
    vector<unsigned int> GammaRayID(NRESEs);
    for (unsigned int h = 0; h < NRESEs; ++h) {
      unsigned int ID = 0;
      Float_t Value = Origins[h][0];
      for (unsigned int g = 1; g < m_MaxNGroups; ++g) {
        if (Origins[h][g] > Value) {
          Value = Origins[h][g];
          ID = g;
        }
      }
      GammaRayID[h] = ID;
      //cout<<"RESE "<<h<<" --> gamma "<<ID<<endl;
    }
    
    
    /*
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
    */
    
    
    // (2) Check how many origins we have -- they are currently sorted, this the highest value + 1 is the number of origins
    unsigned int NOrigins = 0;
    for (unsigned int h = 0; h < NRESEs; ++h) {
      if (GammaRayID[h] + 1 > NOrigins) {
        NOrigins = GammaRayID[h] + 1;
      }
    }
    
    mdebug<<"Number of origins: "<<NOrigins<<endl;
    
    if (NOrigins == 1) continue; // Perfect, one event -- nothing to split
    
    if (NOrigins == 0) {
      merr<<"Event clustering: The event has no origins -- something might have gone wronf during machine learning..."<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventClusteringNoOrigins);  // This automatically marks the event invalid for further analysis...
      continue;      
    }

    
    // (3) Split the event up:
    
    // Create empty events to be filled
    vector<MRERawEvent*> REs;
    REs.push_back(RE);
    REs.back()->RemoveAllAndCompress(); // So that we just copy the additional information
    for (unsigned int o = 1; o < NOrigins; ++o) {
      REs.push_back(RE->Duplicate());
    }

    
    // Now add back the correct hits:
    unsigned int REIIndex = 0;
    for (unsigned int g = 0; g < m_MaxNGroups; ++g) {
      int NHitOrigins = 0; 
      for (unsigned int h = 0; h < NRESEs; ++h) {
        if (GammaRayID[h] == g) {
          ++NHitOrigins;
          break;
        }
      }
      if (NHitOrigins > 0) {
        // Since it it the first remove all RESEs not belonging to 
        for (unsigned int h = 0; h < NRESEs; ++h) {
          if (GammaRayID[h] == g) {
             REs[REIIndex]->AddRESE(RESEs[h]);
          }
        }
        ++REIIndex;
      }
    }
    
    // (4) Create a new incarnation list for the new gamma rays
    List->RemoveAll();
    for (unsigned int i = 0; i < REs.size(); ++i) {
      MRawEventIncarnations* New = new MRawEventIncarnations();
      New->AddRawEvent(REs[i]);
      List->Add(New);
    }
    
    // Done!
  }
  
  // Print the result:
  mdebug<<"Split events: "<<endl;
  mdebug<<List->ToString()<<endl;
  
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
