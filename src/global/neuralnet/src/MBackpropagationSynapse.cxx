/*
 * MBackpropagationSynapse.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


////////////////////////////////////////////////////////////////////////////////
//
// MBackpropagationSynapse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackpropagationSynapse.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
#include <fstream>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MNeuron.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackpropagationSynapse)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackpropagationSynapse::MBackpropagationSynapse(int Size) : 
MSynapse(Size) 
{
  // Default and standard constructor
  
  m_Weight = 0.5*gRandom->Rndm()-0.25;
  m_Delta = 0.0;
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationSynapse::MBackpropagationSynapse(const MBackpropagationSynapse& S) : 
MSynapse(S) 
{
  // Copy constructor
  
  m_Delta = S.m_Delta;
}


////////////////////////////////////////////////////////////////////////////////



void MBackpropagationSynapse::SetWeight(double Delta) 
{
  double Momentum = m_OutNeuron->GetMomentum();
  m_Weight += Delta + (Momentum*m_Delta);
  m_Delta = Delta;
  //cout<<"Delta: "<<m_Delta<<" Weight: "<<m_Weight<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackpropagationSynapse::Stream(MFile& File, const int Version, const bool Read)
{
  // Read/Write capability
  
  bool Return = true;
  
  if (Read == false) {
    File.Write(ToString());
  } else {
    MSynapse::Stream(File, Version, Read);
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MBackpropagationSynapse::ParseLine(MString Line)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached
  
  int Ret = MSynapse::ParseLine(Line);
  if (Ret != 2) {
    return Ret;
  }
  
  Ret = 0;
  
  if (Line[0] == 'D' && Line[1] == 'E') {
    if (sscanf(Line, "DE %lf", &m_Delta) != 1) {
      mout<<"Unable to parse DE of synapse "<<m_ID<<"!"<<endl;
      Ret = 1;
    }
  } else {
    Ret = 2;
  }
  
  
  return Ret;
}


////////////////////////////////////////////////////////////////////////////////


MString MBackpropagationSynapse::ToString() const
{
  ostringstream S;
  
  S<<MSynapse::ToString();
  S<<"DE "<<m_Delta<<endl;
  
  return MString(S);
}


// MBackpropagationSynapse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
