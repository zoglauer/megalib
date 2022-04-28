/*
 * MImage2DUpdate.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImage2DUpdate__
#define __MImage2DUpdate__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImage2DUpdate : public MImage2D
{
  // public interface:
 public:
  MImage2DUpdate(MString Title, double *IA, int NEntries, 
                 MString xTitle, double xMin, double xMax, int xNBins, 
                 MString yTitle, double yMin, double yMax, int yNBins, 
                 int Spectrum = c_Viridis, int DrawOption = c_COLCONT4Z);
  virtual ~MImage2DUpdate();

  //! Clone this image
  virtual MImage* Clone();

  void Display(TCanvas *Canvas = 0);
  void Add(double *Image);
  void Reset();

  TH1* GetHistogram();
  void DrawCopy();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  unsigned int m_NAdds;                    // Number of calls to Add(...)

  unsigned int m_DisplayUpdateFrequency;   // After m_DisplayUpdateFrequency-Adds the Display/Canvas is updated
  unsigned int m_StorageUpdateFrequency;   // After m_StorageUpdateFrequency Adds a new summed spectra is started

  TCanvas *m_Canvas;
  TH2D *m_Histogram;


#ifdef ___CLING___
 public:
  ClassDef(MImage2DUpdate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
