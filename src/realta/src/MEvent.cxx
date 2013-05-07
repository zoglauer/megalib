/*
 * MEvent.cxx                                         v0.1  24/01/2001
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
// MEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEvent.h"

// Standard libs:
#include <iostream>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MEvent::MEvent()
{
  // default constructor

  m_EventType = c_Unkown;

  SetRotationXAxis(MVector(1,0,0));
  SetRotationZAxis(MVector(0,0,1));
  SetTime(-1);
}


////////////////////////////////////////////////////////////////////////////////


MEvent::~MEvent()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


Int_t MEvent::GetEventType()
{
  // Return the event type:
  //
  // MET_Unkown = -1,
  // MET_Compton = 0,
  // MET_Pair = 1

  return m_EventType;
}


////////////////////////////////////////////////////////////////////////////////


void MEvent::SetRotationXAxis(MVector Rot)
{
  //

  m_RotationXAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MEvent::GetRotationXAxis()
{
  //

  return m_RotationXAxis;
}


////////////////////////////////////////////////////////////////////////////////


void MEvent::SetRotationZAxis(MVector Rot)
{
  //

  m_RotationZAxis = Rot;
}


////////////////////////////////////////////////////////////////////////////////


MVector MEvent::GetRotationZAxis()
{
  //

  return m_RotationZAxis;
}


////////////////////////////////////////////////////////////////////////////////


TMatrix MEvent::GetRotation()
{
  //

  TMatrix M(3,3);
  MVector m_RotationYAxis;

  // First compute the y-Axis vector:
  m_RotationYAxis = m_RotationZAxis.Cross(m_RotationXAxis);
  
  //cout<<"RotVector x: "<<m_RotationXAxis.X()<<"!"<<m_RotationXAxis.Y()<<"!"<<m_RotationXAxis.Z()<<endl;
  //cout<<"RotVector y: "<<m_RotationYAxis.X()<<"!"<<m_RotationYAxis.Y()<<"!"<<m_RotationYAxis.Z()<<endl;
  //cout<<"RotVector z: "<<m_RotationZAxis.X()<<"!"<<m_RotationZAxis.Y()<<"!"<<m_RotationZAxis.Z()<<endl;


  M(0,0) = m_RotationXAxis.X();
  M(1,0) = m_RotationXAxis.Y();
  M(2,0) = m_RotationXAxis.Z();
  M(0,1) = m_RotationYAxis.X();
  M(1,1) = m_RotationYAxis.Y();
  M(2,1) = m_RotationYAxis.Z();
  M(0,2) = m_RotationZAxis.X();
  M(1,2) = m_RotationZAxis.Y();
  M(2,2) = m_RotationZAxis.Z();


  return M; 
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MEvent::Assimilate(MEvent *E)
{
  //

  SetRotationXAxis(E->GetRotationXAxis());
  SetRotationZAxis(E->GetRotationZAxis());

  return kTRUE;
}


////////////////////////////////////////////////////////////////////////////////


void MEvent::SetTime(Double_t Time)
{
  //

  m_Time = Time;
}


////////////////////////////////////////////////////////////////////////////////


Double_t MEvent::GetTime()
{
  //

  return m_Time;
}


////////////////////////////////////////////////////////////////////////////////


MString MEvent::ToString()
{
  return "I'm an event!";
}

// MEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
