/*
 * MERTrackWithinCrossStripDetectorTMVADataSet.cxx
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
#include "MERTrackWithinCrossStripDetectorTMVADataSet.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"
#include "MComptonEvent.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERTrackWithinCrossStripDetectorTMVADataSet)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERTrackWithinCrossStripDetectorTMVADataSet::MERTrackWithinCrossStripDetectorTMVADataSet()
{
  m_NXStrips = 0;
  m_NYStrips = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERTrackWithinCrossStripDetectorTMVADataSet::MERTrackWithinCrossStripDetectorTMVADataSet(unsigned int NXStrips, unsigned int NYStrips)
{
  Initialize(NXStrips, NYStrips);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MERTrackWithinCrossStripDetectorTMVADataSet::~MERTrackWithinCrossStripDetectorTMVADataSet()
{
}


////////////////////////////////////////////////////////////////////////////////


// Create for the given sequence length, 2..N
void MERTrackWithinCrossStripDetectorTMVADataSet::Initialize(unsigned int NXStrips, unsigned int NYStrips)
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

  m_ResultPositionX = 0;
  m_ResultPositionY = 0;
  m_ResultPositionZ = 0;
  m_ResultDirectionX = 0;
  m_ResultDirectionY = 0;
  m_ResultDirectionZ = 0;
}


////////////////////////////////////////////////////////////////////////////////


TMVA::Reader* MERTrackWithinCrossStripDetectorTMVADataSet::CreateReader()
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
  
  Name = "ResultPositionX";
  Reader->AddVariable(Name, &m_ResultPositionX);
  Name = "ResultPositionY";
  Reader->AddVariable(Name, &m_ResultPositionY);
  Name = "ResultPositionZ";
  Reader->AddVariable(Name, &m_ResultPositionZ);

  Name = "ResultDirectionX";
  Reader->AddVariable(Name, &m_ResultDirectionX);
  Name = "ResultDirectionY";
  Reader->AddVariable(Name, &m_ResultDirectionY);
  Name = "ResultDirectionZ";
  Reader->AddVariable(Name, &m_ResultDirectionZ);

  return Reader;
}


////////////////////////////////////////////////////////////////////////////////


// Fill the data sets from RESEs
bool MERTrackWithinCrossStripDetectorTMVADataSet::FillEventData(Long64_t ID, vector<unsigned int>& XStripIDs, vector<unsigned int>& YStripIDs, vector<double>& XStripEnergies, vector<double>& YStripEnergies)
{
  if (XStripIDs.size() != m_XStripIDs.size()) {
    mout<<"ERROR: number of x strips does not match: in="<<XStripIDs.size()<<" vs. stored="<<m_XStripIDs.size()<<endl;
    return false;
  }
  if (YStripIDs.size() != m_YStripIDs.size()) {
    mout<<"ERROR: number of y strips does not match: in="<<YStripIDs.size()<<" vs. stored="<<m_YStripIDs.size()<<endl;
    return false;
  }
  if (XStripEnergies.size() != m_XStripEnergies.size()) {
    mout<<"ERROR: number of x energies does not match: in="<<XStripEnergies.size()<<" vs. stored="<<m_XStripEnergies.size()<<endl;
    return false;
  }
  if (YStripEnergies.size() != m_YStripEnergies.size()) {
    mout<<"ERROR: number of y energies does not match: in="<<YStripEnergies.size()<<" vs. stored="<<m_YStripEnergies.size()<<endl;
    return false;
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
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the evaluation section, whether the event is competely absorbed
bool MERTrackWithinCrossStripDetectorTMVADataSet::FillResultData(const MVector& Position, const MVector& Direction)
{
  m_ResultPositionX = Position[0];
  m_ResultPositionY = Position[1];
  m_ResultPositionZ = Position[2];
  m_ResultDirectionX = Direction[0];
  m_ResultDirectionY = Direction[1];
  m_ResultDirectionZ = Direction[2];

  return true;
}


////////////////////////////////////////////////////////////////////////////////


// Create the tree
TTree* MERTrackWithinCrossStripDetectorTMVADataSet::CreateTree(MString Title)
{
  TTree* Tree = new TTree(Title, Title); 
  
  MString Name("SimulationID");
  Tree->Branch(Name, &m_SimulationID, Name + "/F");
  
  for (unsigned int i = 0; i < m_XStripIDs.size(); ++i) {
    Name = "XStripID";
    Name += i+1;
    Tree->Branch(Name, &m_XStripIDs[i], Name + "/F");
  }
  for (unsigned int i = 0; i < m_YStripIDs.size(); ++i) {
    Name = "YStripID";
    Name += i+1;
    Tree->Branch(Name, &m_YStripIDs[i], Name + "/F");
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

  Name = "ResultPositionX";
  Tree->Branch(Name, &m_ResultPositionX, Name + "/F");
  Name = "ResultPositionY";
  Tree->Branch(Name, &m_ResultPositionY, Name + "/F");
  Name = "ResultPositionZ";
  Tree->Branch(Name, &m_ResultPositionZ, Name + "/F");

  Name = "ResultDirectionX";
  Tree->Branch(Name, &m_ResultDirectionX, Name + "/F");
  Name = "ResultDirectionY";
  Tree->Branch(Name, &m_ResultDirectionY, Name + "/F");
  Name = "ResultDirectionZ";
  Tree->Branch(Name, &m_ResultDirectionZ, Name + "/F");

  return Tree;
}


// MERTrackWithinCrossStripDetectorTMVADataSet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
