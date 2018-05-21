/*
 * MImage2DUpdate.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MImage2DUpdate
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImage2DUpdate.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MImage2DUpdate)
#endif


////////////////////////////////////////////////////////////////////////////////


MImage2DUpdate::MImage2DUpdate(MString Title, double* IA, MString xTitle,
                               double xMin, double xMax, int xNBins, MString yTitle, double yMin, 
                               double yMax, int yNBins, int Spectrum, int DrawOption) :
  MImage2D(Title, IA, NEntries, xTitle,
           xMin, xMax, xNBins, yTitle, yMin, 
           yMax, yNBins, Spectrum, DrawOption)
{
  // Construct an instance of MImage2DUpdate

  m_NAdds = 0;
  m_DisplayUpdateFrequency = 10;

  m_Canvas = 0;
  m_Histogram = 0;
}


////////////////////////////////////////////////////////////////////////////////


MImage2DUpdate::~MImage2DUpdate()
{
  // Delete this instance of MImage2DUpdate
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImage2DUpdate::Clone()
{
  //! Clone this image

  MImage* I = 
    new MImage2DUpdate(m_Title, m_IA, 
                       m_xTitle, m_xMin, m_xMax, m_xNBins, 
                       m_yTitle, m_yMin, m_yMax, m_yNBins, 
                       m_Spectrum, m_DrawOption);

  return I;
}

////////////////////////////////////////////////////////////////////////////////


void MImage2DUpdate::Add(double *Image)
{
  // 

  if (m_Histogram == 0) {
    Error("void MImage2DUpdate::Add(double *Image)",
          "The Histograms is not created!");
    return;
  }

  int Bin;

  m_NAdds++;

  // Update all:
  for (Bin = 0; Bin < m_xNBins*m_yNBins; Bin++) {
    m_IA[Bin] += Image[Bin];
  }
  delete [] Image;
  

  int x, y;
  for (x = 0; x < m_xNBins; x++) {
    for (y = 0; y < m_yNBins; y++) {
      m_Histogram->SetBinContent(x+1, y+1, m_IA[x + y * m_xNBins]);
    }
  }

  if (m_NAdds % m_DisplayUpdateFrequency == 0) {
    m_Canvas->Modified();
    m_Canvas->Update();
    gSystem->ProcessEvents();
  } 
}


////////////////////////////////////////////////////////////////////////////////


void MImage2DUpdate::Display(TCanvas *Canvas)
{
  // Display the image in a canvas

  // Individualize the canvas:
  if (m_Canvas != 0) delete m_Canvas;
  if (Canvas == 0) {
    m_Canvas = new TCanvas();
  } else {
    m_Canvas = Canvas;
  }

   // Name of the canvas
  m_Canvas->SetName(MakeCanvasTitle());
  
  m_Canvas->SetTitle(m_Title);
  m_Canvas->SetWindowPosition(40, 40);
  m_Canvas->SetWindowSize(640, 480);
  m_Canvas->SetFillColor(0);
  m_Canvas->SetFrameBorderSize(0);
  m_Canvas->SetFrameBorderMode(0);
  m_Canvas->SetBorderSize(0);
  m_Canvas->SetBorderMode(0);

  m_Histogram = new TH2D(m_Title, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);
  m_Histogram->SetDirectory(0);
  m_Histogram->SetXTitle(m_xTitle);
  m_Histogram->SetYTitle(m_yTitle);
  m_Histogram->SetFillColor(0);
  m_Histogram->SetLabelOffset(float(-0.003), "Y");
  m_Histogram->SetTitleOffset(float(1.5), "X");
  m_Histogram->SetTitleOffset(float(1.3), "Y");
  m_Histogram->SetTitleOffset(float(1.3), "Z");
  m_Histogram->SetLabelSize(float(0.03), "X");
  m_Histogram->SetLabelSize(float(0.03), "Y");
  m_Histogram->SetLabelSize(float(0.03), "Z");
  m_Histogram->SetStats(false);

  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      m_Histogram->SetBinContent(x, y, m_IA[(x-1) + (y-1) * m_xNBins]);
    }
  }

  m_Histogram->Draw(m_DrawOptionString);
  
  m_Canvas->Modified();
  m_Canvas->Update();

  gSystem->ProcessEvents();

  SetCreated();

  return;
}

////////////////////////////////////////////////////////////////////////////////


void MImage2DUpdate::DrawCopy()
{
  // 

  if (m_Histogram != 0) {
    m_Histogram->SetDirectory(0);
    m_Histogram->SetXTitle(m_xTitle);
    m_Histogram->SetYTitle(m_yTitle);
    m_Histogram->SetFillColor(0);
    m_Histogram->SetLabelOffset(-0.003, "Y");
    m_Histogram->SetTitleOffset(1.5, "X");
    m_Histogram->SetTitleOffset(1.3, "Y");
    m_Histogram->SetTitleOffset(1.3, "Z");
    m_Histogram->SetLabelSize(0.03, "X");
    m_Histogram->SetLabelSize(0.03, "Y");
    m_Histogram->SetLabelSize(0.03, "Z");
    m_Histogram->SetStats(false);
  
    m_Histogram->DrawCopy(m_DrawOptionString);
  } 
}


////////////////////////////////////////////////////////////////////////////////


TH1* MImage2DUpdate::GetHistogram()
{
  //

  return (TH1*) m_Histogram;
}


////////////////////////////////////////////////////////////////////////////////


void MImage2DUpdate::Reset()
{
  cout<<"void MImage2DUpdate::Reset()"<<endl;

  int x, y;
  for (x = 0; x < m_xNBins; x++) {
    for (y = 0; y < m_yNBins; y++) {

      m_IA[x + y * m_xNBins] = 0.0;
      m_Histogram->SetBinContent(x, y, m_IA[x + y * m_xNBins]);
    }
  }
  m_NAdds = 0;


  m_Canvas->Modified();
  m_Canvas->Update();
  gSystem->ProcessEvents();
}


// MImage2DUpdate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
