/*
 * MSynapse.cxx
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
// MSynapse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSynapse.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
#include <fstream>
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
#include "MNeuron.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSynapse)
#endif


////////////////////////////////////////////////////////////////////////////////


unsigned int MSynapse::s_IDCounter = 0;


////////////////////////////////////////////////////////////////////////////////


MSynapse::MSynapse(int NValues) 
{
  // Standard constructor
  
  m_ID = s_IDCounter++;
  m_Weight = 0.5 - gRandom->Rndm();
  m_Values.resize(NValues, 0.0);
  m_InNeuron = 0;
  m_OutNeuron = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSynapse::MSynapse(const MSynapse& S)
{
  // Copy constructor
  
  // Only the neuron ID are copied, not the pointers - this has to be done externally in
  // the network!
  m_ID = S.m_ID;
  m_InNeuron = 0;
  m_InNeuronID = S.m_InNeuron->GetID();
  m_OutNeuron = 0;
  m_OutNeuronID = S.m_OutNeuron->GetID();
  m_Weight = S.m_Weight;
  m_Values = S.m_Values;
}


////////////////////////////////////////////////////////////////////////////////


MSynapse::~MSynapse() 
{
  // Standard destructor
}


////////////////////////////////////////////////////////////////////////////////


void MSynapse::SetWeight(double Weight) 
{ 
  // Set the weight
  
  if (TMath::IsNaN(Weight) == true) {
    cout<<"MSynapse::SetWeight: NaN!"<<endl;
  }
  m_Weight = Weight; 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetWeight() 
{ 
  // Return the weight
  return m_Weight; 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetInValue(int mode) 
{ 
  // Return the value of the input neuron
  
  return m_InNeuron->GetValue(mode); 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetOutValue(int mode) 
{ 
  // Return the value of the output neuron
  
  return m_OutNeuron->GetValue(mode); 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetWeightedInValue(int mode) 
{ 
  // Return the value of the input neuron --- weighted!
  
  return m_InNeuron->GetValue(mode)*m_Weight; 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetWeightedOutValue(int mode) 
{ 
  // Return the value of output neuron --- weighted!
  
  return m_OutNeuron->GetValue(mode)*m_Weight; 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetInError(int mode) 
{ 
  // Return the error of the input neuron
  
  return m_InNeuron->GetError(mode); 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetOutError(int mode) 
{ 
  // Return the error of the output neuron
  
  return m_OutNeuron->GetError(mode); 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetWeightedInError(int mode) 
{ 
  // Return the error of the input neuron --- weighted!
  
  return m_InNeuron->GetError(mode)*m_Weight; 
}


////////////////////////////////////////////////////////////////////////////////


double MSynapse::GetWeightedOutError(int mode) 
{
  // Return the error of the output neuron --- weighted
  
  return m_OutNeuron->GetError(mode)*m_Weight; 
}


////////////////////////////////////////////////////////////////////////////////


void MSynapse::Connect(MNeuron* A, MNeuron* B)
{
  // Set the input and the ouptu neuron
  
  SetInNeuron(A);
  SetOutNeuron(B);
}


////////////////////////////////////////////////////////////////////////////////


bool MSynapse::Stream(MFile& File, const int Version, const bool Read)
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


int MSynapse::ParseLine(MString Line)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
  
  int Ret = 0;
  
  if (Line[0] == 'I' && Line[1] == 'D') {
    if (sscanf(Line, "ID %d", &m_ID) != 1) {
      mout<<"Unable to parse ID of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line[0] == 'V' && Line[1] == 'A') {
    unsigned int i = 0;
    double v = 0.0;
    if (sscanf(Line, "VA %u %lf", &i, &v) != 2) {
      mout<<"Unable to parse VA of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    } else {
      if (m_Values.size() <= i) m_Values.resize(i+1);
      m_Values[i] = v;
    }
  } else if (Line[0] == 'W' && Line[1] == 'E') {
    if (sscanf(Line, "WE %lf", &m_Weight) != 1) {
      mout<<"Unable to parse WE of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line[0] == 'N' && Line[1] == 'I') {
    if (sscanf(Line, "NI %d", &m_InNeuronID) != 1) {
      mout<<"Unable to parse NI of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line[0] == 'N' && Line[1] == 'O') {
    if (sscanf(Line, "NO %d", &m_OutNeuronID) != 1) {
      mout<<"Unable to parse NO of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else if (Line == "SY EN") {
    Ret = -1;
  } else {
    Ret = 2;
  }
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


void MSynapse::SetInNeuron(MNeuron* S) 
{ 
  // Set the incoming Neuron
  
  m_InNeuron = S;
  m_InNeuron->SetOutputSynapse(this);
}


////////////////////////////////////////////////////////////////////////////////


void MSynapse::SetOutNeuron(MNeuron* S) 
{ 
  // Set the outgoing Neuron
  
  m_OutNeuron = S; 
  m_OutNeuron->SetInputSynapse(this);
}


////////////////////////////////////////////////////////////////////////////////


MNeuron* MSynapse::GetInNeuron() 
{ 
  // Return the incoming Neuron
  
  return m_InNeuron; 
}


////////////////////////////////////////////////////////////////////////////////


MNeuron* MSynapse::GetOutNeuron() 
{ 
  // Return the outgoing Neuron
  
  return m_OutNeuron; 
}


////////////////////////////////////////////////////////////////////////////////


MString MSynapse::ToString() const
{
  // Dump the content to a string...
  
  ostringstream S;
  
  S<<"SY"<<endl;
  S<<"ID "<<m_ID<<endl;
  for (unsigned int i = 0; i < m_Values.size(); ++i) {
    S<<"VA "<<i<<" "<<m_Values[i]<<endl;
  }
  S<<"WE "<<m_Weight<<endl;
  S<<"NI "<<m_InNeuron->GetID()<<endl;
  S<<"NO "<<m_OutNeuron->GetID()<<endl;
  
  return MString(S);
}


// MSynapse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
