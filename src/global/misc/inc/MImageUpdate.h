/*
 * MImageUpdate.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImageUpdate__
#define __MImageUpdate__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TText.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage.h"

// Forward declarations:
class TCanvas;
class TH1D;
class TText;


////////////////////////////////////////////////////////////////////////////////


class MImageUpdate : public MImage
{
  // public interface:
 public:
  enum Modus { c_Accumulate, c_Average, c_Normalize, c_History };
  enum AdditionalText { c_None, c_Mean, c_RMS, c_Last };

  MImageUpdate();
  MImageUpdate(MString Title, double *IA, int NEntries, 
               MString xTitle, double xMin, double xMax, int xNBins, 
               unsigned int Mode = c_History, unsigned int ShortStorage = 0, unsigned int LongStorage = 0, 
               int AdditionalText = c_None, int Spectrum = c_Incendescent, int DrawOption = c_COLCONT4Z);
  virtual ~MImageUpdate();

  void Add(double x, double Value = 1);
  void Add(double *Image);
  void AddAdditionalText(int Type);

  void SetDisplayMode(unsigned int Mode);
  void SetMaximumStorage(unsigned int ShortStorage, unsigned int LongStorage);

  void Display(TCanvas *Canvas = 0);

  void Reset();

  double Mean();
  double StandardDeviation();

  void DrawCopy();

  // protected methods:
 protected:
  void DisplayText(double x, double y, MString Text);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  unsigned int m_NAdds;                    // Number of calls to Add(...)

  unsigned int m_DisplayUpdateFrequency;   // After m_DisplayUpdateFrequency-Adds the Display/Canvas is updated
  unsigned int m_StorageUpdateFrequency;   // After m_StorageUpdateFrequency Adds a new summed spectra is started

  double *m_MAC;                   // "Many Adds Chart"
  unsigned int m_NAddsMAC;                 // Number of Adds contained in MAC
  unsigned int m_NMACStorages;

  double *m_FAC;                   // "Few Adds Chart"
  unsigned int m_NAddsFAC;                 // Number of Adds contained in FAC
  unsigned int m_NFACStorages;

  double **m_Storage;              // Storage of m_NMACStorages of summed spectra of m_StorageUpdateFrequency adds

  unsigned int m_DisplayMode;              // 0: Summed, 1: Normalized, 2: Averaged

  TH1D *m_AAH;                       // "All Adds Histogram"
  TH1D *m_MAH;                       // "Many Adds Histogram"
  TH1D *m_FAH;                       // "Few Adds Histogram"

  TCanvas *m_Canvas;                 // the canvas, where the data is displayed - may be embedded in a TGFrame

  unsigned int m_AdditionalTextType;
  TText *m_AdditionalText;


#ifdef ___CLING___
 public:
  ClassDef(MImageUpdate, 0) // an updateable image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
