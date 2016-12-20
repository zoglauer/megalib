/*
 * MImageGalactic.cxx
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
// MImageGalactic
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImageGalactic.h"

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
#include "MAssert.h"
#include "MStreams.h"
#include "MVector.h"
#include "MPointSourceSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImageGalactic)
#endif


////////////////////////////////////////////////////////////////////////////////


MImageGalactic::MImageGalactic() : MImage2D()
{
}


////////////////////////////////////////////////////////////////////////////////


MImageGalactic::MImageGalactic(MString Title, double* IA,
                               MString xTitle, double xMin, double xMax, int xNBins, 
                               MString yTitle, double yMin, double yMax, int yNBins, 
                               MString vTitle, int Spectrum, int DrawOption, MString SourceCatalog) :
  MImage2D(Title, IA, xTitle, xMin, xMax, xNBins, yTitle, yMin, 
           yMax, yNBins, vTitle, Spectrum, DrawOption), m_XAxis(0), m_SourceCatalog(SourceCatalog)
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


MImageGalactic::~MImageGalactic()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImageGalactic::Clone()
{
  //! Clone this image

  MImageGalactic* I = 
    new MImageGalactic(m_Title, m_IA, 
                       m_xTitle, m_xMin, m_xMax, m_xNBins, 
                       m_yTitle, m_yMin, m_yMax, m_yNBins, 
                       m_vTitle, m_Spectrum, m_DrawOption, m_SourceCatalog);

  I->SetProjection(m_Projection);
  I->Normalize(m_Normalize);
    
  return I;
}

////////////////////////////////////////////////////////////////////////////////


void MImageGalactic::SetImageArray(double* IA)
{
  // Copy the data array

  if (m_IA != 0) delete [] m_IA;
  m_IA = new double[m_NEntries];

  for (int x = 0; x < m_NEntries; x++) {
    if (IA != 0) {
      m_IA[x] = IA[x];
    } else {
      m_IA[x] = 0.0;
    }
  }

  if (dynamic_cast<TH2*>(m_Histogram) != 0) {
    if (m_Projection == MImageProjection::c_None) {
      DisplayProjectionNone();
    } else {
      DisplayProjectionHammer();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImageGalactic::Display(TCanvas* Canvas)
{
  // Display the image in a canvas
  
  if (Canvas == 0) {
    m_CanvasTitle = MakeCanvasTitle();
    Canvas = new TCanvas(m_CanvasTitle, m_Title, 40, 40, 900, int(900.0/m_xNBins*m_yNBins));
  } else {
    if (MString(Canvas->GetTitle()).IsEmpty() == false) {
      m_CanvasTitle = Canvas->GetTitle();
    } else {
      m_CanvasTitle = MakeCanvasTitle();
      Canvas->SetTitle(m_CanvasTitle);
    }
  }
  m_Canvas = Canvas;

  if (m_xNBins > m_yNBins) {
    m_Canvas->SetWindowSize(900, int(900.0/m_xNBins*m_yNBins));
  } else {
    m_Canvas->SetWindowSize(int(700.0/m_yNBins*m_xNBins), 700);
  }
  
  if (m_Projection == MImageProjection::c_None) {
    DisplayProjectionNone();
  } else {
    DisplayProjectionHammer();
  }
}
  

////////////////////////////////////////////////////////////////////////////////


void MImageGalactic::DisplayProjectionNone()
{
  // Display the image in a canvas
   
  
  bool IsNew = false;
  TH2D* Hist = 0;
  if (m_Histogram == 0) {
    Hist = new TH2D(m_CanvasTitle + "Hist", m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax);
    Hist->SetContour(50);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    Hist->SetStats(false);
    Hist->GetYaxis()->SetTitle(m_yTitle);
    Hist->GetYaxis()->SetTitleOffset(1.5f);
    Hist->GetYaxis()->CenterTitle();
    Hist->GetYaxis()->SetTickLength(-0.02f);
    Hist->GetYaxis()->SetLabelOffset(0.02f);
    Hist->GetYaxis()->SetLabelSize(0.03f);
    Hist->GetZaxis()->SetTitle(m_vTitle);
    Hist->GetZaxis()->SetTitleOffset(1.2f);

    if ((int(m_yMax) - int(m_yMin)) % 60 == 0) {
      Hist->GetYaxis()->SetNdivisions(-606);
    }

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
          // Make sure to invert x-axis
          Hist->SetBinContent(m_xNBins-x+1, y, Content);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<endl;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<endl;
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
    m_XAxis = new TGaxis(gPad->GetUxmax(),
                         gPad->GetUymin(),
                         gPad->GetUxmin(),
                         gPad->GetUymin(),
                         Hist->GetXaxis()->GetXmin(),
                         Hist->GetXaxis()->GetXmax(),
                         510,"+");
    if ((int(m_xMax) - int(m_xMin)) % 360 == 0) {
      m_XAxis->SetNdivisions(612);
      m_XAxis->SetOption("N+");
    } else if ((int(m_xMax) - int(m_xMin)) % 60 == 0) {
      m_XAxis->SetNdivisions(606);
      m_XAxis->SetOption("N+");
    }

    m_XAxis->ImportAxisAttributes(Hist->GetXaxis());
    m_XAxis->CenterTitle(true);
    m_XAxis->SetTitle(m_xTitle);
    m_XAxis->SetTickSize(1.0f);
    m_XAxis->SetLabelOffset(-0.03f);
    m_XAxis->SetLabelSize(0.03f);
    m_XAxis->SetTitleOffset(-1.5f);
    
    // Remove the current axis
    Hist->GetXaxis()->SetLabelOffset(999.0f);
    Hist->GetXaxis()->SetTickLength(0);
  
    // Draw the new one
    m_XAxis->Draw();
  } else {
    m_XAxis->Draw();
  }
  
  AddNamedSources();
  
  m_Canvas->Update();
  
  // Make sure everything is updated - just in case we will be multi-threading later
  gSystem->ProcessEvents();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageGalactic::HammerConv(double Long, double Lat, double CentralMeridian, double& xHammer, double& yHammer) 
{ 
  Long -= CentralMeridian;

  xHammer = 2.0*sqrt(2.0)*cos(Lat)*sin(0.5*Long) / sqrt(1.0 + cos(Lat)*cos(0.5*Long));
  yHammer = sqrt(2.0)*sin(Lat) / sqrt(1.0 + cos(Lat)*cos(0.5*Long));
  
  xHammer += CentralMeridian;
}


////////////////////////////////////////////////////////////////////////////////


bool MImageGalactic::HammerInvConv(double xHammer, double yHammer, double CentralMeridian, double& Long, double& Lat) 
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


void MImageGalactic::DisplayProjectionHammer()
{
  // Display the image in a canvas
   
  // First we always create a new unprojected histogram and fill it
  TH2D* Unprojected = new TH2D(m_CanvasTitle + "Unprojected", m_Title, m_xNBins, m_xMin, m_xMax, m_yNBins, m_yMin, m_yMax); 
  
  // ... and fill it
  double Content = 0.0;
  for (int x = 1; x <= m_xNBins; ++x) {
    for (int y = 1; y <= m_yNBins; ++y) {
      Content = m_IA[(x-1) + (y-1) * m_xNBins];
      if (!TMath::IsNaN(Content)) {
        if (TMath::Finite(Content)) {
          // Make sure to invert x-axis
          Unprojected->SetBinContent(m_xNBins-x+1, y, Content);
        } else {
          merr<<"Image contains Inf ("<<x<<", "<<y<<")"<<endl;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<", "<<y<<")"<<endl;
      }
    }
  }
  
  // Rescale to 1:
  if (m_Normalize == true && Unprojected->GetMaximum() > 0) {
    Unprojected->Scale(1.0/Unprojected->GetMaximum());
  }

  
  // Then we get or create the projected histogram
  bool IsNew = false;
  TH2D* Hist = 0;
  if (m_Histogram == 0) {
    Hist = new TH2D(m_CanvasTitle + "Hist", m_Title, 4*m_xNBins, m_xMin, m_xMax, 4*m_yNBins, m_yMin, m_yMax);
    Hist->SetContour(50);
    Hist->GetZaxis()->SetTitle(m_vTitle);
    Hist->GetZaxis()->SetTitleOffset(1.2f);
    m_Histogram = dynamic_cast<TH1*>(Hist);

    IsNew = true;
  } else {
    Hist = dynamic_cast<TH2D*>(m_Histogram);
    Hist->SetTitle(m_Title);
  }

  double CentralMeridian = 0.5*(m_xMax+m_xMin)*c_Rad;
  
  // Copy the data:
  double Lat, Long;
  double x, y;
  int ux, uy;
  for (int bx = 0; bx < Hist->GetNbinsX(); ++bx) {
    for (int by = 0; by < Hist->GetNbinsY(); ++by) {
      x = Hist->GetXaxis()->GetBinCenter(bx)*c_Rad;
      y = Hist->GetYaxis()->GetBinCenter(by)*c_Rad;
      
      if (HammerInvConv(x, y, CentralMeridian, Long, Lat) == false) continue;
      
      Long *= c_Deg;
      Lat *= c_Deg;
      if (Long < m_xMin || Long > m_xMax || Lat < m_yMin || Lat > m_yMax) continue;
      
      ux = Unprojected->GetXaxis()->FindBin(Long);
      uy = Unprojected->GetYaxis()->FindBin(Lat);
      Hist->SetBinContent(bx, by, Unprojected->GetBinContent(ux, uy));
    }
  }
  
  delete Unprojected;
  
  
  // Draw the axes
  if (IsNew == true) {
    // Draw data:
    Hist->Draw(m_DrawOptionString + " a");
        
    // Paint coordinates:    
    vector<double> Seperators = { 90.0, 60.0, 45.0, 30.0, 15.0, 10.0, 5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01 };

    Color_t AxisColor = kWhite;
    
    
    // (A) Determine the steps for longitude  
    double xSep = 180.0;
    for (double S: Seperators) {
      if ((m_xMax - m_xMin) / S > 5) {
        xSep = S;
        break;
      }
    }
    vector<double> xSteps;
    xSteps.push_back(xSep*int(m_xMin/xSep));
    if (xSteps[0] < m_xMin) xSteps[0] += xSep;
    while (xSteps.back() + xSep <= m_xMax) {
      xSteps.push_back(xSteps.back() + xSep);
      cout<<"Step: "<<xSteps.back()<<endl;
    }
    
    // (B) Determine the steps for latitude 
    double ySep = 180.0;
    for (double S: Seperators) {
      if ((m_yMax - m_yMin) / S > 5) {
        ySep = S;
        break;
      }
    }
    vector<double> ySteps;
    ySteps.push_back(ySep*int(m_yMin/ySep));
    if (ySteps[0] < m_yMin) ySteps[0] += ySep;
    while (ySteps.back() + ySep <= m_yMax) {
      ySteps.push_back(ySteps.back() + ySep);
    }
    
    
    // (C) Draw Latitude lines
    for (double Long: xSteps) {
      // Go in one degree steps
      vector<double> xPoly;
      vector<double> yPoly;
      
      double L = m_xMin+m_xMax - Long;
      
      for (double Lat = m_yMin; Lat <= m_yMax; Lat += 0.01*(m_yMax-m_yMin)) {
        HammerConv(L*c_Rad, Lat*c_Rad, CentralMeridian, x, y);
        xPoly.push_back(x*c_Deg);
        yPoly.push_back(y*c_Deg);
      }
      
      TPolyLine* P = new TPolyLine(xPoly.size(), &xPoly[0], &yPoly[0], "C");
      P->SetLineStyle(3);
      P->SetLineColor(AxisColor);
      P->Draw();
    }
        
    
    // (D) Draw longitude grid lines
    for (double Lat: ySteps) {
      // Go in one degree steps
      vector<double> xPoly;
      vector<double> yPoly;
      
      for (double Long = m_xMin; Long <= m_xMax; Long += 0.01*(m_xMax-m_xMin)) {
        HammerConv(Long*c_Rad, Lat*c_Rad, CentralMeridian, x, y);
        xPoly.push_back(x*c_Deg);
        yPoly.push_back(y*c_Deg);
      }
      
      TPolyLine* P = new TPolyLine(xPoly.size(), &xPoly[0], &yPoly[0], "C");
      P->SetLineStyle(3);
      P->SetLineColor(AxisColor);
      P->Draw();
    }
    
    
    // (E) Draw latitude grid axis labels
    double FontSize = 0.03;
    double LatGap = 0.07*ySep;
    for (unsigned int l = 0; l < ySteps.size(); ++l) {
      
      if (l == 0 && ySteps[l] - m_yMin < 0.3*ySep) continue;
      if (l == ySteps.size() - 1 && m_yMax - ySteps[l] < 0.3*ySep) continue;
      
      HammerConv(m_xMin*c_Rad, ySteps[l]*c_Rad, CentralMeridian, x, y);
      x *= c_Deg;
      y *= c_Deg;
      
      ostringstream out;
      if (ySteps[l] > 0) out<<"+";
      out<<ySteps[l]<<"#circ";
      
      TLatex* T = new TLatex(x + LatGap, y, out.str().c_str());
      T->SetTextFont(42);
      T->SetTextColor(AxisColor);
      T->SetTextAlign(12);
      T->SetTextSize(FontSize);
      T->Draw();
    }
    
    
    // (E) Draw longitude grid axis labels
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
    
    
    for (unsigned int l = 0; l < xSteps.size(); ++l) {
      
      if (l == 0 && xSteps[l] - m_xMin < 0.3*xSep) continue;
      if (l == xSteps.size() - 1 && m_xMax - xSteps[l] < 0.3*xSep) continue;
      
      double Label = m_xMin+m_xMax - xSteps[l];
      HammerConv(Label*c_Rad, LatPosition*c_Rad, CentralMeridian, x, y);
      x *= c_Deg;
      y *= c_Deg;
      
      ostringstream out;
      Label = xSteps[l];
      while (Label < 0) Label += 360;
      while (Label > 360) Label -= 360;
      //Label = 360 - Label;
      out<<Label<<"#circ";
      
      TLatex* T = new TLatex(x, y, out.str().c_str());
      T->SetTextFont(42);
      T->SetTextColor(AxisColor);
      T->SetTextAlign(22);
      T->SetTextSize(FontSize);
      T->Draw();
    }

  }
  
  AddNamedSources();
  
  m_Canvas->Update();
  
  // Make sure everything is updated - just in case we will be multi-threading later
  gSystem->ProcessEvents();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageGalactic::AddNamedSources()
{
  if (m_SourceCatalog == "") return;

  MPointSourceSelector PSS;
  if (PSS.Open(m_SourceCatalog) == false) {
    mout<<"Error: Unable to open source catalog: "<<m_SourceCatalog<<show;
  }

  if (PSS.GetNPointSources() == 0) {
    mout<<"Error: No sources in source catalog."<<show;
    return;
  }


  // Test a pointsource:
  double L = 184.56, B = -5.78;

  double xT, yT;
  unsigned int i;

  double Distance = (m_yMax - m_yMin) / 100;
  double Shadow = (m_yMax - m_yMin) / 1000;

  float TextSize = 0.02f;
  int TextAlign = 12;  
  //float MarkerSize = 0.6*TextSize;
  //int MarkerAlign = 22;
  
  for (i = 0; i < PSS.GetNPointSources(); i++) {
    L = PSS.GetPointSourceAt(i).GetLongitude();
    B = PSS.GetPointSourceAt(i).GetLatitude();
    //cout<<m_xMin<<":"<<L<<":"<<m_xMax<<endl;
    //cout<<m_yMin<<":"<<B<<":"<<m_yMax<<endl;
    //cout<<PSS.GetPointSourceAt(i)<<":"<<PSS.GetPointSourceAt(i).GetName()<<endl;
    // Try to get it into the window
    while (L > m_xMax) L -= 360;
    while (L < m_xMin) L += 360;
    //cout<<"Long: "<<L<<":"<<m_xMin<<":"<<m_xMax<<" --- Lat: "<<B<<":"<<m_yMin<<":"<<m_yMax<<endl;
    if (L >= m_xMin && L <= m_xMax && B >= m_yMin && B <= m_yMax) {
      //xT = ((L-m_xMin)/(m_xMax-m_xMin))*0.8+0.095;
      //yT = ((B-m_yMin)/(m_yMax-m_yMin))*0.8+0.1;
      
      //cout<<"Drawing "<<PSS.GetPointSourceAt(i).GetName()<<" at "<<xT<<":"<<yT<<endl;
      
      
      if (m_Projection == MImageProjection::c_None) {
        xT = (m_xMax+m_xMin) - L;
        yT = B;
      } else if (m_Projection == MImageProjection::c_Hammer) {
        cout<<"Hammer"<<endl;
        HammerConv(((m_xMax+m_xMin) - L)*c_Rad, B*c_Rad, 0.5*(m_xMax+m_xMin)*c_Rad, xT, yT);
        xT *= c_Deg;
        yT *= c_Deg;
      }

      
      
      //t->SetNDC(false);
      TText* WhiteLowRight = new TText();
      //WhiteLowRight->SetTextFont(32);
      WhiteLowRight->SetTextColor(0);
      WhiteLowRight->SetTextSize(TextSize);
      WhiteLowRight->SetTextAlign(TextAlign);
      WhiteLowRight->DrawText(xT + Distance + Shadow, yT + Distance - Shadow, PSS.GetPointSourceAt(i).GetName());

      TText* WhiteLowLeft = new TText();
      //WhiteLowLeft->SetTextFont(32);
      WhiteLowLeft->SetTextColor(0);
      WhiteLowLeft->SetTextSize(TextSize);
      WhiteLowLeft->SetTextAlign(TextAlign);
      WhiteLowLeft->DrawText(xT + Distance - Shadow, yT + Distance - Shadow, PSS.GetPointSourceAt(i).GetName());

      TText* WhiteHighLeft = new TText();
      //WhiteHighLeft->SetTextFont(32);
      WhiteHighLeft->SetTextColor(0);
      WhiteHighLeft->SetTextSize(TextSize);
      WhiteHighLeft->SetTextAlign(TextAlign);
      WhiteHighLeft->DrawText(xT + Distance - Shadow, yT + Distance + Shadow, PSS.GetPointSourceAt(i).GetName());

      TText* WhiteHighRight = new TText();
      //WhiteHighRight->SetTextFont(32);
      WhiteHighRight->SetTextColor(0);
      WhiteHighRight->SetTextSize(TextSize);
      WhiteHighRight->SetTextAlign(TextAlign);
      WhiteHighRight->DrawText(xT + Distance + Shadow, yT + Distance + Shadow, PSS.GetPointSourceAt(i).GetName());

      TText* Black = new TText();
      //Black->SetTextFont(32);
      Black->SetTextColor(1);
      Black->SetTextSize(TextSize);
      Black->SetTextAlign(TextAlign);
      Black->DrawText(xT + Distance, yT + Distance, PSS.GetPointSourceAt(i).GetName());
      
      
      /*
      TText* MarkerWhiteLowRight = new TText();
      //WhiteLowRight->SetTextFont(32);
      MarkerWhiteLowRight->SetTextColor(0);
      MarkerWhiteLowRight->SetTextSize(MarkerSize);
      MarkerWhiteLowRight->SetTextAlign(MarkerAlign);
      MarkerWhiteLowRight->DrawText(xT + Shadow, yT - Shadow, "x");

      TText* MarkerWhiteLowLeft = new TText();
      //WhiteLowLeft->SetTextFont(32);
      MarkerWhiteLowLeft->SetTextColor(0);
      MarkerWhiteLowLeft->SetTextSize(MarkerSize);
      MarkerWhiteLowLeft->SetTextAlign(MarkerAlign);
      MarkerWhiteLowLeft->DrawText(xT - Shadow, yT - Shadow, "x");

      TText* MarkerWhiteHighLeft = new TText();
      //WhiteHighLeft->SetTextFont(32);
      MarkerWhiteHighLeft->SetTextColor(0);
      MarkerWhiteHighLeft->SetTextSize(MarkerSize);
      MarkerWhiteHighLeft->SetTextAlign(MarkerAlign);
      MarkerWhiteHighLeft->DrawText(xT - Shadow, yT + Shadow, "x");

      TText* MarkerWhiteHighRight = new TText();
      //WhiteHighRight->SetTextFont(32);
      MarkerWhiteHighRight->SetTextColor(0);
      MarkerWhiteHighRight->SetTextSize(MarkerSize);
      MarkerWhiteHighRight->SetTextAlign(MarkerAlign);
      MarkerWhiteHighRight->DrawText(xT + Shadow, yT + Shadow, "x");

      TText* MarkerBlack = new TText();
      //MarkerBlack->SetTextFont(32);
      MarkerBlack->SetTextColor(1);
      MarkerBlack->SetTextSize(MarkerSize);
      MarkerBlack->SetTextAlign(MarkerAlign);
      MarkerBlack->DrawText(xT, yT, "x");
      */
      
      TLatex* T2 = new TLatex(xT, yT, "#oplus");
      T2->SetTextSize(0.025);
      T2->SetTextColor(kWhite);
      T2->SetTextAlign(22);
      T2->SetLineWidth(2);
      T2->Draw();

      TLatex* T = new TLatex(xT, yT, "#otimes");
      T->SetTextSize(0.025);
      T->SetTextColor(kBlack);
      T->SetTextAlign(22);
      T->SetLineWidth(2);
      T->Draw();
  
      
    }
  }

}


// MImageGalactic.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
