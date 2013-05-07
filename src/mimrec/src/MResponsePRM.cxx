/*
 * MResponsePRM.cxx
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
// MResponsePRM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponsePRM.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponsePRM)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponsePRM::MResponsePRM()
{
  // default constructor
  
  m_Threshold = 0.01;
}


////////////////////////////////////////////////////////////////////////////////


MResponsePRM::~MResponsePRM()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MResponsePRM::LoadResponseFiles(const MString& TransFileName, 
                                     const MString& LongFileName, 
                                     const MString& PairFileName)
{
  if (MFile::FileExists(TransFileName) == false) {
    mgui<<"File \""<<TransFileName<<"\" does not exist!"<<endl;
    return false;
  }
  if (MFile::FileExists(LongFileName) == false) {
    mgui<<"File \""<<LongFileName<<"\" does not exist!"<<endl;
    return false;
  }
//   if (MFile::FileExists(PairFileName) == false) {
//     mgui<<"File \""<<TransFileName<<"\" does not exist!"<endl;
//     return false;
//   }

  if (m_ComptonTrans.Read(TransFileName) == false) {
    mgui<<"Cannot read response file: \""<<TransFileName<<"\""<<endl;
    return false;
  } 
  if (m_ComptonLong.Read(LongFileName) == false) {
    mgui<<"Cannot read response file: \""<<TransFileName<<"\""<<endl;
    return false;
  } 
//   if (m_PairRadial.Read(PairFileName) == false) {
//     mgui<<"Cannot read response file: \""<<PairFileName<<"\""<<endl;
//     return false;
//   } 

  // Normalize the response files:
  // Trans: Each dphi = 1
  // Normalize the response matrix
  int e_max = m_ComptonTrans.GetAxisBins(3);
  int phi_max = m_ComptonTrans.GetAxisBins(2);
  int dphi_max = m_ComptonTrans.GetAxisBins(1);
  for (int e = 0; e < e_max; ++e) {  
    for (int phi = 0; phi < phi_max; ++phi) {  
      // Normalize by area: 
      for (int dphi = 0; dphi < dphi_max; ++dphi) {
        m_ComptonTrans.SetBinContent(dphi, phi, e, m_ComptonTrans.GetBinContent(dphi, phi, e)/
                                     m_ComptonTrans.GetBinArea(dphi, phi, e));
      }
      double Sum = 0.0;
      for (int dphi = 0; dphi < dphi_max; ++dphi) {
        Sum += m_ComptonTrans.GetBinContent(dphi, phi, e);
      }
      // Normalize to sum=1
      if (Sum != 0) {
        for (int dphi = 0; dphi < dphi_max; ++dphi) {
          m_ComptonTrans.SetBinContent(dphi, phi, e, m_ComptonTrans.GetBinContent(dphi, phi, e)/Sum);
        }
      }
    }
  }

  // Long: Each SPD = 1
  int energy_max = m_ComptonLong.GetAxisBins(3);
  int epsilon_max = m_ComptonLong.GetAxisBins(2);
  int spd_max = m_ComptonLong.GetAxisBins(1);
  for (int energy = 0; energy < energy_max; ++energy) {  
    for (int epsilon = 0; epsilon < epsilon_max; ++epsilon) {  
      // Normalize by area: 
      for (int spd = 0; spd < spd_max; ++spd) {
        m_ComptonLong.SetBinContent(spd, epsilon, energy, m_ComptonLong.GetBinContent(spd, epsilon, energy)/
                                    m_ComptonLong.GetBinArea(spd, epsilon, energy));
      }
      double Sum = 0.0;
      for (int spd = 0; spd < spd_max; ++spd) {
        Sum += m_ComptonLong.GetBinContent(spd, epsilon, energy);
      }
      // Normalize to sum=1
      if (Sum != 0) {
        for (int spd = 0; spd < spd_max; ++spd) {
          m_ComptonLong.SetBinContent(spd, epsilon, energy, m_ComptonLong.GetBinContent(spd, epsilon, energy)/Sum);
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponsePRM::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Event);
    m_Ei = Compton->Ei();
    m_Ee = Compton->Ee();
    m_Phi = Compton->Phi()*c_Deg;
    m_Epsilon = Compton->Epsilon()*c_Deg;

    m_SliceTrans = m_ComptonTrans.GetSliceInterpolated(m_Phi, 2, m_Ei, 3);
    m_SliceTrans.Smooth(10);
    
    /*
    // Testing: smooth curve!
    double Smooth = 2;
    for (unsigned int bx = 0; bx < m_SliceTrans.GetAxisBins(); ++bx) {
      m_SliceTrans.SetBinContent(bx, 1.0/(sqrt(c_Pi)*Smooth)*exp(-0.5*pow(m_SliceTrans.GetAxisContent(bx)/Smooth, 2)));
    }
    */

    if (Compton->HasTrack() == true) {
      m_SliceLong = m_ComptonLong.GetSliceInterpolated(1, 2, m_Ee, 3);
      m_SliceLong.Smooth(100);
      /*
      // Testing: smooth curve!
      double Smooth = 1000000;
      for (unsigned int bx = 0; bx < m_SliceLong.GetAxisBins(); ++bx) {
        m_SliceLong.SetBinContent(bx, 1.0/(sqrt(c_Pi)*Smooth)*exp(-0.5*pow(m_SliceLong.GetAxisContent(bx)/Smooth, 2)));
      }
      */
    }

    m_ComptonIntegral = 0.0;
    if (Compton->HasTrack() == true) {
      // Looking at the maximum in long direction: 
      // How much is really filled compared to the mode where we have no electron tracking:
      double LongRatio = 0.0;

      int t_max = 1800;
      double Theta = 0.0;
      double dTheta = 180.0/t_max;
      for (int t = 0; t < t_max; ++t) {
        LongRatio += m_SliceLong.GetInterpolated(Theta)*dTheta;
        Theta += dTheta;
      }
      LongRatio = LongRatio/m_SliceLong.GetMaximum()/(m_SliceLong.GetAxisMaximum() - m_SliceLong.GetAxisMinimum());

      // Do a simple integration:
      int p_max = 1800;
      double Phi = 0.0;
      double dPhi = 180.0/p_max;
      for (int p = 0; p < p_max-1; ++p) {
        m_ComptonIntegral += m_SliceTrans.GetInterpolated(Phi-m_Phi)*LongRatio*m_SliceLong.GetMaximum()*
          fabs(2 * c_Pi * (cos((Phi+dPhi)*c_Rad) - cos(Phi*c_Rad)));
        Phi += dPhi;
      }
    } else {
      int p_max = 1800;
      double Phi = 0.0;
      double dPhi = 180.0/p_max;
      for (int p = 0; p < p_max-1; ++p) {
        m_ComptonIntegral += m_SliceTrans.GetInterpolated(Phi-m_Phi)*
          fabs(2 * c_Pi * (cos((Phi+dPhi)*c_Rad) - cos(Phi*c_Rad)));
        Phi += dPhi;
      }
    }

  } else {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonResponse(const double t)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t
  //
  // t: transversal distance (in rad) from the cone surface

  return m_SliceTrans.GetInterpolated(t*c_Deg); ///m_ComptonIntegral;
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonResponse(const double t, const double l)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t and l:
  //
  // t: transversal distance (in rad) from the cone surface
  // l: longitudinal distance (in rad) on the cone surface of the 
  //    calculated origin of the gamma-ray 

  return m_SliceTrans.GetInterpolated(t*c_Deg) * m_SliceLong.GetInterpolated(l*c_Deg);
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonMaximum()
{
  // This class has to be overwritten
  //
  // Return the maximum-possible response-value 
                
  return m_SliceLong.GetMaximum()*m_SliceTrans.GetMaximum();
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonTransversalMin()
{
  // Return the maximum transversal angle, at which the response is above the
  // threshold

  int p_max = 1800;
  double Phi = 0.0;
  double dPhi = 180.0/p_max;
  for (int p = 0; p < p_max-1; ++p) {
    if (m_SliceTrans.GetInterpolated(Phi-m_Phi)/m_SliceLong.GetMaximum() > m_Threshold) {
      return Phi - m_Phi; 
    }
    Phi += dPhi;
  }          

  merr<<"Hmmm, ... no GetComptonTransversalMin() found"<<endl;
  
  return -c_Pi;
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonTransversalMax()
{
  // This class has to be overwritten
  //
  // Return the minimum transversal angle, at which the response is above the
  // threshold
          
  int p_max = 1800;
  double Phi = 180.0;
  double dPhi = 180.0/p_max;
  for (int p = p_max-1; p >= 0; --p) {
    if (m_SliceTrans.GetInterpolated(Phi-m_Phi)/m_SliceLong.GetMaximum() > m_Threshold) {
      return (Phi - m_Phi)*c_Rad; 
    }
    Phi -= dPhi;
  }          

  merr<<"Hmmm, ... no GetComptonTransversalMax() found"<<endl;
  
  return c_Pi;
}


////////////////////////////////////////////////////////////////////////////////


double MResponsePRM::GetComptonIntegral(const double Radius) const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  return m_ComptonIntegral;
}


// MResponsePRM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
