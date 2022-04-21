/*
 * MExposure.cxx
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
// MExposure
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MExposure.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MExposure)
#endif


////////////////////////////////////////////////////////////////////////////////


MExposure::MExposure() : m_Mode(MExposureMode::Flat), m_LastTime(0),  m_NExposureUpdates(0)
{
  m_Exposure = new double[1];
  m_Efficiency = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MExposure::~MExposure()
{
  delete m_Efficiency;
  delete [] m_Exposure;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the viewport / image dimensions
bool MExposure::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins,
                              double x2Min, double x2Max, unsigned int x2NBins,
                              double x3Min, double x3Max, unsigned int x3NBins,
                              MVector x1Axis, MVector x3Axis)
{
  if (MViewPort::SetDimensions(x1Min, x1Max, x1NBins,
                               x2Min, x2Max, x2NBins,
                               x3Min, x3Max, x3NBins,
                               x1Axis, x3Axis) == false) {
    return false;
  }

  delete [] m_Exposure;
  m_Exposure = new double[m_NImageBins];
  for (unsigned int i = 0; i < m_NImageBins; ++i) {
    m_Exposure[i] = 0.0;
  }

  m_BinCenterVectors.resize(m_NImageBins);
  m_BinCenterVectorsNearField.resize(m_NImageBins);
  for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // z == radius
    for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // y == theta == lat
      for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // x == phi == long
        m_BinCenterVectors[x1 + x2*m_x1NBins + x3*m_x1NBins*m_x2NBins].SetMagThetaPhi(m_x3BinCenter[x3], m_x2BinCenter[x2], m_x1BinCenter[x1]);
        m_BinCenterVectorsNearField[x1 + x2*m_x1NBins + x3*m_x1NBins*m_x2NBins].SetXYZ(m_x1BinCenter[x1], m_x2BinCenter[x2], m_x3BinCenter[x3]);
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a copy of the current exposure map
double* MExposure::GetExposure()
{
  ApplyExposure();

  cout<<"Exposure generated with "<<m_NExposureUpdates<<" updates"<<endl;

  double* E = new double[m_NImageBins];
  copy(m_Exposure, m_Exposure+m_NImageBins, E);

  if (m_Mode == MExposureMode::CalculateFromEfficiencyFarFieldMoving) {
    // Normalize test wise to 1
    double Sum = 0;
    for (unsigned int i = 0; i < m_NImageBins; ++i) Sum += E[i];
    if (Sum > 0) {
      for (unsigned int i = 0; i < m_NImageBins; ++i) E[i] /= Sum;
    }
  }

  return E;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MExposure::Load(MString ExposureFile)
{
  if (ExposureFile.EndsWith(".efficiency.90y.rsp") == true ||
      ExposureFile.EndsWith(".efficiency.90y.rsp.gz") == true) {
    m_Mode = MExposureMode::CalculateFromEfficiencyFarFieldMoving;
    return LoadFarField(ExposureFile);
  }
  else if (ExposureFile.EndsWith(".emittedxdetectedanywhere.rsp") == true ||
      ExposureFile.EndsWith(".emittedxdetectedanywhere.rsp.gz") == true) {
    m_Mode = MExposureMode::CalculateFromEfficiencyNearFieldStatic;
    return LoadNearField(ExposureFile);
  }

  cout<<"Error: File is not of type .efficiency. or of type .emittedxdetectedanywhere.: \""<<ExposureFile<<"\""<<endl;
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MExposure::LoadFarField(MString ExposureFile)
{
  m_IsLoaded = false;
  m_IsFarField = true;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the efficiency file and switch to that mode
bool MExposure::LoadNearField(MString ExposureFile)
{
  m_IsLoaded = false;

  m_ExposureNearField.Read(ExposureFile);

  //double StartArea = m_ExposureNearField.GetFarFieldStartArea();
  long SimulatedEvents = m_ExposureNearField.GetSimulatedEvents();

  // Normalize

  // The full area:
  double FullArea =
    (m_ExposureNearField.GetAxis(0).GetMinima()[0] - m_ExposureNearField.GetAxis(0).GetMaxima()[0]) *
    (m_ExposureNearField.GetAxis(1).GetMinima()[0] - m_ExposureNearField.GetAxis(1).GetMaxima()[0]) *
    (m_ExposureNearField.GetAxis(2).GetMinima()[0] - m_ExposureNearField.GetAxis(2).GetMaxima()[0]);

    
  vector<double> xAxisEdges = m_ExposureNearField.GetAxis(0).Get1DBinEdges();
  vector<double> yAxisEdges = m_ExposureNearField.GetAxis(1).Get1DBinEdges();
  vector<double> zAxisEdges = m_ExposureNearField.GetAxis(2).Get1DBinEdges();
  vector<double> EAxisEdges = m_ExposureNearField.GetAxis(3).Get1DBinEdges();

    
  for (unsigned int x = 0; x < xAxisEdges.size()-1; ++x) {
    for (unsigned int y = 0; y < yAxisEdges.size()-1; ++y) {
      for (unsigned int z = 0; z < zAxisEdges.size()-1; ++z) {

        double Area = (xAxisEdges[x+1] - xAxisEdges[x]) * (yAxisEdges[y+1] - yAxisEdges[y]) * (zAxisEdges[z+1] - zAxisEdges[z]);

        // (2) Calculate the effective area
        long StartedPhotons = SimulatedEvents * Area / FullArea;
  
        
        for (unsigned int e = 0; e < EAxisEdges.size()-1; ++e) {
          m_ExposureNearField.Set(vector<unsigned long>{x, y, z, e}, m_ExposureNearField.Get(vector<unsigned long>{x, y, z, e}) / StartedPhotons);
        }
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


//! Create the exposure for one event
bool MExposure::Expose(MPhysicalEvent* Event)
{
  if (m_Mode == MExposureMode::Flat) return true;
  if (m_Mode == MExposureMode::CalculateFromEfficiencyNearFieldStatic) return true;

  if (m_LastTime == MTime(0)) {
    m_LastTime = Event->GetTime();
    m_LastRotation = Event->GetGalacticPointingRotationMatrix();

    m_CurrentTime = Event->GetTime();
    m_CurrentRotation = Event->GetGalacticPointingRotationMatrix();

    return true;
  }

  MTime CurrentTime = Event->GetTime();
  MRotation CurrentRotation = Event->GetGalacticPointingRotationMatrix();

  double Metric = DistanceMetric(m_LastRotation, CurrentRotation);

  // First handle time jumps
  if ((CurrentTime - m_CurrentTime).GetAsSeconds() > 60) {
    cout<<"Exposure: Time jump: "<<m_CurrentTime<<" --> "<<CurrentTime<<endl;

    // Update the m_Last... & m_Current afterwards since we want to perform the last
    // not yet done exposure update
    ApplyExposure();

    // Reset:
    m_LastTime = CurrentTime;
    m_LastRotation = CurrentRotation;
    m_CurrentTime = CurrentTime;
    m_CurrentRotation = CurrentRotation;
    return true;
  }

  // Otherwise, only update if the metric is above a certain value:
  m_CurrentTime = CurrentTime;
  m_CurrentRotation = CurrentRotation;
  if (Metric > 1.0*c_Rad) {
    ApplyExposure();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Apply the exposure
bool MExposure::ApplyExposure()
{
  if (m_Mode == MExposureMode::CalculateFromEfficiencyFarFieldMoving) {

    double TimeDiff = (m_CurrentTime - m_LastTime).GetAsSeconds();

    if (TimeDiff > 0.0) {
      cout<<"Applying exposure with dt="<<TimeDiff<<"(last time: "<<m_LastTime<<")"<<endl;
      ++m_NExposureUpdates;

      MRotation Inv = m_CurrentRotation.GetInvers();

      unsigned int index;
      for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // z == radius
        for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // y == theta == lat
          for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // x == phi == long
            index = x1 + x2*m_x1NBins + x3*m_x1NBins*m_x2NBins;

            // Rotate the bin center vectors into detector coordinates
            MVector D = Inv*m_BinCenterVectors[index];

            // Get the efficiency value
            double EfficiencyValue = m_Efficiency->Get(D.ThetaApproximateMaths(), D.PhiApproximateMaths());

            if (std::isnan(EfficiencyValue)) {
              cout<<"NaN!"<<endl;
              continue;
            }

            // Add it to the efficieny
            m_Exposure[index] += EfficiencyValue*TimeDiff;
          }
        }
      }

      m_LastTime = m_CurrentTime;
      m_LastRotation = m_CurrentRotation;
    }
  } else if (m_Mode == MExposureMode::CalculateFromEfficiencyNearFieldStatic) {
    //MRotation Inv = m_CurrentRotation.GetInvers();

    unsigned int index;
    for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // z == radius
      for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // y == theta == lat
        for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // x == phi == long
          index = x1 + x2*m_x1NBins + x3*m_x1NBins*m_x2NBins;

          // Rotate the bin center vectors into detector coordinates
          cout<<"Index: "<<index<<":"<<m_NImageBins<<endl;
          MVector D = /*Inv*/m_BinCenterVectorsNearField[index];

          // Get the efficiency value
          double EfficiencyValue = m_ExposureNearField.Get(vector<double>{D.X(), D.Y(), D.Z(), 511});

          if (std::isnan(EfficiencyValue)) {
            cout<<"NaN!"<<endl;
            continue;
          }

          // Add it to the efficieny
          m_Exposure[index] = EfficiencyValue;
        }
      }
    }
  } else {
    if (m_Exposure[0] != 1) {
      for (unsigned int i = 0; i < m_NImageBins; ++i) {
        m_Exposure[i] = 1.0;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! A distance metric to determine when we have to recalculate
//! The value is just the largest difference between the axis vectors
double MExposure::DistanceMetric(const MRotation& A, const MRotation& B)
{
  double Angle = A.GetX().Angle(B.GetX());

  double Angle2 = A.GetY().Angle(B.GetY());
  if (Angle2 > Angle) Angle = Angle2;

  Angle2 = A.GetZ().Angle(B.GetZ());
  if (Angle2 > Angle) Angle = Angle2;

  return Angle;
}


// MExposure.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
