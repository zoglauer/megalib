/*
 * MResponse.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


double MResponse::GetPETResponse(const double t)
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
