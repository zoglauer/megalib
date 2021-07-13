/*
 * MGeometryRevan.cxx
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
// MGeometryRevan
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGeometryRevan.h"

// Standard libs:
#include <iostream>
#include <limits>
using namespace std;

// ROOT libs:
#include <TMatrix.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDStrip2D.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGeometryRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MGeometryRevan::c_DifferentTracker = numeric_limits<int>::max();


////////////////////////////////////////////////////////////////////////////////

MGeometryRevan::MGeometryRevan()
{
  // Construct an instance of MGeometryRevan

  m_LayerDistance = 1.0;
}


////////////////////////////////////////////////////////////////////////////////


MGeometryRevan::~MGeometryRevan()
{
  // Delete this instance of MGeometryRevan
}


////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::NoiseHit(MVector& Pos, double& E)
{
  // Return true if Test is in the same layer like Orig
  
  massert(false);

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::AreInSameDetectorVolume(MRESE* RESE1, MRESE* RESE2)
{
  // Return true if the two hits are in the same detector volume:

  return RESE1->GetVolumeSequence()->HasSameDetector(RESE2->GetVolumeSequence());
}


////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::AreInSameLayer(MRESE *Orig, MRESE *Test)
{
  // Return true if Test is in the same layer like Orig

  if (Orig->GetDetector() != 1 || Test->GetDetector() != 1) {
    mdebug<<"("<<Orig->GetID()<<"+"<<Test->GetID()<<"): At least one is not D1 "<<endl;
    return false;
  }

  return (GetLayerDistance(Orig, Test) == 0) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::AreInSameTracker(MRESE *Orig, MRESE *Test)
{
  // That is not working for all possibilities...
  
  if (Orig->GetDetector() != 1 || Test->GetDetector() != 1) {
    mdebug<<"Not in D1"<<endl;
    return false;
  }

  if (Orig->GetVolumeSequence()->GetDeepestVolume() == 0 || 
      Test->GetVolumeSequence()->GetDeepestVolume() == 0) {
    mdebug<<"No deepest volume: "
          <<Orig->GetVolumeSequence()->GetDeepestVolume()<<" - "
          <<Test->GetVolumeSequence()->GetDeepestVolume()<<endl;
    mdebug<<"  Orig: "<<Orig->GetVolumeSequence()->ToString();
    mdebug<<"  Test: "<<Test->GetVolumeSequence()->ToString();
    return false;
  }

  if (!(Orig->GetVolumeSequence()->GetRotation() == Test->GetVolumeSequence()->GetRotation())) {
    //TMatrix M =  Orig->GetVolumeSequence()->GetRotation();
    //cout<<"Orig: "<<M(0,0)<<" | "<<M(0,1)<<" | "<<M(0,2)<<endl;
    //cout<<"Orig: "<<M(1,0)<<" | "<<M(1,1)<<" | "<<M(1,2)<<endl;
    //cout<<"Orig: "<<M(2,0)<<" | "<<M(2,1)<<" | "<<M(2,2)<<endl;
    //TMatrix T =  Test->GetVolumeSequence()->GetRotation();
    //cout<<"Test: "<<T(0,0)<<" | "<<T(0,1)<<" | "<<T(0,2)<<endl;
    //cout<<"Test: "<<T(1,0)<<" | "<<T(1,1)<<" | "<<T(1,2)<<endl;
    //cout<<"Test: "<<T(2,0)<<" | "<<T(2,1)<<" | "<<T(2,2)<<endl;
    mdebug<<"Different rotation!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MGeometryRevan::GetLayerDistance(MRESE *Orig, MRESE *Test)
{
  // Return the distance between the hits in layer distances

  double Dist = 0;

  if (m_ComplexER == true) {
    //if (AreInSameTracker(Orig, Test) == false) {
      //mdebug<<"("<<Orig->GetID()<<"+"<<Test->GetID()<<"): Not in same tracker!"<<endl;
    //  return c_DifferentTracker;
    //}

    if (Orig->GetVolumeSequence()->GetDetector() == 0 ||
        Test->GetVolumeSequence()->GetDetector() == 0) {
      merr<<"("<<Orig->GetID()<<"+"<<Test->GetID()<<"): One of the detectors is zero!!!"<<endl;
      return c_DifferentTracker;
    }

    if ((Orig->GetVolumeSequence()->GetDetector()->GetType() != MDDetector::c_Strip2D &&
         Orig->GetVolumeSequence()->GetDetector()->GetType() != MDDetector::c_Strip3D) ||
        (Test->GetVolumeSequence()->GetDetector()->GetType() != MDDetector::c_Strip2D &&
         Test->GetVolumeSequence()->GetDetector()->GetType() != MDDetector::c_Strip3D)) {
      mdebug<<"("<<Orig->GetID()<<"+"<<Test->GetID()<<"): Not a Strip Detector!"<<endl;
      return c_DifferentTracker;
    }

    MDStrip2D* DOrig = (MDStrip2D*) (Orig->GetVolumeSequence()->GetDetector());
    
    MVector OOrig;
    OOrig[DOrig->GetOrientation()] = 1;
    OOrig *= DOrig->GetStructuralPitch()[DOrig->GetOrientation()];
    OOrig = Orig->GetVolumeSequence()->GetRotation() * OOrig;

    if (OOrig.Mag() == 0) {
      merr<<"Tracker layer distance is zero ("<<Orig->GetPosition()<<" vs "<<Test->GetPosition()<<" vs. "<<OOrig<<"): Seems you forgot to set the correct structural pitch for an electron tracker!"<<show;
      return 0;
    }

    Dist = Orig->ComputeMinDistanceVector(Test).Mag()*
      cos(OOrig.Angle(Test->GetPosition()-Orig->GetPosition()))/OOrig.Mag();

    //mdebug<<"("<<Orig->GetID()<<"+"<<Test->GetID()<<"): Normalized distance between hits: "<<Dist<<endl;

  } else {
    mimp<<"Simple ER... - make sure you really wish this..."<<show;
    Dist = Orig->ComputeMinDistanceVector(Test).Z()/m_LayerDistance;
  }

  return (Dist < 0) ? (int) (Dist - 0.5) : (int) (Dist + 0.5);
}


////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::IsBelow(MRESE *Orig, MRESE *Test, int NBelow)
{
  // Return true if Test is in layer "below" Orig's layer

  if (GetLayerDistance(Orig, Test) == -NBelow) return true;

  return false;

//   if (AreInSameTracker(Orig, Test) == false) {
//     return false;
//   }

//   double Max = -NBelow + 0.5;
//   double Min = -NBelow - 0.5;

//   MVector Dist = Orig->ComputeMinDistanceVector(Test);

//   if (Dist.Z() < Max * m_LayerDistance && Dist.Z() > Min * m_LayerDistance) {
//     return true;
//   } else {
//     return false;
//   }
}
 

////////////////////////////////////////////////////////////////////////////////


bool MGeometryRevan::IsAbove(MRESE *Orig, MRESE *Test, int NAbove)
{
  // Return true if Test is in layer "below" Orig's layer

  if (GetLayerDistance(Orig, Test) == NAbove) return true;

  return false;


//   if (AreInSameTracker(Orig, Test) == false) {
//     return false;
//   }

//   double Max = NAbove + 0.5;
//   double Min = NAbove - 0.5;

//   MVector Dist = Orig->ComputeMinDistanceVector(Test);

//   if (Dist.Z() > Min * m_LayerDistance && Dist.Z() < Max * m_LayerDistance) {
//     return true;
//   } else {
//     return false;
//   }
}


// MGeometryRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
