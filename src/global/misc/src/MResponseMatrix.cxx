/*
 * MResponseMatrix.cxx
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
// MResponseMatrix
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrix.h"

// Standard libs:
#include <limits>
#include <iostream>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMatrix)
#endif


////////////////////////////////////////////////////////////////////////////////


const float MResponseMatrix::c_ShowX = numeric_limits<float>::max()/2.22;
const float MResponseMatrix::c_ShowY = numeric_limits<float>::max()/2.23;
const float MResponseMatrix::c_ShowZ = numeric_limits<float>::max()/2.24;
const float MResponseMatrix::c_ShowNo = numeric_limits<float>::max()/2.25;


////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::MResponseMatrix() : m_Name("Unnamed response matrix"), m_Order(0), m_NumberOfSimulatedEvents(0), m_FarFieldStartArea(0), m_SpectralType(""), m_Hash(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::MResponseMatrix(MString Name) : m_Name(Name), m_Order(0), m_NumberOfSimulatedEvents(0), m_FarFieldStartArea(0), m_SpectralType(""), m_Hash(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::~MResponseMatrix()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MResponseMatrix::Clear()
{
  // Clear all data
  
  m_Name = "Unnamed response matrix";
  m_Order = 0;
  m_NumberOfSimulatedEvents = 0;
  m_FarFieldStartArea = 0;
  m_SpectralType = "";
  m_SpectralParameters.clear();
  m_Hash = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Write some basic data to the file/stream 
void MResponseMatrix::WriteHeader(ostringstream& out)
{
  out<<"# Response Matrix "<<m_Order<<endl;
  out<<"Version 1"<<endl;
  out<<endl;
  out<<"# Name"<<endl;
  out<<"NM "<<m_Name<<endl;
  out<<endl;
  out<<"# The order of the matrix"<<endl;
  out<<"OD "<<m_Order<<endl;
  out<<endl;
  out<<"# The number of simulated events"<<endl;
  out<<"TS "<<m_NumberOfSimulatedEvents<<endl;
  out<<endl;
  out<<"# The far-field start area (if zero a non-far-field simulation, or non-spherical start area was used)"<<endl;
  out<<"SA "<<m_FarFieldStartArea<<endl;
  out<<endl;
  out<<"# The spectral parameters (empty if not set)"<<endl;
  out<<"SM "<<m_SpectralType;
  for (unsigned int p = 0; p < m_SpectralParameters.size(); ++p) {
    out<<" "<<m_SpectralParameters[p];
  }
  out<<endl;
  out<<endl;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrix::Read(MString FileName)
{
  // Read the data from file directly into this matrix

  MTimer Timer;
  if (g_Verbosity == c_Chatty) {
    mout<<"Started reading file \""<<FileName<<"\" ... This way take a while ..."<<endl;
  }
  
  MFileResponse Parser;
  if (Parser.Open(FileName) == false) return false;

  bool Ok = true;

  // Store header information:
  int Version = Parser.GetVersion();
  MString Type = Parser.GetFileType(); 
  SetName(Parser.GetName());
  SetHash(Parser.GetHash());
  SetSimulatedEvents(Parser.GetSimulatedEvents());
  SetFarFieldStartArea(Parser.GetFarFieldStartArea());
  SetSpectrum(Parser.GetSpectralType(), Parser.GetSpectralParameters());

  Ok = ReadSpecific(Parser, Type, Version);

  if (g_Verbosity == c_Chatty) {  
    mdebug<<"File \""<<FileName<<"\" with "<<GetNBins()
          <<" entries read in "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  Parser.Close();

  return Ok;
}


// MResponseMatrix.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
