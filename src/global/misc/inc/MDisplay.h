/*
 * MDisplay.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDISPLAY__
#define __MDISPLAY__

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
using namespace std;

#include <TH3.h>
#include <TH2.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TF1.h>
#include <TF2.h>
#include <TStyle.h>
#include <TColor.h>

#include <TArrayF.h>
#include <TObjArray.h>
#include <TRandom.h>
#include <TMath.h>
#include <TPaveText.h>
#include <TText.h>
#include <TFrame.h>
#include <TPaveLabel.h>
#include <TPad.h>
#include <TFile.h>
#include <TSlider.h>


//////////////////////////////////////////////////////////////////////////////////////


class MDisplay
{
  // Public Interface:
 public:
  MDisplay() {m_DrawMode = "CONT0Z";};
  MDisplay(double xMin, double xMax, int xBinx, double yMin, 
	   double yMax, int yBins, TString DrawMode = "CONT0Z");
  ~MDisplay();

  void SetGlobalAttributes(double xMin, double xMax, int xBinx, double yMin, 
			   double yMax, int yBins,  double zMin, 
			   double zMax, int zBins, TString DrawMode = "CONT0Z");
  //  void SetGlobalAttributes(MMatrixInfo Info);

  void SetViewport(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);

  void DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, double* Values, double Max = -1);
  void DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, double* Values, int Unkown);
  void DisplayHistogram(TString Title, TString xTitle, TString yTitle, double* Values, double xMin, double xMax, int NBins);
  void DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, int NBins,
			double* xValues, double xMin, double xMax, int xNBins,
			double* yValues, double yMin, double yMax, int yNBins,
			double* zValues, double zMin, double zMax, int zNBins);
  void DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle,
			double* xValues, double xMin, double xMax, int xNBins,
			double yMin, double yMax, int yNBins);

  void DisplayGausFit1D(TString Title, TString xTitle, TString yTitle, TArrayF* Values, int mode);
  void DisplayGausFit2D(TString Title, TString xTitle, TString yTitle, TString zTitle, TArrayF* Values);

  // private methods:
 private:
  void GausFit1D(double* Mean, double* Sigma, TArrayF* Values, int mode);


  // private members:
 private:
  // Global attributes:
  double m_xMin;
  double m_xMax;
  int m_xBins;
  double m_xInt;

  double m_yMin;
  double m_yMax;
  int m_yBins;
  double m_yInt;

  double m_zMin;
  double m_zMax;
  int m_zBins;
  double m_zInt;

  TString m_DrawMode;

  //  MMatrixInfo m_Info;
};


#endif
