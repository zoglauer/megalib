/*
 * MImage.cxx
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
// MImage
//
// Base class for all other image classes:
//
// Its designed for 2D-Images
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImage.h"

// Standard libs:
using namespace std;
#include <iostream>
#include <fstream>

// ROOT libs:
#include <TMath.h>
#include <TColor.h>
#include <TSeqCollection.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImage)
#endif


////////////////////////////////////////////////////////////////////////////////


int MImage::m_IDCounter = 0;
bool MImage::m_PalettesCreated = false;
int MImage::m_GlobalSpectrum = -1;

// The GUI relies on this sequence - don't modifier unless you modifiy MGUIImageOptions
const int MImage::c_RootDefault    = 0;
const int MImage::c_BlackWhite     = 1;
const int MImage::c_WhiteBlack     = 2;
const int MImage::c_DeepSea        = 3;
const int MImage::c_SkyLine        = 4;
// const int MImage::c_SunRise       = 5;
const int MImage::c_Incendescent   = 6;
const int MImage::c_WhiteRedBlack  = 7;
const int MImage::c_Thesis         = 8;
const int MImage::c_ThesisVarAtLow = 9;
const int MImage::c_Rainbow        = 10;

// The GUI relies on this sequence - don't modifier unless you modifiy MGUIImageOptions
const int MImage::c_COL           = 0;
const int MImage::c_COLZ          = 1;
const int MImage::c_COLCONT4      = 2;
const int MImage::c_COLCONT4Z     = 3;


////////////////////////////////////////////////////////////////////////////////


MImage::MImage()
{
  // Constructor initialising an empty image
  // Caution: What happens when the data-array is empty??

  Init();

  SetTitle("Title here");
  SetImageArray((double *) 0);
  SetXAxis("x-axis", 0, 1, 1);
  SetValueAxisTitle("values");
  SetSpectrum(0);
  SetDrawOption(c_COLCONT4Z);
}


////////////////////////////////////////////////////////////////////////////////


MImage::MImage(MString Title, double* IA,
               MString xTitle, double xMin, double xMax, int xNBins,
               MString vTitle, int Spectrum, int DrawOption)
{
  // Construct an image but do not display it, i.e. save only the data
  //
  // Title:      Title of the image, 0 means no title
  // IA:         data-array
  // xTitle:     title of the x axis
  // xMin:       minimum x-value
  // xMax:       maximum x-value
  // xNBins:     number of bins in x
  // Spectrum:   spectrum
  // DrawOption: ROOT draw option

  Init();
  m_NEntries = xNBins;

  SetTitle(Title);
  SetXAxis(xTitle, xMin, xMax, xNBins);
  SetValueAxisTitle(vTitle);
  SetSpectrum(Spectrum);
  SetDrawOption(DrawOption);

  SetImageArray(IA);
}


////////////////////////////////////////////////////////////////////////////////


MImage::~MImage()
{
  // standard destructor

  if (m_IA != 0) delete [] m_IA;
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImage::Clone()
{
  //! Clone this image

  MImage* I =
    new MImage(m_Title, m_IA,
               m_xTitle, m_xMin, m_xMax, m_xNBins,
               m_vTitle, m_Spectrum, m_DrawOption);

  I->Normalize(m_Normalize);

  return I;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::Init()
{
  // Initializations common to all constructors

  m_IA = 0;
  m_Canvas = 0;
  m_Histogram = 0;
  m_NEntries = 0;

  m_Normalize = false;

  // ID of this image:
  if (m_IDCounter == numeric_limits<int>::max()) {
    m_IDCounter = 0;
  } else {
    m_IDCounter++;
  }

  m_IsCreated = false;

  gStyle->SetTitleX(float(0.1));
  //gStyle->SetTitleY(0.90);
  gStyle->SetTitleW(float(0.8));
  //gStyle->SetTitleH(0.05);

//   const int Number = 8;
//   double Stops[Number] = { 0.051753, 0.365609, 0.602671, 0.749583, 0.789649, 0.864775, 0.925948, 1.000000 };
//   double Red[Number] = { 0.054902, 0.109804, 0.513725, 0.917647, 0.958824, 1.000000, 0.968627, 0.937255 };
//   double Green[Number] = { 0.033334, 0.066667, 0.054902, 0.043137, 0.288235, 0.533333, 0.729411, 0.925490 };
//   double Blue[Number] = { 0.284313, 0.568627, 0.305882, 0.043137, 0.021569, 0.000000, 0.107843, 0.215686 };
//   TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetTitle(MString Title)
{
  // Set the title of the image

  m_Title = Title;

  if (m_Histogram != 0) {
    m_Histogram->SetTitle(m_Title);
  }
  if (m_Canvas != 0) {
    m_Canvas->SetTitle(m_Title);
    m_Canvas->Update();
  }
}


////////////////////////////////////////////////////////////////////////////////


const char* MImage::GetTitle() const
{
  // Return the title

  return m_Title;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetImageArray(double* IA)
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

  if (m_Histogram != 0) {
    double Content = 0.0;
    for (int x = 0; x < m_xNBins; x++) {
      Content = m_IA[x];
      if (!TMath::IsNaN(Content)) {
        if (TMath::Finite(Content)) {
          m_Histogram->Fill((x+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin, m_IA[x]);
        } else {
          merr<<"Image contains Inf ("<<x<<")"<<show;
        }
      } else {
        merr<<"Image contains NaN ("<<x<<")"<<show;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetXAxis(MString xTitle, double xMin, double xMax, int xNBins)
{
  // Set the data of the x-axis

  m_xTitle = xTitle;
  m_xMin = xMin;
  m_xMax = xMax;
  m_xNBins = xNBins;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetSpectrum(int Spectrum)
{
  // Set the spectrum of this image

  m_Spectrum = Spectrum;

  if (m_GlobalSpectrum == m_Spectrum) return;

  if (m_Spectrum == c_RootDefault) {
    gStyle->SetPalette(1, 0);
  } else if (m_Spectrum == c_Thesis) {
    const unsigned int Number = 7;
    Double_t Red[Number]   = { 0.2500, 0.5000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000 };
    Double_t Green[Number] = { 0.0000, 0.0000, 0.0000, 0.5000, 1.0000, 1.0000, 1.0000 };
    Double_t Blue[Number]  = { 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.5000, 0.7500 };
    Double_t Stops[Number] = { 0.0000, 0.1000, 0.3000, 0.5000,  0.700, 0.9000, 1.0000 };

    // Arithmetic-mean:
    double Min = (Red[0]+Green[0]+Blue[0])/3.0;
    double Max = (Red[Number-1]+Green[Number-1]+Blue[Number-1])/3.0;
    Stops[0] = 0.0;
    Stops[Number-1] = 1.0;
    for (unsigned int i = 1; i < Number-1; ++i) {
      double Value = ((Red[i]+Green[i]+Blue[i])/3.0-Min)/(Max-Min);
      Stops[i] = Value;
    }
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_ThesisVarAtLow) {
    const unsigned int Number = 8;
    Double_t Red[Number]   = { 0.0500, 0.5000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000 };
    Double_t Green[Number] = { 0.0000, 0.0000, 0.0000, 0.5000, 1.0000, 1.0000, 1.0000, 1.0000 };
    Double_t Blue[Number]  = { 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.5000, 0.7500, 1.0000 };
    Double_t Stops[Number] = { 0.0000, 0.050, 0.090, 0.1366, 0.2233, 0.5000, 0.7500, 1.0000 };

    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_BlackWhite) {
    int Number = 2;
    double Stops[2] = { 0.00, 1.00 };
    double Red[2] = { 0.00, 1.00 };
    double Green[2] = { 0.00, 1.00 };
    double Blue[2] = { 0.00, 1.00 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_WhiteBlack) {
    int Number = 2;
    double Stops[2] = { 0.00, 1.00 };
    double Red[2] = { 1.00, 0.00 };
    double Green[2] = { 1.00, 0.00 };
    double Blue[2] = { 1.00, 0.00 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_DeepSea) {
    int Number = 5;
    double Stops[5] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    double Red[5] = { 0.00, 0.09, 0.18, 0.09, 0.3 };
    double Green[5] = { 0.01, 0.02, 0.39, 0.68, 0.97 };
    double Blue[5] = { 0.17, 0.39, 0.62, 0.79, 0.97 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_SkyLine) {
    const int Number = 8;
    double Red[Number] = { 0.054902, 0.109804, 0.513725, 0.917647, 0.958824, 1.000000, 0.968627, 0.937255 };
    double Green[Number]  = { 0.033334, 0.066667, 0.054902, 0.043137, 0.288235, 0.533333, 0.729411, 0.925490 };
    double Blue[Number] = { 0.284313, 0.568627, 0.305882, 0.043137, 0.021569, 0.000000, 0.107843, 0.215686 };
    double Stops[Number]   = { 0.051753, 0.365609, 0.602671, 0.749583, 0.789649, 0.864775, 0.925948, 1.000000 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
//   } else if (m_Spectrum == c_SunRise) {
//     const int Number = 8;
//     double Red[Number]     = { 0.054902, 0.109804, 0.513725, 0.917647, 0.958824, 1.000000, 0.968627, 0.937255 };
//     double Green[Number]   = { 0.033334, 0.066667, 0.054902, 0.043137, 0.288235, 0.533333, 0.729411, 0.925490 };
//     double Blue[Number]    = { 0.284313, 0.568627, 0.305882, 0.043137, 0.021569, 0.000000, 0.107843, 0.215686 };
//     double Stops[Number]   = { 0.051753, 0.365609, 0.602671, 0.749583, 0.789649, 0.864775, 0.925948, 1.000000 };
//     TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_Incendescent) {
    const int Number = 9;
    double Red[Number]   = { 0.000000, 0.364706, 0.729412, 0.864706, 1.000000, 0.986274, 0.972549, 0.974510, 0.976471 };
    double Green[Number] = { 0.000000, 0.000000, 0.000000, 0.272549, 0.545098, 0.741176, 0.937255, 0.952941, 0.968627 };
    double Blue[Number]  = { 0.000000, 0.000000, 0.000000, 0.098039, 0.196078, 0.135294, 0.074510, 0.452941, 0.831373 };
    double Stops[Number] = { 0.000000, 0.459098, 0.594324, 0.677796, 0.809683, 0.853088, 0.899833, 0.948247, 1.000000 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_WhiteRedBlack) {
    const int Number = 6;
    double Stops[Number]   = { 0.00, 0.03, 0.08, 0.38, 0.78, 1.00 };
    double Red[Number]     = { 1.00, 1.00, 1.00, 1.00, 0.50, 0.00 };
    double Green[Number]   = { 1.00, 1.00, 0.50, 0.00, 0.00, 0.00 };
    double Blue[Number]    = { 1.00, 1.00, 0.50, 0.00, 0.00, 0.00 };
    TColor::CreateGradientColorTable(Number, Stops, Red, Green, Blue, 100);
  } else if (m_Spectrum == c_Rainbow) {
    gStyle->SetPalette(55, 0);
  } else {
    merr<<"Unknown draw palette option. Using default."<<show;
    gStyle->SetPalette(55, 0);
  }

  m_GlobalSpectrum = m_Spectrum;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetDrawOption(const int Option)
{
  // Set the "ROOT"-draw option, e.g. HIST or LEGO2, CONT0Z, etc.

  m_DrawOption = Option;

  if (Option == c_COL) {
    m_DrawOptionString = "COL";
  } else if (Option == c_COLZ) {
    m_DrawOptionString = "COLZ";
  } else if (Option == c_COLCONT4) {
    m_DrawOptionString = "COLCONT4";
  } else if (Option == c_COLCONT4Z) {
    m_DrawOptionString = "COLCONT4Z";
  } else {
    merr<<"Unknown draw option. Using \"COLCONT4Z\""<<show;
    m_DrawOption = c_COLCONT4Z;
    m_DrawOptionString = "COLCONT4Z";
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MImage::IsCreated()
{
  // Return true if all histograms are created

  return m_IsCreated;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SetCreated()
{
  // Set this flag after all histograms have been initialized

  m_IsCreated = true;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::SaveAs(MString FileName)
{
  //! Save the canvas if it exists

  if (m_Canvas != 0 && CanvasExists() == true) {
    m_Canvas->SaveAs(FileName);
  } else {
    mout<<"Error: Cannot save image because the canvas does not or no longer exist!"<<endl;
  }
}

////////////////////////////////////////////////////////////////////////////////


bool MImage::CanvasExists()
{
  // Check if the window is still existing

  if (gROOT->FindObject(m_CanvasTitle) == 0) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MImage::MakeCanvasTitle()
{
  // Create a Canvas title of form "<m_Title> - <m_ID>"

  char Text[1000];
  sprintf(Text, "%s - %i", (const char *) m_Title.Data(), MImage::m_IDCounter);

  MString Cleaned = Text;
  Cleaned.ReplaceAll(' ', '_');
  Cleaned.ReplaceAll('-', '_');
  Cleaned.ReplaceAll(':', '_');
  Cleaned.ReplaceAll('/', '_');
  Cleaned.ReplaceAll('\\', '_');
  Cleaned.ReplaceAll('!', '_');
  Cleaned.ReplaceAll('$', '_');
  Cleaned.ReplaceAll('@', '_');
  Cleaned.ReplaceAll('#', '_');
  Cleaned.ReplaceAll('$', '_');
  Cleaned.ReplaceAll('%', '_');
  Cleaned.ReplaceAll('^', '_');
  Cleaned.ReplaceAll('&', '_');
  Cleaned.ReplaceAll('*', '_');
  Cleaned.ReplaceAll('(', '_');
  Cleaned.ReplaceAll(')', '_');
  Cleaned.ReplaceAll('+', '_');
  Cleaned.ReplaceAll('=', '_');
  Cleaned.ReplaceAll('{', '_');
  Cleaned.ReplaceAll('[', '_');
  Cleaned.ReplaceAll('}', '_');
  Cleaned.ReplaceAll(']', '_');
  Cleaned.ReplaceAll('|', '_');
  Cleaned.ReplaceAll('<', '_');
  Cleaned.ReplaceAll('>', '_');
  Cleaned.ReplaceAll('\'', '_');
  Cleaned.ReplaceAll('?', '_');

  return Cleaned;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::Display(TCanvas* Canvas)
{
  // Display the image in a canvas

  if (Canvas == 0) {
    m_CanvasTitle = MakeCanvasTitle();
    Canvas = new TCanvas(m_CanvasTitle, m_Title, 40, 40, 640, 480);
  } else {
    if (MString(Canvas->GetTitle()).IsEmpty() == false) {
      m_CanvasTitle = Canvas->GetTitle();
    } else {
      m_CanvasTitle = MakeCanvasTitle();
      Canvas->SetTitle(m_CanvasTitle);
    }
  }
  m_Canvas = Canvas;

  if (m_Histogram == 0) {
    m_Histogram = new TH1D(m_Title, m_Title, m_xNBins, m_xMin, m_xMax);

    m_Histogram->SetDirectory(0);
    m_Histogram->SetXTitle(m_xTitle);
    m_Histogram->SetYTitle(m_vTitle);
    m_Histogram->SetFillColor(0);
    m_Histogram->SetTitleOffset(float(1.1), "X");
    m_Histogram->SetTitleOffset(float(1.1), "Y");
    m_Histogram->SetLabelSize(float(0.03), "X");
    m_Histogram->SetLabelSize(float(0.03), "Y");
    m_Histogram->SetLabelOffset(float(0.015), "X");
    m_Histogram->SetLabelOffset(float(0.02), "Y");
    m_Histogram->SetTickLength(float(-0.02), "X");
    m_Histogram->SetTickLength(float(-0.02), "Y");
    m_Histogram->SetFillColor(32);
    m_Histogram->SetStats(false);
  } else {
    m_Histogram->SetTitle(m_Title);
  }

  double Content = 0.0;
  for (int x = 0; x < m_xNBins; x++) {
    Content = m_IA[x];
    if (!TMath::IsNaN(Content)) {
      if (TMath::Finite(Content)) {
        m_Histogram->Fill((x+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin, Content);
      } else {
        merr<<"Image contains Inf ("<<x<<")"<<endl;
      }
    } else {
      merr<<"Image contains NaN ("<<x<<")"<<endl;
    }
  }

  // Rescale to 1:
  if (m_Normalize == true && m_Histogram->GetMaximum() > 0) {
    m_Histogram->Scale(1.0/m_Histogram->GetMaximum());
  }

  m_Histogram->Draw(m_DrawOptionString);

  SetCreated();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImage::Reset()
{
  // Reset to default values:

  for (int x = 0; x < m_xNBins; x++) {
    m_IA[x] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////


void MImage::DrawCopy()
{
  //

  mimp<<"Not yet implemented"<<show;
}


////////////////////////////////////////////////////////////////////////////////


double MImage::GetAverage()
{
  //! Calculate the average

  if (m_NEntries == 0) return 0;

  double Average = 0.0;
  for (int e = 0; e < m_NEntries; ++e) {
    Average += m_IA[e];
  }

  return Average / m_NEntries;
}


////////////////////////////////////////////////////////////////////////////////


//! Determine the maximum and its coordiantes, the vector is filled up to the number of dimensions the histogram has
void MImage::DetermineMaximum(double& MaxValue, vector<double>& Coordinate)
{
  MaxValue = 0;
  double MaxIndex = 0;

  for (int e = 0; e < m_NEntries; ++e) {
    if (m_IA[e] > MaxValue) {
      MaxValue = m_IA[e];
      MaxIndex = e;
    }
  }

  Coordinate.clear();
  Coordinate.push_back((MaxIndex+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin);
}


////////////////////////////////////////////////////////////////////////////////

/*
void MImage::ExportCSV(MString FileName)
{
  // Export the file as a space delimited table (

  fstream FileStream;
  //FileStream.width(10);
  // gcc 2.95.3: FileStream.open(FileName, ios::out, 0664);
  FileStream.open(FileName, ios_base::out);

  int x;
  for (x = 0; x < m_xNBins; x++) {
      FileStream<<(x+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin<<"      "
                <<m_IA[x]<<endl;
  }

  FileStream.close();

  return;
}
*/

