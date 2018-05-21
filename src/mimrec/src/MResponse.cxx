/*
 * MResponse.cxx
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
// MResponse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponse.h"

// Standard libs:

// ROOT libs:
#include "MStreams.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponse)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponse::MResponse()
{
  // default constructor
  
  m_Threshold = 0.001;
  m_ApproximatedMaths = false;
}


////////////////////////////////////////////////////////////////////////////////


MResponse::~MResponse()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MResponse::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  merr<<"This function has to be overwritten in a derived class!"<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MResponse::SetThreshold(const double Threshold)
{
  // Set the threshold ...

  m_Threshold = Threshold;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetThreshold() const
{
  // Return the threshold

  return m_Threshold;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetPairResponse(const double t)
{
  // This class has to be overwritten
  // 
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t for untracked Comptons
  //
  // t: transversal distance (in degree) from the pair origin

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonResponse(const double t)
{
  // This class has to be overwritten
  // 
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t for untracked Comptons
  //
  // t: transversal distance (in degree) from the cone surface

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonResponse(const double t, const double l)
{
  // This class has to be overwritten
  // 
  // GetResponse calculates and returns the response, determined by the two 
  // parameters t and l for tracked Comptons
  //
  // t: transversal distance (in degree) from the cone surface
  // l: longitudinal distance (in degree) on the cone surface of the 
  //    calculated origin of the gamma-ray 
  //    l is zero for not tracked events.

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonMaximum()
{
  // This class has to be overwritten
  //
  // Return the maximum-possible response-value 

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonTransversalMax()
{
  // This class has to be overwritten
  //
  // Return the maximum transversal angle, at which the response is above the
  // threshold

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonTransversalMin()
{
  // This class has to be overwritten
  //
  // Return the minimum transversal angle, at which the response is above the
  // threshold

  merr<<"This function has to be overwritten in a derived class!"<<endl;
          
  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetComptonIntegral(const double Radius) const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  merr<<"This function has to be overwritten in a derived class!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MResponse::GetPairIntegral() const
{
  // Return the integral from (-inf, -inf) to (inf, inf) of the response
  // function

  merr<<"This function has to be overwritten in a derived class!"<<endl;

  return 0;
}


// MResponse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
