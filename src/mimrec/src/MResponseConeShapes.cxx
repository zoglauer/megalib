/*
 * MResponseConeShapes.cxx
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
// MResponseConeShapes
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseConeShapes.h"

// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseConeShapes)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseConeShapes::MResponseConeShapes()
{
  // default constructor
  
  m_Threshold = 0.01;
}


////////////////////////////////////////////////////////////////////////////////


MResponseConeShapes::~MResponseConeShapes()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseConeShapes::LoadResponseFile(const MString& ARMFileName)
{
  if (MFile::FileExists(ARMFileName) == false) {
    mgui<<"File \""<<ARMFileName<<"\" does not exist!"<<endl;
    return false;
  }

  if (m_ARMShape.Read(ARMFileName) == false) {
    mgui<<"Cannot read response file: \""<<ARMFileName<<"\""<<endl;
    return false;
  } 

  /*
  // Normalize the response files:
  // Trans: Each dphi = 1
  // Normalize the response matrix
  int ia_max = m_ARMShape.GetAxisBins(5);
  int d_max = m_ARMShape.GetAxisBins(4);
  int e_max = m_ARMShape.GetAxisBins(3);
  int phi_max = m_ARMShape.GetAxisBins(2);
  int dphi_max = m_ARMShape.GetAxisBins(1);
  for (int ia = 0; ia < ia_max; ++ia) {  
    for (int d = 0; d < d_max; ++d) {  
      for (int e = 0; e < e_max; ++e) {  
        for (int phi = 0; phi < phi_max; ++phi) {
          // Normalize by area:
          
          for (int dphi = 0; dphi < dphi_max; ++dphi) {
            m_ARMShape.SetBinContent(dphi, phi, e, d, ia,
                                     m_ARMShape.GetBinContent(dphi, phi, e, d, ia) / m_ARMShape.GetBinArea(dphi, phi, e, d, ia));
          }
          
          // Normalize to sum=1
          double Sum = 0.0;
          for (int dphi = 0; dphi < dphi_max; ++dphi) {
            Sum += m_ARMShape.GetBinContent(dphi, phi, e, d, ia);
          }
          if (Sum != 0) {
            for (int dphi = 0; dphi < dphi_max; ++dphi) {
              m_ARMShape.SetBinContent(dphi, phi, e, d, ia, m_ARMShape.GetBinContent(dphi, phi, e, d, ia)/Sum);
            }
          }
        }
      }
    }
  }
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseConeShapes::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  static int Passes = 0;
  //if (Passes++ < 10) return false;
  //if (Passes > 80) return false;

  if (Event->GetType() == c_ComptonEvent) {
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Event);
    m_Ei = Compton->Ei();
    m_Phi = Compton->Phi()*c_Deg;
    m_Distance = Compton->MinLeverArm();
    m_NumberOfInteractions = Compton->SequenceLength();

    m_Slice = m_ARMShape.GetSliceInterpolated(m_Phi, 2, m_Ei, 3, m_Distance, 4, m_NumberOfInteractions, 5);
    
    int dphi_max = m_Slice.GetAxisBins(1);    
    double Sum = 0.0;
    for (int dphi = 0; dphi < dphi_max; ++dphi) {
      //Sum += m_Slice.GetBinContent(dphi)/m_Slice.GetBinArea(dphi);
      Sum += m_Slice.GetBinContent(dphi);
    }
    /*
    if (Sum < 1000) {
      cout<<"Not enough bins - event is unlikely rejecting it"<<endl;
      return false;
    }
    */
    
    
    // Normalize by area
    for (int dphi = 0; dphi < dphi_max; ++dphi) {
      m_Slice.SetBinContent(dphi, m_Slice.GetBinContent(dphi)/m_Slice.GetBinArea(dphi));
    }

    m_Slice.Smooth(100);    
    
    // Normalize to 1:
    Sum = 0.0;
    for (int dphi = 0; dphi < dphi_max; ++dphi) {
      //Sum += m_Slice.GetBinContent(dphi)/m_Slice.GetBinArea(dphi);
      Sum += m_Slice.GetBinContent(dphi);
    }
    if (Sum != 0) {
      for (int dphi = 0; dphi < dphi_max; ++dphi) {
        //m_Slice.SetBinContent(dphi, m_Slice.GetBinContent(dphi)/m_Slice.GetBinArea(dphi)/Sum);
        m_Slice.SetBinContent(dphi, m_Slice.GetBinContent(dphi)/Sum);
      }
    }
    
    /*
    TH1D* Hist = dynamic_cast<TH1D*>(m_Slice.GetHistogram(false));
    ostringstream Title;
    Title<<"phi="<<m_Phi<<" energy="<<m_Ei<<" Dist="<<m_Distance<<" IA: "<<m_NumberOfInteractions<<" counts: "<<Sum<<endl;
    Hist->SetTitle(Title.str().c_str());
    
    TCanvas* C = new TCanvas();
    C->cd();
    Hist->Draw();
    C->Update();
    
    m_Slice.Smooth(100);  
    TH1D* Hist2 = dynamic_cast<TH1D*>(m_Slice.GetHistogram(false));
    Hist2->SetLineWidth(2);
    Hist2->SetLineColor(kRed);
    Hist2->Draw("SAME");
    C->Update();
    */
    
    m_ComptonIntegral = 0.0;

    int p_max = 1800;
    double Phi = 0.0;
    double dPhi = 180.0/p_max;
    for (int p = 0; p < p_max-1; ++p) {
      m_ComptonIntegral += m_Slice.GetInterpolated(Phi-m_Phi)*fabs(2 * c_Pi * (cos((Phi+dPhi)*c_Rad) - cos(Phi*c_Rad)));
      Phi += dPhi;
    }
    
    /*
    // Find the peak to average value:
    double Peak = 0.0;
    double Average = 0.0;
    for (int dphi = 0; dphi < dphi_max; ++dphi) {
      double Value = m_Slice.GetBinContent(dphi);
      if (Value > Peak) Peak = Value;
      Average += Value;
    }
    Average /= dphi_max;
    
    cout<<"Peak: "<<Peak<<" vs average: "<<Average<<" --> "<<Peak/Average<<endl;
    if (Peak/Average > 10) {
      cout<<"Response quality not good enough"<<endl;
      return false;
    }
    */

  } else {
    return false;
  }

  ++Passes;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseConeShapes::GetComptonResponse(const double t)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t
  //
  // t: transversal distance (in rad) from the cone surface
  
  const double Stretchfactor = 0.75;
  
  return m_Slice.GetInterpolated(Stretchfactor*t*c_Deg)/m_ComptonIntegral;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseConeShapes::GetComptonMaximum()
{
  // This class has to be overwritten
  //
  // Return the maximum-possible response-value 
                
  return m_Slice.GetMaximum();
}


