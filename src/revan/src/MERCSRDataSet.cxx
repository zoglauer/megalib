/*
 * MERCSRDataSet.cxx
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
#include "MERCSRDataSet.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERCSRDataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERCSRDataSet::MERCSRDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERCSRDataSet::MERCSRDataSet(unsigned int SequenceLength)
{
  Initialize(SequenceLength);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MERCSRDataSet::~MERCSRDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


// Create for the given sequence length, 2..N
void MERCSRDataSet::Initialize(unsigned int SequenceLength)
{
  m_SimulationIDs.clear();
  m_Energies.clear();
  m_PositionsX.clear();
  m_PositionsY.clear();
  m_PositionsZ.clear();
  m_InteractionDistances.clear();
  m_CosComptonScatterAngles.clear();
  m_KleinNishinaProbability.clear();
  m_CosComptonScatterAngleDifference.clear();
  m_AbsorptionProbabilities.clear();
  m_AbsorptionProbabilityToFirstIAAverage.clear();
  m_AbsorptionProbabilityToFirstIAMaximum.clear();
  m_AbsorptionProbabilityToFirstIAMinimum.clear();
  m_ZenithAngle.clear();
  m_NadirAngle.clear();
  
  if (SequenceLength < 2) {
    merr<<"Error: The sequence length must at least be 2"<<endl;
    return;
  }
  
  for (unsigned int c = 0; c <= SequenceLength-2; ++c) {
    unsigned int l = c+2;
    
    m_SimulationIDs.push_back(0);
    
    m_Energies.push_back(vector<Float_t>(l));
    
    m_PositionsX.push_back(vector<Float_t>(l));
    m_PositionsY.push_back(vector<Float_t>(l));
    m_PositionsZ.push_back(vector<Float_t>(l));
    
    m_InteractionDistances.push_back(vector<Float_t>(l-1));
    
    m_CosComptonScatterAngles.push_back(vector<Float_t>(l-1));
    m_KleinNishinaProbability.push_back(vector<Float_t>(l-1));
    
    if (l > 2) {
      m_CosComptonScatterAngleDifference.push_back(vector<Float_t>(l-2));
    }
    
    m_AbsorptionProbabilities.push_back(vector<Float_t>(l-1));
    m_AbsorptionProbabilityToFirstIAAverage.push_back(0);
    m_AbsorptionProbabilityToFirstIAMaximum.push_back(0);
    m_AbsorptionProbabilityToFirstIAMinimum.push_back(0);
    m_ZenithAngle.push_back(0);
    m_NadirAngle.push_back(0);
  } 
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRDataSet::CreateReaders(vector<TMVA::Reader*>& Readers)
{
  TString Name;
  
  for (unsigned int c = 0; c < m_SimulationIDs.size(); ++c) {
    int l = c+2;
  
    TMVA::Reader* Reader = new TMVA::Reader("!Color:!Silent");
    
    for (unsigned int i = 0; i < m_Energies[c].size(); ++i) {
      Name = "Energy";
      Name += i+1;
      Reader->AddVariable(Name, &m_Energies[c][i]);
    }
    
    for (unsigned int i = 0; i < m_PositionsX[c].size(); ++i) {
      Name = "X";
      Name += i+1;
      Reader->AddVariable(Name, &m_PositionsX[c][i]);
    }
    for (unsigned int i = 0; i < m_PositionsY[c].size(); ++i) {
      Name = "Y";
      Name += i+1;
      Reader->AddVariable(Name, &m_PositionsY[c][i]);
    }
    for (unsigned int i = 0; i < m_PositionsZ[c].size(); ++i) {
      Name = "Z";
      Name += i+1;
      Reader->AddVariable(Name, &m_PositionsZ[c][i]);
    }
    
    for (unsigned int i = 0; i < m_InteractionDistances[c].size(); ++i) {
      Name = "InteractionDistances";
      Name += i+1;
      Reader->AddVariable(Name, &m_InteractionDistances[c][i]);
    }
    
    for (unsigned int i = 0; i < m_CosComptonScatterAngles[c].size(); ++i) {
      Name = "CosComptonScatterAngle";
      Name += i+1;
      Reader->AddVariable(Name, &m_CosComptonScatterAngles[c][i]);
    }
    
    for (unsigned int i = 0; i < m_KleinNishinaProbability[c].size(); ++i) {
      Name = "NormalizedKleinNishinaValue";
      Name += i+1;
      Reader->AddVariable(Name, &m_KleinNishinaProbability[c][i]);
    }
    
    if (l > 2) {
      for (unsigned int i = 0; i < m_CosComptonScatterAngleDifference[c-1].size(); ++i) { // "-1" since we only start at 3 interactions
        Name = "CosComptonScatterAngleDifference";
        Name += i+1;
        Reader->AddVariable(Name, &m_CosComptonScatterAngleDifference[c-1][i]);
      }
    }
    
    for (unsigned int i = 0; i < m_AbsorptionProbabilities[c].size(); ++i) {
      Name = "AbsorptionProbabilities";
      Name += i+1;
      Reader->AddVariable(Name, &m_AbsorptionProbabilities[c][i]);
    }
    
    Name = "AbsorptionProbabilityToFirstIAAverage";
    Reader->AddVariable(Name, &m_AbsorptionProbabilityToFirstIAAverage[c]);
    
    Name = "AbsorptionProbabilityToFirstIAMaximum";
    Reader->AddVariable(Name, &m_AbsorptionProbabilityToFirstIAMaximum[c]);
    
    Name = "AbsorptionProbabilityToFirstIAMinimum";
    Reader->AddVariable(Name, &m_AbsorptionProbabilityToFirstIAMinimum[c]);
    
    Name = "ZenithAngle";
    Reader->AddVariable(Name, &m_ZenithAngle[c]);
    
    Name = "NadirAngle";
    Reader->AddVariable(Name, &m_NadirAngle[c]);
    
    Readers.push_back(Reader);
  }
}


////////////////////////////////////////////////////////////////////////////////


// Create trees
void MERCSRDataSet::CreateTrees(vector<TTree*>& GoodTree, vector<TTree*>& BadTree)
{
  GoodTree.clear();
  BadTree.clear();
  
  // Round 2: Create the trees
  for (unsigned int c = 0; c < m_SimulationIDs.size(); ++c) {
    int l = c+2;
    
    TTree* Good = new TTree("Good", "Good Compton ER tree"); 
    TTree* Bad = new TTree("Bad", "Bad Compton ER tree"); 
    
    MString Name("SimulationIDs");
    Good->Branch(Name, &m_SimulationIDs[c], Name + "/L");
    Bad->Branch(Name, &m_SimulationIDs[c], Name + "/L");
    
    for (unsigned int i = 0; i < m_Energies[c].size(); ++i) {
      Name = "Energy";
      Name += i+1;
      Good->Branch(Name, &m_Energies[c][i], Name + "/F");
      Bad->Branch(Name, &m_Energies[c][i], Name + "/F");
    }
    
    for (unsigned int i = 0; i < m_PositionsX[c].size(); ++i) {
      Name = "X";
      Name += i+1;
      Good->Branch(Name, &m_PositionsX[c][i], Name + "/F");
      Bad->Branch(Name, &m_PositionsX[c][i], Name + "/F");
    }
    for (unsigned int i = 0; i < m_PositionsY[c].size(); ++i) {
      Name = "Y";
      Name += i+1;
      Good->Branch(Name, &m_PositionsY[c][i], Name + "/F");
      Bad->Branch(Name, &m_PositionsY[c][i], Name + "/F");
    }
    for (unsigned int i = 0; i < m_PositionsZ[c].size(); ++i) {
      Name = "Z";
      Name += i+1;
      Good->Branch(Name, &m_PositionsZ[c][i], Name + "/F");
      Bad->Branch(Name, &m_PositionsZ[c][i], Name + "/F");
    }
    
    for (unsigned int i = 0; i < m_InteractionDistances[c].size(); ++i) {
      Name = "InteractionDistances";
      Name += i+1;
      Good->Branch(Name, &m_InteractionDistances[c][i], Name + "/F");
      Bad->Branch(Name, &m_InteractionDistances[c][i], Name + "/F");
    }
    
    for (unsigned int i = 0; i < m_CosComptonScatterAngles[c].size(); ++i) {
      Name = "CosComptonScatterAngle";
      Name += i+1;
      Good->Branch(Name, &m_CosComptonScatterAngles[c][i], Name + "/F");
      Bad->Branch(Name, &m_CosComptonScatterAngles[c][i], Name + "/F");
    }
    
    for (unsigned int i = 0; i < m_KleinNishinaProbability[c].size(); ++i) {
      Name = "NormalizedKleinNishinaValue";
      Name += i+1;
      Good->Branch(Name, &m_KleinNishinaProbability[c][i], Name + "/F");
      Bad->Branch(Name, &m_KleinNishinaProbability[c][i], Name + "/F");
    }
    
    if (l > 2) {
      for (unsigned int i = 0; i < m_CosComptonScatterAngleDifference[c-1].size(); ++i) { // "-1" since we only start at 3 interactions
        Name = "CosComptonScatterAngleDifference";
        Name += i+1;
        Good->Branch(Name, &m_CosComptonScatterAngleDifference[c-1][i], Name + "/F");
        Bad->Branch(Name, &m_CosComptonScatterAngleDifference[c-1][i], Name + "/F");
      }
    }
    
    for (unsigned int i = 0; i < m_AbsorptionProbabilities[c].size(); ++i) {
      Name = "AbsorptionProbabilities";
      Name += i+1;
      Good->Branch(Name, &m_AbsorptionProbabilities[c][i], Name + "/F");
      Bad->Branch(Name, &m_AbsorptionProbabilities[c][i], Name + "/F");
    }
    
    Name = "AbsorptionProbabilityToFirstIAAverage";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAAverage[c], Name + "/F");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAAverage[c], Name + "/F");
    
    Name = "AbsorptionProbabilityToFirstIAMaximum";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAMaximum[c], Name + "/F");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAMaximum[c], Name + "/F");
    
    Name = "AbsorptionProbabilityToFirstIAMinimum";
    Good->Branch(Name, &m_AbsorptionProbabilityToFirstIAMinimum[c], Name + "/F");
    Bad->Branch(Name, &m_AbsorptionProbabilityToFirstIAMinimum[c], Name + "/F");
    
    Name = "ZenithAngle";
    Good->Branch(Name, &m_ZenithAngle[c], Name + "/F");
    Bad->Branch(Name, &m_ZenithAngle[c], Name + "/F");
    
    Name = "NadirAngle";
    Good->Branch(Name, &m_NadirAngle[c], Name + "/F");
    Bad->Branch(Name, &m_NadirAngle[c], Name + "/F");
    
    GoodTree.push_back(Good);
    BadTree.push_back(Bad);
  }
}


// MERCSRDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
