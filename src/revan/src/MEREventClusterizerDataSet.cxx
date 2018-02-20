/*
 * MEREventClusterizerDataSet.cxx
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
#include "MEREventClusterizerDataSet.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"
#include "MComptonEvent.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEREventClusterizerDataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MEREventClusterizerDataSet::MEREventClusterizerDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MEREventClusterizerDataSet::MEREventClusterizerDataSet(unsigned int NHits, unsigned int NMaxGroups)
{
  Initialize(NHits, NMaxGroups);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MEREventClusterizerDataSet::~MEREventClusterizerDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


// Create for the given sequence length, 2..N
void MEREventClusterizerDataSet::Initialize(unsigned int NHits, unsigned int NMaxGroups)
{
  m_NHits = NHits;
  m_NMaxGroups = NMaxGroups;
  
  if (m_NHits < 1) {
    merr<<"Error: We need at least one hit"<<endl;
    return;
  }  
  if (m_NMaxGroups < 1) {
    merr<<"Error: We need at least one possible output hit groups"<<endl;
    return;
  }
  
  m_SimulationID = -1;
  m_Energies.resize(m_NHits);
  m_PositionsX.resize(m_NHits);
  m_PositionsY.resize(m_NHits);
  m_PositionsZ.resize(m_NHits);
  
  m_ResultHitGroups.resize(m_NHits);
  for (unsigned int h = 0; h < m_NHits; ++h) {
    m_ResultHitGroups[h] = vector<Float_t>(m_NMaxGroups+1, 0); 
  }
}


////////////////////////////////////////////////////////////////////////////////


TMVA::Reader* MEREventClusterizerDataSet::CreateReader()
{
  TString Name;
  
  TMVA::Reader* Reader = new TMVA::Reader("!Color:!Silent");
  
  Name = "SimulationID";
  Reader->AddVariable(Name, &m_SimulationID);  
  
  for (unsigned int i = 0; i < m_Energies.size(); ++i) {
    Name = "Energy_";
    Name += i+1;
    Reader->AddVariable(Name, &m_Energies[i]);
  }
  for (unsigned int i = 0; i < m_PositionsX.size(); ++i) {
    Name = "PositionX_";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsX[i]);
  }
  for (unsigned int i = 0; i < m_PositionsY.size(); ++i) {
    Name = "PositionY_";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsY[i]);
  }
  for (unsigned int i = 0; i < m_PositionsZ.size(); ++i) {
    Name = "PositionZ_";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsZ[i]);
  }
  
  
  for (unsigned int i = 0; i < m_ResultHitGroups.size(); ++i) {
    for (unsigned int g = 0; g < m_ResultHitGroups[i].size(); ++g) {
      Name = "ResultHitGroups_";
      Name += i+1;
      Name += "_";
      Name += g+1;
      Reader->AddVariable(Name, &m_ResultHitGroups[i][g]);
    }
  }
  
  return Reader;
}


////////////////////////////////////////////////////////////////////////////////


// Fill the data sets from RESEs
bool MEREventClusterizerDataSet::FillEventData(Long64_t ID, vector<MRESE*>& RESEs)
{
  if (RESEs.size() != m_NHits) {
    mout<<"Error: The number of hits in the RESE list ("<<RESEs.size()<<") must be identical with the number of hits set here ("<<m_NHits<<")"<<endl;
    return false;
  }
  
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    m_Energies[r] = RESEs[r]->GetEnergy();
    m_PositionsX[r] = RESEs[r]->GetPosition().X();
    m_PositionsY[r] = RESEs[r]->GetPosition().Y();
    m_PositionsZ[r] = RESEs[r]->GetPosition().Z();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, whether the event is competely absorbed
bool MEREventClusterizerDataSet::FillResultData(vector<vector<int>>& RESEGroups)
{
  if (RESEGroups.size() != m_NHits) {
    mout<<"Error: The number of hits in the RESE group ("<<RESEGroups.size()<<") must be identical with the number of hits set here ("<<m_NHits<<")"<<endl;
    return false;
  }
  
  // Zero it first:
  for (unsigned int r = 0; r < m_ResultHitGroups.size(); ++r) {
    for (unsigned int g = 0; g < m_ResultHitGroups[r].size(); ++g) {
      m_ResultHitGroups[r][g] = 0.0;
    }
  }
      
  
  for (unsigned int r = 0; r < RESEGroups.size(); ++r) {
    for (unsigned int h = 0; h < RESEGroups[r].size(); ++h) {
      unsigned int Group = RESEGroups[r][h];
      if (Group > m_NMaxGroups) Group = m_NMaxGroups;
      m_ResultHitGroups[r][Group] = 1.0;
    }
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


// Create the tree
TTree* MEREventClusterizerDataSet::CreateTree(MString Title)
{
  TTree* Tree = new TTree(Title, Title); 
  
  MString Name("SimulationID");
  Tree->Branch(Name, &m_SimulationID, Name + "/F");
  
  for (unsigned int i = 0; i < m_Energies.size(); ++i) {
    Name = "Energy_";
    Name += i+1;
    Tree->Branch(Name, &m_Energies[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_PositionsX.size(); ++i) {
    Name = "PositionX_";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsX[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_PositionsY.size(); ++i) {
    Name = "PositionY_";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsY[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_PositionsZ.size(); ++i) {
    Name = "PositionZ_";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsZ[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_ResultHitGroups.size(); ++i) {
    for (unsigned int g = 0; g < m_ResultHitGroups[i].size(); ++g) {
      Name = "ResultHitGroups_";
      Name += i+1;
      Name += "_";
      Name += g+1;
      Tree->Branch(Name, &m_ResultHitGroups[i][g], Name + "/F");
    }
  }

  return Tree;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the currently loaded data set to a string
MString MEREventClusterizerDataSet::ToString()
{
  ostringstream out;
  
  out<<"ID "<<m_SimulationID<<endl;
  for (unsigned int i = 0; i < m_NHits; ++i) {
    out<<"("<<i<<") E="<<m_Energies[i]<<" P=("<<m_PositionsX[i]<<"/"<<m_PositionsY[i]<<"/"<<m_PositionsZ[i]<<") --> ";
    for (unsigned g = 0; g < m_NMaxGroups; ++g) {
      out<<m_ResultHitGroups[i][g]<<" ";
    }
    out<<endl;
  }
  
  return out.str();
}


// MEREventClusterizerDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