////////////////////////////////////////////////////////////////////////////////


double MResponseConeShapes::GetComptonTransversalMin()
{
  // Return the maximum transversal angle, at which the response is above the
  // threshold

  int p_max = 1800;
  double Phi = 0.0;
  double dPhi = 180.0/p_max;
  for (int p = 0; p < p_max-1; ++p) {
    if (m_Slice.GetInterpolated(Phi-m_Phi)/m_Slice.GetMaximum() > m_Threshold) {
      return Phi - m_Phi; 
    }
    Phi += dPhi;
  }          

  merr<<"Hmmm, ... no GetComptonTransversalMin() found"<<endl;
  
  return -c_Pi;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseConeShapes::GetComptonTransversalMax()
{
  // This class has to be overwritten
  //
  // Return the minimum transversal angle, at which the response is above the
  // threshold
          
  int p_max = 1800;
  double Phi = 180.0;
  double dPhi = 180.0/p_max;
  for (int p = p_max-1; p >= 0; --p) {
    if (m_Slice.GetInterpolated(Phi-m_Phi)/m_Slice.GetMaximum() > m_Threshold) {
      return (Phi - m_Phi)*c_Rad; 
    }
    Phi -= dPhi;
  }          

  merr<<"Hmmm, ... no GetComptonTransversalMax() found"<<endl;
  
  return c_Pi;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseConeShapes::GetComptonIntegral(const double Radius) const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  return m_ComptonIntegral;
}


// MResponseConeShapes.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
