/* 
 * Converter.cxx
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

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TEllipse.h>
#include <TLatex.h>
#include <TMarker.h>
#include <TFile.h>
#include <TKey.h>
#include <TMath.h>
#include <TColor.h>
#include <TPaletteAxis.h>
#include <TRandom.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MTokenizer.h"

// Special
#include "fitsio.h"


////////////////////////////////////////////////////////////////////////////////


//! A Converter program based on MEGAlib and ROOT
class Converter
{
public:
  //! Default constructor
  Converter();
  //! Default destructor
  ~Converter();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  //!
  void Galactify(TH2D* Galaxy, TCanvas* Canvas, double LatLimit=90, MString FileName="");


private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  
  //! Fits file
  MString m_FitsFileName;
  
  //! The line ID
  unsigned int m_LineID;
  
  //! The map ID
  unsigned int m_MapID;
  
  //! 
  int m_LongBins;
  double m_LongMin;
  double m_LongMax;

  int m_LatBins;
  double m_LatMin;
  double m_LatMax;

};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
Converter::Converter() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  
  m_LongMin  = -180;
  m_LongMax  = +180;
  m_LatMin   = -40;
  m_LatMax   = +40;

  m_LineID = 1;
  m_MapID = 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
Converter::~Converter()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool Converter::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Converter <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -l:   line ID (1: 26-Al, 2: 60-Fe)"<<endl;
  Usage<<"         -m:   map ID (1: DIRBE 240um, 2: Free-free)"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-m") {
      m_MapID = atoi(argv[++i]);
      cout<<"Accepting map ID: "<<m_MapID<<endl;
    } else if (Option == "-l") {
      m_LineID = atoi(argv[++i]);
      cout<<"Accepting line ID: "<<m_LineID<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool Converter::Analyze()
{
  if (m_Interrupt == true) return false;

  MString OutputFileName;
  MString Element;
  double TotalFluxInnerGalaxy;
  double TotalFlux;
  
  // Normalize the flux by the inner Galxy contribution
  bool NormalizeByInnerGalaxyFlux = true;

  
  
  if (m_LineID == 1) { // Al
  
    OutputFileName = "AllSky_Al26";
    Element = "Galactic ^{26}Al map";
  
    TotalFluxInnerGalaxy = 3.3E-4; // in ph/cm2/s --- Diehl - only inner galaxy
    TotalFlux = 7.4E-4; // in ph/cm2/s --- DA Plueschke - MREM
    // TotalFlux = 5.7E-4; // in ph/cm2/s --- DA Plueschke - MaxEnt
    // TotalFlux = 7.9E-4; // in ph/cm2/s --- DA Oberlack
    
    if (NormalizeByInnerGalaxyFlux == true) {
      OutputFileName += "_NormInnerGalaxyDiehl";
    } else {
      OutputFileName += "_NormAllSkyPlueschke";
    }
 
  } else if (m_LineID == 2) { // Fe

    OutputFileName = "AllSky_Fe60";
    Element = "Galactic ^{60}Fe map";
  
    TotalFluxInnerGalaxy = 3.7E-5; // in ph/cm2/s --- Harris - IRON    PER LINE
    TotalFlux = 7.4E-4 * (3.7E-5/3.3E-4); // in ph/cm2/s --- IRON scaled from DA Plueschke - MREM

    if (NormalizeByInnerGalaxyFlux == true) {
      OutputFileName += "_NormInnerGalaxyHarris";
    } else {
      OutputFileName += "_NormAllSkyPlueschkeAl26Scaled";
    }

  } else {
    mout<<"Error: Unknown line ID "<<m_LineID<<endl; 
    return false;
  }

  
    
  MString Filename;
  MString Tracer;

  if (m_MapID == 1) {
    Filename = "skymap.surveys_convert_1.DIRBE_240micron_MIS_13119.fits";
    Tracer = "DIRBE 240 micron map";
    OutputFileName += "_DIRBE240um";
  } else if (m_MapID == 2) {
    Filename = "skymap.surveys_convert_1.free-free_MIS_13081.fits";
    Tracer = "53 GHz free-free emission map";
    OutputFileName += "_FreeFree";
  } else {
    mout<<"Error: Unknown map ID "<<m_MapID<<endl; 
    return false;
  }
  
  OutputFileName += ".dat";
  
  
  
  // --------------------------------------------
  // Load the fits file
  //
  // Load map...
  int status = 0;
  fitsfile* MapFile = 0; 
  if (fits_open_file(&MapFile, Filename.Data(), READONLY, &status)) {
    cout<<"Error Unable to open fits file \""<<Filename<<"\""<<endl;
    return false;
  }

  // Checkhow many HDU's we have:
  int hdunum = 0;
  fits_get_num_hdus(MapFile, &hdunum, &status);
  cout<<"Number of HDU's: "<<hdunum<<endl;

  int maxdim = 3;
  int bitpix = 0;
  int naxis  = 0;
  long naxes[maxdim];
  int imagehdu = -1;
  int hdutype = 0;
  for (int i = 1; i <= hdunum; ++i) {
    fits_movabs_hdu(MapFile, i, &hdutype, &status);
    if (hdutype == IMAGE_HDU) {
      //cout<<"Found first image: hdu="<<i<<endl;

      fits_get_img_param(MapFile, maxdim, &bitpix, &naxis, naxes, &status);
      if (naxis < 2) {
        cout<<"Error: First image is no map..."<<endl;
      } else {
        imagehdu = i;
        break;
      }
    }
  }

  if (imagehdu == -1) {
    cout<<"Error: No image found in fitsfile"<<endl;
    return false;
  }    

  // Get the image parameters:
  //cout<<naxis<<":"<<bitpix<<endl;
  //for (int i = 1; i <= naxis; ++i) {
  //  cout<<"Axis "<<i<<": "<<naxes[i-1]<<endl;
  //}
    
  // Let's setup a ROOT image...

  double InputCutOff = 1E-55;
  //cout<<"Cutting off input at "<<InputCutOff<<endl;

  TH2D* FitsImage = new TH2D(Tracer, Tracer, naxes[0], -180, 180, naxes[1], -90, 90);
  FitsImage->SetContour(50);
  FitsImage->SetStats(true);
  FitsImage->SetXTitle("Gal. Longitude [rad]");
  FitsImage->SetYTitle("Gal. Latitude [rad]");

  // Read the data:
  long fpixel[2];
  float data;
  int anynull = 0;
  for (long x = 1; x <= naxes[0]; ++x) {
    for (long y = 1; y <= naxes[1]; ++y) {
      fpixel[0] = x;
      fpixel[1] = y;
      fits_read_pix(MapFile, TFLOAT, fpixel, 1, NULL, &data, &anynull, &status);
      if (data < InputCutOff) data = 0.0;
      FitsImage->SetBinContent(naxes[0]-x+1, y, data);
    }
  }

  
  // --------------------------------------------
  // Normalize and draw the fits file
  //

  TCanvas* FitsImageCanvas = new TCanvas();

  double InnerGalaxyPhiMin = -30;
  double InnerGalaxyPhiMax = +30;

  double InnerGalaxyThetaMin = -10;
  double InnerGalaxyThetaMax = +10;

  // Calculate total fluxes:
  double TotalFluxInImage = 0.0;
  double TotalFluxInnerGalaxyInImage = 0.0;
  for (int bphi = 1; bphi <= FitsImage->GetNbinsX(); bphi++) {
    for (int btheta = 1; btheta <= FitsImage->GetNbinsY(); btheta++) {
      double PBinSize = 2*c_Pi*(cos(FitsImage->GetYaxis()->GetBinLowEdge(btheta)*c_Rad+c_Pi/2)-cos(FitsImage->GetYaxis()->GetBinUpEdge(btheta)*c_Rad+c_Pi/2))*FitsImage->GetXaxis()->GetBinWidth(bphi)*c_Rad/(2*c_Pi);
      TotalFluxInImage += FitsImage->GetBinContent(bphi, btheta)*PBinSize;
      if (FitsImage->GetYaxis()->GetBinCenter(btheta) >= InnerGalaxyThetaMin && 
          FitsImage->GetYaxis()->GetBinCenter(btheta) <= InnerGalaxyThetaMax &&
          FitsImage->GetXaxis()->GetBinCenter(bphi) >= InnerGalaxyPhiMin &&
          FitsImage->GetXaxis()->GetBinCenter(bphi) <= InnerGalaxyPhiMax) {
        TotalFluxInnerGalaxyInImage += FitsImage->GetBinContent(bphi, btheta)*PBinSize;
      }                                                   
    }
  }
  // cout<<"Total flux in image: "<<TotalFluxInImage<<" ph/cm2/s"<<endl;
  // cout<<"Total flux inner galaxy in image: "<<TotalFluxInnerGalaxyInImage<<" ph/cm2/s"<<endl;


  

  double FluxCorrection = 1.0;
  if (NormalizeByInnerGalaxyFlux == true) {
    FluxCorrection = TotalFluxInnerGalaxy/TotalFluxInnerGalaxyInImage;
  } else {
    FluxCorrection = TotalFlux/TotalFluxInImage;
  }

  // Rescale the image so that each bin center contains a point source and all of them added up result in the correct total flux:
  double NewTotalFluxInImage = 0.0;
  double NewTotalFluxInnerGalaxyInImage = 0.0;
  for (int bphi = 1; bphi <= FitsImage->GetNbinsX(); bphi++) {
    for (int btheta = 1; btheta <= FitsImage->GetNbinsY(); btheta++) {
      double PBinSize = 2*c_Pi*(cos(FitsImage->GetYaxis()->GetBinLowEdge(btheta)*c_Rad+c_Pi/2)-cos(FitsImage->GetYaxis()->GetBinUpEdge(btheta)*c_Rad+c_Pi/2))*FitsImage->GetXaxis()->GetBinWidth(bphi)*c_Rad/(2*c_Pi);
      FitsImage->SetBinContent(bphi, btheta, FitsImage->GetBinContent(bphi, btheta)*FluxCorrection*PBinSize);
      NewTotalFluxInImage += FitsImage->GetBinContent(bphi, btheta);
      if (FitsImage->GetYaxis()->GetBinCenter(btheta) >= InnerGalaxyThetaMin && 
          FitsImage->GetYaxis()->GetBinCenter(btheta) <= InnerGalaxyThetaMax &&
          FitsImage->GetXaxis()->GetBinCenter(bphi) >= InnerGalaxyPhiMin &&
          FitsImage->GetXaxis()->GetBinCenter(bphi) <= InnerGalaxyPhiMax) {
        NewTotalFluxInnerGalaxyInImage += FitsImage->GetBinContent(bphi, btheta);
      }                                                   
    }
  }
  cout<<"Total flux in image after correction: "<<NewTotalFluxInImage<<" ph/cm2/s"<<endl;
  cout<<"Total flux inner galaxy in image after correction: "<<NewTotalFluxInnerGalaxyInImage<<" ph/cm2/s"<<endl;

  MString ImageFileName = OutputFileName;
  ImageFileName.ReplaceAtEndInPlace(".dat", ".pdf");
  Galactify(FitsImage, FitsImageCanvas, 90, ImageFileName);

  // Finally convert to COSIMA file:
  ofstream out;
  out.open(OutputFileName);
  out<<"IP LIN"<<endl;
  out<<"PA ";
  for (int bphi = 1; bphi <= FitsImage->GetNbinsX(); bphi++) {
    out<<FitsImage->GetXaxis()->GetBinCenter(bphi)<<" ";
  }
  out<<FitsImage->GetXaxis()->GetBinCenter(1)+360<<endl;
  out<<"TA 0.0 ";
  for (int btheta = 1; btheta <= FitsImage->GetNbinsY(); btheta++) {
    out<<FitsImage->GetYaxis()->GetBinCenter(btheta)+90<<" ";
  }
  out<<180<<endl;
  
  if (m_LineID == 1) {
    out<<"EA 1808.719 1808.720 1808.721"<<endl;
  } else if (m_LineID == 2) {
    out<<"EA 1173.239 1173.240 1173.241  1332.507 1332.508 1332.509"<<endl;
  } else {
    mout<<"Error: Unknown line ID "<<m_LineID<<endl; 
    return false;
  }
  
  for (int bphi = 1; bphi <= FitsImage->GetNbinsX(); bphi++) {
    for (int btheta = 1; btheta <= FitsImage->GetNbinsY(); btheta++) {
      double Area = 2*c_Pi*(cos(FitsImage->GetYaxis()->GetBinLowEdge(btheta)*c_Rad+c_Pi/2)-cos(FitsImage->GetYaxis()->GetBinUpEdge(btheta)*c_Rad+c_Pi/2))*FitsImage->GetXaxis()->GetBinWidth(bphi)*c_Rad/(2*c_Pi);
      if (m_LineID == 1) {
        out<<"AP "<<bphi-1<<" "<<btheta-1<<" 1 "<<FitsImage->GetBinContent(bphi, btheta)/0.001/Area<<endl;
      } else if (m_LineID == 2) {
        out<<"AP "<<bphi-1<<" "<<btheta-1<<" 1 "<<FitsImage->GetBinContent(bphi, btheta)/0.001/Area<<endl;
        out<<"AP "<<bphi-1<<" "<<btheta-1<<" 4 "<<FitsImage->GetBinContent(bphi, btheta)/0.001/Area<<endl;
      }
    }
  }
  
  out<<"EN"<<endl;

  

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
void Converter::Galactify(TH2D* Galaxy, TCanvas* Canvas, double LatLimit, MString FileName)
{
  // 

  double Scaler = 3.0;
  Canvas->SetWindowSize(int(Scaler*(m_LongMax-m_LongMin)), int(Scaler*(20+(m_LatMax-m_LatMin))));
  Canvas->cd();

//   double Minimum = numeric_limits<double>::max();
//   for (int bphi = 1; bphi <= Galaxy->GetXaxis()->GetNbins(); bphi++) {
//     for (int btheta = 1; btheta <= Galaxy->GetYaxis()->GetNbins(); btheta++) {
//       if (Galaxy->GetBinContent(bphi, btheta) < Minimum && Galaxy->GetBinContent(bphi, btheta) > 0) {
//         Minimum = Galaxy->GetBinContent(bphi, btheta);
//       }
//     }
//   }
//   for (int bphi = 1; bphi <= Galaxy->GetXaxis()->GetNbins(); bphi++) {
//     for (int btheta = 1; btheta <= Galaxy->GetYaxis()->GetNbins(); btheta++) {
//       if (Galaxy->GetBinContent(bphi, btheta) < Minimum) {
//         Galaxy->SetBinContent(bphi, btheta, Minimum);
//       }
//     }
//   }


  //Galaxy->SetContour(50);
  Galaxy->SetStats(true);
  Galaxy->SetXTitle("galactic longitude [degree]");
  Galaxy->SetYTitle("galactic latitude [degree]");
  //Galaxy->SetZTitle("ph/cm^{2}/s/sr");

  //Galaxy->GetYaxis()->SetNdivisions(504);
  Galaxy->GetYaxis()->SetTickLength(0.013);
  Galaxy->GetYaxis()->SetTitleOffset(0.8);
  Galaxy->GetYaxis()->CenterTitle();
  Galaxy->GetXaxis()->SetNdivisions(-512);
  Galaxy->GetXaxis()->SetLabelColor(0);
  Galaxy->GetXaxis()->CenterTitle();
  Galaxy->GetXaxis()->SetTitleOffset(1.25);

  Galaxy->Draw("AITOFF");
  //Galaxy->Draw("z col");
  //Canvas->SetLogz();
  Canvas->Update();

  TPaletteAxis* Palette = (TPaletteAxis*) Galaxy->GetListOfFunctions()->FindObject("palette");

  if (Palette != 0) {
    Palette->SetTitleOffset(0.8);
    Palette->SetLabelOffset(0.005);
  } else {
    cout<<"Cannot find palette!"<<endl;
  }

  TText* Text;

  unsigned int w = 0, h = 0;
  //double xOffset = 7-180.0;
  double xOffset = -0.575;
  //double yOffset = -7+yMin;
  double xInc = 0.095833;
  //double yOffset = -7+yMin;
  double yOffset = -0.63;
  for (int i = 0; i <= 5; ++i) {
    ostringstream out;
    out<<180-i*30;
    //Text = new TText(i*30.0+xOffset, yOffset,  out.str().c_str());
    Text = new TText(xOffset+i*xInc, yOffset,  out.str().c_str());
    cout<<i*30.0+xOffset<<":"<<yOffset<<":"<<out.str().c_str()<<endl;
    Text->SetTextAlign(22);
    Text->SetTextFont(42);
    Text->SetTextSize(0.04);
    Text->GetTextExtent(w, h, out.str().c_str());
    //Text->SetX(i*30.0+xOffset-w/2.0);
    Text->Draw();
  }

  //xOffset = 3.5;
  //Text = new TText(180.0+xOffset, yOffset, "0");
  Text = new TText(xOffset+6*xInc, yOffset, "0");
  Text->SetTextAlign(22);
  Text->SetTextSize(0.04);
  Text->SetTextFont(42);
  Text->GetTextExtent(w, h, "0");
  //Text->SetX(180+xOffset-w/2.0);
  Text->Draw();

  //xOffset = 9;
  for (int i = 7; i <= 12; ++i) {
    ostringstream out;
    out<<330-(i-7)*30;
    //Text = new TText(i*30.0+xOffset, yOffset,  out.str().c_str());
    Text = new TText(xOffset+i*xInc, yOffset,  out.str().c_str());
    Text->SetTextAlign(22);
    Text->SetTextFont(42);
    Text->SetTextSize(0.04);
    Text->GetTextExtent(w, h, out.str().c_str());
    //Text->SetX(i*30.0+xOffset-w/2.0);
    Text->Draw();
  }

  Canvas->Update();
  
  if (FileName != "") {
    Canvas->SaveAs(FileName);
  }
}


////////////////////////////////////////////////////////////////////////////////


Converter* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Converter", "a Converter example program");

  TApplication ConverterApp("ConverterApp", 0, 0);

  g_Prg = new Converter();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //ConverterApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
