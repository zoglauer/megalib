/*
 * MImage.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImage__
#define __MImage__


////////////////////////////////////////////////////////////////////////////////

// Standard libs
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TSystem.h>
#include <TStyle.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


enum class MImageProjection : int { 
  c_None = 0, c_Hammer = 1 
};

////////////////////////////////////////////////////////////////////////////////


class MImage
{
  // Public Interface:
 public:
  //! Default constructor
  MImage();
  //! Standard constructor
  MImage(MString Title, double* IA, 
         MString xTitle, double xMin, double xMax, int xNBins,
         MString vTitle = "", int Spectrum = c_Rainbow, int DrawOption = c_COLCONT4Z);
  //! Standard destructor
  virtual ~MImage();

  //! Clone this image
  virtual MImage* Clone();

  //! Initializations common to all constructors
  void Init();

  //! Set the titel of the image
  virtual void SetTitle(MString Title);
  //! Return the title
  const char* GetTitle() const;
  //! Set the "ROOT"-draw option, e.g. HIST or LEGO2, CONT0Z, etc. 
  virtual void SetDrawOption(const int DrawOption);
  //! Set the image array and redisplay it
  virtual void SetImageArray(double* IA);
  //! Set the x-Axis attributes
  virtual void SetXAxis(MString xTitle, double xMin, double xMax, int xNBins);
  //! Set the x-Axis title
  virtual void SetXAxisTitle(MString xTitle) { m_xTitle = xTitle; }
  //! Set the y-Axis title
  virtual void SetValueAxisTitle(MString vTitle) { m_vTitle = vTitle; }
  //! Set the spectrum of this image
  virtual void SetSpectrum(int Spectrum);

  //! If set to true the image is normalize to one
  virtual void Normalize(bool Normalize) { m_Normalize = Normalize; }
  
  //! Display the histogram in the given canvas
  virtual void Display(TCanvas* Canvas = nullptr);
  //! Reset to default values:
  virtual void Reset();
  //! Save the canvas if it exists
  virtual void SaveAs(MString FileName);
  
  //! Set this flag after all histograms have been initialized
  bool IsCreated();
  //! Return true if the main window still exists
  bool CanvasExists();

  virtual void DrawCopy();

  //! Get the dimensions of the histogram
  virtual unsigned int GetDimensions() const { return 1; }
  
  //! Calculate the average
  double GetAverage();
  
  //! Determine the maximum, the vector is filled up to the number of dimensions the histogram has
  virtual void DetermineMaximum(double& MaxValue, vector<double>& Coordinate);
  
 public:
  static const int c_RootDefault;
  static const int c_BlackWhite;
  static const int c_WhiteBlack;
  static const int c_DeepSea;
  static const int c_SkyLine;
/*   static const int c_SunRise; */
  static const int c_Incendescent;
  static const int c_WhiteRedBlack;
  static const int c_Thesis;
  static const int c_ThesisVarAtLow;
  static const int c_Rainbow;
  static const int c_Bird;
  static const int c_Viridis;
  static const int c_Cividis;
  
  static const int c_COL;
  static const int c_COLZ;
  static const int c_COLCONT4;
  static const int c_COLCONT4Z;

  // protected methods:
 protected:
  void SetCreated();

  //! Create a Canvas title of form "<m_Title> - <m_ID>" 
  virtual MString MakeCanvasTitle();

  // private methods:
 private:



  // protected members:
 protected:
  //! The image array
  double* m_IA;
  //! Size of the image array
  int m_NEntries;

  //! Title of the histogram
  MString m_Title;
  //! Title of the canvas
  MString m_CanvasTitle;

  //! Title of the x-axis
  MString m_xTitle;
  //! Minimum x value
  double m_xMin;
  //! Maximum x value;
  double m_xMax;
  //! Number of x bins
  int m_xNBins;

  //! Title of the value axis
  MString m_vTitle;

  //! ID of the used spectrum
  int m_Spectrum;
  //! ID of the used draw options
  int m_DrawOption;
  //! Draw option of the main histogram, e.g. CONT4COLZ
  MString m_DrawOptionString;

  //! True if all histrograms are already initialized
  bool m_IsCreated;        

  //! The main canvas
  TCanvas* m_Canvas;
  //! The  main histogram
  TH1* m_Histogram;

  //! True if the histogram should be normalized
  bool m_Normalize;
  
  //! counts the distributed IDs
  static int m_IDCounter;  
  //! true if the color palettes have already been allocated
  static bool m_PalettesCreated; 
  //! ID of the globally used spectrum
  static int m_GlobalSpectrum;
  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MImage, 0)        // basic image class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
