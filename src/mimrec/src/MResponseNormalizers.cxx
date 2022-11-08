/*
 * MResponseNormalizers.cxx
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
// MResponseNormalizers
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseNormalizers.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseNormalizers)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseNormalizers::MResponseNormalizers() : m_UseNormalizers(false), m_IsLoaded(false)
{
}


////////////////////////////////////////////////////////////////////////////////


MResponseNormalizers::~MResponseNormalizers()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Set the viewport / image dimensions
bool MResponseNormalizers::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins,
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

  m_BinnedNearFieldDetectionEfficiency.clear();
  m_BinnedNearFieldDetectionEfficiency.resize(m_NImageBins);
  for (unsigned int i = 0; i < m_NImageBins; ++i) {
    m_BinnedNearFieldDetectionEfficiency[i] = 0.0;
  }

  vector<unsigned long> Bins(3);
  for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // z == radius
    for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // y == theta == lat
      for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // x == phi == long
        Bins[0] = m_x1BinCenter[x1];
        Bins[1] = m_x2BinCenter[x2];
        Bins[2] = m_x3BinCenter[x3];
        m_BinnedNearFieldDetectionEfficiency[x1 + x2*m_x1NBins + x3*m_x1NBins*m_x2NBins] = m_NearFieldDetectionEfficiency.Get(Bins);
      }
    }
  }

  /*
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
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseNormalizers::GetNearFieldDetectionEfficiency(unsigned int ImageBin)
{
  //! Return the near-field detection efficiency

  return m_BinnedNearFieldDetectionEfficiency[ImageBin];
}


////////////////////////////////////////////////////////////////////////////////


double MResponseNormalizers::GetNearFieldParameterEfficiency(const vector<double>& AxesValues)
{
  //! Return the near-field parameter efficiency
  
  return m_NearFieldDetectionEfficiency.Get(AxesValues);
}


////////////////////////////////////////////////////////////////////////////////


/*
//! Return a copy of the current ResponseNormalizers map
double* MResponseNormalizers::GetResponseNormalizers()
{
  ApplyResponseNormalizers();

  cout<<"ResponseNormalizers generated with "<<m_NResponseNormalizersUpdates<<" updates"<<endl;

  double* E = new double[m_NImageBins];
  copy(m_ResponseNormalizers, m_ResponseNormalizers+m_NImageBins, E);

  if (m_Mode == MResponseNormalizersMode::CalculateFromEfficiencyFarFieldMoving) {
    // Normalize test wise to 1
    double Sum = 0;
    for (unsigned int i = 0; i < m_NImageBins; ++i) Sum += E[i];
    if (Sum > 0) {
      for (unsigned int i = 0; i < m_NImageBins; ++i) E[i] /= Sum;
    }
  }

  return E;
}
*/


////////////////////////////////////////////////////////////////////////////////


//! Load the near-field efficiency data files
bool MResponseNormalizers::LoadNearFieldEfficiencies(MString NearFieldDetectionEfficiency, MString NearFieldParameterEfficiency)
{
  m_IsLoaded = false;

  if (m_NearFieldDetectionEfficiency.Read(NearFieldDetectionEfficiency) == false) {
    return false;
  }

  if (m_NearFieldParameterEfficiency.Read(NearFieldParameterEfficiency) == false) {
    return false;
  }

  long SimulatedEvents = m_NearFieldDetectionEfficiency.GetSimulatedEvents();

  // Normalize

  // The full area:
  double FullArea =
    (m_NearFieldDetectionEfficiency.GetAxis(0).GetMinima()[0] - m_NearFieldDetectionEfficiency.GetAxis(0).GetMaxima()[0]) *
    (m_NearFieldDetectionEfficiency.GetAxis(1).GetMinima()[0] - m_NearFieldDetectionEfficiency.GetAxis(1).GetMaxima()[0]) *
    (m_NearFieldDetectionEfficiency.GetAxis(2).GetMinima()[0] - m_NearFieldDetectionEfficiency.GetAxis(2).GetMaxima()[0]);

    
  vector<double> xAxisEdges = m_NearFieldDetectionEfficiency.GetAxis(0).Get1DBinEdges();
  vector<double> yAxisEdges = m_NearFieldDetectionEfficiency.GetAxis(1).Get1DBinEdges();
  vector<double> zAxisEdges = m_NearFieldDetectionEfficiency.GetAxis(2).Get1DBinEdges();
  vector<double> EAxisEdges = m_NearFieldDetectionEfficiency.GetAxis(3).Get1DBinEdges();

    
  for (unsigned int x = 0; x < xAxisEdges.size()-1; ++x) {
    for (unsigned int y = 0; y < yAxisEdges.size()-1; ++y) {
      for (unsigned int z = 0; z < zAxisEdges.size()-1; ++z) {

        double Area = (xAxisEdges[x+1] - xAxisEdges[x]) * (yAxisEdges[y+1] - yAxisEdges[y]) * (zAxisEdges[z+1] - zAxisEdges[z]);

        long StartedPhotons = SimulatedEvents * Area / FullArea;
  
        for (unsigned int e = 0; e < EAxisEdges.size()-1; ++e) {
          m_NearFieldDetectionEfficiency.Set(vector<unsigned long>{x, y, z, e}, m_NearFieldDetectionEfficiency.Get(vector<unsigned long>{x, y, z, e}) / StartedPhotons);
        }
      }
    }
  }
  
  m_NearFieldParameterEfficiency /= SimulatedEvents;
  
  
  // m_NearFieldDetectionEfficiency.Show(MResponseMatrix::c_ShowX, 0, MResponseMatrix::c_ShowY, 511);
  // m_NearFieldDetectionEfficiency.Show(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 10, 511);

  m_IsLoaded = true;
  m_UseNormalizers = true;

  return true;
}


// MResponseNormalizers.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
