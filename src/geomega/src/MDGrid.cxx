/*
 * MDGrid.cxx
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
// MDGrid
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGrid.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDGrid)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MDGrid::c_Unknown = 0;
const unsigned int MDGrid::c_Voxel = 1;
const unsigned int MDGrid::c_Strip = 2;
const unsigned int MDGrid::c_MinType = 1;
const unsigned int MDGrid::c_MaxType = 2;


////////////////////////////////////////////////////////////////////////////////


MDGrid::MDGrid() : m_Type(c_Unknown)
{
  // Construct an instance of MDGrid 
  m_XNBins = 1;
  m_YNBins = 1;
  m_ZNBins = 1;
}


////////////////////////////////////////////////////////////////////////////////


MDGrid::MDGrid(unsigned int Type, unsigned int XBins, 
               unsigned int YBins, unsigned int ZBins)
{
  // Construct an instance of MDGrid

  Set(Type, XBins, YBins, ZBins);
}


////////////////////////////////////////////////////////////////////////////////


MDGrid::MDGrid(const MDGrid& Grid)
{
  // Copy constructor

  m_Type = Grid.m_Type;
  m_Grid = Grid.m_Grid;
  m_XNBins = Grid.m_XNBins;
  m_YNBins = Grid.m_YNBins;
  m_ZNBins = Grid.m_ZNBins;
}


////////////////////////////////////////////////////////////////////////////////


MDGrid::~MDGrid()
{
  // Delete this instance of MDGrid
}


////////////////////////////////////////////////////////////////////////////////


bool MDGrid::Set(unsigned int Type, unsigned int XBins, 
                 unsigned int YBins, unsigned int ZBins)
{
  // Set the grid

  bool Return = true;

  if (XBins < 1) {
    merr<<"XBins must be larger than 0"<<show;
    XBins = 1;
    Return = false;
  }

  if (YBins < 1) {
    merr<<"YBins must be larger than 0"<<show;
    YBins = 1;
    Return = false;
  }

  if (ZBins < 1) {
    merr<<"ZBins must be larger than 0"<<show;
    ZBins = 1;
    Return = false;
  }

  m_XNBins = XBins;
  m_YNBins = YBins;
  m_ZNBins = ZBins;

  m_Grid.resize(m_XNBins*m_YNBins*m_ZNBins, 0.0);

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGrid::SetVoxelValue(double Value, unsigned int x, 
                           unsigned int y, unsigned int z)
{
  // Store Value at the correct position in the grid

  if (x < m_XNBins && y < m_YNBins && z < m_ZNBins) {
    m_Grid[x + y*m_XNBins + z*m_XNBins*m_YNBins] = Value;
    return true;
  } else {
    mout<<"   ***  Error  ***  in a grid "<<endl;
    mout<<"Index out of bounds: "<<x<<"?<?"<<m_XNBins<<"  "<<y<<"?<?"<<m_YNBins<<" "<<z<<"?<?"<<m_ZNBins<<endl;
    massert(false);
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDGrid::GetVoxelValue(unsigned int x, unsigned int y, 
                             unsigned int z) const
{
  // Store Value at the correct position in the grid

  if (x < m_XNBins && y < m_YNBins && z < m_ZNBins) {
    return m_Grid[x + y*m_XNBins + z*m_XNBins*m_YNBins];
  } else {
    mout<<"   ***  Error  ***  in a grid "<<endl;
    mout<<"Index out of bounds: "<<x<<"?<?"<<m_XNBins<<"  "<<y<<"?<?"<<m_YNBins<<" "<<z<<"?<?"<<m_ZNBins<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDGrid::GetVoxelValue(const MDGridPoint& Point) const
{
  // Store Value at the correct position in the grid

  if (Point.GetXGrid() < m_XNBins && 
      Point.GetYGrid() < m_YNBins && 
      Point.GetZGrid() < m_ZNBins) {
    return m_Grid[Point.GetXGrid() + Point.GetYGrid()*m_XNBins + Point.GetZGrid()*m_XNBins*m_YNBins];
  } else {
    mout<<"   ***  Error  ***  in a grid "<<endl;
    mout<<"Index out of bounds: "<<Point.GetXGrid()<<"?<?"<<m_XNBins<<"  "<<Point.GetYGrid()<<"?<?"<<m_YNBins<<" "<<Point.GetZGrid()<<"?<?"<<m_ZNBins<<endl;
    massert(false);
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MDGrid& Grid)
{
  os<<"Grid (x:"<<Grid.m_XNBins<<" y:"<<Grid.m_YNBins<<" z:"<<Grid.m_ZNBins<<")"<<endl;

  for (unsigned int z = 0; z < Grid.m_ZNBins; ++z) {
    os<<"z = "<<z<<endl;
    for (unsigned int y = 0; y < Grid.m_YNBins; ++y) {
      for (unsigned int x = 0; x < Grid.m_XNBins; ++x) {
        os<<Grid.m_Grid[x + y*Grid.m_XNBins + z*Grid.m_XNBins*Grid.m_YNBins]<<"\t";
      }
      os<<endl;
    }
    os<<endl;
  }

  return os;
}

// MDGrid.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
