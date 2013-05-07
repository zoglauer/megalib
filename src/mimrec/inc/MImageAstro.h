/*
 * MImageAstro.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImageAstro__
#define __MImageAstro__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TMatrix.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"
#include "MPointSourceSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImageAstro : public MImage2D
{
  // public interface:
 public:
  MImageAstro();
  MImageAstro(TString Title, double *IA, int NEntries, TString xTitle,
              double xMin, double xMax, int xNBins, TString yTitle, double yMin, 
              double yMax, int yNBins, int Spectrum = c_WhiteRedBlack, TString DrawOption = "COLCONT4Z");
  ~MImageAstro();

	virtual void Display(TCanvas *Canvas = 0);
  virtual void SetRotation(TMatrix Matrix);

  // protected methods:
 protected:
  virtual void AddPointSources();


  // private methods:
 private:



  // protected members:
 protected:
  MPointSourceSelector m_PSS;

  // private members:
 private:
  TMatrix m_Matrix;


#ifdef ___CINT___
 public:
  ClassDef(MImageAstro, 0) // class containing an astropysical image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
