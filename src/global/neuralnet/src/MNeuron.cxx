/*
 * MNeuron.cxx
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
// MNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
using namespace std;

// ROOT libs:
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "MString.h"
#include "TRandom.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"

// MEGAlib libs:
#include "MSynapse.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuron::s_IDCounter = 0;


////////////////////////////////////////////////////////////////////////////////


MNeuron::MNeuron() 
{
  // Default constructor
  
  m_ID = s_IDCounter++;
  
  Set(1, 1);
  m_Momentum = 0.5;
}


////////////////////////////////////////////////////////////////////////////////


MNeuron::MNeuron(int NValues, int NErrors) 
{
  // Standard constructor
  
  m_ID = s_IDCounter++;
  
  Set(NValues, NErrors);
  m_Momentum = 0.5;
}


////////////////////////////////////////////////////////////////////////////////


MNeuron::MNeuron(const MNeuron& N)
{
  // Copy constructor
  
  // We are only copying the IDs of our synapses!
  // Since the pointers will be new, they have to be set externally!!!
  
  m_ID = N.m_ID;
  m_InputSynapses.clear();
  // The IDs are not kept up to date
  for (unsigned int i = 0; i < N.m_InputSynapses.size(); ++i) {
    m_InputSynapsesIDs.push_back(N.m_InputSynapses[i]->GetID());
  }
  m_OutputSynapses.clear();
  for (unsigned int i = 0; i < N.m_OutputSynapses.size(); ++i) {
    m_OutputSynapsesIDs.push_back(N.m_OutputSynapses[i]->GetID());
  }
  m_Values = N.m_Values;
  m_Errors = N.m_Errors;
  
  m_Momentum = N.m_Momentum;
}


////////////////////////////////////////////////////////////////////////////////


MNeuron::~MNeuron() 
{
  // Standard destructor
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::Set(int NValues, int NErrors) 
{
  // Set all parameters
  
  m_Values.resize(NValues, 0.0);
  m_Errors.resize(NValues, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


double MNeuron::GetMomentum() 
{ 
  // Return the momentum of this neuron
  
  return m_Momentum; 
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::SetValue(double Value, int i) 
{ 
  // Set the current value
  
  if (TMath::IsNaN(Value) == true) {
    cout<<"MNeuron::SetValue: NaN!"<<endl;
  }
  m_Values[i] = Value; 
}


////////////////////////////////////////////////////////////////////////////////


double MNeuron::GetValue(int i) 
{ 
  // Return the current value
  
  return m_Values[i]; 
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::SetError(double Error, int i) 
{
  // Set the current Error
  
  if (TMath::IsNaN(Error) == true) {
    cout<<"MNeuron::SetError: NaN!"<<endl;
  }
  m_Errors[i] = Error; 
}


////////////////////////////////////////////////////////////////////////////////


double MNeuron::GetError(int i) 
{ 
  // Return the current Error
  
  return m_Errors[i]; 
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::Run(int mode) 
{
  // Start the action -- empty by default
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::Learn(int mode) 
{
  // Start learning process -- empty by default
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuron::Stream(MFile& File, const int Version, const bool Read)
{
  // Read/Write capability
  
  bool Return = true;
  
  if (Read == false) {
    File.Write(ToString());
  } else {
    int Ret = 1;
    
    MString Line;
    while (File.IsGood()) {
      File.ReadLine(Line);
      if (Line.Length() < 2) continue;
      
      Ret = ParseLine(Line);
      if (Ret != 0) break;
    }
    if (Ret == 1) Return = false;
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MNeuron::ParseLine(MString Line)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
  
  int Ret = 0;
  
  //cout<<"Line (neuron): "<<Line<<endl;
  
  if (Line[0] == 'I' && Line[1] == 'D') {
    if (sscanf(Line, "ID %d", &m_ID) != 1) {
      mout<<"Unable to parse ID of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line[0] == 'V' && Line[1] == 'A') {
    unsigned int i = 0;
    double v = 0.0;
    if (sscanf(Line, "VA %u %lf", &i, &v) != 2) {
      mout<<"Unable to parse VA of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    } else {
      if (m_Values.size() <= i) m_Values.resize(i+1);
      m_Values[i] = v;
    }
  } else if (Line[0] == 'E' && Line[1] == 'R') {
    unsigned int i = 0;
    double v = 0.0;
    if (sscanf(Line, "ER %u %lf", &i, &v) != 2) {
      mout<<"Unable to parse ER of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    } else {
      if (m_Errors.size() <= i) m_Errors.resize(i+1);
      m_Errors[i] = v;
    }
  } else if (Line[0] == 'S' && Line[1] == 'I') {
    unsigned int i = 0;
    unsigned int id = 0;
    if (sscanf(Line, "SI %u %u", &i, &id) != 2) {
      mout<<"Unable to parse SI of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    } else {
      if (m_InputSynapsesIDs.size() <= i) m_InputSynapsesIDs.resize(i+1);
      m_InputSynapsesIDs[i] = id;
    }
  } else if (Line[0] == 'S' && Line[1] == 'O') {
    unsigned int i = 0;
    unsigned int id = 0;
    if (sscanf(Line, "SO %u %u", &i, &id) != 2) {
      mout<<"Unable to parse SO of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    } else {
      if (m_OutputSynapsesIDs.size() <= i) m_OutputSynapsesIDs.resize(i+1);
      m_OutputSynapsesIDs[i] = id;
    }
  } else if (Line[0] == 'M' && Line[1] == 'O') {
    if (sscanf(Line, "MO %lf", &m_Momentum) != 1) {
      mout<<"Unable to parse MO of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line == "NE EN") {
    Ret = -1;
  } else {
    Ret = 2;
  }
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuron::GetID() const 
{ 
  // Return the ID
  
  return m_ID; 
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::SetInputSynapse(MSynapse* S) 
{
  // Set the input synapse
  
  m_InputSynapses.push_back(S);
}


////////////////////////////////////////////////////////////////////////////////


void MNeuron::SetOutputSynapse(MSynapse* S) 
{
  // Set the output synapse
  
  m_OutputSynapses.push_back(S);
}


////////////////////////////////////////////////////////////////////////////////


MString MNeuron::ToString() const
{
  // Dump the content to a string...
  
  ostringstream S;
  
  S<<"NE"<<endl;
  S<<"ID "<<m_ID<<endl;
  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    S<<"VA "<<i<<" "<<m_Values[i]<<endl;
  }
  for (unsigned int i = 0; i < m_Errors.size(); ++i) {
    S<<"ER "<<i<<" "<<m_Errors[i]<<endl;
  }
  for (unsigned int i = 0; i < m_InputSynapses.size(); ++i) {
    S<<"SI "<<i<<" "<<m_InputSynapses[i]->GetID()<<endl;
  }
  for (unsigned int i = 0; i < m_OutputSynapses.size(); ++i) {
    S<<"SO "<<i<<" "<<m_OutputSynapses[i]->GetID()<<endl;
  }
  S<<"MO "<<m_Momentum<<endl;
  
  return MString(S);
}


// MNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
