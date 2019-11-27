/*
 * MEREventClusterizerDistance.cxx
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
// MEREventClusterizerDistance
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEREventClusterizerDistance.h"

// Standard libs:
#include "MStreams.h"
#include "MRESE.h"
#include "MRawEventIncarnations.h"

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEREventClusterizerDistance)
#endif


////////////////////////////////////////////////////////////////////////////////


MEREventClusterizerDistance::MEREventClusterizerDistance() : MEREventClusterizer()
{
  // Construct an instance of MEREventClusterizerDistance
  
  m_DistanceCutOff = numeric_limits<double>::max(); // == no cutoff
}


////////////////////////////////////////////////////////////////////////////////


MEREventClusterizerDistance::~MEREventClusterizerDistance()
{
  // Delete this instance of MEREventClusterizerDistance
}


////////////////////////////////////////////////////////////////////////////////


//! Set the distance cut off
bool MEREventClusterizerDistance::SetDistanceCutOff(double DistanceCutOff)
{
  if (DistanceCutOff > 0) {
    m_DistanceCutOff = DistanceCutOff;
    return true;
  } else {
    merr<<"The distance cut off must be a positive number!"<<endl;
    m_DistanceCutOff = numeric_limits<double>::max(); 
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Do the analysis - this will add events to the whole incarnation list
bool MEREventClusterizerDistance::Analyze(MRawEventIncarnationList* List)
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
    
    cout<<RE->GetEventID()<<":  "<<NRESEs<<endl;
    
    // If we have just one there is nothing to clusterize - thus continue;
    if (NRESEs <= 1) continue;
    
    
    struct DataSet {
      unsigned int First;
      unsigned int Second;
      double Distance;
    };
    
    vector<int> Clusters(NRESEs, -1);
    
    vector<DataSet> DataSets;
    
    // Calculate all event pair and distances
    for (unsigned int r = 0; r < NRESEs; ++r) {
      for (unsigned int s = r+1; s < NRESEs; ++s) {
        
        DataSets.push_back(DataSet{r, s, (RE->GetRESEAt(r)->GetPosition() - RE->GetRESEAt(s)->GetPosition()).Mag()});
        //cout<<r<<":"<<s<<":"<<(RE->GetRESEAt(r)->GetPosition() - RE->GetRESEAt(s)->GetPosition()).Mag()<<endl;
      }
    }
    
    // Sort by distance 
    sort(DataSets.begin(), DataSets.end(), [](const DataSet & a, const DataSet & b) -> bool { return a.Distance < b.Distance; });
    
    // Highest cluster ID
    long HighestID = -1;
    for (unsigned int i = 0; i < DataSets.size(); ++i) {
      if (DataSets[i].Distance > m_DistanceCutOff) {
        break;
      }
      if (Clusters[DataSets[i].First] == -1 && Clusters[DataSets[i].Second] == -1) {
        HighestID++;
        Clusters[DataSets[i].First] = HighestID;
        Clusters[DataSets[i].Second] = HighestID;
      } else if (Clusters[DataSets[i].First] == -1 && Clusters[DataSets[i].Second] != -1) {
        Clusters[DataSets[i].First] = Clusters[DataSets[i].Second];
      } else if (Clusters[DataSets[i].First] != -1 && Clusters[DataSets[i].Second] == -1) {
        Clusters[DataSets[i].Second] = Clusters[DataSets[i].First] ;
      } else if (Clusters[DataSets[i].First] != Clusters[DataSets[i].Second]) {
        int Replace = Clusters[DataSets[i].Second];
        int ReplaceWith = Clusters[DataSets[i].First];
        for (unsigned int j = 0; j < Clusters.size(); ++j) {
          if (Clusters[j] == Replace) {
            Clusters[j] = ReplaceWith;
          }
        }
      } else {
        // Already identical
      }
    }
    
    // Everything unassigned is its own cluster
    for (unsigned int c = 0; c < Clusters.size(); ++c) {
      if (Clusters[c] == -1) {
        HighestID++;
        Clusters[c] = HighestID;
      }
    }
    
    vector<vector<MRESE*>> RESEs(HighestID+1, vector<MRESE*>());
    
    for (unsigned int r = 0; r < NRESEs; ++r) {
      cout<<"RESE "<<r<<": "<<Clusters[r]<<": "<<RE->GetRESEAt(r)->GetPosition()<<":"<<RE->GetRESEAt(r)->GetEnergy()<<endl; 
      RESEs[Clusters[r]].push_back(RE->GetRESEAt(r));
    }
    
    //for (unsigned int r = 0; r < RESEs.size(); ++r) {
    //  cout<<r<<": "<<RESEs.size()<<endl; 
    //}
    
    remove_if(RESEs.begin(), RESEs.end(), [](vector<MRESE*> V){return V.size() == 0;});
    
    //for (unsigned int r = 0; r < RESEs.size(); ++r) {
    //  cout<<r<<": "<<RESEs.size()<<endl; 
    //}
    
    
    // (3) Split the event up:
    
    // Create empty events to be filled
    vector<MRERawEvent*> REs;
    REs.push_back(RE);
    REs.back()->RemoveAllAndCompress(); // So that we just copy the additional information
    for (unsigned int o = 1; o < RESEs.size(); ++o) {
      REs.push_back(RE->Duplicate());
    }

    for (unsigned int l = 0; l < RESEs.size(); ++l) {
      for (unsigned int r = 0; r < RESEs[l].size(); ++r) {
        REs[l]->AddRESE(RESEs[l][r]); 
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
  mout<<"Split events: "<<endl;
  mout<<List->ToString()<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MEREventClusterizerDistance::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:
  
  ostringstream out;
  
  out<<"# Event clustering options:"<<endl;
  out<<"# "<<endl;
  out<<"# Distance cut "<<m_DistanceCutOff<<endl; 
  out<<"# "<<endl;
  
  return out.str().c_str();
}



// MEREventClusterizerDistance.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
