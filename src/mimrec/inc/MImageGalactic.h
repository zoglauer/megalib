/*
 * MImageGalactic.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImageGalactic__
#define __MImageGalactic__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGaxis.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImageGalactic : public MImage2D
{
  // public interface:
 public:
  //! Default constructor
  MImageGalactic();
  //! Standard constructor
  MImageGalactic(MString Title, double* Array,
                 MString xTitle, double xMin, double xMax, int xNBins, 
                 MString yTitle, double yMin, double yMax, int yNBins, 
                 int Spectrum = c_Rainbow, int DrawOption = c_COLCONT4Z,
                 MString SourceCatalog = "");
  //! Standard destructor
  virtual ~MImageGalactic();

  //! Clone this image
  virtual MImage* Clone();

  //! Set the image array and redisplay it
  virtual void SetImageArray(double* Array);

  //! Display the histogram in the given canvas
  virtual void Display(TCanvas* Canvas = nullptr, bool Normalize = true);
 
  // protected methods:
 protected:
  void AddNamedSources();

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:
  //! The new x axis's
  TGaxis* m_XAxis;

  //! The name of the source catalog
  MString m_SourceCatalog;

#ifdef ___CINT___
 public:
  ClassDef(MImageGalactic, 0) // class containing an astropysical image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
