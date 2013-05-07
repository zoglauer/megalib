/*
 * MResponseImagingCodedMask.cxx
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
// MResponseImagingCodedMask
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingCodedMask.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"
#include "MPhotoEvent.h"
#include "MResponseMatrixO7.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO1.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseImagingCodedMask)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseImagingCodedMask::MResponseImagingCodedMask()
{
  // Construct an instance of MResponseImagingCodedMask
}


////////////////////////////////////////////////////////////////////////////////


MResponseImagingCodedMask::~MResponseImagingCodedMask()
{
  // Delete this instance of MResponseImagingCodedMask
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImagingCodedMask::CreateResponse()
{
  // Create the multiple Compton response

  m_OnlyINITRequired = true;

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  MSettingsMimrec MimrecCfg(false);
  MimrecCfg.Read(m_MimrecCfgFileName);
  MVector xAxis = MimrecCfg.GetImageRotationXAxis();
  MVector zAxis = MimrecCfg.GetImageRotationZAxis();

  cout<<"Generating binned mode coded mask imaging pdfs"<<endl;

  int AngleBinWidth = 1;

  vector<float> XAxis = CreateEquiDist(-4*1.4, 4*1.4, 8);
  vector<float> YAxis = CreateEquiDist(-4*1.4, 4*1.4, 8);
  vector<float> ZAxis = CreateEquiDist(-20, 20, 2);
  
  vector<float> AxisLambda = CreateEquiDist(-180, 180, 360/AngleBinWidth);
  vector<float> AxisNy = CreateEquiDist(0, 180, 180/AngleBinWidth);

  MResponseMatrixO5 Response("Coded Mask Response", XAxis, YAxis, ZAxis, AxisLambda, AxisNy);

  double Lambda;
  // double Ny;
  
  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MPhotoEvent* Photo = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();
    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event, true) == true) {
          if (Event->GetEventType() == MPhysicalEvent::c_Photo) {
            Photo = (MPhotoEvent*) Event;

            // First compute the y-Axis vector:
            MVector yAxis = zAxis.Cross(xAxis);
            
            TMatrix CoordinateRotation;
            CoordinateRotation.ResizeTo(3,3);
            CoordinateRotation(0,0) = xAxis.X();
            CoordinateRotation(1,0) = xAxis.Y();
            CoordinateRotation(2,0) = xAxis.Z();
            CoordinateRotation(0,1) = yAxis.X();
            CoordinateRotation(1,1) = yAxis.Y();
            CoordinateRotation(2,1) = yAxis.Z();
            CoordinateRotation(0,2) = zAxis.X();
            CoordinateRotation(1,2) = zAxis.Y();
            CoordinateRotation(2,2) = zAxis.Z();

            TMatrix Rotation = Photo->GetDetectorRotationMatrix();

            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = -m_SiEvent->GetIAAt(0)->GetSecondaryDirection();
              IdealOriginDir = Rotation*IdealOriginDir;
              IdealOriginDir = CoordinateRotation*IdealOriginDir;
              Lambda = IdealOriginDir.Phi()*c_Deg;
              while (Lambda < -180) Lambda += 360.0;
              while (Lambda > +180) Lambda -= 360.0;
              // Ny = IdealOriginDir.Theta()*c_Deg;

              Response.Add(Photo->GetPosition().X(),
                           Photo->GetPosition().Y(),
                           Photo->GetPosition().Z(),
                           IdealOriginDir.Phi()*c_Deg,
                           IdealOriginDir.Theta()*c_Deg);

              cout<<"Adding: "<<Photo->GetPosition().X()<<":"<<Photo->GetPosition().Y()<<":"<<Photo->GetPosition().Z()<<":"<<IdealOriginDir.Phi()*c_Deg<<":"<<IdealOriginDir.Theta()*c_Deg<<endl;


              ++Counter;
              if (Counter % m_SaveAfter == 0) {
                cout<<"Saving intermediate results..."<<endl;
                Response.Write(m_ResponseName + ".cmbi.rsp", true);
              }
            }
          }
        }
      }    
    }
  }  
  
  Response.Write(m_ResponseName + ".cmbi.rsp", true);

  return true;
}


// MResponseImagingCodedMask.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
