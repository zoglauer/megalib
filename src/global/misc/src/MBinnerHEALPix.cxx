// Include the header:
#include "MBinnerHEALPix.h"

// Standard libs:
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


#ifdef ___CLING___
ClassImp(MBinnerHEALPix)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerHEALPix::MBinnerHEALPix(unsigned int NSide) : m_NumberOfBins(NSide)
{
  // TODO Check nside valid
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

  //TODO actual calculation
  return 0
}

//! Return axis bins edges for external drawing
vector<vector<double>> MBinnerHEALPix::GetDrawingAxisBinEdges() const
{

}

//! Return the bin center(s) of the given axis bin
//! Can throw: MExceptionIndexOutOfBounds
vector<double> MBinnerHEALPix::GetBinCenters(unsigned int Bin) const
{


}


//! Returns all bin centers as vector
vector<MVector> MBinnerHEALPix::GetAllBinCenters() const
{

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
  out<<"AD "<< GetNSide() << "  " << "RING" << endl;
}  

// MBinnerHEALPix.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

