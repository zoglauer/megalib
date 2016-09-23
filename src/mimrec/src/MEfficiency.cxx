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
MEfficiency::MEfficiency() : m_IsLoaded(false)
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
  
  if (EfficiencyFile.EndsWith(".efficiency.90y.rsp") == false &&
      EfficiencyFile.EndsWith(".efficiency.90y.rsp.gz") == false) {
    cout<<"Error: File is not of type .efficiency.90y.rsp: \""<<EfficiencyFile<<"\""<<endl;
    return false;
  }
      
  
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
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the data - theta and phi are in detector coordiantes
double MEfficiency::Get(double Theta, double Phi, bool Interpolate)
{
  if (m_IsLoaded == false) return 1;
  
  // Rotate vector into detector coordinates
  MVector D;
  D.SetMagThetaPhi(1.0, Theta, Phi);
          
  // Rotate D into efficiency coordiante system: 90 degree rotation around the y-axis.
  D = m_EfficiencyRotation*D;      
          
  // Get the efficiency value
  double EfficiencyValue = 1;
  if (Interpolate == true) {
    EfficiencyValue = m_Efficiency.GetInterpolated(D.PhiFastMath()*c_Deg, D.ThetaFastMath()*c_Deg);
  } else {
    EfficiencyValue = m_Efficiency.Get(D.PhiFastMath()*c_Deg, D.ThetaFastMath()*c_Deg);
  }
          
  if (std::isnan(EfficiencyValue)) {
    cout<<"NaN!"<<endl;
    EfficiencyValue = 1;
  }

  return EfficiencyValue;
}


// MEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
