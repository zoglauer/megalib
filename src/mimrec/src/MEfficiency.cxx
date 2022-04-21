/*
 * MEfficiency.cxx
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
#include "MEfficiency.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEfficiency)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MEfficiency::MEfficiency() : m_IsLoaded(false), m_IsFarField(false)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MEfficiency::~MEfficiency()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MEfficiency::Load(MString EfficiencyFile)
{
  m_IsLoaded = false;

  if (EfficiencyFile.EndsWith(".efficiency.90y.rsp") == true ||
      EfficiencyFile.EndsWith(".efficiency.90y.rsp.gz") == true) {
    return LoadFarField(EfficiencyFile);
  }
  else if (EfficiencyFile.EndsWith(".emittedxdetectedy.rsp") == true ||
      EfficiencyFile.EndsWith(".emittedxdetectedy.rsp.gz") == true) {
    return LoadNearField(EfficiencyFile);
  }

  cout<<"Error: File is not of type .efficiency. or of type .emittedxdetectedy.: \""<<EfficiencyFile<<"\""<<endl;
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MEfficiency::LoadFarField(MString EfficiencyFile)
{
  m_IsLoaded = false;

  m_Efficiency.Read(EfficiencyFile);
  m_Efficiency.Smooth(10);

  m_EfficiencyStartArea = m_Efficiency.GetFarFieldStartArea();
  m_EfficiencySimulatedEvents = m_Efficiency.GetSimulatedEvents();

  m_EfficiencyRotation = MRotation(90.0 * c_Rad, MVector(0, 1, 0));


  // Normalize
  for (unsigned int lo = 0; lo < m_Efficiency.GetAxisBins(1); ++lo) {
    for (unsigned int la = 0; la < m_Efficiency.GetAxisBins(2); ++la) {
      // The photons have been simulated into 4pi
      // (1) Calculate the photons per sr in the GetAxisBins
      double Area = (cos(m_Efficiency.GetAxisLowEdge(la, 2)*c_Rad) - cos(m_Efficiency.GetAxisHighEdge(la, 2)*c_Rad)) * 2*c_Pi/(m_Efficiency.GetAxisHighEdge(lo, 1)*c_Rad - m_Efficiency.GetAxisLowEdge(lo, 1)*c_Rad);
      // (2) Calculate the effective area
      long StartedPhotons = m_EfficiencySimulatedEvents * Area / 4 / c_Pi;
      double EffectiveArea = m_EfficiencyStartArea * m_Efficiency.GetBinContent(lo, la) / StartedPhotons;
      m_Efficiency.SetBinContent(lo, la, EffectiveArea);
    }
  }
  //m_Efficiency.Show();

  m_IsLoaded = true;
  m_IsFarField = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MEfficiency::LoadNearField(MString EfficiencyFile)
{
  mout<<"Loading the efficiency file .. that may take a while..."<<endl;
    
  m_IsLoaded = false;

  m_EfficiencyNearField.Read(EfficiencyFile);
  // m_EfficiencyNearField.Smooth(10);

  m_EfficiencyStartArea = m_EfficiencyNearField.GetFarFieldStartArea();
  m_EfficiencySimulatedEvents = m_EfficiencyNearField.GetSimulatedEvents();

  MTimer Timer;
  m_EfficiencyNearField.SwitchToNonSparse();
  mout<<"Switching to non-sparse matrix done in "<<Timer.GetElapsed()<<" sec "<<endl;
  
  // Normalize
  mout<<"Normalizing the efficiency file .. that may take a while..."<<endl;
  Timer.Reset();

  // The full area:
  double FullArea =
    (m_EfficiencyNearField.GetAxis(0).GetMinima()[0] - m_EfficiencyNearField.GetAxis(0).GetMaxima()[0]) *
    (m_EfficiencyNearField.GetAxis(1).GetMinima()[0] - m_EfficiencyNearField.GetAxis(1).GetMaxima()[0]) *
    (m_EfficiencyNearField.GetAxis(2).GetMinima()[0] - m_EfficiencyNearField.GetAxis(2).GetMaxima()[0]);

    
  vector<double> xAxisEdges = m_EfficiencyNearField.GetAxis(0).Get1DBinEdges();
  vector<double> yAxisEdges = m_EfficiencyNearField.GetAxis(1).Get1DBinEdges();
  vector<double> zAxisEdges = m_EfficiencyNearField.GetAxis(2).Get1DBinEdges();
  vector<double> EAxisEdges = m_EfficiencyNearField.GetAxis(3).Get1DBinEdges();
    
  vector<double> x2AxisEdges = m_EfficiencyNearField.GetAxis(4).Get1DBinEdges();
  vector<double> y2AxisEdges = m_EfficiencyNearField.GetAxis(5).Get1DBinEdges();
  vector<double> z2AxisEdges = m_EfficiencyNearField.GetAxis(6).Get1DBinEdges();
  vector<double> E2AxisEdges = m_EfficiencyNearField.GetAxis(7).Get1DBinEdges();
  
  unsigned long xBins = m_EfficiencyNearField.GetAxis(0).GetNumberOfBins();
  unsigned long yBins = m_EfficiencyNearField.GetAxis(1).GetNumberOfBins();
  unsigned long zBins = m_EfficiencyNearField.GetAxis(2).GetNumberOfBins();
  unsigned long EBins = m_EfficiencyNearField.GetAxis(3).GetNumberOfBins();
  unsigned long x2Bins = m_EfficiencyNearField.GetAxis(4).GetNumberOfBins();
  unsigned long y2Bins = m_EfficiencyNearField.GetAxis(5).GetNumberOfBins();
  unsigned long z2Bins = m_EfficiencyNearField.GetAxis(6).GetNumberOfBins();
  unsigned long E2Bins = m_EfficiencyNearField.GetAxis(7).GetNumberOfBins();
  
  unsigned long M1 = 1;
  unsigned long M2 = M1*xBins;
  unsigned long M3 = M2*yBins;
  unsigned long M4 = M3*zBins;
  unsigned long M5 = M4*EBins;
  unsigned long M6 = M5*x2Bins;
  unsigned long M7 = M6*y2Bins;
  unsigned long M8 = M7*z2Bins;
  
  for (unsigned int x = 0; x < xBins; ++x) {
    cout<<"x:"<<x<<endl;
    unsigned long B1 = M1*x;
    for (unsigned int y = 0; y < yBins; ++y) {
      //cout<<"y:"<<y<<endl;
      unsigned long B2 = B1 + M2*y;
      for (unsigned int z = 0; z < zBins; ++z) {
        //cout<<"z:"<<z<<endl;
        unsigned long B3 = B2 + M3*z;
        
        double Area =
          (xAxisEdges[x+1] - xAxisEdges[x]) *
          (yAxisEdges[y+1] - yAxisEdges[y]) *
          (zAxisEdges[z+1] - zAxisEdges[z]);

        // (2) Calculate the effective area
        long StartedPhotons = m_EfficiencySimulatedEvents * Area / FullArea;
 
        double Normalizer = 1.0 / StartedPhotons;
        
        for (unsigned int e = 0; e < EBins; ++e) {
          unsigned long B4 = B3 + M4*e;
          for (unsigned int x2 = 0; x2 < x2Bins; ++x2) {
            unsigned long B5 = B4 + M5*x2;
            for (unsigned int y2 = 0; y2 < y2Bins; ++y2) {
              unsigned long B6 = B5 + M6*y2;
              for (unsigned int z2 = 0; z2 < z2Bins; ++z2) {
                unsigned long B7 = B6 + M7*z2;
                for (unsigned int e2 = 0; e2 < E2Bins; ++e2) {
                  unsigned long B8 = B7 + M8*e2;
                  m_EfficiencyNearField.Multiply(B8, Normalizer);
                }
              }
            }
          }
        }
      }
    }
  }

  
  
  //m_EfficiencyNearField.Show(MResponseMatrix::c_ShowX, 0, MResponseMatrix::c_ShowY, 511);
  //m_EfficiencyNearField.Show(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 10, 511);

  m_IsLoaded = true;
  m_IsFarField = false;

  mout<<"Normalizig the efficiency file .. DONE... after "<<Timer.GetElapsed()<<" sec"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the efficiency - theta and phi are in detector coordinates
double MEfficiency::Get(double Theta, double Phi, bool Interpolate)
{
  if (m_IsLoaded == false) return 1;
  if (m_IsFarField == false) return 1;

  // Rotate vector into detector coordinates
  MVector D;
  D.SetMagThetaPhi(1.0, Theta, Phi);

  // Rotate D into efficiency coordiante system: 90 degree rotation around the y-axis.
  D = m_EfficiencyRotation*D;

  // Get the efficiency value
  double EfficiencyValue = 1;
  if (Interpolate == true) {
    EfficiencyValue = m_Efficiency.GetInterpolated(D.PhiApproximateMaths()*c_Deg, D.ThetaApproximateMaths()*c_Deg);
  } else {
    EfficiencyValue = m_Efficiency.Get(D.PhiApproximateMaths()*c_Deg, D.ThetaApproximateMaths()*c_Deg);
  }

  if (std::isnan(EfficiencyValue)) {
    cout<<"NaN!"<<endl;
    EfficiencyValue = 1;
  }

  return EfficiencyValue;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the efficiency - theta and phi are in detector coordinates
double MEfficiency::GetNearField(double x, double y, double z, double x_fc, double y_fc, double z_fc, bool Interpolate)
{
  if (m_IsLoaded == false) return 1;
  if (m_IsFarField == true) return 1;


  // Get the efficiency value
  double EfficiencyValue = 1;
  if (Interpolate == true) {
    EfficiencyValue = m_EfficiencyNearField.GetInterpolated(vector<double>{ x, y, z, 511, x_fc, y_fc, z_fc, 511 });
  } else {
    EfficiencyValue = m_EfficiencyNearField.Get(vector<double>{ x, y, z, 511, x_fc, y_fc, z_fc, 511 });
  }

  if (std::isnan(EfficiencyValue)) {
    cout<<"NaN!"<<endl;
    EfficiencyValue = 1;
  }

  return EfficiencyValue;
}


// MEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
