/*
 * MERQualityDataSet.cxx
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
#include "MERQualityDataSet.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"
#include "MComptonEvent.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERQualityDataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERQualityDataSet::MERQualityDataSet()
{
  m_NumberOfPathSamples = 18;
  m_UsePathToFirstIA = true;
  m_MaximumSequenceLength = 2;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERQualityDataSet::MERQualityDataSet(unsigned int MaximumSequenceLength, bool UsePathToFirstIA)
{
  m_NumberOfPathSamples = 18;
  Initialize(MaximumSequenceLength, UsePathToFirstIA);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MERQualityDataSet::~MERQualityDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


// Create for the given sequence length, 2..N
void MERQualityDataSet::Initialize(unsigned int MaximumSequenceLength, bool UsePathToFirstIA)
{
  m_MaximumSequenceLength = MaximumSequenceLength;
  m_UsePathToFirstIA = UsePathToFirstIA;
  
  if (m_MaximumSequenceLength < 2) {
    merr<<"Error: The sequence length must at least be 2"<<endl;
    return;
  }
  
  m_SimulationID = -1;
  m_Energies.resize(m_MaximumSequenceLength);
  m_PositionsX.resize(m_MaximumSequenceLength);
  m_PositionsY.resize(m_MaximumSequenceLength);
  m_PositionsZ.resize(m_MaximumSequenceLength);
  m_InteractionDistances.resize(m_MaximumSequenceLength-1);
  m_CosComptonScatterAngles.resize(m_MaximumSequenceLength-1);
  m_KleinNishinaProbability.resize(m_MaximumSequenceLength-1);
  m_CosComptonScatterAngleDifference.resize(m_MaximumSequenceLength-2);
  m_AbsorptionProbabilities.resize(m_MaximumSequenceLength-1);
  m_MinimumNadirAngle = -1;
  
  m_PathSamples.resize(m_NumberOfPathSamples);
  
  m_EvaluationIsCompletelyAborbed = -1;
  m_EvaluationIsReconstructable = -1;
  m_EvaluationZenithAngle = -1;
}


////////////////////////////////////////////////////////////////////////////////


TMVA::Reader* MERQualityDataSet::CreateReader()
{
  TString Name;
  
  TMVA::Reader* Reader = new TMVA::Reader("!Color:!Silent");
  
  Name = "SimulationID";
  Reader->AddVariable(Name, &m_SimulationID);
  
  Name = "SequenceLength";
  Reader->AddVariable(Name, &m_SequenceLength);    
  
  for (unsigned int i = 0; i < m_Energies.size(); ++i) {
    Name = "Energy";
    Name += i+1;
    Reader->AddVariable(Name, &m_Energies[i]);
  }
  
  for (unsigned int i = 0; i < m_PositionsX.size(); ++i) {
    Name = "X";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsX[i]);
  }
  for (unsigned int i = 0; i < m_PositionsY.size(); ++i) {
    Name = "Y";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsY[i]);
  }
  for (unsigned int i = 0; i < m_PositionsZ.size(); ++i) {
    Name = "Z";
    Name += i+1;
    Reader->AddVariable(Name, &m_PositionsZ[i]);
  }
  
  for (unsigned int i = 0; i < m_InteractionDistances.size(); ++i) {
    Name = "InteractionDistances";
    Name += i+1;
    Reader->AddVariable(Name, &m_InteractionDistances[i]);
  }
  
  for (unsigned int i = 0; i < m_CosComptonScatterAngles.size(); ++i) {
    Name = "CosComptonScatterAngle";
    Name += i+1;
    Reader->AddVariable(Name, &m_CosComptonScatterAngles[i]);
  }
  
  for (unsigned int i = 0; i < m_KleinNishinaProbability.size(); ++i) {
    Name = "NormalizedKleinNishinaValue";
    Name += i+1;
    Reader->AddVariable(Name, &m_KleinNishinaProbability[i]);
  }
  
  for (unsigned int i = 0; i < m_CosComptonScatterAngleDifference.size(); ++i) { 
    Name = "CosComptonScatterAngleDifference";
    Name += i+1;
    Reader->AddVariable(Name, &m_CosComptonScatterAngleDifference[i]);
  }
  
  for (unsigned int i = 0; i < m_AbsorptionProbabilities.size(); ++i) {
    Name = "AbsorptionProbabilities";
    Name += i+1;
    Reader->AddVariable(Name, &m_AbsorptionProbabilities[i]);
  }
  
  Name = "MinimumNadirAngle";
  Reader->AddVariable(Name, &m_MinimumNadirAngle);  
  
  if (m_UsePathToFirstIA == true) {
    for (unsigned int i = 0; i < m_PathSamples.size(); ++i) {
      Name = "PathSample";
      Name += i+1;
      Reader->AddVariable(Name, &m_PathSamples[i]);
    }
  }
  
  Name = "EvaluationIsCompletelyAborbed";
  Reader->AddVariable(Name, &m_EvaluationIsCompletelyAborbed);  
  
  Name = "EvaluationIsReconstructable";
  Reader->AddVariable(Name, &m_EvaluationIsReconstructable);  
  
  Name = "EvaluationZenithAngle";
  Reader->AddVariable(Name, &m_EvaluationZenithAngle);
  
  return Reader;
}


////////////////////////////////////////////////////////////////////////////////


// Fill the data sets from RESEs
void MERQualityDataSet::FillEventData(Long64_t ID, vector<MRESE*>& SequencedRESEs, MDGeometryQuest* Geometry)
{
  // (0) Basics the sequence length and the simulation ID
  m_SimulationID = ID;
  m_SequenceLength = SequencedRESEs.size();
  
  
  // (a) Raw data:
  double FullEnergy = 0;
  for (int r = 0; r < m_SequenceLength; ++r) {
    FullEnergy += SequencedRESEs[r]->GetEnergy();
    m_Energies[r] = SequencedRESEs[r]->GetEnergy();
    m_PositionsX[r] = SequencedRESEs[r]->GetPosition().X();
    m_PositionsY[r] = SequencedRESEs[r]->GetPosition().Y();
    m_PositionsZ[r] = SequencedRESEs[r]->GetPosition().Z();
  }
  
  // (b) Interaction distances
  for (int r = 0; r < m_SequenceLength-1; ++r) {
    MVector P1 = SequencedRESEs[r]->GetPosition();
    MVector P2 = SequencedRESEs[r+1]->GetPosition();
    m_InteractionDistances[r] = (P2-P1).Mag();
  }
  
  // (c) Compton scatter angle & Klein-Nishina
  double EnergyIncomingGamma = FullEnergy;
  double EnergyElectron = 0.0;
  double CosPhi = 0.0;
  double Phi = 0.0;
  for (int r = 0; r < m_SequenceLength-1; ++r) {
    EnergyElectron = SequencedRESEs[r]->GetEnergy();
    CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(EnergyElectron, EnergyIncomingGamma - EnergyElectron);
    Phi = MComptonEvent::ComputePhiViaEeEg(EnergyElectron, EnergyIncomingGamma - EnergyElectron);
    m_CosComptonScatterAngles[r] = CosPhi;
    m_KleinNishinaProbability[r] = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergyIncomingGamma, Phi);
    EnergyIncomingGamma -= EnergyElectron;
  }
  
  // (d) Compton scatter angle difference
  for (int r = 0; r < m_SequenceLength-2; ++r) {
    // Via Angle:
    MVector FirstDir = SequencedRESEs[r+1]->GetPosition() - SequencedRESEs[r]->GetPosition();
    MVector SecondDir = SequencedRESEs[r+2]->GetPosition() - SequencedRESEs[r+1]->GetPosition();
    double CosPhiGeo = cos(FirstDir.Angle(SecondDir));
    m_CosComptonScatterAngleDifference[r] = CosPhiGeo - m_CosComptonScatterAngles[r+1]; // "SequenceLength-3" for first argument since we only start for 3-site events
  }
  
  // (e) Absorption probabilities
  EnergyIncomingGamma = FullEnergy;
  for (int r = 0; r < m_SequenceLength-1; ++r) {
    EnergyIncomingGamma -= SequencedRESEs[r]->GetEnergy();
    
    m_AbsorptionProbabilities[r] = Geometry->GetAbsorptionProbability(SequencedRESEs[r]->GetPosition(), SequencedRESEs[r+1]->GetPosition(), EnergyIncomingGamma);
  }
  
  // (f) Minimum Nadir distance of the Compton cone
  if (m_CosComptonScatterAngles[0] >= -1 && m_CosComptonScatterAngles[0] <= 1) { 
    double Phi = acos(m_CosComptonScatterAngles[0]);
    MVector ConeAxis = SequencedRESEs[0]->GetPosition() - SequencedRESEs[1]->GetPosition();
    double AxisDist = MVector(0, 0, -1).Angle(ConeAxis);
  
    m_MinimumNadirAngle = fabs(AxisDist - Phi)*c_Deg;
  } else {
    m_MinimumNadirAngle = -1;  
  }
  
  // (g) Incoming probabilities
  if (m_UsePathToFirstIA == true) {
    if (m_CosComptonScatterAngles[0] > -1 && m_CosComptonScatterAngles[0] < 1) {
      EnergyIncomingGamma = FullEnergy;
      Phi = acos(m_CosComptonScatterAngles[0]);
      MVector FirstIAPos = SequencedRESEs[0]->GetPosition();
      MVector SecondIAPos = SequencedRESEs[1]->GetPosition();
      MVector FirstScatteredGammaRayDir = SecondIAPos - FirstIAPos;
      // Create a vector orthogonal to FirstScatteredGammaRayDir which we can use to create the first direction on the cone
      MVector Ortho = FirstScatteredGammaRayDir.Orthogonal();
      // Create the first direction on the cone by rotating FirstScatteredGammaRayDir by Phi around Ortho
      MVector Incoming = FirstScatteredGammaRayDir;
      Incoming.RotateAroundVector(Ortho, Phi);
      
      double StepWidth = c_TwoPi/m_NumberOfPathSamples;
      for (unsigned int a = 0; a < m_NumberOfPathSamples; ++a) {
        MVector Outgoing = -Incoming;
        Outgoing.RotateAroundVector(FirstScatteredGammaRayDir, a*StepWidth);
        Outgoing.Unitize();
        double P = Geometry->GetComptonAbsorptionProbability(FirstIAPos + 1000000*Outgoing, FirstIAPos, EnergyIncomingGamma);
        m_PathSamples[a] = P;
      }
    } else {
      for (unsigned int a = 0; a < m_NumberOfPathSamples; ++a) {
        m_PathSamples[a] = 0.0;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, whether the event is competely absorbed
void MERQualityDataSet::FillEvaluationIsCompletelyAborbed(bool IsCompletelyAborbed)
{
  m_EvaluationIsCompletelyAborbed = IsCompletelyAborbed;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, whether the event is competely absorbed
void MERQualityDataSet::FillEvaluationIsReconstructable(bool IsReconstructable)
{
  m_EvaluationIsReconstructable = IsReconstructable;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, about the zenith angle of the incoming gamma ray
void MERQualityDataSet::FillEvaluationZenithAngle(Float_t ZenithAngle)
{
  m_EvaluationZenithAngle = ZenithAngle;
}


////////////////////////////////////////////////////////////////////////////////


// Create the tree
TTree* MERQualityDataSet::CreateTree(MString Title)
{
  TTree* Tree = new TTree(Title, Title); 
  
  MString Name("SimulationID");
  Tree->Branch(Name, &m_SimulationID, Name + "/I");
  
  Name = "SequenceLength";
  Tree->Branch(Name, &m_SequenceLength, Name + "/I");
  
  for (unsigned int i = 0; i < m_Energies.size(); ++i) {
    Name = "Energy";
    Name += i+1;
    Tree->Branch(Name, &m_Energies[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_PositionsX.size(); ++i) {
    Name = "X";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsX[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_PositionsY.size(); ++i) {
    Name = "Y";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsY[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_PositionsZ.size(); ++i) {
    Name = "Z";
    Name += i+1;
    Tree->Branch(Name, &m_PositionsZ[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_InteractionDistances.size(); ++i) {
    Name = "InteractionDistances";
    Name += i+1;
    Tree->Branch(Name, &m_InteractionDistances[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_CosComptonScatterAngles.size(); ++i) {
    Name = "CosComptonScatterAngle";
    Name += i+1;
    Tree->Branch(Name, &m_CosComptonScatterAngles[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_KleinNishinaProbability.size(); ++i) {
    Name = "NormalizedKleinNishinaValue";
    Name += i+1;
    Tree->Branch(Name, &m_KleinNishinaProbability[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_CosComptonScatterAngleDifference.size(); ++i) { 
    Name = "CosComptonScatterAngleDifference";
    Name += i+1;
    Tree->Branch(Name, &m_CosComptonScatterAngleDifference[i], Name + "/F");
  }

  
  for (unsigned int i = 0; i < m_AbsorptionProbabilities.size(); ++i) {
    Name = "AbsorptionProbabilities";
    Name += i+1;
    Tree->Branch(Name, &m_AbsorptionProbabilities[i], Name + "/F");
  }
  
  Name = "MinimumNadirAngle";
  Tree->Branch(Name, &m_MinimumNadirAngle, Name + "/F");  
  
  if (m_UsePathToFirstIA == true) {
    for (unsigned int i = 0; i < m_PathSamples.size(); ++i) {
      Name = "PathSample";
      Name += i+1;
      Tree->Branch(Name, &m_PathSamples[i], Name + "/F");
    }  
  }
  
  Name = "EvaluationIsCompletelyAborbed";
  Tree->Branch(Name, &m_EvaluationIsCompletelyAborbed, Name + "/I");  
  
  Name = "EvaluationIsReconstructable";
  Tree->Branch(Name, &m_EvaluationIsReconstructable, Name + "/I");  
  
  Name = "EvaluationZenithAngle";
  Tree->Branch(Name, &m_EvaluationZenithAngle, Name + "/F"); 
  
  return Tree;
}


// MERQualityDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
