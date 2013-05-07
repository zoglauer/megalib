/*
 * MFunction3DSpherical.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFunction3DSpherical__
#define __MFunction3DSpherical__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TSpline.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MFunction3D.h"

// Standard libs:
#include <vector>
using std::vector;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFunction3DSpherical : public MFunction3D
{
  // public interface:
 public:
  //! Default constructor (x:phi, y:theta, z:energy)
  MFunction3DSpherical();
  //! Copy constructor
  MFunction3DSpherical(const MFunction3DSpherical& Function3DSpherical);
  //! Default destructor
  virtual ~MFunction3DSpherical();

  //! Default assignment constructor
  const MFunction3DSpherical& operator=(const MFunction3DSpherical& F);

  //! Set the basic data, load the file and parse it
  bool Set(const MString FileName,
           const MString KeyWord,
           const unsigned int InterpolationType = c_InterpolationLinear);

  bool Set(const vector<double>& X, const vector<double>& Y, const vector<double>& Z, const vector<double>& Values, 
           unsigned int InterpolationType = c_InterpolationLinear) { return MFunction3D::Set(X, Y, Z, Values, InterpolationType); }

  //! Return the total content of the function
  //! This is a simplified integration which uses the value at the bin centers and the (3D) bin size
  double Integrate() const;

  //! Return random numbers x, y, z distributed as the underlying function
  void GetRandom(double& x, double& y, double& z);

  //! Plot the function in a Canvas (diagnostics only)
  void Plot(bool Random = false);

  //! Save as a file
  virtual bool Save(const MString FileName, const MString Keyword = "AP");

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  

#ifdef ___CINT___
 public:
  ClassDef(MFunction3DSpherical, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
