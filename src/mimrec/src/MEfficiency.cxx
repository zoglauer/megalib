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


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
  else if (EfficiencyFile.EndsWith(".efficiencynearfield.rsp") == true ||
      EfficiencyFile.EndsWith(".efficiencynearfield.rsp.gz") == true) {
    return LoadNearField(EfficiencyFile);
  }

  cout<<"Error: File is not of type .efficiency. or of type .efficiencyfarfield.: \""<<EfficiencyFile<<"\""<<endl;
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
  m_IsLoaded = false;

  m_EfficiencyNearField.Read(EfficiencyFile);
  // m_EfficiencyNearField.Smooth(10);

  m_EfficiencyStartArea = m_EfficiencyNearField.GetFarFieldStartArea();
  m_EfficiencySimulatedEvents = m_EfficiencyNearField.GetSimulatedEvents();

  // Normalize

  // The full area:
  double FullArea =
    (m_EfficiencyNearField.GetAxisLowEdge(0, 1) - m_EfficiencyNearField.GetAxisHighEdge(m_EfficiencyNearField.GetAxisBins(1) - 1, 1)) *
    (m_EfficiencyNearField.GetAxisLowEdge(0, 2) - m_EfficiencyNearField.GetAxisHighEdge(m_EfficiencyNearField.GetAxisBins(2) - 1, 2)) *
    (m_EfficiencyNearField.GetAxisLowEdge(0, 3) - m_EfficiencyNearField.GetAxisHighEdge(m_EfficiencyNearField.GetAxisBins(3) - 1, 3));

  for (unsigned int x = 0; x < m_EfficiencyNearField.GetAxisBins(1); ++x) {
    for (unsigned int y = 0; y < m_EfficiencyNearField.GetAxisBins(2); ++y) {
      for (unsigned int z = 0; z < m_EfficiencyNearField.GetAxisBins(3); ++z) {

        double Area =
          (m_EfficiencyNearField.GetAxisLowEdge(x, 1) - m_EfficiencyNearField.GetAxisHighEdge(x, 1)) *
          (m_EfficiencyNearField.GetAxisLowEdge(y, 2) - m_EfficiencyNearField.GetAxisHighEdge(y, 2)) *
          (m_EfficiencyNearField.GetAxisLowEdge(z, 3) - m_EfficiencyNearField.GetAxisHighEdge(z, 3));

        // (2) Calculate the effective area
        long StartedPhotons = m_EfficiencySimulatedEvents * Area / FullArea;

        m_EfficiencyNearField.SetBinContent(x, y, z, 0, m_EfficiencyNearField.GetBinContent(x, y, z, 0) / StartedPhotons);
      }
    }
  }
  //m_EfficiencyNearField.Show(MResponseMatrix::c_ShowX, 0, MResponseMatrix::c_ShowY, 511);
  //m_EfficiencyNearField.Show(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 10, 511);

  m_IsLoaded = true;
  m_IsFarField = false;

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
double MEfficiency::GetNearField(double x, double y, double z, bool Interpolate)
{
  if (m_IsLoaded == false) return 1;
  if (m_IsFarField == true) return 1;


  // Get the efficiency value
  double EfficiencyValue = 1;
  if (Interpolate == true) {
    EfficiencyValue = m_EfficiencyNearField.GetInterpolated(x, y, z, 1000);
  } else {
    EfficiencyValue = m_EfficiencyNearField.Get(x, y, z, 1000);
  }

  if (std::isnan(EfficiencyValue)) {
    cout<<"NaN!"<<endl;
    EfficiencyValue = 1;
  }

  return EfficiencyValue;
}


// MEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
