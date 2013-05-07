/*
 * MImageAstro.cxx
 *
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
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
// MImageAstro
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImageAstro.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TMath.h>
#include <TText.h>
#include <TMarker.h>
#include <TGaxis.h>

// MEGAlib libs:
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImageAstro)
#endif


////////////////////////////////////////////////////////////////////////////////


MImageAstro::MImageAstro() : MImage2D()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MImageAstro::MImageAstro(TString Title, double *IA, int NEntries, TString xTitle,
                         double xMin, double xMax, int xNBins, TString yTitle, double yMin, 
                         double yMax, int yNBins, int Spectrum, TString DrawOption) :
  MImage2D(Title, IA, NEntries, xTitle, xMin, xMax, xNBins, yTitle, yMin, 
         yMax, yNBins, Spectrum, DrawOption)
{
  // Construct an image but do not display it, i.e. save only the data
  // 
  // Title:      Title of the image, 0 means no title
  // IA:         data-array
  // NEntries:   length of the data array
  // xTitle:     title of the x axis
  // xMin:       minimum x-value
  // xMax:       maximum x-value
  // xNBins:     number of bins in x
  // yTitle:     title of the y-axis
  // yMin:       minimum y-value
  // yMax:       maximum y-value
  // yNBins:     number of bins in y
  // Spectrum:   spectrum
  // DrawOption: ROOT draw option

  //m_PSS = new MPointSourceSelector();
  //m_PSS->LoadList("$(MEGALIB)/resource/mimrec/Egret.pq.xml");
}


////////////////////////////////////////////////////////////////////////////////


MImageAstro::~MImageAstro()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MImageAstro::SetRotation(TMatrix Matrix)
{
  //

  m_Matrix.ResizeTo(3,3);
  m_Matrix = Matrix;
}


////////////////////////////////////////////////////////////////////////////////


void MImageAstro::Display(TCanvas* Canvas)
{
  // Display the image in a canvas

  if (Canvas == 0) {
    m_CanvasTitle = MakeCanvasTitle();
    Canvas = new TCanvas(m_CanvasTitle, m_Title, 40, 40, 600, 600);
    m_Canvas = Canvas;
  }

  if (m_xNBins > m_yNBins) {
    m_Canvas->SetWindowSize(900, int(900.0/m_xNBins*m_yNBins));
  } else {
    m_Canvas->SetWindowSize(int(700.0/m_yNBins*m_xNBins), 700);
  }

  TH2D* Hist = new TH2D(m_CanvasTitle + "Hist", m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);

  Hist->SetDirectory(0);
  Hist->SetXTitle(m_xTitle);
  Hist->SetYTitle(m_yTitle);
//   Hist->SetFillColor(1);
//   Hist->SetLabelOffset(0.003, "Y");
//   //Hist->SetTitleOffset(1.5, "X");
//   //Hist->SetTitleOffset(1.3, "Y");
//   //Hist->SetTitleOffset(1.3, "Z");
//   Hist->SetLabelSize(0.03, "X");
//   Hist->SetLabelSize(0.03, "Y");
//   Hist->SetLabelSize(0.03, "Z");
  Hist->GetXaxis()->CenterTitle();
  Hist->GetYaxis()->CenterTitle();
  Hist->SetNdivisions(509, "X");
  Hist->SetNdivisions(509, "Y");
  //Hist->GetXaxis()->SetNdivisions(0);
  Hist->SetStats(false);

  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      if (TMath::IsNaN(m_IA[x + y*m_xNBins]) == false) {
        Hist->SetBinContent(x, y, m_IA[(x-1) + (y-1)*m_xNBins]);
      }
    }
  }

 

//   Canvas->SetFillColor(10);
//   Canvas->SetFrameBorderSize(0);
//   Canvas->SetFrameBorderMode(0);
//   Canvas->SetBorderSize(0);
//   Canvas->SetBorderMode(0);


  TPad *Pad = new TPad("A pad!","My pad!",0.0,0.0,1.0,1.0,0);
  Pad->SetBorderSize(0);
  Pad->SetBorderMode(0);
  Pad->Draw();
  Pad->cd();

  //Hist->SetLabelSize(0, "X");
  //Hist->SetTickLength(0, "X");

  //AddPointSources();

  Pad->cd();
  Hist->SetContour(50);
  Hist->Draw(m_DrawOption);

  /*
  TGaxis *axis8 = new TGaxis(0.578,-0.576,-0.578,-0.5761, m_xMin, m_xMax, 510, "S=");
  axis8->SetLabelOffset(0.04);
  axis8->SetLabelSize(0.03);
  axis8->SetName("axis8");
  axis8->Draw();
  */

  Pad->Update();
  Canvas->Update();

  m_Histogram = (TH1D*) Hist;

  gSystem->ProcessEvents();

  /*
  temp = m_xMax;
  m_xMax = -m_xMin;
  m_xMin = -temp;
  */

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageAstro::AddPointSources()
{
  // Draws pointsource names over the image

  // Test a pointsource:
  double L = 184.56, B = -5.78;

  TText *t;
  double xT, yT;
  unsigned int i;

  for (i = 0; i < m_PSS.GetNPointSources(); i++) {
    t = new TText();
    L = -m_PSS.GetPointSourceAt(i).GetLongitude();
    B = m_PSS.GetPointSourceAt(i).GetLatitude();
    if (L >= m_xMin && L <= m_xMax && B >= m_yMin && B <= m_yMax) {
      xT = ((L-m_xMin)/(m_xMax-m_xMin))*0.8+0.095;
      yT = ((B-m_yMin)/(m_yMax-m_yMin))*0.8+0.1;
      t->SetNDC(false);
      t->SetTextFont(32);
      t->SetTextColor(1);
      t->SetTextSize(0.01);
      t->SetTextAlign(12);
      t->DrawText(xT, yT, m_PSS.GetPointSourceAt(i).GetName());
    }
  }
}


// MImageAstro.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
