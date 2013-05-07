/*
 * MDisplay.cxx
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


#include "MDisplay.h"

#define pi          3.14159265359
//#define bogenmass   pi / 180.0
//#define grad        180.0 / pi
 

//////////////////////////////////////////////////////////////////////////////////
//
//  ctor
//
// 
//
MDisplay::MDisplay(double xMin, double xMax, int xBins, double yMin, 
									 double yMax, int yBins, TString DrawMode)
{
  m_xMin = xMin;
  m_xMax = xMax;
  m_xBins = xBins;
  m_xInt = (xMax - xMin)/xBins;
  m_yMin = yMin;
  m_yMax = yMax;
  m_yBins = yBins;
  m_yInt = (yMax - yMin)/yBins;
  m_DrawMode = DrawMode;
}



//////////////////////////////////////////////////////////////////////////////////
//
// dtor
//
MDisplay::~MDisplay()
{

}


//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::SetGlobalAttributes(double xMin, double xMax, int xBins, 
																	 double yMin, double yMax, int yBins, 
																	 double zMin, double zMax, int zBins, TString DrawMode)
{
  m_xMin = xMin;
  m_xMax = xMax;
  m_xBins = xBins;
  m_xInt = (xMax - xMin)/xBins;

  m_yMin = yMin;
  m_yMax = yMax;
  m_yBins = yBins;
  m_yInt = (yMax - yMin)/yBins;

  m_zMin = zMin;
  m_zMax = zMax;
  m_zBins = zBins;
  m_zInt = (zMax - zMin)/zBins;

  m_DrawMode = DrawMode;
}


/*
	//////////////////////////////////////////////////////////////////////////////////
	//
	// 
	//
	void MDisplay::SetGlobalAttributes(MMatrixInfo Info)
	{
  m_Info = Info;

  if (Info.GetCoordinateSystem() == 1)
	{
	m_xMin = Info.GetPhiMin() * grad;
	m_xMax = Info.GetPhiMax() * grad;
	m_xBins = Info.GetBinsPhi();
	m_xInt = (m_xMax - m_xMin)/m_xBins;
	m_yMin = Info.GetThetaMin() * grad;
	m_yMax = Info.GetThetaMax() * grad;
	m_yBins = Info.GetBinsTheta();
	m_yInt = (m_yMax - m_yMin)/m_yBins;
	m_DrawMode = "CONT0";
	}
  else
	{
	m_xMin = Info.GetPhiMin();
	m_xMax = Info.GetPhiMax();
	m_xBins = Info.GetBinsPhi();
	m_xInt = (m_xMax - m_xMin)/m_xBins;
	m_yMin = Info.GetThetaMin();
	m_yMax = Info.GetThetaMax();
	m_yBins = Info.GetBinsTheta();
	m_yInt = (m_yMax - m_yMin)/m_yBins;
	m_DrawMode = "CONT0";
	}


	}
*/


//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::SetViewport(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
{
  m_xMin = xMin;
  m_xMax = xMax;
  m_yMin = yMin;
  m_yMax = yMax;
  m_zMin = zMin;
  m_zMax = zMax;
}



