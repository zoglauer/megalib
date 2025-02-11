/*
 * MResponseMatrixAxisSpheric.cxx
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


// Include the header:
#include "MResponseMatrixAxisSpheric.h"

// Standard libs:
#include <math.h>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseMatrixAxisSpheric)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseMatrixAxisSpheric::MResponseMatrixAxisSpheric(const MString& ThetaAxisName, const MString& PhiAxisName) : MResponseMatrixAxis(ThetaAxisName)
{
  m_Dimension = 2;
  m_Names.push_back(PhiAxisName);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseMatrixAxisSpheric::~MResponseMatrixAxisSpheric()
{
}


////////////////////////////////////////////////////////////////////////////////



//! Clone this axis
MResponseMatrixAxisSpheric* MResponseMatrixAxisSpheric::Clone() const
{
  MResponseMatrixAxisSpheric* Axis = new MResponseMatrixAxisSpheric(m_Names[0], m_Names[1]); // Takes care of names and dimension
  Axis->m_BinEdges = m_BinEdges; // Should be empty anyway
  Axis->m_Binner = m_Binner;
  Axis->m_NumberOfBins = m_NumberOfBins;
  Axis->m_IsLogarithmic = m_IsLogarithmic;
  
  return Axis;
}


////////////////////////////////////////////////////////////////////////////////


//! Equality operator
// bool MResponseMatrixAxisSpheric::operator==(const MResponseMatrixAxisSpheric& Axis) const
// {
//   // We don't care about names, only the physical properties
  
//   if (m_Dimension != Axis.m_Dimension) {
//     return false;
//   }
  
//   if (*m_Binner != *Axis.m_Binner) {
//     return false;
//   }
  
//   return true;
// }


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in FISBEL mode
void MResponseMatrixAxisSpheric::SetFISBELByNumberOfBins(unsigned long NBins, double LongitudeShift)
{
  std::shared_ptr<MBinnerFISBEL> m_Binner_fisbel = std::make_shared<MBinnerFISBEL>();
  m_Binner_fisbel->Create(NBins, LongitudeShift*c_Rad);
  
  m_Binner = m_Binner_fisbel;

  m_NumberOfBins = NBins;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in HEALPix mode (ring scheme)
void MResponseMatrixAxisSpheric::SetHEALPixByOrder(int Order)
{
  std::shared_ptr<MBinnerHEALPix> m_Binner_healpix = std::make_shared<MBinnerHEALPix>(Order);

  m_Binner = m_Binner_healpix;

  m_NumberOfBins = m_Binner->GetNBins();
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in FISBEL based on a target pixel size
void MResponseMatrixAxisSpheric::SetFISBELByPixelSize(double PixelSize)
{
  int AngleBins = 4*c_Pi*c_Deg*c_Deg / PixelSize / PixelSize;
  if (AngleBins < 1) AngleBins = 1;

  SetFISBELByNumberOfBins(AngleBins);
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in HEALPIX based on the targeted number of bins
//! The actual number of bins is downscaled to the nearest order
//! O(-1): 1 bin, O(0): 12, O(1): 48, O(2): 192, O(3): 768, O(4): 3072, O(5): 12288, O(6): 49152, O(7): 196608, O(8): 786432, etc. (x4 each further increase)
void MResponseMatrixAxisSpheric::SetHEALPixByNumberOfBins(unsigned long NBins)
{
  // Find closest order:
  int Order = int(floor(log2(sqrt(NBins/12.0))));

  SetHEALPixByOrder(Order);
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in HEALPIX based on a target pixel size
void MResponseMatrixAxisSpheric::SetHEALPixByPixelSize(double PixelSize)
{
  // Make sure pixel size is > 0

  // First find ideal angle bins
  double AngleBins = 4*c_Pi*c_Deg*c_Deg / PixelSize / PixelSize;
  // Then find closest order:
  int Order = int(floor(log2(sqrt(AngleBins/12.0))));

  SetHEALPixByOrder(Order);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bin, given theta=latitude and phi=longitude 
unsigned long MResponseMatrixAxisSpheric::GetAxisBin(double Theta, double Phi) const
{
  return m_Binner->FindBin(Theta*c_Rad, Phi*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the area of the given axis bin
double MResponseMatrixAxisSpheric::GetArea(unsigned long Bin) const
{
  return 4*c_Pi/m_Binner->GetNBins() * c_Deg*c_Deg;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bins for drawing --- those might be narrower than the real bins
vector<vector<double>> MResponseMatrixAxisSpheric::GetDrawingAxisBinEdges() const
{
  vector<vector<double>> AxisBinEdges = m_Binner->GetDrawingAxisBinEdges();
  AxisBinEdges.push_back(AxisBinEdges[0]);
  AxisBinEdges.erase(AxisBinEdges.begin());
  
  for (unsigned long x = 0; x < AxisBinEdges.size(); ++x) {
    for (unsigned long y = 0; y < AxisBinEdges[x].size(); ++y) {
      AxisBinEdges[x][y] *= c_Deg;
    }
  }
  
  
  return AxisBinEdges;
}


////////////////////////////////////////////////////////////////////////////////


//! Test if theta and phi (in degree) are within the range of the axis - the second value is required
bool MResponseMatrixAxisSpheric::InRange(double Theta, double Phi) const
{
  if (Theta < 0 || Theta > 180) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the minimum axis values
vector<double> MResponseMatrixAxisSpheric::GetMinima() const
{
  return m_Binner->GetMinima();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the minimum axis values
vector<double> MResponseMatrixAxisSpheric::GetMaxima() const
{
  return m_Binner->GetMaxima();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bin center(s) of the given axis bin
//! Can throw: MExceptionIndexOutOfBounds
vector<double> MResponseMatrixAxisSpheric::GetBinCenters(unsigned long Bin) const
{
  if (Bin >= m_BinEdges.size() - 1) {
    throw MExceptionIndexOutOfBounds(0, m_BinEdges.size() - 1, Bin);
  }
  
  vector<double> Centers = m_Binner->GetBinCenters(Bin);
  Centers[0] *= c_Deg;
  Centers[1] *= c_Deg;
  
  return Centers;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bin centers of all axis bins as vector
//! Can throw: MExceptionIndexOutOfBounds
vector<MVector> MResponseMatrixAxisSpheric::GetAllBinCenters() const
{
  return m_Binner->GetAllBinCenters();
}


////////////////////////////////////////////////////////////////////////////////


//! Write the content to a stream
void MResponseMatrixAxisSpheric::Write(ostringstream& out)
{

  MString name = m_Names[0] + "\" \"" + m_Names[1];
  
  m_Binner->Write(name, out);
  
}


// MResponseMatrixAxisSpheric.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
