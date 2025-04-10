/*
 * MResponseMatrixOx.cxx
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
// MResponseMatrixOx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMatrixOx.h"

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
ClassImp(MResponseMatrixOx)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned long MResponseMatrixOx::c_SizeLimit = numeric_limits<long>::max();
const unsigned long MResponseMatrixOx::c_Outside = numeric_limits<long>::max()-1;
const unsigned int MResponseMatrixOx::c_UnusedAxis = 98765432;

////////////////////////////////////////////////////////////////////////////////


MResponseMatrixOx::MResponseMatrixOx() : MResponseMatrix(), m_ValuesCentered(true)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixOx::MResponseMatrixOx(MString Name) : MResponseMatrix(Name), m_ValuesCentered(true)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MResponseMatrixOx::~MResponseMatrixOx()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixOx::IsIncreasing(vector<float> Axis) const
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


bool MResponseMatrixOx::AreIncreasing(unsigned int order,
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


int MResponseMatrixOx::FindBinCentered(const vector<float>& Array, float Value) const
{
  // Does a simple binary search to find the correct bin centered interpolation:

  //                   x-----.-----x-----.-----x-----.-----x
  //  Case 1: value =               ^^^^^
  //                         ^ This bin
  //  Case 2: value =                     ^^^^^
  //                                     ^ This bin
  //  Case 3: value =   ^^^^^
  //                    -1


  int Bin = FindBin(Array, Value);

  // Array.size() is guaranteed to be >= 2
  if (Bin >= 0 && Bin < int(Array.size()) - 1 && Value < 0.5*(Array[Bin+1] + Array[Bin])) {
    --Bin;
  }

  return Bin;



  // Old:
  /*
  massert(Array.size() >= 2);

  if (Value < Array[0]) return -1;
  if (Value >= Array.back()) return Array.size();

  // Very simple search:
  int i_max = int(Array.size());
  for (int i = 1; i < i_max; ++i) {
    if (0.5*(Array[i] + Array[i-1]) > Value) {
      return i-2; // Why "-2"
    }
  }

  return i_max-1;
  */
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMatrixOx::FindBin(const vector<float>& Array, float Value) const
{
  // Does a simple binary search to find the correct bin:
  // Finds the last bin which is smaller


  massert(Array.size() >= 2);

  if (Value < Array.front()) return -1;
  if (Value >= Array.back()) return Array.size();

  // C++ version
  auto UpperBound = upper_bound(Array.begin(), Array.end(), Value);
  return int(UpperBound - Array.begin()) - 1;


  /*
  // The following has been optimized for the icc compiler!
  if (Array.size() < 32) {

    // Simple search:
    vector<float>::const_iterator Iter;
    for (Iter = Array.begin(); Iter != Array.end(); ++Iter) {
      if ((*Iter) > Value) {
        return int(Iter - Array.begin()) - 1;
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
    return int(min);
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMatrixOx::Read(MString FileName, const bool MultiThreaded)
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

  Ok = ReadSpecific(Parser, Type, Version, MultiThreaded);

  if (g_Verbosity == c_Chatty) {
    mdebug<<"File \""<<FileName<<"\" with "<<GetNBins()
          <<" entries read in "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  Parser.Close();

  return Ok;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a string with statistics numbers
MString MResponseMatrixOx::GetStatistics() const
{
  ostringstream out;

  out<<"Statistics for response matrix \""<<m_Name<<"\":"<<endl;
  out<<endl;
  out<<"Number of axes:         "<<m_Order<<endl;
  out<<"Number of dimensions:   "<<m_Order<<endl;
  out<<"Number of bins:         "<<GetNBins()<<endl;
  out<<"Maximum:                "<<GetMaximum()<<endl;
  out<<"Minimum:                "<<GetMinimum()<<endl;
  out<<"Sum:                    "<<GetSum()<<endl;
  out<<"Average value:         "<<GetSum()/GetNBins()<<endl;
  out<<endl;

  out<<"Axes:"<<endl;
  for (unsigned int i = 1; i <= GetOrder(); ++i) {
    out<<"  x"<<i<<":  "<<GetAxisName(i)<<" (from "<<GetAxisContent(0, i)
      <<" to "<<GetAxisContent(GetAxisBins(i), i)
      <<" in "<<GetAxisBins(i)<<" bins)"<<endl;
  }

  return out.str();
}


// MResponseMatrixOx.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
