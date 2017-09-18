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
#include "MComptonEvent.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERCSRDataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERCSRDataSet::MERCSRDataSet()
{
  m_EventID = -1111; 
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


// Fill the data sets from RESEs
void MERCSRDataSet::Fill(Long64_t ID, vector<MRESE*>& SequencedRESEs, MDGeometryQuest* Geometry)
{
  // Check if we have a new event
  if (ID != m_EventID) {
    m_EventID = ID;
    m_Indices.clear();
    m_StoreAbsorptionProbabilityToFirstIAAverage.clear();
    m_StoreAbsorptionProbabilityToFirstIAMaximum.clear();
    m_StoreAbsorptionProbabilityToFirstIAMinimum.clear();
    m_StoreZenithAngle.clear();
    m_StoreNadirAngle.clear();
  }
  
  unsigned int SequenceLength = SequencedRESEs.size();
  
  m_SimulationIDs[SequenceLength-2] = ID;
  
  // (a) Raw data:
  double FullEnergy = 0;
  for (unsigned int r = 0; r < SequenceLength; ++r) {
    FullEnergy += SequencedRESEs[r]->GetEnergy();
    m_Energies[SequenceLength-2][r] = SequencedRESEs[r]->GetEnergy();
    m_PositionsX[SequenceLength-2][r] = SequencedRESEs[r]->GetPosition().X();
    m_PositionsY[SequenceLength-2][r] = SequencedRESEs[r]->GetPosition().Y();
    m_PositionsZ[SequenceLength-2][r] = SequencedRESEs[r]->GetPosition().Z();
  }
  
  // (b) Interaction distances
  for (unsigned int r = 0; r < SequenceLength-1; ++r) {
    MVector P1 = SequencedRESEs[r]->GetPosition();
    MVector P2 = SequencedRESEs[r+1]->GetPosition();
    m_InteractionDistances[SequenceLength-2][r] = (P2-P1).Mag();
  }
  
  // (c) Compton scatter angle & Klein-Nishina
  double EnergyIncomingGamma = FullEnergy;
  double EnergyElectron = 0.0;
  double CosPhi = 0.0;
  double Phi = 0.0;
  for (unsigned int r = 0; r < SequenceLength-1; ++r) {
    EnergyElectron = SequencedRESEs[r]->GetEnergy();
    CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(EnergyElectron, EnergyIncomingGamma - EnergyElectron);
    Phi = MComptonEvent::ComputePhiViaEeEg(EnergyElectron, EnergyIncomingGamma - EnergyElectron);
    m_CosComptonScatterAngles[SequenceLength-2][r] = CosPhi;
    m_KleinNishinaProbability[SequenceLength-2][r] = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergyIncomingGamma, Phi);
    EnergyIncomingGamma -= EnergyElectron;
    //if (p == 0 && StartResolved == true && CompletelyAbsorbed == true && Phi*c_Deg > 179.99) {
    //  cout<<"Large Compton scatter angle (Sim ID: "<<RE->GetEventID()<<") -- Start:"<<SequencedRESEs[m_Permutator[SequenceLength][p][0]]->GetEnergy()<<endl;
    //  mout<<RE->ToString()<<endl;
    //}
  }
  
  // (d) Compton scatter angle difference
  for (unsigned int r = 0; r < SequenceLength-2; ++r) {
    // Via Angle:
    MVector FirstDir = SequencedRESEs[r+1]->GetPosition() - SequencedRESEs[r]->GetPosition();
    MVector SecondDir = SequencedRESEs[r+2]->GetPosition() - SequencedRESEs[r+1]->GetPosition();
    double CosPhiGeo = cos(FirstDir.Angle(SecondDir));
    m_CosComptonScatterAngleDifference[SequenceLength-3][r] = CosPhiGeo - m_CosComptonScatterAngles[SequenceLength-2][r+1]; // "SequenceLength-3" for first argument since we only start for 3-site events
  }
  
  // (e) Absorption probabilities
  EnergyIncomingGamma = FullEnergy;
  for (unsigned int r = 0; r < SequenceLength-1; ++r) {
    EnergyIncomingGamma -= SequencedRESEs[r]->GetEnergy();
    
    m_AbsorptionProbabilities[SequenceLength-2][r] = Geometry->GetAbsorptionProbability(SequencedRESEs[r]->GetPosition(), SequencedRESEs[r+1]->GetPosition(), EnergyIncomingGamma);
  }
  
  // (f) Incoming probabilities
  if (m_CosComptonScatterAngles[SequenceLength-2][0] > -1 && m_CosComptonScatterAngles[SequenceLength-2][0] < 1) {
    
    // First check if we have already calculated it:
    bool Found = false;
    if (m_Indices.size() > 0 && SequenceLength > 3) {
      auto IndexIter = find(m_Indices.begin(), m_Indices.end(), pair<int, int>(SequencedRESEs[0]->GetID(), SequencedRESEs[1]->GetID()));
      if (IndexIter != m_Indices.end()) {
        unsigned int Index = IndexIter - m_Indices.begin(); 
        m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] = m_StoreAbsorptionProbabilityToFirstIAAverage[Index];
        m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = m_StoreAbsorptionProbabilityToFirstIAMaximum[Index];
        m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = m_StoreAbsorptionProbabilityToFirstIAMinimum[Index];
        m_ZenithAngle[SequenceLength-2] = m_StoreZenithAngle[Index];
        m_NadirAngle[SequenceLength-2] = m_StoreNadirAngle[Index];
        Found = true;
      }
    }
    
    if (Found == false) {
      EnergyIncomingGamma = FullEnergy;
      Phi = acos(m_CosComptonScatterAngles[SequenceLength-2][0]);
      MVector FirstIAPos = SequencedRESEs[0]->GetPosition();
      MVector SecondIAPos = SequencedRESEs[1]->GetPosition();
      MVector FirstScatteredGammaRayDir = SecondIAPos - FirstIAPos;
      // Create a vector orthogonal to FirstScatteredGammaRayDir which we can use to create the first direction on the cone
      MVector Ortho = FirstScatteredGammaRayDir.Orthogonal();
      // Create the first direction on the cone by rotating FirstScatteredGammaRayDir by Phi around Ortho
      MVector Incoming = FirstScatteredGammaRayDir;
      Incoming.RotateAroundVector(Ortho, Phi);
      
      m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] = 0.0;
      m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = 0.0;
      m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = numeric_limits<double>::max();
      unsigned int Steps = 18*sin(Phi); // 10 deg for CA=90
      if (Steps < 1) Steps = 1;
                            double StepWidth = c_TwoPi/Steps;
      for (unsigned int a = 0; a < Steps; ++a) {
        MVector Outgoing = -Incoming;
        Outgoing.RotateAroundVector(FirstScatteredGammaRayDir, a*StepWidth);
        Outgoing.Unitize();
        double P = Geometry->GetComptonAbsorptionProbability(FirstIAPos + 1000000*Outgoing, FirstIAPos, EnergyIncomingGamma);
        m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] += P;
        if (P > m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2]) {
          m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = P;
        }
        if (P < m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2]) {
          m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = P;
        }
      }
      m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] /= Steps;
      
      // (g) Zenith and Nadir angles
      MVector Zenith(0, 0, 1);
      m_ZenithAngle[SequenceLength-2] = (FirstIAPos - SecondIAPos).Angle(Zenith - FirstIAPos) - Phi;
      MVector Nadir(0, 0, -1);
      m_NadirAngle[SequenceLength-2] = (FirstIAPos - SecondIAPos).Angle(Nadir - FirstIAPos) - Phi;
      
      if (SequenceLength > 3) {
        m_Indices.push_back(pair<int, int>(SequencedRESEs[0]->GetID(), SequencedRESEs[1]->GetID()));
        m_StoreAbsorptionProbabilityToFirstIAAverage.push_back(m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2]);
        m_StoreAbsorptionProbabilityToFirstIAMaximum.push_back(m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2]);
        m_StoreAbsorptionProbabilityToFirstIAMinimum.push_back(m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2]); 
        m_StoreZenithAngle.push_back(m_ZenithAngle[SequenceLength-2]); 
        m_StoreNadirAngle.push_back(m_NadirAngle[SequenceLength-2]);
      }
    }
    
  } else {
    m_AbsorptionProbabilityToFirstIAAverage[SequenceLength-2] = -0.1;
    m_AbsorptionProbabilityToFirstIAMaximum[SequenceLength-2] = -0.1;
    m_AbsorptionProbabilityToFirstIAMinimum[SequenceLength-2] = -0.1;
    m_ZenithAngle[SequenceLength-2] = -4.0; // good one's are from -pi..pi
    m_NadirAngle[SequenceLength-2] = -4.0;  // good one's are from -pi..pi
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
