/*
 * MImageSpheric.cxx
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
// MImageSpheric
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImageSpheric.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include "TMath.h"
#include "TText.h"
#include "TMarker.h"
#include "TGaxis.h"
#include "TLatex.h"
#include "TPolyLine.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MVector.h"
#include "MPointSourceSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MImageSpheric)
#endif


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::MImageSpheric() : MImage2D()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::MImageSpheric(MString Title, double* IA,
                             MString xTitle, double xMin, double xMax, int xNBins,
                             MString yTitle, double yMin, double yMax, int yNBins,
                             MString vTitle, int Spectrum, int DrawOption) :
  MImage2D(Title, IA, xTitle, xMin, xMax, xNBins, yTitle, yMin,
           yMax, yNBins, vTitle, Spectrum, DrawOption), m_YAxis(nullptr)
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
  // vTitle:     title of the value axis
  // Spectrum:   spectrum
  // DrawOption: ROOT draw option
  
  m_Projection = MImageProjection::c_None;
}


////////////////////////////////////////////////////////////////////////////////


MImageSpheric::~MImageSpheric()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImageSpheric::Clone()
{
  //! Clone this image

  MImage* I =
    new MImageSpheric(m_Title, m_IA,
                      m_xTitle, m_xMin, m_xMax, m_xNBins,
                      m_yTitle, m_yMin, m_yMax, m_yNBins,
                      m_vTitle, m_Spectrum, m_DrawOption);

  I->Normalize(m_Normalize);

  return I;
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::SetImageArray(double* IA)
{
  // Copy the data array

  if (IA == nullptr) {
    merr<<"Input array is nullptr!"<<endl;
    return;
  }
  
  if (m_IA != nullptr) delete [] m_IA;
  m_IA = new double[m_NEntries];
  
  for (int x = 0; x < m_NEntries; ++x) {
    m_IA[x] = IA[x];
  }
  
  if (dynamic_cast<TH2*>(m_Histogram) != nullptr) {
    if (m_Projection == MImageProjection::c_None) {
      DisplayProjectionNone();
    } else {
      DisplayProjectionHammer();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::HammerConv(double Long, double Lat, double CentralMeridian, double& xHammer, double& yHammer)
{
  Long -= CentralMeridian;

  xHammer = 2.0*sqrt(2.0)*cos(Lat)*sin(0.5*Long) / sqrt(1.0 + cos(Lat)*cos(0.5*Long));
  yHammer = sqrt(2.0)*sin(Lat) / sqrt(1.0 + cos(Lat)*cos(0.5*Long));

  xHammer += CentralMeridian;
}


////////////////////////////////////////////////////////////////////////////////


bool MImageSpheric::HammerInvConv(double xHammer, double yHammer, double CentralMeridian, double& Long, double& Lat)
{
  xHammer -= CentralMeridian;

  double z = 1 - xHammer*xHammer/16.0 - yHammer*yHammer/4.0;
  if (z < 0) return false;
  z = sqrt(z);

  //Long = 2*atan(z*xHammer/(2*(2*z*z-1)));
  Long = 2*atan2(z*xHammer, 2*(2*z*z-1));
  //Long = 2*atan2(2*(2*z*z-1), z*xHammer);
  Lat = asin(z*yHammer);

  if (Long < -TMath::Pi() || Long > TMath::Pi() || Lat > TMath::Pi()/2 || Lat  < -TMath::Pi()/2) return false;
  Long += CentralMeridian;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::Display(TCanvas* Canvas)
{
  // Display the image in a canvas

  if (Canvas == 0) {
    m_CanvasTitle = MakeCanvasTitle();
    Canvas = new TCanvas(m_CanvasTitle, m_Title, 40, 40, 900, 900);
  } else {
    if (MString(Canvas->GetTitle()).IsEmpty() == false) {
      m_CanvasTitle = Canvas->GetTitle();
    } else {
      m_CanvasTitle = MakeCanvasTitle();
      Canvas->SetTitle(m_CanvasTitle);
    }
  }
  m_Canvas = Canvas;

  int xSize = 1400;
  int ySize = 700;
  
  double xImageSize = (m_xMax-m_xMin);
  double yImageSize = (m_yMax-m_yMin);
  
  if (xImageSize > yImageSize) {
    ySize = int(xSize/xImageSize*yImageSize);
    if (ySize < 0.15*xSize) ySize = 0.15*xSize;
  } else {
    xSize = int(ySize/yImageSize*xImageSize);
    if (xSize < 0.15*ySize) xSize = 0.15*ySize;
  }
  m_Canvas->SetWindowSize(xSize, ySize);
  
  if (m_Projection == MImageProjection::c_None) {
    DisplayProjectionNone();
  } else {
    DisplayProjectionHammer();
  }
}



////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::DisplayProjectionNone()
{
  // Display the image in a canvas

  bool IsNew = false;
  TH2D* Hist = nullptr;
  if (m_Histogram == nullptr) {
    Hist = new TH2D(m_CanvasTitle + "ImageSpheric" + m_IDCounter, m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    Hist->SetStats(false);
    Hist->SetContour(50);

    Hist->GetXaxis()->SetTitle(m_xTitle);
    Hist->GetXaxis()->CenterTitle();
    Hist->GetXaxis()->SetTitleOffset(1.6f);
    Hist->GetXaxis()->SetTitleSize(0.04f);
    Hist->GetXaxis()->CenterTitle();
    Hist->GetXaxis()->SetTickLength(-0.03f);
    Hist->GetXaxis()->SetLabelOffset(0.03f);
    Hist->GetXaxis()->SetLabelSize(0.035f);
    if ((int(m_xMax) - int(m_xMin)) % 360 == 0) {
      Hist->GetXaxis()->SetNdivisions(612);
      //Hist->GetXaxis()->SetOption("N+");
    } else if ((int(m_xMax) - int(m_xMin)) % 60 == 0) {
      Hist->GetXaxis()->SetNdivisions(606);
      //Hist->GetXaxis()->SetOption("N+");
    }

    Hist->GetZaxis()->SetTitle(m_vTitle);
    Hist->GetZaxis()->SetTitleOffset(1.2f);
    Hist->GetZaxis()->SetTitleSize(0.04f);
    Hist->GetZaxis()->SetLabelSize(0.035f);


    IsNew = true;
  } else {
    Hist = dynamic_cast<TH2D*>(m_Histogram);
    Hist->SetTitle(m_Title);
  }

  double Content = 0.0;
  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      Content = m_IA[(x-1) + (y-1) * m_xNBins];
      if (!TMath::IsNaN(Content)) {
        if (TMath::Finite(Content)) {
          // Make sure to invert y-axis
          Hist->SetBinContent(x, m_yNBins-y+1, m_IA[(x-1) + (y-1)*m_xNBins]);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
      }
    }
  }

  // Rescale to 1:
  if (m_Normalize == true && Hist->GetMaximum() > 0) {
    Hist->Scale(1.0/Hist->GetMaximum());
  }

  if (IsNew == true) {
    Hist->Draw(m_DrawOptionString);

    // Redraw the new axis
    gPad->Update();
    if (m_DrawOption == c_COLZ || m_DrawOption == c_COL) {
      m_YAxis = new TGaxis(gPad->GetUxmin(),
                           gPad->GetUymax(),
                           gPad->GetUxmin()-0.001,
                           gPad->GetUymin(),
                           Hist->GetYaxis()->GetXmin(),
                           Hist->GetYaxis()->GetXmax(),
                           510,"-S");
      m_YAxis->ImportAxisAttributes(Hist->GetYaxis());
      m_YAxis->CenterTitle(true);
      m_YAxis->SetTitle(m_yTitle);
      m_YAxis->SetLabelOffset(-0.03f);
      m_YAxis->SetTickLength(0.015f);
      m_YAxis->SetTitleOffset(-1.5f);
    } else {
      m_YAxis = new TGaxis(gPad->GetUxmin(),
                           gPad->GetUymax(),
                           0.9999*gPad->GetUxmin(),
                           gPad->GetUymin(),
                           Hist->GetYaxis()->GetXmin(),
                           Hist->GetYaxis()->GetXmax(),
                           510,"-S");
      m_YAxis->ImportAxisAttributes(Hist->GetYaxis());
      m_YAxis->CenterTitle(true);
      m_YAxis->SetTitle(m_yTitle);
      m_YAxis->SetLabelOffset(-0.025f);
      m_YAxis->SetLabelSize(0.035f);
      m_YAxis->SetTitleOffset(-1.9f);
      m_YAxis->SetTitleSize(0.04f);
      m_YAxis->SetTickLength(0.015f);
      m_YAxis->SetBit(TAxis::kRotateTitle);
    }


    // Remove the current axis
    Hist->GetYaxis()->SetLabelOffset(999);
    Hist->GetYaxis()->SetTickLength(0);

    // Draw the new one
    m_YAxis->Draw();
  }

  m_Canvas->Update();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageSpheric::DisplayProjectionHammer()
{
  // Display the image in a canvas

  //cout<<"Hammer projection on"<<endl;

  double xMin = m_xMin;
  double xMax = m_yMax;
  
  // Invert coordinates, and make then go fromm -90 to 90:
  double yMin = -(m_yMax-90);
  double yMax = -(m_yMin-90);

  
  Color_t DefaultTextColor = kWhite;
  if (m_Spectrum == c_Bird || m_Spectrum == c_Cividis) {
    DefaultTextColor = kBlack; 
  }
  
  // First we always create a new unprojected histogram and fill it
  TH2D* Unprojected = new TH2D(m_CanvasTitle + "Unprojected", m_Title, m_xNBins, xMin, xMax, m_yNBins, yMin, yMax);

  // ... and fill it
  double Content = 0.0;
  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      Content = m_IA[(x-1) + (y-1) * m_xNBins];
      if (!TMath::IsNaN(Content)) {
        if (TMath::Finite(Content)) {
          // Make sure to invert y-axis
          Unprojected->SetBinContent(x, m_yNBins-y+1, m_IA[(x-1) + (y-1)*m_xNBins]);
          //Unprojected->SetBinContent(x, y, m_IA[(x-1) + (y-1)*m_xNBins]);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<show;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<show;
      }
    }
  }

  // Rescale to 1:
  if (m_Normalize == true && Unprojected->GetMaximum() > 0) {
    Unprojected->Scale(1.0/Unprojected->GetMaximum());
  }


  // Then we get or create the projected histogram
  bool IsNew = false;
  TH2D* Hist = nullptr;
  if (m_Histogram == nullptr) {
    Hist = new TH2D(m_CanvasTitle + "Hist", m_Title, 8*m_xNBins, xMin, xMax, 8*m_yNBins, yMin, yMax);
    Hist->SetContour(50);
    Hist->GetXaxis()->SetTitle(m_xTitle);
    Hist->GetYaxis()->SetTitle(m_yTitle);
    //Hist->GetZaxis()->SetTitle(m_vTitle);
    Hist->GetZaxis()->SetTitleOffset(1.2f);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    IsNew = true;
  } else {
    Hist = dynamic_cast<TH2D*>(m_Histogram);
    Hist->SetTitle(m_Title);
  }

  double CentralMeridian = 0.5*(xMax+xMin)*c_Rad;

  // Copy the data:
  double Lat, Long;
  double x, y;
  int ux, uy;
  for (int bx = 0; bx < Hist->GetNbinsX(); ++bx) {
    for (int by = 0; by < Hist->GetNbinsY(); ++by) {
      x = Hist->GetXaxis()->GetBinCenter(bx)*c_Rad;
      y = Hist->GetYaxis()->GetBinCenter(by)*c_Rad;

      if (HammerInvConv(x, y, CentralMeridian, Long, Lat) == false) {
		continue;
      }
      Long *= c_Deg;
      Lat *= c_Deg;
      
      //if (Long < xMin || Long > xMax || Lat < yMin || Lat > yMax) continue;
      
      ux = Unprojected->GetXaxis()->FindBin(Long);
      uy = Unprojected->GetYaxis()->FindBin(Lat);
      Hist->SetBinContent(bx, by, Unprojected->GetBinContent(ux, uy));
    }
  }
  
  /*
  TCanvas* CC = new TCanvas();
  CC->cd();
  Unprojected->DrawCopy("colz");
  CC->Update();
  */

  delete Unprojected;


  // Draw the axes
  if (IsNew == true) {
        
    // Draw data:
    m_Canvas->cd();
    Hist->Draw(m_DrawOptionString + " a");

    // Paint coordinates:
    vector<double> Seperators = { 90.0, 60.0, 45.0, 30.0, 15.0, 10.0, 5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01 };


    // (A) Determine the steps for longitude
    double xSep = 180.0;
    for (double S: Seperators) {
      if ((xMax - xMin) / S > 5) {
        xSep = S;
        break;
      }
    }
    vector<double> xSteps;
    xSteps.push_back(xSep*int(xMin/xSep));
    if (xSteps[0] < xMin) xSteps[0] += xSep;
    while (xSteps.back() + xSep <= xMax) {
      xSteps.push_back(xSteps.back() + xSep);
    }

    // (B) Determine the steps for latitude
    double ySep = 180.0;
    for (double S: Seperators) {
      if ((yMax - yMin) / S > 5) {
        ySep = S;
        break;
      }
    }
    vector<double> ySteps;
    ySteps.push_back(ySep*int(yMin/ySep));
    if (ySteps[0] < yMin) ySteps[0] += ySep;
    while (ySteps.back() + ySep <= yMax) {
      ySteps.push_back(ySteps.back() + ySep);
    }


    // (C) Draw longitude grid lines
    for (double Long: xSteps) {
      // Go in one degree steps
      vector<double> xPoly;
      vector<double> yPoly;

      double L = Long;

      for (double Lat = yMin; Lat <= yMax; Lat += 0.01*(yMax-yMin)) {
        HammerConv(L*c_Rad, Lat*c_Rad, CentralMeridian, x, y);
        xPoly.push_back(x*c_Deg);
        yPoly.push_back(y*c_Deg);
      }
      
      TPolyLine* P = new TPolyLine(xPoly.size(), &xPoly[0], &yPoly[0], "C");
      P->SetLineStyle(3);
      P->SetLineColor(DefaultTextColor);
      P->Draw();
    }


    // (D) Draw latitude grid lines
    for (double Lat: ySteps) {
      // Go in one degree steps
      vector<double> xPoly;
      vector<double> yPoly;

      for (double Long = xMin; Long <= xMax; Long += 0.01*(xMax-xMin)) {
        HammerConv(Long*c_Rad, Lat*c_Rad, CentralMeridian, x, y);
        xPoly.push_back(x*c_Deg);
        yPoly.push_back(y*c_Deg);
      }

      TPolyLine* P = new TPolyLine(xPoly.size(), &xPoly[0], &yPoly[0], "C");
      P->SetLineStyle(3);
      P->SetLineColor(DefaultTextColor);
      P->Draw();
    }


    // (E) Draw longitude grid axis labels
    double FontSize = 0.03;
    double LatGap = 0; //0.07*ySep;
    for (unsigned int l = 0; l < ySteps.size(); ++l) {

      if (l == 0 && ySteps[l] - yMin < 0.3*ySep) continue;
      if (l == ySteps.size() - 1 && yMax - ySteps[l] < 0.3*ySep) continue;

      HammerConv(xMin*c_Rad, ySteps[l]*c_Rad, CentralMeridian, x, y);
      x *= c_Deg;
      y *= c_Deg;

      ostringstream out;
      out<<" "<<90-ySteps[l]<<"#circ";

      TLatex* T = new TLatex(x + LatGap, y, out.str().c_str());
      T->SetTextFont(42);
      T->SetTextColor(DefaultTextColor);
      if (y > 0) {
        T->SetTextAlign(13);
      } else if (y < 0) {
        T->SetTextAlign(11);
      } else {
        T->SetTextAlign(12);
      }
      T->SetTextSize(FontSize);
      T->Draw();
    }


    // (F) Draw longitude grid axis labels
    double LatPosition = 0.0;
    // Determine ideal longitude first:
    if (fabs(ySteps.front()) < fabs(ySteps.back())) {
      for (unsigned int i = ySteps.size() - 1; i < ySteps.size(); --i) {
        if (LatPosition == 0.0 || LatPosition < -45) {
          LatPosition = ySteps[i];
        }
        if (LatPosition <= 60) break;
        LatPosition -= ySep/2.0;
      }
      LatPosition += ySep/2.0;
    } else {
      for (double Lat: ySteps) {
        if (LatPosition == 0.0 || LatPosition < -45) {
          LatPosition = Lat;
        }
        if (LatPosition >= -60) break;
      }
      LatPosition += ySep/2.0;
    }

    // (G) Draw latitude grid axis labels
    for (unsigned int l = 0; l < xSteps.size(); ++l) {

      if (l == 0 && xSteps[l] - xMin < 0.3*xSep) continue;
      if (l == xSteps.size() - 1 && xMax - xSteps[l] < 0.3*xSep) continue;

      double Label = xSteps[l];
      HammerConv(Label*c_Rad, LatPosition*c_Rad, CentralMeridian, x, y);
      x *= c_Deg;
      y *= c_Deg;

      ostringstream out;
      out<<xSteps[l]<<"#circ";

      TLatex* T = new TLatex(x, y, out.str().c_str());
      T->SetTextFont(42);
      T->SetTextColor(DefaultTextColor);
      T->SetTextAlign(22);
      T->SetTextSize(FontSize);
      T->Draw();
    }

  
    TLatex* GalLong = new TLatex(0.5*(xMin+xMax), yMin - 0.05*(yMax-yMin), "Phi [deg]");
    GalLong->SetTextFont(42);
    GalLong->SetTextColor(kBlack);
    GalLong->SetTextAlign(22);
    GalLong->SetTextSize(0.04);
    GalLong->Draw();
  
    TLatex* GalLat = new TLatex(xMin - 0.025*(xMax-xMin), 0.5*(yMin+yMax), "Theta [deg]");
    GalLat->SetTextFont(42);
    GalLat->SetTextColor(kBlack);
    GalLat->SetTextAlign(22);
    GalLat->SetTextSize(0.04);
    GalLat->SetTextAngle(90);
    GalLat->Draw();
  
    TLatex* Value = new TLatex(xMax + 0.175*(xMax-xMin), 0.5*(yMin+yMax), m_vTitle);
    Value->SetTextFont(42);
    Value->SetTextColor(kBlack);
    Value->SetTextAlign(22);
    Value->SetTextSize(0.04);
    Value->SetTextAngle(90);
    Value->Draw();
  }

  m_Canvas->Update();

  // Make sure everything is updated - just in case we will be multi-threading later
  gSystem->ProcessEvents();


  return;
}


// MImageSpheric.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
