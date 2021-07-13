/*
 * MResponseEnergyLeakage.cxx
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
// MResponseEnergyLeakage
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEnergyLeakage.h"

// Standard libs:
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <limits>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MComptonEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseEnergyLeakage)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseEnergyLeakage::MResponseEnergyLeakage(const double Electron, 
                                               const double Gamma)
{
  // default constructor

  SetGaussians(Electron, Gamma);
}


////////////////////////////////////////////////////////////////////////////////


MResponseEnergyLeakage::~MResponseEnergyLeakage()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////


void MResponseEnergyLeakage::SetGaussians(const double Electron, 
                                          const double Gamma)
{
  m_FitElectron = Electron * c_Rad;
  m_FitGamma    = Gamma * c_Rad;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonResponse(const double tau)
{
  merr<<"This response is only valid for tracked events..."<<endl;

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonResponse(const double tau, 
                                                  const double lambda)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters tau and lambda:
  //
  // l: Distance on a great circle

  double f, v = 0;

  //mimp<<"Added this in the hope it is correct..."<<endl;
  //tau += m_Compton->Phi();

  //double l = 0;
  //if (tau == 0) {
  //  l = 0.0;
  //} else {
  //  l = asin(sin(lambda)/sin(tau));
  //}
  // double t = 2 * atan( tan(0.5*(tau - l))*sin(0.5*(0.5*c_Pi+lambda))/sin(0.5*(0.5*c_Pi-lambda)));

  m_tmin = 0;
  m_tmax = m_Theta;

  //if (!(t >= -100000 && t < 10000)) t = 10000;

  //cout<<t<<"!"<<m_tmin<<"!"<<m_tmax<<endl;


  // The sequence of the following cases is important!
  if (tau > m_tmax) {
    f = m_FitGamma + m_tmax/m_Theta*(m_FitElectron-m_FitGamma);
    v = 1.0/(f*sqrt(2*c_Pi))*exp(-0.5/(f*f)*lambda*lambda)*exp(-0.5/(f*f)*(tau-m_tmax)*(tau-m_tmax));
  } else if (tau < m_tmin) {
    f = m_FitGamma + m_tmin/m_Theta*(m_FitElectron-m_FitGamma);
    v = 1.0/(f*sqrt(2*c_Pi))*exp(-0.5/(f*f)*lambda*lambda)*exp(-0.5/(f*f)*(m_tmin-tau)*(m_tmin-tau));    
  } else {
    // For all other cases:
    f = m_FitGamma + tau/m_Theta*(m_FitElectron-m_FitGamma);
    v = 1.0/(f*sqrt(2*c_Pi))*exp(-0.5/(f*f)*lambda*lambda);
  }


  // Very simple version for testing...
  //if (tau < m_Theta && lambda < m_FitElectron) {
  //  v = 1;
  //} else {
  //  v = 0;
  //}

  return v;
}



////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetKleinNishina(const double Phi) const
{
  // Compute value of Klein-Nishina-Equation

  mimp<<"Replace this one with the original in MComptonEvent!"<<endl;

  if (Phi == 0) {
    //cout<<"Case: phi = 0"<<endl;
    return 0;
  }
  double Result = 0;
  double Re = 2.8E-15; // m
  double g;

  // Step 1: calculate g
  double Ee, Eg;
  double E0 = 511.004;
  double cosphi = cos(Phi);
  double costheta = cos(m_Theta);
  
  Ee = -(-cosphi+2*costheta*costheta*cosphi+2*sqrt(costheta*costheta*(costheta*costheta-1)*(-1+cosphi*cosphi))+1)*E0
    /(costheta*costheta*cosphi-cosphi+sqrt(costheta*costheta*(costheta*costheta-1)*(-1+cosphi*cosphi)));
      
  
  Eg = (-costheta*costheta+1+costheta*costheta*cosphi-cosphi+sqrt((-1+cosphi*cosphi)*costheta*costheta*(costheta*costheta-1)))*E0
    /(costheta*costheta*cosphi-costheta*costheta-cosphi+1);
    
  g = (Ee+Eg)/E0;


  double inv = 1.0/(1+g*(1-cosphi));
 
  Result = 0.5*Re*Re*(inv*inv)*(1+cosphi*cosphi+(g*g*(1-cosphi)*(1-cosphi)*inv))*sin(Phi);
  
  //cout<<"Result: "<<Result<<endl;
  
  if (fabs(Result) < DBL_MAX) {
    return 1; //Result*1E30;
  } else {
    return 1;
  }
  /*
  if (Ee <= 0 || Eg <= 0) {
    return 0; 
  }else {
    cout<<"Result:"<<Result<<" Phi:"<<Phi*grad<<" Eg: "<<Eg<<"  Ee:"<<Ee<<endl;
    return 1; //Result;
  }
  */

  return 1; //Result;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonMaximum()
{
  // Return the maximum-possible response-value 

  return GetComptonResponse(0, 0);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonTransversalMax()
{
  // Return the maximum transversal angle, at which the response is above the
  // threshold

  return c_Pi;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonTransversalMin()
{
  // Return the minimum transversal angle, at which the response is above the
  // threshold

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseEnergyLeakage::GetComptonIntegral(const double Radius) const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  
  return 1;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEnergyLeakage::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  // Only the Compton angle is of interest:

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    if (((MComptonEvent*) Event)->HasTrack() == false) return false;
    m_Compton = (MComptonEvent*) Event;
    
    //cout<<m_Compton->ToString()<<endl;

    m_Theta = m_Compton->CalculateThetaViaAngles();
    //cout<<"Theta: "<<m_Theta<<endl;
    double E0 = 511.004;

    double LowerZero = (E0/(1+cos(m_Theta)) > E0/(1-cos(m_Theta))) ? E0/(1-cos(m_Theta)) : E0/(1+cos(m_Theta));
    double UpperZero = (E0/(1+cos(m_Theta)) < E0/(1-cos(m_Theta))) ? E0/(1-cos(m_Theta)) : E0/(1+cos(m_Theta));

    double EeLow = m_Compton->Ee();
    double EgLow = m_Compton->Eg();

    double EeHigh = 9E+99;
    double EgHigh = 9E+99;

    //cout<<EeLow<<"!"<<EeHigh<<"!"<<EgLow<<"!"<<EgHigh<<endl;

    // if EgLow is lower than the lowest zero point we can't do any restriction 
    if (EgLow > LowerZero) {
      // We can not be lower than Upper zero:
      if (EgLow < UpperZero) {
        EgLow = UpperZero;
      }

      // otherwise we have a new upper limit on Ee:
      if (EeHigh > MComptonEvent::ComputeEeViaThetaEg(m_Theta, EgLow)) {
        //cout<<"???"<<EeHigh<<"!"<<MComptonEvent::ComputeEe(m_Theta, EgLow)<<"!"<<m_Theta<<endl;
        EeHigh = MComptonEvent::ComputeEeViaThetaEg(m_Theta, EgLow);
      }

      //cout<<EeLow<<"!"<<EeHigh<<"!"<<EgLow<<"!"<<EgHigh<<endl;

      // Check if Ee is above its allowed minimum:
      if (EeLow < 2*E0/tan(m_Theta)/tan(m_Theta)) {
        EeLow = 2*E0/tan(m_Theta)/tan(m_Theta);
      }

      //cout<<"Eg"<<endl;
      EgHigh = MComptonEvent::ComputeEgViaThetaEe(m_Theta, EeLow);
      //cout<<"Eg"<<endl;

      //cout<<EeLow<<"!"<<EeHigh<<"!"<<EgLow<<"!"<<EgHigh<<endl;


      m_tmin = MComptonEvent::ComputePhiViaEeEg(EeLow, EgHigh);
      m_tmax = MComptonEvent::ComputePhiViaEeEg(EeHigh, EgLow);

    } else {
      // Fall back to minimum restrictions:
      m_tmin = 0;
      m_tmax = m_Theta;
    }

    //m_tmin = 0;
    //m_tmax = m_Theta;

    cout<<"T: "<<m_Theta<<"!"<<m_tmin<<"!"<<m_tmax<<endl;

    //if (m_tmin < m_tmax) return false;

    // Due to large theta angles it is sometimes necessary to fall back
    // to the minimum requirement, which runs still through the correct
    // origin
    if (m_tmin > m_tmax) {
      m_tmin = 0;
      m_tmax = m_Theta;
    }

    return true;
  }

  cout<<"Rejected: Eg < lower zero point!"<<endl;

  return false;
}


// MResponseEnergyLeakage.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