//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, double* Values, double Maximum)
{ 
	// Hier her für rotationen!!


  TCanvas* Canvas = new TCanvas(Title, Title, 40, 40, 600, 600);
 
  //TH2D* Hist = new TH2D(Title, Title, m_yBins, m_yMin, m_yMax, m_xBins, m_xMin, m_xMax);
  TH2D* Hist = new TH2D(Title, Title, m_xBins, m_xMin, m_xMax, m_yBins, m_yMin, m_yMax);
  //cout<<"Display: "<<m_xMin<<"!"<<m_xMax<<"!"<<m_xBins<<"!"<<m_yMin<<"!"<<m_yMax<<"!"<<m_yBins<<endl;


  Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  Hist->SetZTitle(zTitle);
  Hist->SetFillColor(0);
  Hist->SetLabelOffset(float(-0.003), "Y");
  Hist->SetTitleOffset(float(1.5), "X");
  Hist->SetTitleOffset(float(1.3), "Y");
  Hist->SetTitleOffset(float(1.3), "Z");
  Hist->SetLabelSize(float(0.03), "X");
  Hist->SetLabelSize(float(0.03), "Y");
  Hist->SetLabelSize(float(0.03), "Z");
  Hist->SetStats(false);

  if (Maximum != -1)
    Hist->SetMaximum(Maximum);

  int x, y;
  for (x = 0; x < m_xBins; x++) {
    for (y = 0; y < m_yBins; y++) {
      Hist->Fill((x+0.5) * m_xInt + m_xMin, (y+0.5) * m_yInt + m_yMin, Values[x + y * m_xBins]);
    }
  }
      //Hist->Fill(j * m_yInt + m_yMin + m_yInt/2.0, i * m_xInt + m_xMin + m_xInt/2.0, Values[j + (m_yBins - i -1) * m_yBins]);

  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  // schwarz nach weiss
  //int Color[9] = {12, 13, 14, 15, 16, 17, 18, 19, 10};
  //gStyle->SetPalette(9, Color);
  //weiss nach schwarz
  //int Color[9] = {10, 19, 18, 17, 16, 15, 14, 13, 12};
  //gStyle->SetPalette(9, Color);

  // Spektrum:
  gStyle->SetPalette(1, 0);


  Hist->Draw("COLCONT4");

  //Canvas->Update();
  
  gSystem->ProcessEvents();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MDisplay::DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, double* Values, int Unkown)
{ 
  // Display a three dimensional histogram for 3D reconstruction

  //cout<<"3D Histogram"<<endl;

  int x, y, z;
  //char Text[100];

  // Determine the maximum

  double Maximum = 0;
  for (z = 0; z < m_zBins*m_xBins*m_yBins; z++) {
    if (Values[z] > Maximum) {
      Maximum = Values[z];
    }
  }

  char Text[1000];
  
  m_DrawMode = "COLCONT4Z";

//   // Let's create subimages and paint them 
//   double* SubImage = new double[m_xBins*m_yBins];

//   for (z = 0; z < m_zBins; z++) {
//     for (x = 0; x < m_xBins; x++) {
//       for (y = 0; y < m_yBins; y++) {
// 				SubImage[x + y*m_xBins] = Values[x + y*m_xBins + z*m_xBins*m_yBins];
// 				//cout<<x<<", "<<y<<", "<<z<<": "<<SubImage[x + y*m_xBins]<<"|  ";
//       }
//     }
//     //cout<<endl;
//     sprintf(Text, "%s (z=%1.1fcm)", Title, (z + 0.5)*m_zInt + m_zMin );
//     sprintf(Text, "z=%1.1fcm", (z + 0.5)*m_zInt + m_zMin );
//     DisplayHistogram(Text, xTitle, yTitle, zTitle, SubImage, Maximum);
//   }
  


  // let's create three images for the maxima in x, y and z -Direction:

  // x: for fixed x search the maxima for the belonging y and z

  double Max;
  //m_DrawMode = "COLZ";
  double* Projection = new double[m_yBins*m_zBins];

  for (y = 0; y < m_yBins; y++) {
    for (z = 0; z < m_zBins; z++) {
      Max = 0;
      for (x = 0; x < m_xBins; x++) {
				// Maximum:
				//if (Values[x + y*m_xBins + z*m_xBins*m_yBins] > Max) Max = Values[x + y*m_xBins + z*m_xBins*m_yBins];
				// average:
				Max += Values[x + y*m_xBins + z*m_xBins*m_yBins];
      }
      Projection[z + y*m_zBins] = Max/m_xBins;
    }
  }

  sprintf(Text, "Projection along x-axis");
  DisplayHistogram(Text, "y [cm]", "z [cm]", "Intensity", Projection, m_yMin, m_yMax, m_yBins, m_zMin, m_zMax, m_zBins);

  
  
  Projection = new double[m_xBins*m_zBins];
  /////
  for (x = 0; x < m_xBins; x++) {
    for (z = 0; z < m_zBins; z++) {
      Max = 0;
      for (y = 0; y < m_yBins; y++) {
				// Maximum:
				//if (Values[x + y*m_xBins + z*m_xBins*m_yBins] > Max) Max = Values[x + y*m_xBins + z*m_xBins*m_yBins];
				// Average:
				Max += Values[x + y*m_xBins + z*m_xBins*m_yBins];
      }
      Projection[z + x*m_zBins] = Max/m_yBins;
    }
  }
  
  sprintf(Text, "Projection along y-axis");
  DisplayHistogram(Text, "x [cm]", "z [cm]", "Intensity", Projection, m_xMin, m_xMax, m_xBins, m_zMin, m_zMax, m_zBins);
 
  Projection = new double[m_xBins*m_yBins];
  /////
  for (x = 0; x < m_xBins; x++) {
    for (y = 0; y < m_yBins; y++) {
      Max = 0;
      for (z = 0; z < m_zBins; z++) {
				// Maximum:
				//if (Values[x + y*m_xBins + z*m_xBins*m_yBins] > Max) Max = Values[x + y*m_xBins + z*m_xBins*m_yBins];
				// Average:
				Max += Values[x + y*m_xBins + z*m_xBins*m_yBins];
      }
      Projection[y + x*m_yBins] = Max/m_zBins;
    }
  }
  
  sprintf(Text, "Projection along z-axis");
  DisplayHistogram(Text, "x [cm]", "y [cm]", "Intensity",  Projection, m_xMin, m_xMax, m_xBins, m_yMin, m_yMax, m_yBins);
  
  

  //delete [] SubImage;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MDisplay::DisplayHistogram(TString Title, TString xTitle, TString yTitle, double* Values, double xMin, double xMax, int NBins)
{  
  // Display a one dimensional histogram

  TH1D* Hist = new TH1D(Title, Title, NBins, xMin, xMax);

  Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  Hist->SetFillColor(0);
  Hist->SetTitleOffset(float(1.1), "X");
  Hist->SetTitleOffset(float(1.1), "Y");
  Hist->SetLabelSize(float(0.03), "X");
  Hist->SetLabelSize(float(0.03), "Y");
  Hist->SetLabelOffset(float(0.015), "X");
  Hist->SetLabelOffset(float(0.02), "Y");
  Hist->SetTickLength(float(-0.02), "X");
  Hist->SetTickLength(float(-0.02), "Y");
  Hist->SetFillColor(32);
  Hist->SetStats(false);

  int i;
  for (i = 0; i < NBins; i++) {
    Hist->Fill(xMin + (xMax-xMin)/NBins*(i+0.5), Values[i]);
  }

  TCanvas* Canvas = new TCanvas(Title, Title, 40, 40, 700, 500);

  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  Hist->Draw();
  
  gSystem->ProcessEvents();
}


//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::DisplayGausFit1D(TString Title, TString xTitle, TString yTitle, TArrayF* Values, int mode)
{
  TH1D* Hist;

  double Max;
  int i, j;
  TF1* f1;
  if (mode == 0)
    {
      Hist = new TH1D(Title, Title, m_xBins, m_xMin, m_xMax);
      for (i = 0; i < m_xBins; i++)
				{
					Max = 0.0;
					for (j = 0; j < m_yBins; j++)
						{
							if (Values->At(j + i*m_yBins) > Max)
								Max = Values->At(j + i*m_yBins);
						}				  
					Hist->Fill((i * m_xInt + m_xMin + m_xInt/2.0), Max);
				}
      f1 = new TF1("acz", "gaus", m_yMin, m_yMax);
    }
  else
    {
      Hist = new TH1D(Title, Title, m_yBins, m_yMin, m_yMax);
      for (j = 0; j < m_yBins  ; j++)
				{
					Max = 0.0;
					for (i = 0; i < m_xBins; i++)
						{
							if (Values->At(j + i*m_yBins) > Max)
								Max = Values->At(j + i*m_yBins);
						}				  
					Hist->Fill((j * m_yInt + m_yMin + m_yInt/2.0), Max);
				}
      f1 = new TF1("acz", "gaus", m_xMin, m_xMax);
    }
  
  Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetZTitle(yTitle);

  Hist->Fit("acz", "W0" );
  TCanvas* Canvas = new TCanvas(Title, Title, 80, 80, 600, 600);

  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetBorderMode(0);

  Hist->Draw();
  gSystem->ProcessEvents();

  delete f1;
}




//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::GausFit1D(double* Mean, double* Sigma, TArrayF* Values, int mode)
{
  TH1D* Hist;

  double Max;
  int i, j;
  TF1* f1;

  
  if (mode == 0)
    {
      Hist = new TH1D("nonamey", "nonamey", m_yBins, m_yMin, m_yMax);
      for (i = 0; i < m_xBins; i++)
				{
					Max = 0.0;
					for (j = 0; j < m_yBins; j++)
						{
							if (Values->At(j + i*m_yBins) > Max)
								Max = Values->At(j + i*m_yBins);
						}				  
					Hist->Fill((i * m_xInt + m_xMin + m_xInt/2.0), Max);
				}
      f1 = new TF1("acz", "gaus", m_yMin, m_yMax);
    }
  else
    {
      Hist = new TH1D("nonamex", "nonamex", m_xBins, m_xMin, m_xMax);
      for (j = 0; j < m_yBins  ; j++)
				{
					Max = 0.0;
					for (i = 0; i < m_xBins; i++)
						{
							if (Values->At(j + i*m_yBins) > Max)
								Max = Values->At(j + i*m_yBins);
						}				  
					Hist->Fill((j * m_yInt + m_yMin + m_yInt/2.0), Max);
				}
      f1 = new TF1("acz", "gaus", m_xMin, m_xMax);
    }


  Hist->Fit("acz", "W0Q" );
  //Hist->SetTitle("Double gauss fit");
  *Mean = f1->GetParameter(1);
  *Sigma = f1->GetParameter(2);

  delete f1;
}





//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MDisplay::DisplayGausFit2D(TString Title, TString xTitle, TString yTitle, TString zTitle, TArrayF* Values)
{
  TH2D* Hist = new TH2D(Title, Title, m_xBins, m_xMin, m_xMax, m_yBins, m_yMin, m_yMax);

  Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  Hist->SetZTitle(zTitle);

  int i, j;
  for (i = 0; i < m_xBins; i++)
    for (j = 0; j < m_yBins; j++)
      Hist->Fill((i * m_xInt + m_xMin + m_xInt/2.0), (j * m_yInt + m_yMin + m_yInt/2.0), Values->At(j + i*m_yBins));


  TF2* f1 = new TF2("Andy", "[0] * exp((-0.5*((x-[1])/[2])**2) + ( -0.5*((y-[3])/[4])**2))", 
										m_xMin, m_xMax, m_yMin, m_yMax);
  
  f1->SetNpx(m_xBins);
  f1->SetNpy(m_yBins);

  double a, b, c, d;
  GausFit1D(&a, &b, Values, 0);
  GausFit1D(&c, &d, Values, 1);
  f1->SetParameters(1, a, b, c, d); 
  
  f1->SetParLimits(0, 0.0, 5.0);
  f1->SetParLimits(1, (double) (m_xMin), (double) (m_xMax));
  f1->SetParLimits(2, 3.0, 0.01);
  f1->SetParLimits(3, (double) (m_yMin), (double) (m_yMax));
  f1->SetParLimits(4, 3.0, 0.01);
  

  f1->SetParNames("Konstante","Mittelwert Phi","Sigma Phi", "Mittelwert Theta", "Sigma Theta");

  Hist->Fit("Andy", "W0");
  f1->SetTitle("2D Gauss fit");
  TCanvas* Canvas = new TCanvas(Title, Title, 0, 0, 600, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetBorderMode(0);

  f1->Draw(m_DrawMode);
  gSystem->ProcessEvents();

}


////////////////////////////////////////////////////////////////////////////////


void MDisplay::DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle, int NBins,
																double* xValues, double xMin, double xMax, int xNBins,
																double* yValues, double yMin, double yMax, int yNBins,
																double* zValues, double zMin, double zMax, int zNBins)
{
  // Display a three dimensional histogram

  TH3F* Hist = new TH3F(Title, Title, xNBins, xMin, xMax, yNBins, yMin, yMax, zNBins, zMin, zMax);

  Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  Hist->SetZTitle(zTitle);
  Hist->SetFillColor(0);
  Hist->SetTitleOffset(float(1.1), "X");
  Hist->SetTitleOffset(float(1.1), "Y");
  Hist->SetLabelSize(float(0.03), "X");
  Hist->SetLabelSize(float(0.03), "Y");
  Hist->SetLabelOffset(float(0.015), "X");
  Hist->SetLabelOffset(float(0.02), "Y");
  Hist->SetTickLength(float(-0.02), "X");
  Hist->SetTickLength(float(-0.02), "Y");
  Hist->SetFillColor(39);
  Hist->SetStats(false);

  int i;

  double xBinWidth, yBinWidth, zBinWidth;

  xBinWidth = (xMax - xMin)/xNBins; 
  yBinWidth = (yMax - yMin)/yNBins; 
  zBinWidth = (zMax - zMin)/zNBins; 

  
  for (i = 0; i < NBins; i++)
    {
      /*
				Hist->Fill3((int) ((xValues[i] - xMin)/xBinWidth), 
				(int) ((yValues[i] - yMin)/yBinWidth), 
				(int) ((zValues[i] - zMin)/zBinWidth), 1);
				cout<<(xValues[i] - xMin)/xBinWidth<<"!"<<(yValues[i] - yMin)/yBinWidth<<"!"<<(zValues[i] - zMin)/zBinWidth<<endl;
      */
      Hist->Fill(xValues[i], yValues[i], zValues[i], 1);
    }
  

  //Hist->Fill3(1,1,1,1);

  TCanvas* Canvas = new TCanvas(Title, Title, 40, 40, 600, 600);
  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  Hist->Draw();
  
  gSystem->ProcessEvents();
}




////////////////////////////////////////////////////////////////////////////////


void MDisplay::DisplayHistogram(TString Title, TString xTitle, TString yTitle, TString zTitle,
																double* Values, double xMin, double xMax, int xBins,
																double yMin, double yMax, int yBins)
{
  // Display a three dimensional histogram for 3D reconstruction

  TCanvas* Canvas = new TCanvas(Title, Title, 40, 40, 600, 600);
  TH2D* Hist = new TH2D(Title, Title, xBins, xMin, xMax, yBins, yMin, yMax);

  for (int x = 0; x < xBins; ++x)
    for (int y = 0; y < yBins; ++y) {
      Hist->SetBinContent(x+1, y+1, Values[y + x*yBins]);
    }

  Canvas->SetFillColor(0);
  Canvas->SetFrameBorderSize(0);
  Canvas->SetFrameBorderMode(0);
  Canvas->SetBorderSize(0);
  Canvas->SetBorderMode(0);

  // Spektren:
  // schwarz nach weiss
  //int Color[9] = {12, 13, 14, 15, 16, 17, 18, 19, 10};
  //gStyle->SetPalette(9, Color);

  // weiss nach schwarz
//   int Color[9] = {10, 19, 18, 17, 16, 15, 14, 13, 12};
//   gStyle->SetPalette(9, Color);

  // Bunt:
  //gStyle->SetPalette(1, 0);


  //Hist->Draw(m_DrawMode);
  //Hist->SetDirectory(0);
  Hist->SetXTitle(xTitle);
  Hist->SetYTitle(yTitle);
  //Hist->SetFillColor(0);
  //Hist->SetTitleOffset(float(1.4), "X");
  //Hist->SetTitleOffset(float(1.4), "Y");
  //Hist->SetLabelSize(float(0.025), "X");
  //Hist->SetLabelSize(float(0.025), "Y");
  //Hist->SetLabelOffset(float(0.015), "X");
  //Hist->SetLabelOffset(float(0.01), "Y");
  //Hist->SetTickLength(float(0.02), "X");
  //Hist->SetTickLength(float(0.015), "Y");
  //Hist->SetFillColor(39);
  //Hist->SetStats(false);
  Hist->Draw(m_DrawMode);
  
  gSystem->ProcessEvents();

  return;
}


// The end ...
//////////////////////////////////////////////////////////////////////////////////
