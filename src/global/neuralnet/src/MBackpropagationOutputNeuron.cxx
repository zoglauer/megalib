/*
 * MBackpropagationOutputNeuron.cxx
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
// MBackpropagationOutputNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackpropagationOutputNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

#include "MSynapse.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___b
ClassImp(MBackpropagationOutputNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackpropagationOutputNeuron::MBackpropagationOutputNeuron() : 
MBackpropagationNeuron() 
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationOutputNeuron::MBackpropagationOutputNeuron(const MBackpropagationOutputNeuron& N) : 
MBackpropagationNeuron(N)
{
  // Default copy constructor
  
  m_LearningRate = N.m_LearningRate;
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationOutputNeuron::MBackpropagationOutputNeuron(double LearningRate, double Momentum, int NValues, int NErrors) : 
MBackpropagationNeuron(NValues, NErrors) 
{
  // Standard constructor
  
  m_LearningRate = LearningRate;
  m_Momentum = Momentum;
}


////////////////////////////////////////////////////////////////////////////////



void MBackpropagationOutputNeuron::Set(double LearningRate, double Momentum, int NValues, int NErrors)
{
  m_LearningRate = LearningRate;
  m_Momentum = Momentum;
  
  MBackpropagationNeuron::Set(NValues, NErrors);
}


////////////////////////////////////////////////////////////////////////////////


void MBackpropagationOutputNeuron::Learn(int Mode) 
{
  double Delta;
  m_Errors[0] = ComputeError();
  
  for (unsigned int i = 0; i < m_InputSynapses.size(); ++i) {
    Delta = m_LearningRate*m_Errors[0]*m_InputSynapses[i]->GetInValue();
    //cout<<"MBackpropagationOutputNeuron::Learn: "
    //    <<Delta<<":"<<m_Errors[0]<<":"<<m_InputSynapses[i]->GetInValue()<<endl;
    m_InputSynapses[i]->SetWeight(Delta);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MBackpropagationOutputNeuron::ComputeError(int Mode)
{
  //cout<<"CE1: "<<m_Values[0]<<":"<<m_Errors[0]<<endl;
  //cout<<ToString()<<endl;
  return m_Values[0]*(1.0-m_Values[0])*(m_Errors[0]-m_Values[0]);
}


////////////////////////////////////////////////////////////////////////////////


bool MBackpropagationOutputNeuron::Stream(MFile& File, const int Version, const bool Read)
{
  // Read/Write capability
  
  bool Return = true;
  
  if (Read == false) {
    File.Write(ToString());
  } else {
    MNeuron::Stream(File, Version, Read);
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MBackpropagationOutputNeuron::ParseLine(MString Line)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
  
  
  int Ret = MNeuron::ParseLine(Line);
  if (Ret != 2) {
    return Ret;
  }
  
  Ret = 0;
  
  if (Line[0] == 'L' && Line[1] == 'E') {
    if (sscanf(Line, "LE %lf", &m_LearningRate) != 1) {
      mout<<"Unable to parse LE of neuron "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else {
    Ret = 2;
  }
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


MString MBackpropagationOutputNeuron::ToString() const
{
  ostringstream S;
  
  S<<MNeuron::ToString();
  S<<"LE "<<m_LearningRate<<endl;
  
  return MString(S);
}


// MBackpropagationOutputNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
