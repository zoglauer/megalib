/*
 * MDGrid.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDGrid__
#define __MDGrid__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGridPoint.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDGrid
{
  // public interface:
 public:
  //! Default constructor
  MDGrid();
  //! Constructor which sets the grid
  MDGrid(unsigned int Type, unsigned int XBins = 1, 
         unsigned int YBins = 1, unsigned int ZBins = 1);
  //! Copy constructor
  MDGrid(const MDGrid& Grid);
  //! Default destructor
  virtual ~MDGrid();

  //! Set the grid 
  bool Set(unsigned int Type, unsigned int XBins = 1, 
           unsigned int YBins = 1, unsigned int ZBins = 1);

  //! Set a value to the grid voxel
  bool SetVoxelValue(double Value, unsigned int x, 
                         unsigned int y = 0, unsigned int z = 0);
  //! Retrieve the value of a grid voxel
  double GetVoxelValue(unsigned int x, unsigned int y = 0, 
                           unsigned int z = 0) const;
  //! Retrieve the value of a grid voxel
  double GetVoxelValue(const MDGridPoint& Point) const;
  
  static const unsigned int c_Unknown;
  static const unsigned int c_Voxel;
  static const unsigned int c_Strip;
  static const unsigned int c_MinType;
  static const unsigned int c_MaxType;

 
  // protected methods:
 protected:
  
  // private methods:
 private:
  friend ostream& operator<<(ostream& os, const MDGrid& Grid);


  // protected members:
 protected:


  // private members:
 private:
  //! Type of the grid
  unsigned int m_Type;
  //! The grid (3D):
  vector<double> m_Grid;
  //! Number of bins in x direction
  unsigned int m_XNBins;
  //! Number of bins in y direction
  unsigned int m_YNBins;
  //! Number of bins in z direction
  unsigned int m_ZNBins;


#ifdef ___CINT___
 public:
  ClassDef(MDGrid, 0) // no description
#endif

};

ostream& operator<<(ostream& os, const MDGrid& Grid);

#endif


////////////////////////////////////////////////////////////////////////////////
