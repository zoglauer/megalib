#ifndef __MBinnerHEALPix__
#define __MBinnerHEALPix__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBinnerSpherical.h"

// Forward declarations:

// Other libs:
#include <healpix_base.h>

////////////////////////////////////////////////////////////////////////////////

//! HEALPix: https://healpix.sourceforge.io/
//! RING scheme by default
//! This is just the binner, it does not store any data itself
class MBinnerHEALPix : public MBinnerSpherical
{
  // public interface:
 public:
  //! Default constructor
  MBinnerHEALPix(unsigned int order);
  //! Default destuctor 
  virtual ~MBinnerHEALPix();
    
  // //! Check if we have equal bins
  // bool operator ==(const MBinnerFISBEL& Binner) const;
  // //! ... or not
  // bool operator !=(const MBinnerFISBEL& Binner) const { return !operator==(Binner); }
  
  //! Find a bin
  //! Theta (= latitude) and phi (= longitude) are in (mathematical) spherical coordinates
  //! The bins are arranged along the iso-latitude lines starting at the north pole (theta = latitude = 0)
  //! Theta and phi are in radians!
  unsigned int FindBin(double Theta, double Phi) const;
  
  //! Get number of bins
  unsigned int GetNBins() const {return m_healpix.Npix();};
  
  //! Get NSIDE parameters
  unsigned int GetNSide() const {return m_healpix.Nside();};

  //! Get NSIDE parameters
  unsigned int GetOrder() const {return m_healpix.Order();};    
    
  //! Return the minimum axis values [min theta, min phi]
  vector<double> GetMinima() const;

  //! Return the minimum axis values [max theta, max phi]
  vector<double> GetMaxima() const;
    
  //! Get the bin center (returns: theta, phi in radians)
  //! Can throw: MExceptionIndexOutOfBounds
  vector<double> GetBinCenters(unsigned int Bin) const;
  
  //! Returns all bin centers as vector
  vector<MVector> GetAllBinCenters() const;
  
  //! Return axis bins edges for external drawing (1st array: longitude/phi, 2nd array: latitude/theta)
  vector<vector<double>> GetDrawingAxisBinEdges() const;

  //! Write the content to a stream
  void Write(MString name, ostringstream& out) const;
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:

  // Healpix grid nside and scheme
  Healpix_Base m_healpix; 
    
  // Scheme
  // TODO
    
  // private members:
 private:

  
#ifdef ___CLING___
 public:
  ClassDef(MBinnerHEALPix, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

