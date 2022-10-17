// Include the header:
#include "MBinnerHEALPix.h"

// Standard libs:
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

// Other libs
#include <pointing.h>

#ifdef ___CLING___
ClassImp(MBinnerHEALPix)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerHEALPix::MBinnerHEALPix(unsigned int order)
{
  m_healpix = Healpix_Base(order, Healpix_Ordering_Scheme::RING);
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

  int pix = m_healpix.ang2pix(p);
  
  return pix;
}


//! Return axis bins edges for external drawing
vector<vector<double>> MBinnerHEALPix::GetDrawingAxisBinEdges() const
{

}

//! Return the bin center(s) of the given axis bin
//! Can throw: MExceptionIndexOutOfBounds
vector<double> MBinnerHEALPix::GetBinCenters(unsigned int Bin) const
{
  pointing p = m_healpix.pix2ang(Bin);

  return {p.theta, p.phi};
}


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

//! Return the minimum axis values
vector<double> MBinnerHEALPix::GetMinima() const
{
  return { 0, 0};
}


////////////////////////////////////////////////////////////////////////////////


//! Return the minimum axis values
vector<double> MBinnerHEALPix::GetMaxima() const
{
  return { 180, 360 };
}


void MBinnerHEALPix::Write(MString name, ostringstream& out) const
{
  out<<"# Axis name"<<endl;
  out<<"AN \"" << name << "\""<<endl;
  out<<"# Axis type"<<endl;
  out<<"AT 2D HEALPix"<<endl;
  out<<"# Axis data"<<endl;
  out<<"AD "<< GetOrder() << "  " << "RING" << endl;
}  

// MBinnerHEALPix.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

