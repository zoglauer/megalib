#ifndef __MBinnerSpherical__
#define __MBinnerSpherical__


class MBinnerSpherical
{

  public :

    //! Find a bin
    //! Theta (= latitude) and phi (= longitude) are in (mathematical) spherical coordinates
    //! The bins are arranged along the iso-latitude lines starting at the north pole (theta = latitude = 0)
    //! Theta and phi are in radians!
    virtual unsigned int FindBin (double Theta, double Phi) const = 0;

    //! Get number of bins
    virtual unsigned int GetNBins() const  = 0;

    //! Returns all bin centers as vector
    virtual vector<double> GetBinCenters(unsigned int Bin) const = 0;
  
    
// #ifdef ___CLING___
//  public:
//   ClassDef(MBinnerSpherical, 0) // no description
// #endif

};

#endif


////////////////////////////////////////////////////////////////////////////////


