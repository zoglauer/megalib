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

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMatrixAxisSpheric)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseMatrixAxisSpheric::MResponseMatrixAxisSpheric(const MString& ThetaAxisName, const MString& PhiAxisName) : MResponseMatrixAxis(ThetaAxisName)
{
  m_Dimension = 2;
  m_Names.push_back(PhiAxisName);
  m_Binner.Create(1);
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
  Axis->m_IsLogarithmic = m_IsLogarithmic;
  
  return Axis;
}


////////////////////////////////////////////////////////////////////////////////


//! Equality operator
bool MResponseMatrixAxisSpheric::operator==(const MResponseMatrixAxisSpheric& Axis)
{
  // We don't care about names, only the physical properties
  
  if (m_Dimension != Axis.m_Dimension) {
    return false;
  }
  
  if (m_Binner != Axis.m_Binner) {
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the axis in FISBEL mode
void MResponseMatrixAxisSpheric::SetFISBEL(unsigned int NBins) 
{
  m_Binner.Create(NBins);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bin, given theta=latitude and phi=longitude 
unsigned int MResponseMatrixAxisSpheric::GetAxisBin(double Theta, double Phi) const
{
  return m_Binner.FindBin(Theta*c_Rad, Phi*c_Rad);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the axis bins for drawing --- those might be narrower than the real bins
vector<vector<double>> MResponseMatrixAxisSpheric::GetDrawingAxisBinEdges() const
{
  vector<vector<double>> AxisBinEdges = m_Binner.GetDrawingAxisBinEdges();
  AxisBinEdges.push_back(AxisBinEdges[0]);
  AxisBinEdges.erase(AxisBinEdges.begin());
  
  for (unsigned int x = 0; x < AxisBinEdges.size(); ++x) {
    for (unsigned int y = 0; y < AxisBinEdges[x].size(); ++y) {
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
  return { 0, 0 };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the minimum axis values
vector<double> MResponseMatrixAxisSpheric::GetMaxima() const
{
  return { 180, 360 };
}


////////////////////////////////////////////////////////////////////////////////


//! Write the content to a stream
void MResponseMatrixAxisSpheric::Write(ostringstream& out)
{
  out<<"# Axis name"<<endl;
  out<<"AN \""<<m_Names[0]<<"\" \""<<m_Names[1]<<"\""<<endl;
  out<<"# Axis type"<<endl;
  out<<"AT 2D FISBEL"<<endl;
  out<<"# Axis data"<<endl;
  out<<"AD "<<m_Binner.GetNBins()<<endl;
}


// MResponseMatrixAxisSpheric.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
