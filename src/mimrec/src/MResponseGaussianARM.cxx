/*
 * MResponseGaussian.cxx
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
// MResponseGaussian
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseGaussian.h"

// Standard libs:
#include <cmath>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFitFunctions.h"
#include "MComptonEvent.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseGaussian)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseGaussian::MResponseGaussian(const double ComptonTransversal, 
                                     const double ComptonLongitudinal, 
                                     const double PairDistance)
{
  // default constructor

  SetGaussians(ComptonTransversal, ComptonLongitudinal, PairDistance);
}


////////////////////////////////////////////////////////////////////////////////


MResponseGaussian::~MResponseGaussian()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////


void MResponseGaussian::SetGaussians(const double Transversal,
                                     const double Longitudinal, 
                                     const double Pair)
{
  m_LongitudinalFit = Longitudinal*c_Rad;
  m_TransversalFit = Transversal*c_Rad;
  m_PairFit = Pair*c_Rad;

  m_GaussSquareSigmaLong = -0.5/(m_LongitudinalFit*m_LongitudinalFit);
  m_GaussSquareSigmaTrans = -0.5/(m_TransversalFit*m_TransversalFit);
  m_GaussSquareSigmaPair = -0.5/(m_PairFit*m_PairFit);

  m_GaussFactorsLong = 1.0/(m_LongitudinalFit*sqrt(2*c_Pi));
  m_GaussFactorsTrans = 1.0/(m_TransversalFit*sqrt(2*c_Pi));
  m_GaussFactorsPair = 1.0/(m_PairFit*sqrt(2*c_Pi));
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonResponse(const double t)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t:
  //
  // t: transversal distance (in degree) from the cone surface

  // HACK:
  double Offset = 0.00000e+00;
  double Mean = 8.24296e-02;
  double LorentzWidth1 = 1.68504e+00;
  double LorentzHeight1 = 2.08654e+03;
  double LorentzWidth2 = 1.11093e+01;
  double LorentzHeight2 = -1.83469e+04;
  double GausHeight = 2.22252e+02;
  double GausSigma1 = 2.44192e+00; 
  double GausSigma2 = 2.34953e+00;

  double x[1];
  double par[9];

  x[0] = t*c_Deg;

  par[0] = Offset;
  par[1] = Mean;
  par[2] = LorentzWidth1;
  par[3] = LorentzHeight1;
  par[4] = LorentzWidth2;
  par[5] = LorentzHeight2;
  par[6] = GausHeight;
  par[7] = GausSigma1;
  par[8] = GausSigma2;

  return DoubleLorentzAsymGausArm(x, par);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonResponse(const double t, const double l)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t and l:
  //
  // t: transversal distance (in degree) from the cone surface
  // l: longitudinal distance (in degree) on the cone surface of the 
  //    calculated origin of the gamma-ray 

  // HACK:
  double Offset = 0.00000e+00;
  double Mean = 8.24296e-02;
  double LorentzWidth1 = 1.68504e+00;
  double LorentzHeight1 = 2.08654e+03;
  double LorentzWidth2 = 1.11093e+01;
  double LorentzHeight2 = -1.83469e+04;
  double GausHeight = 2.22252e+02;
  double GausSigma1 = 2.44192e+00; 
  double GausSigma2 = 2.34953e+00;

  double x[1];
  double par[9];

  x[0] = t;

  par[0] = Offset;
  par[1] = Mean;
  par[2] = LorentzWidth1;
  par[3] = LorentzHeight1;
  par[4] = LorentzWidth2;
  par[5] = LorentzHeight2;
  par[6] = GausHeight;
  par[7] = GausSigma1;
  par[8] = GausSigma2;

  return DoubleLorentzAsymGausArm(x, par);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonMaximum()
{
  // Return the maximum-possible response-value 

  return GetComptonResponse(0, 0);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonTransversalMax()
{
  // Return the maximum transversal angle, at which the response is above the
  // threshold

  return 180*c_Rad;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonTransversalMin()
{
  // Return the minimum transversal angle, at which the response is above the
  // threshold

  return -GetComptonTransversalMax();
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonIntegral(const double Radius) const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  double Ring = 1.0;
//   if (m_HasTrack == false) {
//     Ring = 
//       c_Pi*(Radius +
//             0.5*sqrt(c_Pi)*m_TransversalFit*exp(-0.5*Radius*Radius/(m_TransversalFit*m_TransversalFit))+
//             Radius*TMath::Erf(sqrt(0.5)*Radius/m_TransversalFit));

//   } else {
//     Ring = 
//       c_Pi*(Radius +
//             0.5*sqrt(c_Pi)*m_TransversalFit*m_LongitudinalFit*exp(-0.5*Radius*Radius/(m_TransversalFit*m_TransversalFit))+
//             Radius*TMath::Erf(sqrt(0.5)*Radius/m_TransversalFit));
//     mimp<<"This normalization is only guesstimated... - i.e. a crude approximation"<<endl;
//     double Corr = (TMath::Erf(c_Pi/m_LongitudinalFit/sqrt(2.0)))/(2*c_Pi + Radius);
//     Ring *= Corr;
//   }

  return Ring;
}



////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetPairResponse(const double t)
{
  // Get pair response, t is the distance to the reconstructed origin

 return m_GaussFactorsPair * exp(m_GaussSquareSigmaPair*t*t);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetPairIntegral() const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  mimp<<"Function returns 1"<<show;

  return 1.0;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseGaussian::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  // Only the Compton angle is of interest:
  if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
    m_Phi = dynamic_cast<MComptonEvent*>(Event)->Phi();
    m_HasTrack = dynamic_cast<MComptonEvent*>(Event)->HasTrack();
    return true;
  } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
    return true;
  }

  return false;
}


// MResponseGaussian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
