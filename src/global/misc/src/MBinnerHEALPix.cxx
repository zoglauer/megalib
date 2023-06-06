/*
 * MBinnerHEALPix.cxx
 *
 *
 * Copyright (C) by Israel Martinez Castellanos & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Martinez Castellanos & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MBinnerHEALPix.h"
#include "MExceptions.h"

// Standard libs:
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

// HEALPix libs:
#include <pointing.h>

// Other libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinnerHEALPix)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerHEALPix::MBinnerHEALPix(unsigned int Order)
{
  m_HealPix = Healpix_Base(Order, Healpix_Ordering_Scheme::RING);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinnerHEALPix::~MBinnerHEALPix()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Find a bin
unsigned int MBinnerHEALPix::FindBin(double Theta, double Phi) const
{
  pointing p(Theta, Phi);
  return m_HealPix.ang2pix(p);
}


////////////////////////////////////////////////////////////////////////////////


//! Return axis bins edges for external drawing
vector<vector<double>> MBinnerHEALPix::GetDrawingAxisBinEdges() const
{
  throw MExceptionArbitrary("GetDrawingAxisBinEdges not implemented for HEALPix");

  return {{}};
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bin center(s) of the given axis bin
//! Can throw: MExceptionIndexOutOfBounds
vector<double> MBinnerHEALPix::GetBinCenters(unsigned int Bin) const
{
  pointing P = m_HealPix.pix2ang(Bin);

  return { P.theta, P.phi };
}


////////////////////////////////////////////////////////////////////////////////


//! Returns all bin centers as vector
vector<MVector> MBinnerHEALPix::GetAllBinCenters() const
{
  vector<MVector> Vectors;
  
  for (unsigned int b = 0; b < GetNBins(); ++b) {
    vector<double> Centers = GetBinCenters(b);
    
    MVector V;
    V.SetMagThetaPhi(1.0, Centers[0], Centers[1]);
    Vectors.push_back(V);
  }
  
  return Vectors;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the minimum axis values
vector<double> MBinnerHEALPix::GetMinima() const
{
  return { 0, 0 };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the maximum axis values
vector<double> MBinnerHEALPix::GetMaxima() const
{
  return { 180, 360 };
}


////////////////////////////////////////////////////////////////////////////////


void MBinnerHEALPix::Write(MString name, ostringstream& out) const
{
  out<<"# Axis name"<<endl;
  out<<"AN \"" << name << "\""<<endl;
  out<<"# Axis type"<<endl;
  out<<"AT 2D HEALPix"<<endl;
  out<<"# Axis data"<<endl;
  out<<"AD "<< GetOrder() << " " << "RING" << endl;
}  


// MBinnerHEALPix.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

