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
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrix)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned long MResponseMatrix::c_SizeLimit = numeric_limits<long>::max();
const unsigned long MResponseMatrix::c_Outside = numeric_limits<long>::max()-1;
const float MResponseMatrix::c_ShowX = numeric_limits<float>::max()/2.22;
const float MResponseMatrix::c_ShowY = numeric_limits<float>::max()/2.23;
const float MResponseMatrix::c_ShowZ = numeric_limits<float>::max()/2.24;
const float MResponseMatrix::c_ShowNo = numeric_limits<float>::max()/2.25;
const unsigned int MResponseMatrix::c_UnusedAxis = 98765432;

////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::MResponseMatrix() : 
  m_ValuesCentered(true), m_Name("None"), m_Order(0), m_NumberOfSimulatedEvents(0), m_FarFieldStartArea(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::MResponseMatrix(MString Name) : 
  m_ValuesCentered(true), m_Name(Name), m_Order(0), m_NumberOfSimulatedEvents(0), m_FarFieldStartArea(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrix::~MResponseMatrix()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrix::IsIncreasing(vector<float> Axis) const
{
  // Checks whether the axis is sorted in increasing order:

  for (unsigned int i = 0; i < Axis.size()-1; ++i) {
    if (Axis[i] >= Axis[i+1]) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrix::AreIncreasing(unsigned int order, 
                                    unsigned int a1,
                                    unsigned int a2,
                                    unsigned int a3,
                                    unsigned int a4,
                                    unsigned int a5,
                                    unsigned int a6,
                                    unsigned int a7,
                                    unsigned int a8,
                                    unsigned int a9,
                                    unsigned int a10,
                                    unsigned int a11,
                                    unsigned int a12,
                                    unsigned int a13,
                                    unsigned int a14,
                                    unsigned int a15,
                                    unsigned int a16,
                                    unsigned int a17,
                                    unsigned int a18) const
{
  if (order >= 1 && a1 != c_UnusedAxis && (a1 < 1 || a1 > order)) return false;
  if (order >= 2 && a2 != c_UnusedAxis && (a2 < 1 || a2 > order)) return false;
  if (order >= 3 && a3 != c_UnusedAxis && (a3 < 1 || a3 > order)) return false;
  if (order >= 4 && a4 != c_UnusedAxis && (a4 < 1 || a4 > order)) return false;
  if (order >= 5 && a5 != c_UnusedAxis && (a5 < 1 || a5 > order)) return false;
  if (order >= 6 && a6 != c_UnusedAxis && (a6 < 1 || a6 > order)) return false;
  if (order >= 7 && a7 != c_UnusedAxis && (a7 < 1 || a7 > order)) return false;
  if (order >= 8 && a8 != c_UnusedAxis && (a8 < 1 || a8 > order)) return false;
  if (order >= 9 && a9 != c_UnusedAxis && (a9 < 1 || a9 > order)) return false;
  if (order >= 10 && a10 != c_UnusedAxis && (a10 < 1 || a10 > order)) return false;
  if (order >= 11 && a11 != c_UnusedAxis && (a11 < 1 || a11 > order)) return false;
  if (order >= 12 && a12 != c_UnusedAxis && (a12 < 1 || a12 > order)) return false;
  if (order >= 13 && a13 != c_UnusedAxis && (a13 < 1 || a13 > order)) return false;
  if (order >= 14 && a14 != c_UnusedAxis && (a14 < 1 || a14 > order)) return false;
  if (order >= 15 && a15 != c_UnusedAxis && (a15 < 1 || a15 > order)) return false;
  if (order >= 16 && a16 != c_UnusedAxis && (a16 < 1 || a16 > order)) return false;
  if (order >= 17 && a17 != c_UnusedAxis && (a17 < 1 || a17 > order)) return false;
  if (order >= 18 && a18 != c_UnusedAxis && (a18 < 1 || a18 > order)) return false;

  if (order >= 2 && a2 != c_UnusedAxis && a2 <= a1) return false;
  if (order >= 3 && a3 != c_UnusedAxis && a3 <= a2) return false;
  if (order >= 4 && a4 != c_UnusedAxis && a4 <= a3) return false;
  if (order >= 5 && a5 != c_UnusedAxis && a5 <= a4) return false;
  if (order >= 6 && a6 != c_UnusedAxis && a6 <= a5) return false;
  if (order >= 7 && a7 != c_UnusedAxis && a7 <= a6) return false;
  if (order >= 8 && a8 != c_UnusedAxis && a8 <= a7) return false;
  if (order >= 9 && a9 != c_UnusedAxis && a9 <= a8) return false;
  if (order >= 10 && a10 != c_UnusedAxis && a10 <= a9) return false;
  if (order >= 11 && a11 != c_UnusedAxis && a11 <= a10) return false;
  if (order >= 12 && a12 != c_UnusedAxis && a12 <= a11) return false;
  if (order >= 13 && a13 != c_UnusedAxis && a13 <= a12) return false;
  if (order >= 14 && a14 != c_UnusedAxis && a14 <= a13) return false;
  if (order >= 15 && a15 != c_UnusedAxis && a15 <= a14) return false;
  if (order >= 16 && a16 != c_UnusedAxis && a16 <= a15) return false;
  if (order >= 17 && a17 != c_UnusedAxis && a17 <= a16) return false;
  if (order >= 18 && a18 != c_UnusedAxis && a18 <= a16) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMatrix::FindBinCentered(const vector<float>& Array, float Value) const
{
  // Does a simple binary search to find the correct !centered! bin:
  
  massert(Array.size() >= 2);

  if (Value < Array[0]) return -1;
  if (Value >= Array.back()) return Array.size(); 

  // Very simple search:
  int i_max = int(Array.size());
  for (int i = 1; i < i_max; ++i) {
    if (0.5*(Array[i] + Array[i-1]) > Value) {
      return i-2;
    }
  }

  return i_max-1;
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMatrix::FindBin(const vector<float>& Array, float Value) const
{
  // Does a simple binary search to find the correct bin:
  
  massert(Array.size() >= 2);

  if (Value < Array.front()) return -1;
  if (Value >= Array.back()) return Array.size(); 

  // The following has been optimized for the icc compiler!

  if (Array.size() < 32) {

    // Simple search:
    vector<float>::const_iterator Iter;
    for (Iter = Array.begin(); Iter != Array.end(); ++Iter) {
      if ((*Iter) > Value) {
        return int(Iter - Array.begin()) - 1;
        break;
      } 
    }
    return -1; // Should never be reached...

  } else {

    // Binary search:
    unsigned int upper = Array.size();
    unsigned int center = 1;
    unsigned int lower = 0;

    while (upper-lower > 1) {
      center = (upper+lower) >> 1;
      if (Value == Array[center]) {
        return int(center);
      }
      if (Value < Array[center]) {
        upper = center;
      } else {
        lower = center;
      }
    }
    return int(lower);
  }
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
  out<<"# Are the values centered?"<<endl;
  out<<"CE "<<((m_ValuesCentered == true) ? "true" : "false")<<endl;
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
  SetValuesCenteredFlag(Parser.AreValuesCentered());
  SetHash(Parser.GetHash());
  SetSimulatedEvents(Parser.GetSimulatedEvents());
  SetFarFieldStartArea(Parser.GetFarFieldStartArea());

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
