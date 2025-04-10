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
#include <memory>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixAxis.h"
#include "MBinnerFISBEL.h"
#include "MBinnerHEALPix.h"

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
  
  // //! Equality operator
  // bool operator==(const MResponseMatrixAxisSpheric& Axis) const;
  // //! Inequality operator
  // bool operator!=(const MResponseMatrixAxisSpheric& Axis) const { return !(operator==(Axis)); }

  //! Clone this axis
  virtual MResponseMatrixAxisSpheric* Clone() const;
  
  //! Set the axis in FISBEL mode with a longitude shift in degrees
  void SetFISBELByNumberOfBins(unsigned long NBins, double LongitudeShift = 0);
  //! Set the axis in FISBEL based on a target pixel size (deg)
  void SetFISBELByPixelSize(double PixelSize);

  //! Set the axis in HEALPIX based on the Order (ring scheme)
  void SetHEALPixByOrder(int Order);
  //! Set the axis in HEALPIX based on the targeted number of bins
  //! The actual number of bins is downscaled to the nearest order
  //! O(-1): 1 bin, O(0): 12, O(1): 48, O(2): 192, O(3): 768, O(4): 3072, O(5): 12288, O(6): 49152, O(7): 196608, O(8): 786432, etc. (x4 each further increase)
  void SetHEALPixByNumberOfBins(unsigned long NBins);
  //! Set the axis in HEALPIX based on a target pixel size (deg)
  //! The actual pixel size of bins is downscaled to the nearest order
  //! O(-1): 180, O(0): 58.632, O(1): 29.316, O(2): 14.658, O(3): 7.329, O(4): 3.664, O(5): 1.832, O(6): 0.916, O(7): 0.458, O(8): 0.229, etc. (div 2 each further decrease)
  void SetHEALPixByPixelSize(double PixelSize);
    
  //! Return the axis bin, given theta=latitude and phi=longitude in degrees
  virtual unsigned long GetAxisBin(double Theta, double Phi) const;
  
  //! Test if theta and phi (in degree) are within the range of the axis - the second value is required
  virtual bool InRange(double Theta, double Phi) const;
  
  //! True if the axis has 1D bin edges
  virtual bool Has1DBinEdges() const { return false; }
  //! Get the 1D bin edges
  //! Check with Has1DBinEdges first, because this is not guaranteed
  virtual vector<double> Get1DBinEdges() const { return vector<double>(); }
    
  //! Return the area of the given axis bin
  virtual double GetArea(unsigned long Bin) const;
  
  //! Return the axis bin edges for drawing --- those might be narrower than the real bins
  virtual vector<vector<double>> GetDrawingAxisBinEdges() const;
  //! Return the minimum axis values
  virtual vector<double> GetMinima() const;
  //! Return the minimum axis values
  virtual vector<double> GetMaxima() const;
  
  //! Return the bin center(s) of the given axis bin, theta & phi 
  //! Can throw: MExceptionIndexOutOfBounds
  virtual vector<double> GetBinCenters(unsigned long Bin) const;
  
  //! Return the bin center of all axis bins as vectors 
  //! Can throw: MExceptionIndexOutOfBounds
  virtual vector<MVector> GetAllBinCenters() const;
  
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
  //! The binner - either FISBEL or HEALPix
  std::shared_ptr<MBinnerSpherical> m_Binner;
  

#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixAxisSpheric, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