////////////////////////////////////////////////////////////////////////////////

/*
void MImage::ExportFits()
{
  fitsfile *fptr;

  int status;
  long exposure;

  long naxis = 2;
  long naxes[2];
  naxes[0] = m_xNBins;
  naxes[1] = m_yNBins;


  status = 0;
  fits_create_file(&fptr, "!testfile.fits", &status);
  cout<<"1. "<<endl;
  fits_report_error(stderr, status);


  fits_create_img(fptr, DOUBLE_IMG, naxis, naxes, &status);
  cout<<"2. "<<endl;
  fits_report_error(stderr, status);

  exposure = 1500;

  fits_update_key(fptr, TLONG, m_Title, &exposure, "more text", &status);
  cout<<"3. "<<endl;
  fits_report_error(stderr, status);

  double Image[m_NEntries];

  for (int i = 0; i < m_yNBins; i++)
    for (int j = 0; j < m_xNBins; j++) {
      Image[j + i*m_yNBins] = m_IA[j + (m_yNBins - i -1) * m_yNBins];
    }

  fits_write_img(fptr, TDOUBLE, 1, m_NEntries, Image, &status);
  cout<<"4. "<<endl;
  fits_report_error(stderr, status);

  fits_close_file(fptr, &status);
  cout<<"5. "<<endl;
  fits_report_error(stderr, status);

  fits_report_error(stderr, status);
  cout<<status<<endl;
}
*/


// MImage: the end...
////////////////////////////////////////////////////////////////////////////////
