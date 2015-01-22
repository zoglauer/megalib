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

  // The default threshold is 2.5 sigmas:
  m_Threshold = 2.5;

  // 1.0 sigma :  1.0 - 0.632215; 
  // 1.5 sigma :  1.0 - 0.894141;
  // 2.0 sigma :  1.0 - 0.981645;
  // 2.5 sigma :  1.0 - 0.998066;
  // 3.0 sigma :  1.0 - 0.999873;
  // 3.5 sigma :  1.0 - 0.999971;
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
  m_GaussFactorsTransLong = m_GaussFactorsTrans*m_GaussFactorsLong;
  m_GaussFactorsPair = 1.0/(m_PairFit*sqrt(2*c_Pi));
}


////////////////////////////////////////////////////////////////////////////////


double MResponseGaussian::GetComptonResponse(const double t)
{
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t:
  //
  // t: transversal distance (in degree) from the cone surface

  if (m_ApproximatedMaths == true) {
    return m_GaussFactorsTrans * GaussExp(-m_GaussSquareSigmaTrans*t*t);
  } else {
    return m_GaussFactorsTrans * exp(m_GaussSquareSigmaTrans*t*t);
  }
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

  if (m_ApproximatedMaths == true) {
    return m_GaussFactorsTransLong * GaussExp(-m_GaussSquareSigmaLong*l*l-m_GaussSquareSigmaTrans*t*t);
  } else {
    return m_GaussFactorsTransLong * exp(m_GaussSquareSigmaLong*l*l + m_GaussSquareSigmaTrans*t*t);
  }
//   if (m_ApproximatedMaths == true) {
//     return m_GaussFactorsLong * GaussExp(-m_GaussSquareSigmaLong*l*l) *       
//       m_GaussFactorsTrans * GaussExp(-m_GaussSquareSigmaTrans*t*t);
//   } else {
//     return m_GaussFactorsLong * exp(m_GaussSquareSigmaLong*l*l) *       
//       m_GaussFactorsTrans * exp(m_GaussSquareSigmaTrans*t*t);
//   }
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

  return m_Threshold*m_TransversalFit;
  // return sqrt(log(m_Threshold*GetComptonMaximum()/m_GaussFactorsTrans)/m_GaussSquareSigmaTrans);
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

  double Ring;
  if (m_HasTrack == false) {
    Ring = 
      c_Pi*(Radius +
            0.5*sqrt(c_Pi)*m_TransversalFit*exp(-0.5*Radius*Radius/(m_TransversalFit*m_TransversalFit))+
            Radius*TMath::Erf(sqrt(0.5)*Radius/m_TransversalFit));

  } else {
    Ring = 
      c_Pi*(Radius +
            0.5*sqrt(c_Pi)*m_TransversalFit*m_LongitudinalFit*exp(-0.5*Radius*Radius/(m_TransversalFit*m_TransversalFit))+
            Radius*TMath::Erf(sqrt(0.5)*Radius/m_TransversalFit));
    mimp<<"This normalization is only guesstimated... - i.e. a crude approximation"<<endl;
    double Corr = (TMath::Erf(c_Pi/m_LongitudinalFit/sqrt(2.0)))/(2*c_Pi + Radius);
    Ring *= Corr;
  }

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
  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    m_Phi = dynamic_cast<MComptonEvent*>(Event)->Phi();
    m_HasTrack = dynamic_cast<MComptonEvent*>(Event)->HasTrack();
    return true;
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Special version of exp for Gauss-calculations: exp(-x) --> this input x has to be positive!!
//! We have a cut-off at x = 10.4: y < 0.00003 -> y = 0
//! The cut-off is at the absolute accuracy limit of the algorithm
float MResponseGaussian::GaussExp(float x) {

  //static const float ln2 = log(2);
  //static const float cutoff = -log(0.00003);
  
  //if (x < 0.0f) abort(); // That's hard but this is highly optimized code for the caluclation of a gaussian...

//   // Take care of negative values --- useless since we have the cut-off
// 	bool IsNegative = false;
// 	if (x < 0.0f) {
// 		IsNegative = true;
// 		x = -x;
// 	}

  // Cut-off at x == 10.4:
  if (x >= 10.4f) return 0.0f;

  // Scale into the Abramowitz-range: 0 <= x <= ln(2)
  // Trick: Try to achieve ((exp(-a)^2)^2)^2...

  int Scaler = 0;
  while (x > 0.69314718f) { // ln(2), that's the range where Abramowitz' approximation is valid
    ++Scaler;
    x /= 2.0f; // <- this reduces the accuracy of the algorithm, but we don't care about the accuracy in the Gauss tail.
  }

  // Now x is in the range for Abramowitz: 
	x = 1 - x*(0.9998684f - x*(0.4982926f - x*(0.1595332f - x*(0.0293641f))));

  // Now unscale
	while (Scaler != 0) {
		--Scaler;
		x = x*x;
	}

// 	if (IsNegative == true) {
// 		return 1.0f / x;
// 	}

  return x;
}


// MResponseGaussian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
