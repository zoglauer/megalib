/*
 * MResponseMatrixAxisSpheric.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMatrixAxisSpheric__
#define __MResponseMatrixAxisSpheric__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixAxis.h"
#include "MBinnerFISBEL.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This defines a response matrix axis
class MResponseMatrixAxisSpheric : public MResponseMatrixAxis
{
  // public interface:
 public:
  //! Default constructor
  MResponseMatrixAxisSpheric(const MString& Theta, const MString& Phi);
  //! Default destuctor 
  virtual ~MResponseMatrixAxisSpheric();
  
  //! Equality operator
  bool operator==(const MResponseMatrixAxisSpheric& Axis);
  //! Inequality operator
  bool operator!=(const MResponseMatrixAxisSpheric& Axis) { return !(operator==(Axis)); };

  //! Clone this axis
  virtual MResponseMatrixAxisSpheric* Clone() const;
  
  //! Set the axis in FISBEL mode
  void SetFISBEL(unsigned int NBins);
  

  //! Return the number of axis bins
  virtual unsigned int GetNumberOfBins() const { return m_Binner.GetNBins(); }
  //! Return the axis bin, given theta=latitude and phi=longitude 
  virtual unsigned int GetAxisBin(double Theta, double Phi) const;
  
  //! Test if theta and phi (in degree) are within the range of the axis - the second value is required
  virtual bool InRange(double Theta, double Phi) const;
  
  //! True if the axis has 1D bin edges
  virtual bool Has1DBinEdges() { return false; }
  //! Get the 1D bin edges
  //! Check with Has1DBinEdges first, because this is not guaranteed
  virtual vector<double> Get1DBinEdges() { return vector<double>(); }
  
  //! Return the area of the given axis bin
  virtual double GetArea(unsigned int Bin) const;
  
  //! Return the axis bin edges for drawing --- those might be narrower than the real bins
  virtual vector<vector<double>> GetDrawingAxisBinEdges() const;
  //! Return the minimum axis values
  virtual vector<double> GetMinima() const;
  //! Return the minimum axis values
  virtual vector<double> GetMaxima() const;
  
  //! Write the content to a stream
  virtual void Write(ostringstream& out);
  
  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The binner
  MBinnerFISBEL m_Binner;
  

#ifdef ___CINT___
 public:
  ClassDef(MResponseMatrixAxisSpheric, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
