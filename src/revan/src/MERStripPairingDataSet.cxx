/*
 * MERStripPairingDataSet.cxx
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
#include "MERStripPairingDataSet.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"
#include "MComptonEvent.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERStripPairingDataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERStripPairingDataSet::MERStripPairingDataSet()
{
  m_NXStrips = 0;
  m_NYStrips = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERStripPairingDataSet::MERStripPairingDataSet(unsigned int NXStrips, unsigned int NYStrips)
{
  Initialize(NXStrips, NYStrips);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MERStripPairingDataSet::~MERStripPairingDataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


// Create for the given sequence length, 2..N
void MERStripPairingDataSet::Initialize(unsigned int NXStrips, unsigned int NYStrips)
{
  m_NXStrips = NXStrips;
  m_NYStrips = NYStrips;
  
  if (m_NXStrips < 1 || m_NYStrips < 1) {
    merr<<"Error: We need at least one strip in x and y direction"<<endl;
    return;
  }
  
  m_SimulationID = -1;
  m_XStripIDs.resize(m_NXStrips);
  m_YStripIDs.resize(m_NYStrips);
  m_XStripEnergies.resize(m_NXStrips);
  m_YStripEnergies.resize(m_NYStrips);
  
  m_EvaluationXStripIDs.resize(m_NXStrips + m_NYStrips);
  m_EvaluationYStripIDs.resize(m_NXStrips + m_NYStrips);
}


////////////////////////////////////////////////////////////////////////////////


TMVA::Reader* MERStripPairingDataSet::CreateReader()
{
  TString Name;
  
  TMVA::Reader* Reader = new TMVA::Reader("!Color:!Silent");
  
  Name = "SimulationID";
  Reader->AddVariable(Name, &m_SimulationID);  
  
  for (unsigned int i = 0; i < m_XStripIDs.size(); ++i) {
    Name = "XStripID";
    Name += i+1;
    Reader->AddVariable(Name, &m_XStripIDs[i]);
  }
  for (unsigned int i = 0; i < m_YStripIDs.size(); ++i) {
    Name = "YStripID";
    Name += i+1;
    Reader->AddVariable(Name, &m_YStripIDs[i]);
  }
  for (unsigned int i = 0; i < m_XStripEnergies.size(); ++i) {
    Name = "XStripEnergy";
    Name += i+1;
    Reader->AddVariable(Name, &m_XStripEnergies[i]);
  }
  for (unsigned int i = 0; i < m_YStripEnergies.size(); ++i) {
    Name = "YStripEnergy";
    Name += i+1;
    Reader->AddVariable(Name, &m_YStripEnergies[i]);
  }
  
  for (unsigned int i = 0; i < m_EvaluationXStripIDs.size(); ++i) {
    Name = "EvaluationXStripID";
    Name += i+1;
    Reader->AddVariable(Name, &m_EvaluationXStripIDs[i]);
  }
  for (unsigned int i = 0; i < m_EvaluationYStripIDs.size(); ++i) {
    Name = "EvaluationYStripID";
    Name += i+1;
    Reader->AddVariable(Name, &m_EvaluationYStripIDs[i]);
  }
  
  return Reader;
}


////////////////////////////////////////////////////////////////////////////////


// Fill the data sets from RESEs
void MERStripPairingDataSet::FillEventData(Long64_t ID, vector<unsigned int>& XStripIDs, vector<unsigned int>& YStripIDs, vector<double>& XStripEnergies, vector<double>& YStripEnergies)
{
  if (XStripIDs.size() != m_XStripIDs.size()) {
    mout<<"ERROR: number of x strips does not match: in="<<XStripIDs.size()<<" vs. stored="<<m_XStripIDs.size()<<endl;
    return;
  }
  if (YStripIDs.size() != m_YStripIDs.size()) {
    mout<<"ERROR: number of y strips does not match: in="<<YStripIDs.size()<<" vs. stored="<<m_YStripIDs.size()<<endl;
    return;
  }
  if (XStripEnergies.size() != m_XStripEnergies.size()) {
    mout<<"ERROR: number of x energies does not match: in="<<XStripEnergies.size()<<" vs. stored="<<m_XStripEnergies.size()<<endl;
    return;
  }
  if (YStripEnergies.size() != m_YStripEnergies.size()) {
    mout<<"ERROR: number of y energies does not match: in="<<YStripEnergies.size()<<" vs. stored="<<m_YStripEnergies.size()<<endl;
    return;
  }
  
  m_SimulationID = ID;
  
  for (unsigned int i = 0; i < m_XStripIDs.size(); ++i) {
    m_XStripIDs[i] = XStripIDs[i];
  }
  for (unsigned int i = 0; i < m_YStripIDs.size(); ++i) {
    m_YStripIDs[i] = YStripIDs[i];
  }
  for (unsigned int i = 0; i < m_XStripEnergies.size(); ++i) {
    m_XStripEnergies[i] = XStripEnergies[i];
  }
  for (unsigned int i = 0; i < m_YStripEnergies.size(); ++i) {
    m_YStripEnergies[i] = YStripEnergies[i];
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, whether the event is competely absorbed
void MERStripPairingDataSet::FillEvaluationInteractions(vector<unsigned int>& XStripIDs, vector<unsigned int>& YStripIDs)
{
  if (XStripIDs.size() > m_EvaluationXStripIDs.size()) {
    mout<<"ERROR: number of x strips is to large: in="<<XStripIDs.size()<<" vs. max="<<m_EvaluationXStripIDs.size()<<endl;
    return;
  }
  if (YStripIDs.size() > m_EvaluationYStripIDs.size()) {
    mout<<"ERROR: number of y strips does not match: in="<<YStripIDs.size()<<" vs. max="<<m_EvaluationYStripIDs.size()<<endl;
    return;
  }
  
  for (unsigned int i = 0; i < XStripIDs.size(); ++i) {
    m_EvaluationXStripIDs[i] = XStripIDs[i];
  }
  for (unsigned int i = XStripIDs.size(); i < m_EvaluationXStripIDs.size(); ++i) {
    m_EvaluationXStripIDs[i] = -1;
  }
  for (unsigned int i = 0; i < YStripIDs.size(); ++i) {
    m_EvaluationYStripIDs[i] = YStripIDs[i];
  }
  for (unsigned int i = YStripIDs.size(); i < m_EvaluationYStripIDs.size(); ++i) {
    m_EvaluationYStripIDs[i] = -1;
  }
}




////////////////////////////////////////////////////////////////////////////////


// Create the tree
TTree* MERStripPairingDataSet::CreateTree(MString Title)
{
  TTree* Tree = new TTree(Title, Title); 
  
  MString Name("SimulationID");
  Tree->Branch(Name, &m_SimulationID, Name + "/I");
  
  for (unsigned int i = 0; i < m_XStripIDs.size(); ++i) {
    Name = "XStripID";
    Name += i+1;
    Tree->Branch(Name, &m_XStripIDs[i], Name + "/I");
  }
  for (unsigned int i = 0; i < m_YStripIDs.size(); ++i) {
    Name = "YStripID";
    Name += i+1;
    Tree->Branch(Name, &m_YStripIDs[i], Name + "/I");
  }
  for (unsigned int i = 0; i < m_XStripEnergies.size(); ++i) {
    Name = "XStripEnergy";
    Name += i+1;
    Tree->Branch(Name, &m_XStripEnergies[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_YStripEnergies.size(); ++i) {
    Name = "YStripEnergy";
    Name += i+1;
    Tree->Branch(Name, &m_YStripEnergies[i], Name + "/F");
  }
  
  for (unsigned int i = 0; i < m_EvaluationXStripIDs.size(); ++i) {
    Name = "EvaluationXStripID";
    Name += i+1;
    Tree->Branch(Name, &m_EvaluationXStripIDs[i], Name + "/I");
  }
  for (unsigned int i = 0; i < m_EvaluationYStripIDs.size(); ++i) {
    Name = "EvaluationYStripID";
    Name += i+1;
    Tree->Branch(Name, &m_EvaluationYStripIDs[i], Name + "/I");
  }  
  
  return Tree;
}


// MERStripPairingDataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
