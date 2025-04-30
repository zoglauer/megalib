/*
 * MBinnerFISBEL.cxx
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


// Include the header:
#include "MBinnerFISBEL.h"

// Standard libs:
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TMath.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TSystem.h"

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinnerFISBEL)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MBinnerFISBEL::MBinnerFISBEL(unsigned int NBins) : m_NumberOfBins(NBins), m_LongitudeShift(0)
{
  if (m_NumberOfBins > 0) Create(m_NumberOfBins);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MBinnerFISBEL::~MBinnerFISBEL()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Check if we have equal bins
bool MBinnerFISBEL::operator==(const MBinnerFISBEL& Binner) const
{
  if (m_NumberOfBins != Binner.m_NumberOfBins) return false;
  if (m_LongitudeBins != Binner.m_LongitudeBins) return false;
  if (m_LatitudeBinEdges != Binner.m_LatitudeBinEdges) return false;
  if (m_LongitudeShift != Binner.m_LongitudeShift) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create the binning
void MBinnerFISBEL::Create(unsigned int NBins, double LongitudeShift)
{
  // CASEBALL - Constant area, squares at equator, borders along latitude & longitude
  
  // Rules:
  // (1) constant area
  // (2) squarish at equator
  // (3) All borders along latitude & longitude lines
  // (4) latitude distance close to equal



  m_NumberOfBins = NBins;
  m_LongitudeShift = LongitudeShift;
  
  m_LatitudeBinEdges.clear();
  m_LongitudeBins.clear();
  m_NumberOfBinsBeforeLatitudeBin.clear();

  if (m_NumberOfBins == 1) {

    m_LatitudeBinEdges.push_back(0);
    m_LatitudeBinEdges.push_back(TMath::Pi());
    m_LongitudeBins.push_back(1);

  } else {

    int NSubDivisions = m_NumberOfBins;  // <-- remove

    double FixBinArea = 4.0*TMath::Pi() / NSubDivisions;
    double SquareLength = sqrt(FixBinArea);
    int NCollars = int((TMath::Pi() / SquareLength - 1) + 0.5);  // -1 for half one top AND Bottom, 0.5 to round to next integer
    NCollars += 2; // for the half on top and bottom
    //if (NCollars % 2 == 1) ++NCollars;

    //cout<<"Bins: "<<NBins<<endl;
    //cout<<"Fix bin area: "<<FixBinArea *TMath::RadToDeg() *TMath::RadToDeg()<<" deg^2"<<endl;
    //cout<<"Square length: "<<SquareLength*TMath::RadToDeg()<<endl;
    //cout<<"Collars: "<<NCollars<<endl;

    m_LongitudeBins.resize(NCollars, 0);
    m_LatitudeBinEdges.resize(NCollars + 1, 0); // we have one more edge than bins

    // Top and bottom first
    m_LatitudeBinEdges[0] = 0;
    m_LatitudeBinEdges[NCollars] = TMath::Pi();

    // Start on top and bottom with a circular region:
    m_LongitudeBins[0] = 1;
    m_LongitudeBins[NCollars - 1] = m_LongitudeBins[0];

    m_LatitudeBinEdges[1] = acos(1 - 2.0/NSubDivisions);
    m_LatitudeBinEdges[NCollars - 1] = TMath::Pi() - m_LatitudeBinEdges[1];

    // Now the rest iteratively:
    for (int Collar = 1; ((NCollars % 2  == 0) ? Collar < NCollars/2 : Collar <= NCollars/2); ++Collar) {
      double UnusedLatitude = m_LatitudeBinEdges[NCollars - Collar] - m_LatitudeBinEdges[Collar];
      double UnusedCollars = NCollars - 2*Collar;

      double NextEdgeEstimate = m_LatitudeBinEdges[Collar] + UnusedLatitude/UnusedCollars;

      double NextBinsEstimate = 2*TMath::Pi() * (cos(m_LatitudeBinEdges[Collar]) - cos(NextEdgeEstimate)) / FixBinArea;

      // Round
      int NextBins = int(NextBinsEstimate + 0.5);
      double NextEdge = acos(cos(m_LatitudeBinEdges[Collar]) - NextBins*FixBinArea/2.0/TMath::Pi());

      //cout<<"Unused lat: "<<UnusedLatitude*TMath::RadToDeg()<<endl;
      //cout<<"Next edge estimate: "<<NextEdgeEstimate*TMath::RadToDeg()<<"  -->  real: "<<NextEdge*TMath::RadToDeg()<<endl;
      //cout<<"Next bins estimate: "<<NextBinsEstimate<<"  ---> real: "<<NextBins<<endl;

      // Add:
      m_LongitudeBins[Collar] = NextBins;
      if (Collar != NCollars/2) m_LatitudeBinEdges[Collar+1] = NextEdge;
      // Do the same at the bottom (in the center it just self assignment):
      m_LongitudeBins[NCollars - Collar - 1] = NextBins;
      if (Collar != NCollars/2) m_LatitudeBinEdges[NCollars - (Collar+1)] = TMath::Pi() - NextEdge;
    }

  }

  //cout<<"FISBEL::Create() Bin layout (latitude bin edges (amount: "<<m_LatitudeBinEdges.size()<<") followed be longitude bins): "<<endl;
  //for (unsigned int l = 0; l < m_LatitudeBinEdges.size(); ++l) {
    //cout<<m_LatitudeBinEdges[l]*TMath::RadToDeg();
    //if (l < m_LongitudeBins.size()) cout<<" bins: "<<m_LongitudeBins[l];
    //cout<<endl;
  //}

  // Calculate the number of bins *before* the given latitude bin
  // Required to speed up bin search
  m_NumberOfBinsBeforeLatitudeBin.resize(m_LongitudeBins.size());
  unsigned int Sum = 0;
  for (unsigned int b = 0; b < m_LongitudeBins.size(); ++b) {
    m_NumberOfBinsBeforeLatitudeBin[b] = Sum;
    Sum += m_LongitudeBins[b];
  }

  // Just for debugging
  //View();
}


////////////////////////////////////////////////////////////////////////////////


void MBinnerFISBEL::Set(vector<unsigned int>& LongitudeBins, vector<double>& LatitudeBinEdges, unsigned int NumberOfBins, double LongitudeShift)
{
  m_LongitudeBins = LongitudeBins;
  m_LatitudeBinEdges = LatitudeBinEdges;
  m_NumberOfBins = NumberOfBins;
  m_LongitudeShift = LongitudeShift;

  m_NumberOfBinsBeforeLatitudeBin.resize(m_LongitudeBins.size());
  unsigned int Sum = 0;
  for (unsigned int b = 0; b < m_LongitudeBins.size(); ++b) {
    m_NumberOfBinsBeforeLatitudeBin[b] = Sum;
    Sum += m_LongitudeBins[b];
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Find a bin
unsigned int MBinnerFISBEL::FindBin(double Theta, double Phi) const
{
  // Theta (= latitude) and phi (= longitude) are in (mathematical) spherical coordinates
  // The bins are arranged along the iso-latitude lines starting at the north pole (theta = latitude = 0)

  // Sanity checks
  if (Theta < 0 || Theta > c_Pi) {
    throw MExceptionParameterOutOfRange(Theta, 0, c_Pi, "Theta/Latitude");
    return 0;
  }

  // Get rid of some boarder cases:
  if (Theta == 0) return 0;
  if (Theta == c_Pi) return m_NumberOfBins - 1;

  while (Phi < m_LongitudeShift) Phi += 2*c_Pi;
  while (Phi >= m_LongitudeShift + 2*c_Pi) Phi -= 2*c_Pi; // >= get's rid of the other boarder case: require: phi = [0..360[

  unsigned int Bins = 0;

  // First find theta bin greater or equal (that's what lower_bound does) to Theta
  vector<double>::const_iterator ThetaIter = lower_bound(m_LatitudeBinEdges.begin(), m_LatitudeBinEdges.end(), Theta);

  if (ThetaIter == m_LatitudeBinEdges.begin()) {
    throw MExceptionNeverReachThatLineOfCode("The case: Theta == 0 has been handled before...");
    return 0;
  }

  // The bin (-1 since we have "greater or equal")
  unsigned int LatBin = ThetaIter - m_LatitudeBinEdges.begin() - 1;

  // Add the bins so far
  Bins += m_NumberOfBinsBeforeLatitudeBin[LatBin];

  // Then find longitude bin (phi goes [m_LongitudeShift .. m_LongitudeShift + 2*pi[ )
  Bins += int((Phi - m_LongitudeShift) / (2*c_Pi / m_LongitudeBins[LatBin]));

  if (Bins >= m_NumberOfBins) {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfBins, Bins);
    return 0;
  }

  //cout<<"Found bin: "<<Theta*c_Deg<<":"<<Phi*c_Deg<<endl;

  return Bins;
}


////////////////////////////////////////////////////////////////////////////////


//! Return axis bins edges for external drawing
vector<vector<double>> MBinnerFISBEL::GetDrawingAxisBinEdges() const
{
  vector<double> xAxis;
  xAxis.push_back(m_LongitudeShift);
  for (unsigned int l = 0; l < m_LongitudeBins.size(); ++l) {
    for (unsigned int b = 1; b < m_LongitudeBins[l]; ++b) {
      xAxis.push_back(m_LongitudeShift + (double(b)/m_LongitudeBins[l]) * 2*TMath::Pi());
    }
  }
  xAxis.push_back(m_LongitudeShift + 2*TMath::Pi());

  // Sort and remove duplicates
  sort(xAxis.begin(), xAxis.end());
  xAxis.erase(unique( xAxis.begin(), xAxis.end() ), xAxis.end());

  vector<vector<double>> Axes;
  Axes.push_back(xAxis);
  Axes.push_back(m_LatitudeBinEdges);

  return Axes;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the bin center(s) of the given axis bin
//! Can throw: MExceptionIndexOutOfBounds
vector<double> MBinnerFISBEL::GetBinCenters(unsigned int Bin) const
{
  if (Bin >= m_NumberOfBins) {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfBins, Bin);
  }
  
  // --> This is a bit too complicated to be right... or good
  
  //cout<<"Bin: "<<Bin<<endl;
  
  vector<double> Return;
  
  // We always have one bin at bottom and top, which need to be handled differently:
  if (Bin == 0) {
    Return = { 0, 0 };
  } else if (Bin == m_NumberOfBins - 1) {
    Return = { c_Pi, 0 };
  } else {
    
    //cout<<"m_NumberOfBinsBeforeLatitudeBin: "<<m_NumberOfBinsBeforeLatitudeBin.size()<<endl;
    //for (unsigned int i = 0; i < m_NumberOfBinsBeforeLatitudeBin.size(); ++i) {
    //  cout<<i<<": "<<m_NumberOfBinsBeforeLatitudeBin[i]<<endl;
    //}
    
    
    // Find the latitude bin - we cannot use upper_bound or lower_bound
    unsigned int LatitudeBin = 0;
    for (unsigned int i = 0; i < m_NumberOfBinsBeforeLatitudeBin.size() - 1; ++i) {
      if (Bin >= m_NumberOfBinsBeforeLatitudeBin[i] && Bin < m_NumberOfBinsBeforeLatitudeBin[i+1]) {
        LatitudeBin = i;
        break;
      }
    }
    //cout<<"Edges: "<<m_LatitudeBinEdges[LatitudeBin]*c_Deg<<":"<<m_LatitudeBinEdges[LatitudeBin+1]*c_Deg<<endl;
    
    unsigned int LongitudeBins = Bin - m_NumberOfBinsBeforeLatitudeBin[LatitudeBin];
        
    Return = { 0.5*(m_LatitudeBinEdges[LatitudeBin]+m_LatitudeBinEdges[LatitudeBin+1]), m_LongitudeShift + double(LongitudeBins+0.5)*2*c_Pi/m_LongitudeBins[LatitudeBin] };
  }
  
  //cout<<Bin<<" --> lat: "<<Return[0]*c_Deg<<" - long: "<<Return[1]*c_Deg<<endl;
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Returns all bin centers as vector
vector<MVector> MBinnerFISBEL::GetAllBinCenters() const
{
  vector<MVector> Vectors;
  
  for (unsigned int b = 0; b < m_NumberOfBins; ++b) {
    vector<double> Centers = GetBinCenters(b);
    
    MVector V;
    V.SetMagThetaPhi(1.0, Centers[0], Centers[1]);
    Vectors.push_back(V);
  }
  
  return Vectors;
}


////////////////////////////////////////////////////////////////////////////////


//! Show a histogram of the data
void MBinnerFISBEL::View(vector<double> Data) const
{
  cout<<"Long bins: "<<m_LongitudeBins.size()<<": "; for (auto b: m_LongitudeBins) cout<<b<<" "; cout<<endl;

  // Create a histogram

  vector<vector<double>> Axes = GetDrawingAxisBinEdges();
  vector<double> xAxis = Axes[0];
  vector<double> yAxis = Axes[1];

  // Some conversion to allow display for ROOT in AITOFF projection...
  for (unsigned int a = 0; a < xAxis.size(); ++a) xAxis[a] = xAxis[a]*TMath::RadToDeg() - 180;
  for (unsigned int a = 0; a < yAxis.size(); ++a) yAxis[a] = yAxis[a]*TMath::RadToDeg() -  90;

  bool UseAitoff = false;
  if ( fabs(yAxis[0] + 90) < 0.001 && fabs(yAxis.back() - 90) < 0.001 ) UseAitoff = true;


  cout<<"xAxis entries: "<<xAxis.size()<<" (vs. "<<m_NumberOfBins<<" bins)"<<endl;

  TH2D* Hist = new TH2D("Skyview", "Skyview", xAxis.size()-1, &xAxis[0], yAxis.size()-1, &yAxis[0]);
  Hist->SetXTitle("Longitude [deg]");
  Hist->SetYTitle("Latitude [deg]");
  Hist->SetStats(false);

  // Do a checkerbox fill - or use the given data
  if (Data.size() != m_NumberOfBins) {
    int Min = 1;
    int Max = 2;
    int Switch = 1;
    int Start = 1;
    for (unsigned int l = 0; l < m_LongitudeBins.size(); ++l) {
      //++Start;
      if (Start > Max) Start = Min;
      Switch = Start;
      for (unsigned int b = 1; b <= m_LongitudeBins[l]; ++b) {
        ++Switch;
        if (Switch > Max) Switch = Min;
        int bb_start = Hist->GetXaxis()->FindBin(double(b-1)/m_LongitudeBins[l] * 360 - 180);
        int bb_stop = Hist->GetXaxis()->FindBin(double(b)/m_LongitudeBins[l] * 360 - 180);
        //cout<<bb_start<<":"<<bb_stop<<endl;
        for (int bb = bb_start; bb < bb_stop; ++bb) {
          Hist->SetBinContent(bb, l+1, Switch);
        }
      }
    }
  } else {
    // Use given data:
    unsigned int CurrentBin = 0;
    for (unsigned int l = 0; l < m_LongitudeBins.size(); ++l) {
      for (unsigned int b = 1; b <= m_LongitudeBins[l]; ++b) {
        int bb_start = Hist->GetXaxis()->FindBin(double(b-1)/m_LongitudeBins[l] * 360 - 180);
        int bb_stop = Hist->GetXaxis()->FindBin(double(b)/m_LongitudeBins[l] * 360 - 180);
        for (int bb = bb_start; bb < bb_stop; ++bb) {
          double Content = Data[CurrentBin];
          if (Content == 0) Content = 1E-9;
          Hist->SetBinContent(bb, l+1, Content);
        }
        ++CurrentBin;
      }
    }
  }

  TCanvas* C = new TCanvas("C", "C", 4*360, 4*180);
  C->cd();
  if (UseAitoff == true) {
    Hist->Draw("AITOFFz");
  } else {
    Hist->Draw("colz");
  }
  C->Update();

  gSystem->ProcessEvents();
}


// MBinnerFISBEL.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
