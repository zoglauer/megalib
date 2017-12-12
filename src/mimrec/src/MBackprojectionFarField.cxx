/*
 * MBackprojectionFarField.cxx
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
// MBackprojectionFarField.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionFarField.h"

// Standard libs:
#include <iostream>
#include <cmath>
#include <limits>
using namespace std;

// ROOT libs:
#include "TRotation.h"
#include "TMatrix.h"

// MEGAlib libs:
#include "MPointSource.h"
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackprojectionFarField)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionFarField::MBackprojectionFarField(MCoordinateSystem CoordinateSystem) : MBackprojection(CoordinateSystem)
{
  // Initialize a MBackprojectionFarField object

  m_AreaBin = nullptr;

  m_xBin = nullptr;
  m_yBin = nullptr;
  m_zBin = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionFarField::~MBackprojectionFarField()
{
  // Delete a MBackprojectionFarField object

  delete [] m_AreaBin;

  delete [] m_xBin;
  delete [] m_yBin;
  delete [] m_zBin;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins, 
                                            double x2Min, double x2Max, unsigned int x2NBins, 
                                            double x3Min, double x3Max, unsigned int x3NBins,
                                            MVector xAxis, MVector zAxis)
{
  // We only have one bin in x3 (= radius) dimension - in a galaxy far, far away (32.4 Mio pc) ;-)
  return MViewPort::SetDimensions(x1Min, x1Max, x1NBins, 
                                  x2Min, x2Max, x2NBins, 
                                  0.9999*c_FarAway, 1.00*c_FarAway, 1,
                                  xAxis, zAxis);
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::PrepareBackprojection()
{
  // Make some initial computations, compute the center of each bin, 
  // create an image with the sensitivities


  delete [] m_xBin;
  m_xBin = new double[m_NImageBins];
  delete [] m_yBin;
  m_yBin = new double[m_NImageBins];
  delete [] m_zBin;
  m_zBin = new double[m_NImageBins];

  for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // phi!
    for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { //theta!
      ToCartesean(m_x2BinCenter[x2], m_x1BinCenter[x1], m_x3BinCenter[0], 
                  m_xBin[x1+x2*m_x1NBins], m_yBin[x1+x2*m_x1NBins], m_zBin[x1+x2*m_x1NBins]);
    }
  }

  m_InvSquareDist = 1.0/(m_x3BinCenter[0]*m_x3BinCenter[0]);


  // Steradian per bin normalization...
  delete [] m_AreaBin;
  m_AreaBin = new double[m_x2NBins];
  double TotalArea = 0.0;
  for (unsigned int x2 = 0; x2 < m_x2NBins; x2++) { // theta
    m_AreaBin[x2] = fabs(m_x1IntervalLength * (cos(m_x2Min + (x2+1)*m_x2IntervalLength) - cos(m_x2Min + x2*m_x2IntervalLength)));
    TotalArea += m_AreaBin[x2];
    m_AreaBin[x2] = m_AreaBin[x2];
  }
  //mout<<"Image Area: "<<TotalArea*m_x1NBins<<" sr"<<endl;
  //mout<<"Average Area: "<<TotalArea/m_x2NBins<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::ConeCenter()
{  
  // Calculate the center of the cone in sperical coordinates
  // Return true if this was successful

  ToSpherical(-m_C->Dg().X(),
              -m_C->Dg().Y(),
              -m_C->Dg().Z(),
              m_ThetaConeCenter, m_PhiConeCenter, m_RadiusConeCenter);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::RotateDetectorSystemImagingSystem(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coordinate system

  MVector P(x, y, z);
 
  // Apply the detector rotation of the individual event
  if (m_Event->HasDetectorRotation() == true) {
    P = m_Event->GetDetectorRotationMatrix() * P;
  }
  // Apply the galactic pointing rotation to the event if we have galactic coordinates
  if (m_Event->HasGalacticPointing() == true && m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
    P = m_Event->GetGalacticPointingRotationMatrix() * P;
  }
  // Apply the specific rotation in spherical coordinate systems
  if (m_CoordinateSystem == MCoordinateSystem::c_Spheric) {
    P = m_Rotation * P;
  }

  x = P.X();
  y = P.Y();
  z = P.Z();
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::RotateImagingSystemDetectorSystem(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coordinate system

  MVector P(x, y, z);
 
  // Apply the specific rotation in spherical coordinate systems
  if (m_CoordinateSystem == MCoordinateSystem::c_Spheric) {
    P = m_InvertedRotation * P;
  }  
  // Apply the galactic pointing rotation to the event if we have galactic coordinates
  if (m_Event->HasGalacticPointing() == true && m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
    P = m_Event->GetGalacticPointingInverseRotationMatrix() * P;
  }
  // Apply the detector rotation of the individual event
  if (m_Event->HasDetectorRotation() == true) {
    P = m_Event->GetDetectorInverseRotationMatrix() * P;
  }

  x = P.X();
  y = P.Y();
  z = P.Z();
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::ToSpherical(double x, double y, double z, 
                                          double &t, double &p, double &r)
{
  // Transfer Cartesian Coordinates to Spherical

  // Now transform:
  r =  sqrt(x*x + y*y + z*z);

  if (x == 0.0 && y == 0.0) {
    p = 0.0;
  } else {
    if (x != 0) {
      p = atan2(y, x);
    } else { 
      if (y > 0) {
        p = c_Pi/2;
      } else {
        p = -c_Pi/2;
      }
    }
  }

  if (x == 0.0 && y == 0.0 && z == 0.0) {
    t = 0.0;
  } else {
    if (z != 0) {
      t = atan2(sqrt(x*x + y*y), z);
    } else { 
      if (sqrt(x*x + y*y) > 0) {
        t = c_Pi/2;
      } else {
        t = -c_Pi/2;
      }
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::ToCartesean(double t, double p, double r, 
                                        double &x, double &y, double &z)
{
  // Transfer Spherical Coordinates to Cartesean

  x = r * sin(t) * cos(p);
  y = r * sin(t) * sin(p);
  z = r * cos(t);
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::Assimilate(MPhysicalEvent *Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray, 
  // and here: the intersection of the cone axis with the sphere. 
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  if (MBackprojection::Assimilate(Event) == false) return false;
  
  if (Event->GetType() == MPhysicalEvent::c_Compton || 
      Event->GetType() == MPhysicalEvent::c_Pair || 
      Event->GetType() == MPhysicalEvent::c_Photo) {
    return true;
  } else {
    return false;
  }

  return true;  
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone. 

  if (Assimilate(Event) == false) return false;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    return BackprojectionCompton(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return BackprojectionPair(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
    return BackprojectionPhoto(Image, Bins, NUsedBins, Maximum);
  } else {
    cout<<"Error: Unknown event type for imaging: "<<Event->GetType()<<". Skipping event."<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana

  // Obviously in the beginning no bins are above the threshold to use the bin...
  NUsedBins = 0;
  Maximum = 0.0;

  unsigned int index;
  //int index_offset;
  

  double Temp;
  
  // The current image bin content;
  double Content;

  // Direction to the cone center towards the sky:
  double xCC = -m_C->Dg().X()*m_x3BinCenter[0];
  double yCC = -m_C->Dg().Y()*m_x3BinCenter[0];
  double zCC = -m_C->Dg().Z()*m_x3BinCenter[0]; 
  //   xCC = 0*m_x3BinCenter[0]; // for debugging detector rotations
  //   yCC = 0*m_x3BinCenter[0];
  //   zCC = 1*m_x3BinCenter[0];
  RotateDetectorSystemImagingSystem(xCC, yCC, zCC);
  ToSpherical(xCC, yCC, zCC, m_ThetaConeCenter, m_PhiConeCenter, m_RadiusConeCenter);

  // The origin of the event on the sky:
  double xOrigin = m_C->DiOnCone().X()*m_x3BinCenter[0];
  double yOrigin = m_C->DiOnCone().Y()*m_x3BinCenter[0];
  double zOrigin = m_C->DiOnCone().Z()*m_x3BinCenter[0];
  RotateDetectorSystemImagingSystem(xOrigin, yOrigin, zOrigin);

  // The Compton scatter angle:
  double Phi = m_C->Phi();
  // Phi = 0.01; // for debugging detector rotations


  // Determine the maximum of the gauss-function on the circle:

  // Min and max Trans-angle above threshold:
  //double Trans_min = m_Response->GetComptonTransversalMin(); // negative!
  //double Trans_max = m_Response->GetComptonTransversalMax();

  //double PhiMin = Phi + Trans_min;
  //double PhiMax = Phi + Trans_max;
  //double cosPhiMin = cos(PhiMin);
  //double cosPhiMax = cos(PhiMax);


  // Now determine for all Bins
  // A. the smallest angle between the center of the bin and the circle
  // B. the smallest angle between the nearest point on the circle and the 
  //    maximum of the gauss-function on the circle
  double AngleLong, AngleTrans;
  double AngleA, AngleB, AngleC;
  double Sum = 0;
  double InnerSum = 0.0;

  // Later we need the angle between the cone center and the gauss-maximum:
  AngleA = Angle(xOrigin, yOrigin, zOrigin, xCC, yCC, zCC);

  // Precompute some correction factors:
  
  // The minimum electron angle error
  //double EAngleError = fabs(AngleA - m_C->Phi());

  double InvIntegral = 1.0/m_Response->GetComptonIntegral(Phi);
  //double InvIntegral = 1.0;

  //mimp<<"InvIntegral is only part of the response!"<<show;

  /*
  double ValueMax = 0;
  double ValueMin = 0;

  bool NoInner = true;
  bool NoOuter = true;

  double PhiRelativeInner = 0;
  double PhiRelativeOuter = 0;

  double PhiOuterIn = 0;
  double PhiInnerIn = 0;
  double PhiInnerOut = 0;
  double PhiOuterOut = 0;

 
  int x1Start1 = 0;  
  bool x1Start1Outside = false;
  int x1End1 = 0;
  bool x1End1Outside = false;
  bool x1Range1Outside = false;

  int x1Start2 = 0;
  bool x1Start2Outside = false;
  int x1End2 = 0;
  bool x1End2Outside = false;
  bool x1Range2Outside = false;

  // Optimization and precalcualtion:
  double cosThetaConeCenter = cos(m_ThetaConeCenter);
  double sinThetaConeCenter = sin(m_ThetaConeCenter);
  

  double x2InRad;
  */
  // ---------> time critical --------->


  bool HasTrack = m_C->HasTrack();

  for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
    /* Start comment out the following section if you want to use simple mode 

    x2InRad = (0.5+x2)*m_x2IntervalLength + m_x2Min;
 
    // The following is an intelligent line skipping method:
    // Although is seems lengthy, the most part is just fast case differentiation, thus in the end saves a lot of time...
      
    // Calculate the phi start and stop positions on this ring:
    // This is the intersection between a circle determine by the current theta, 
    // a longitudinal circle through the cone center, and the circle defined by the cone opening.
    // Therefore we have three angles:
    // ThetaConeCenter: angle from zenith to the cone center
    // Phi: Opening of the Compton cone
    // x2InRad: the current theta
    // Using the cosinus rule, we can calulate the angle on the latitude circle between 
    // the longitudinal circle through the cone center and the intersection with the compton cone on the latitude circle 
    ValueMax = (cosPhiMax-cos(x2InRad)*cosThetaConeCenter)/(sin(x2InRad)*sinThetaConeCenter);
    ValueMin = (cosPhiMin-cos(x2InRad)*cosThetaConeCenter)/(sin(x2InRad)*sinThetaConeCenter);

    // If both are not within acos range (both above or both below), 
    // then we do not intersect the Compton cone and can skip this line of code
    if ((ValueMax > 1 &&  ValueMin > 1) || (ValueMin < -1 &&  ValueMin < -1)) {
      //cout<<"Skipped theta row due to no intersection with Compton cone: "<<x2InRad*c_Deg<<endl; 
      continue;
    }

    // Now check if we have an inner or an outer ring
    if (ValueMax > 1 || ValueMax < -1) {
      NoOuter = true;
    } else {
      NoOuter = false;
      PhiRelativeOuter = acos(ValueMax);
    }
    if (ValueMin > 1 || ValueMin < -1) {
      NoInner = true;
    } else {
      NoInner = false;
      PhiRelativeInner = acos(ValueMin);
    }

    // If we have an inner or an outer ring then convert it to real, not just realtive angles
    if (NoOuter == false) {
      PhiOuterIn  = m_PhiConeCenter - PhiRelativeOuter;
      PhiOuterOut = m_PhiConeCenter + PhiRelativeOuter;
      //cout<<endl<<"Angles: oi: "<<PhiOuterIn*c_Deg<<" oo: "<<PhiOuterOut*c_Deg<<endl;
      while (PhiOuterIn < m_x1Min) PhiOuterIn += c_TwoPi;
      while (PhiOuterOut < m_x1Min) PhiOuterOut += c_TwoPi;
      while (PhiOuterIn > m_x1Min + c_TwoPi) PhiOuterIn -= c_TwoPi;
      while (PhiOuterOut > m_x1Min + c_TwoPi) PhiOuterOut -= c_TwoPi;
    } 
    if (NoInner == false) {
      PhiInnerIn  = m_PhiConeCenter - PhiRelativeInner;
      PhiInnerOut = m_PhiConeCenter + PhiRelativeInner;
      //cout<<endl<<"Angles: ii: "<<PhiInnerIn*c_Deg<<" io: "<<PhiInnerOut*c_Deg<<endl;
      while (PhiInnerIn < m_x1Min) PhiInnerIn += c_TwoPi;
      while (PhiInnerOut < m_x1Min) PhiInnerOut += c_TwoPi;
      while (PhiInnerIn > m_x1Min + c_TwoPi) PhiInnerIn -= c_TwoPi;
      while (PhiInnerOut > m_x1Min + c_TwoPi) PhiInnerOut -= c_TwoPi;
    } 

    //cout<<endl<<"Angles: oi: "<<PhiOuterIn*c_Deg<<" ii: "<<PhiInnerIn*c_Deg<<" io: "<<PhiInnerOut*c_Deg<<" oo: "<<PhiOuterOut*c_Deg<<endl;
    
    // Determine the first interval start and end bins and if the start is within the image limits
    if (PhiOuterIn < m_x1Max && NoOuter == false) {
      x1Start1Outside = false;
      x1Start1 = int((PhiOuterIn-m_x1Min)/m_x1IntervalLength);
    } else {
      x1Start1Outside = true;
      x1Start1 = 0;
    }
    if (PhiInnerIn < m_x1Max && NoInner == false) {
      x1End1Outside = false;
      x1End1 = int((PhiInnerIn-m_x1Min)/m_x1IntervalLength);
    } else {
      x1End1Outside = true;
      x1End1 = m_x1NBins-1;
    }
    
    // Determine the second interval start and end bins and if the start is within the image limits
    if (PhiInnerOut < m_x1Max && NoInner == false) {
      x1Start2Outside = false;
      x1Start2 = int((PhiInnerOut-m_x1Min)/m_x1IntervalLength);
    } else {
      x1Start2Outside = true;
      x1Start2 = 0;
    }
    if (PhiOuterOut < m_x1Max && NoOuter == false) {
      x1End2Outside = false;
      x1End2 = int((PhiOuterOut-m_x1Min)/m_x1IntervalLength);
    } else {
      x1End2Outside = true;
      x1End2 = m_x1NBins-1;
    }

    //cout<<"i:"<<x2InRad*c_Deg<<" x1s: "<<x1Start1<<" x1e: "<<x1End1<<" x2s: "<<x1Start2<<" x2e: "<<x1End2<<endl;

    // 
    bool BinsLeft = true;
    bool FirstInterval = true;
    int x1;
    
    // If one of the rings is missing, or we are completely inside we only handle one of the segment
    if (NoInner == true && NoOuter == true) {
      //cout<<"No inner, no outer"<<endl;
      // Let's assume we are completely inside, we cannot distinguish yet if this is a partial image and we just don't hit the ring, this is handles in the next section
      x1Start2Outside = false;
      x1Start2 = 0;
      x1End2Outside = false;
      x1End2 = m_x1NBins -1;
      FirstInterval = false;
    } else if (NoInner == true) {
      //cout<<"No inner"<<endl;
      x1Start2Outside = x1Start1Outside;
      x1Start2 = x1Start1;
      FirstInterval = false;
    } else if (NoOuter == true) {
      //cout<<"No outer"<<endl;
      x1End2Outside = x1End1Outside;
      x1End2 = x1End1;
      FirstInterval = false;
    } 

    // Determine if interval is completely inside or completely outside just because we only have a partial image
    if (FirstInterval == true && x1Start1Outside == true && x1End1Outside == true) {
      index = (x1Start1+x1End1)/2 + x2*m_x1NBins; // Don't use the start bin, since it might be at the edge which can be inside
      AngleTrans = Angle(m_xBin[index], m_yBin[index], m_zBin[index], xCC, yCC, zCC);
      if (AngleTrans < PhiMin || AngleTrans > PhiMax) {
        //cout<<"Range 1 outside"<<endl;
        x1Range1Outside = true;
      } else {
        x1Range1Outside = false;
      }
    } else {
      x1Range1Outside = false;
    }
    if (x1Start2Outside == true && x1End2Outside == true) {
      index = (x1Start2+x1End2)/2 + x2*m_x1NBins;
      AngleTrans = Angle(m_xBin[index], m_yBin[index], m_zBin[index], xCC, yCC, zCC);
      if (AngleTrans < PhiMin || AngleTrans > PhiMax) {
        //cout<<"Range 2 outside"<<endl;
        x1Range2Outside = true;
      } else {
        x1Range2Outside = false;
      }
    } else {
      x1Range2Outside = false;
    }

    // If both ranges are outside due to partial image, we can skip here
    if ((x1Range1Outside == true && x1Range2Outside == true) || (FirstInterval == false && x1Range2Outside == true)) {
      //cout<<"Skipped theta row since SELECTION has no intersection with Compton cone: "<<x2InRad*c_Deg<<endl; 
      continue;
    }

    // Merge the intervals if they are adjacent:
    if (NoInner == false && NoOuter == false && x1Range1Outside == false && x1Range2Outside == false) {
      if (x1End1 == x1Start2) {
        //cout<<"Merging: x1End1 == x1Start2"<<endl;
        x1Start2 = x1Start1;
        x1Start2Outside = x1Start1Outside;
        FirstInterval = false;
      } else if (x1End2 == x1Start1) {
        //cout<<"Merging: x1End2 == x1Start1"<<endl;
        x1End2 = x1End1;
        x1End2Outside = x1End1Outside;
        FirstInterval = false;
      }
    }

    // Set the start bins
    x1 = x1Start1;
    if (FirstInterval == false || (x1Start1Outside == true && x1End1Outside == true)) {
      FirstInterval = false;
      x1 = x1Start2;
    }

    // The end bin needs to be one beyond the last one calculated:
    if (++x1End1 >= m_x1NBins) x1End1 -= m_x1NBins;
    if (++x1End2 >= m_x1NBins) x1End2 -= m_x1NBins;
    
    //cout<<"i:"<<x2InRad*c_Deg<<" x1s: "<<x1Start1<<" x1e: "<<x1End1<<" x2s: "<<x1Start2<<" x2e: "<<x1End2<<": starting at: "<<x1<<":"<<int(BinsLeft)<<endl;

    // This marks the end of the precalculations of the start end end phi angles 


    // New we can loop over the sections
    index_offset = x2*m_x1NBins;
    while (BinsLeft) {
       
      index = x1 + index_offset;   // Loop over all image bins:
    
     End comment out if you want to use simple mode and comment in the follwoing to lines */

    // Comment the following two line in if you want to use the not-optimized mode:
    for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) {
      index = x1 + x2*m_x1NBins;
      
      // Start the backprojections:
      
      // For each image bin calculate two parameters:
      // a. the transversal angle:
      //    the shortest distance to the cone 
      //    i.e. the angle between the center of the cone and the bin center 
      //    minus the angle of the cone 
      // b. the longitudinal angle:
      //    the angle we search is the angle between these points:
      //    center of the cone, center of the bin and gauss-maximum
      //    This is a spherical triangle and the angle can be solved with the "Halbwinkelsatz"
      
      // For those two angles retrieve the response, do all the normalizations
      // and store the value.

      if (HasTrack == false) {
        // ad A.:
        // the angle we search is the angle between the center of the cone and the bin center 
        // minus the angle of the cone 
        // Therefore its the (smallest) angle between the bin center and the event circle:
        AngleTrans = FastAngle(m_xBin[index], m_yBin[index], m_zBin[index], xCC, yCC, zCC) - Phi;
        
        Content = m_Response->GetComptonResponse(AngleTrans)*InvIntegral; //*m_AreaBin[x2];
      } else {
        // ad A.:
        // the angle we search is the angle between the center of the cone and the bin center 
        // minus the angle of the cone 
        AngleB = FastAngle(m_xBin[index], m_yBin[index], m_zBin[index], xCC, yCC, zCC);
        AngleTrans = AngleB - Phi;

        // ad B.:
        // the angle we search is the angle between these points:
        // center of the cone, center of the bin and gauss-maximum
        // This is a spherical triangle and the angle can be solved with the "Halbwinkelsatz"

        AngleC = Angle(m_xBin[index], m_yBin[index], m_zBin[index], xOrigin, yOrigin, zOrigin);
        Sum = (AngleA + AngleB + AngleC)*0.5;
        if (m_ApproximatedMaths == true) {
          Temp = MFastMath::sin(Sum-AngleA)*MFastMath::sin(Sum-AngleB)/(MFastMath::sin(Sum)*MFastMath::sin(Sum-AngleC));
        } else {
          Temp = sin(Sum-AngleA)*sin(Sum-AngleB)/(sin(Sum)*sin(Sum-AngleC));          
        }
        AngleLong = 2 * atan(sqrt(fabs(Temp)));

        // Sample the 2d-Gauss-function
        Content = m_Response->GetComptonResponse(AngleTrans, AngleLong)*InvIntegral; //*m_AreaBin[x2];
      }      index = x1 + x2*m_x1NBins;


      if (Content > 0.0) {

        // Now take into account absorption probabilities:
        if (m_UseAbsorptions == true) {
          double rx = m_xBin[index];
          double ry = m_yBin[index];
          double rz = m_zBin[index];
          RotateDetectorSystemImagingSystem(rx, ry, rz);
          double Abs = m_Geometry->GetAbsorptionProbability(m_C->C1(), MVector(rx, ry, rz), m_C->Ei()); // Inverse direction to avoid rounding errors!
          Content *= (1-Abs);
        }
        // Take care of data management:
        // The maximum --- required for one-byte storage
        if (Content > Maximum) Maximum = Content;
        // The sum is for sanity checks that we don't propagate a NaN (faster than checking it each time...)
        InnerSum += Content;
        
        // Save data:
        Image[NUsedBins] = Content;
        Bins[NUsedBins] = index;

        // Finally advance
        ++NUsedBins;
        
      } else {
        //cout<<"Event "<<m_Event->GetId()<<": I seem to still calculate image bins with no content: phi="<<x1<<", theta="<<x2<<endl;
      }

      // Comment the last lines *in the loop* out if you want to use the simple mode:
      /*
      
      // Prepare for next bin
      ++x1;
      // Take care of holes
      if (x1 == m_x1NBins) {
        x1 = 0;
      }
      // Take care of end
      if (FirstInterval == true) {
        if (x1 == x1End1) {
          x1 = x1Start2;
          FirstInterval = false;
          if (x1Start2Outside == true && x1End2Outside == true && FirstInterval == false) BinsLeft = false;
        }
      } else {
        if (x1 == x1End2) {      index = x1 + x2*m_x1NBins;
          BinsLeft = false;
        }
      }
      */
    }
  }

  //cout<<"Used bins: "<<NUsedBins<<endl;
  //cout<<"InnerSum:"<<InnerSum<<" compared to : "<<m_Response->GetComptonIntegral(Phi)<<" - Maximum: "<<Maximum<<endl;

  // If one of the above computed number has been NaN
  // InnerSum is also NaN. Then we have to reject this event.
  if (InRange(InnerSum) == false) {
    cout<<"Event "<<m_Event->GetId()<<": Catched a NaN!"<<endl;
    cout<<m_C->ToString()<<endl;

    NUsedBins = 0;
    Maximum = 0.0;
    return false;
  }


  // If we have an efficiency the use it:
  if (m_Efficiency != nullptr) {
    Maximum = 0.0;
    InnerSum = 0.0;
    for (int i = 0; i < NUsedBins; ++i) {
      double x = m_xBin[Bins[i]];
      double y = m_yBin[Bins[i]];
      double z = m_zBin[Bins[i]];
      RotateImagingSystemDetectorSystem(x, y, z);
      MVector D(x, y, z);

      if (m_ApproximatedMaths == false) {
        Image[i] *= m_Efficiency->Get(D.Theta(), D.Phi());
      } else {
        Image[i] *= m_Efficiency->Get(D.ThetaApproximateMaths(), D.PhiApproximateMaths());        
      }
      InnerSum += Image[i];
      
      if (Image[i] > Maximum) Maximum = Image[i];
    }
  }
  
  // If the image does not contain any content, return also false
  // This case should not really happen due to protections before, thus the error message
  if (InnerSum == 0 && NUsedBins != 0) {
    cout<<"Event "<<m_Event->GetId()<<": The image seems to be empty..."<<endl;
    NUsedBins = 0;
    Maximum = 0.0;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::BackprojectionPhoto(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Backprojection of a photo events

  Maximum = 0.0;
  NUsedBins = 0;



  double Energy = m_Photo->GetEnergy();
  Energy = 50;

  MVector Position = m_Photo->GetPosition();
  double Radius = sqrt(Position[0]*Position[0] + Position[1]*Position[1]);
  double Angle = 0.0;
  if (Position[0] >= 0) Angle = asin(Position[1]/Radius);
  if (Position[0] < 0) Angle = -asin(Position[1]/Radius) + c_Pi;
  
  Radius += 0.5;
  Position[0] = Radius*cos(Angle);
  Position[1] = Radius*sin(Angle);

//   Position[0] += gRandom->Rndm()-0.5;
//   Position[1] += gRandom->Rndm()-0.5;
  //Position[2] += 0.5;

  MVector SkyDir;

  unsigned int i;
  double InnerSum = 0.0;

  // We are going over all bins here...
  for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
    for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
      i = x1+x2*m_x1NBins; // image bin i
      
      Image[NUsedBins] = 1; //m_Photo->GetWeight();
      Bins[NUsedBins] = i;

      if (m_Geometry != 0) {
        double rx = m_xBin[i]-Position.X();
        double ry = m_yBin[i]-Position.Y();
        double rz = m_zBin[i]-Position.Z();
        //cout<<m_x2BinCenter[x2]*c_Deg<<":"<<m_x1BinCenter[x1]*c_Deg<<":  "<<m_Photo->GetPosition()<<":"<<MVector(rx, ry, rz)<<endl;
        RotateDetectorSystemImagingSystem(rx, ry, rz);
        //cout<<m_x2BinCenter[x2]*c_Deg<<":"<<m_x1BinCenter[x1]*c_Deg<<":  "<<m_Photo->GetPosition()<<":"<<MVector(rx, ry, rz)<<endl;

        //SkyDir = -MVector(rx, ry, rz) - Position;
        SkyDir = -MVector(rx, ry, rz);
        SkyDir.Unitize();

        double Abs = m_Geometry->GetAbsorptionProbability(Position, Position + 1000*SkyDir, Energy);
        //cout<<"Abs: "<<Abs<<":"<<Position<<":"<<-MVector(rx, ry, rz)<<endl;
        Image[NUsedBins] *= (1-Abs); 
      } else {
        merr<<"Error: Geometry needed for backprojection of photons!"<<show;
      }
      
      if (Maximum < Image[NUsedBins]) Maximum = Image[NUsedBins];
      InnerSum += Image[NUsedBins];

      // Finally increase
      ++NUsedBins;
    }
  }
  
//   if (InnerSum < 0.00001) {
//     // Event not inside the viewport
//     return false;
//   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

// Addition by Christian Lang - BackprojectionComptonLine
//---------------------------------------------------------
bool MBackprojectionCartesian::BackprojectionComptonLine(double* Image, int* Bins, int& NUsedBins, double& Maximum,
double X1Position, double Y1Position, double Z1Position, double X2Position, double Y2Position, double Z2Position
)
{
// Compton-Backprojection-LOR-Intersection-algorithm:
// The event expands to a double-gausshaped banana and gets intersected with a Line of Response
//
// Image
//: the produced binned image - must have the correct dimensions
// Limit
//: the limit
// NABoveLimit
//
Maximum = 0.0;
NUsedBins = 0;
// Let the response get the event data:
m_Response->AnalyzeEvent(m_C); // <- not the correct place...
int x, y, z;
double AngleTrans, AngleLong;
double InnerSum = 0;
double ConeRadius = 0;
double Content;

// LOR axis vector
double ContentLine, Distance;
double ax = X2Position - X1Position;
double ay = Y2Position - Y1Position;
double az = Z2Position - Z1Position;
double bx, by, bz;
double Deno = ax*ax+ay*ay+az*az;
// Cone axis vector (reverse direction of scattered gamma ray)
double xCA = -m_C->Dg().X();
double yCA = -m_C->Dg().Y();
double zCA = -m_C->Dg().Z();
Rotate(xCA, yCA, zCA);
// Position of first interaction:
double xCC = m_C->C1().X();
double yCC = m_C->C1().Y();
double zCC = m_C->C1().Z();
Rotate(xCC, yCC, zCC);
//cout<<"Cone center: "<<xCC<<":"<<yCC<<":"<<zCC<<endl;
// The origin of the gamma ray:
double xOrigin = -m_C->De().X();
double yOrigin = -m_C->De().Y();
double zOrigin = -m_C->De().Z();
Rotate(xOrigin, yOrigin, zOrigin);
double xBin, yBin, zBin;
double xn1, yn1, zn1, xn2, yn2, zn2;
// Important pre-computations
double Phi = m_C->Phi();
double tanPhi = tan(Phi);
//double Threshold = 0.001;
// Min and max Trans-angle above threshold:
double Trans_max = m_Response->GetComptonTransversalMax();
double Trans_min = m_Response->GetComptonTransversalMin();
double PhiMin = Phi + Trans_min;
double PhiMax = Phi + Trans_max;
double cosPhiMin = cos(PhiMin);
double cosPhiMax = cos(PhiMax);

double L;
bool HasTrack = m_C->HasTrack();

// ---------> time critical --------->
// Start the backprojections

unsigned int i;
unsigned int i_z;
unsigned int i_yz;
for (z =0; z < m_x3NBins; z++) {
i_z = z*m_x1NBins*m_x2NBins;
for (y = 0; y < m_x2NBins; y++) {
i_yz = y*m_x1NBins + i_z;
for (x = 0; x < m_x1NBins; x++) {
i = x + i_yz; // We fill each x-row
Content = 0;
ContentLine = 0;
xBin = m_xBinCenter[x]-xCC;
yBin = m_yBinCenter[y]-yCC;
zBin = m_zBinCenter[z]-zCC;
AngleTrans = Angle(xBin, yBin, zBin, xCA, yCA, zCA) - Phi;

// tmp: time coincidence only
////////////////////////////////////////////
// Skip pixels below threshold:

if (x < m_x1NBins - 1) { // We just do the optimization if there are bins left in x-direction:
// We now calculate the intersection point between this axis and the cone
// The cone is defined by (cone center is origin):
// 0 = (X dot CA)ˆ2 - (X dot X) * (CA dot CA) * sin phi (CA: cone axis)
// The axis is defined by:
// X = P + l*(1, 0, 0) where P is the bin center relative to the cone center, i.e. xBin, yBin, zBin
// Then we determine a, b, c of the Mitternachtsformel and calculate lambda1,2
// Outside outer ring
if (AngleTrans > Trans_max + 0.002 || AngleTrans < Trans_min - 0.002) {
//cout<<m_xBinCenter[x]<<":"<<m_yBinCenter[y]<<endl;
double CAdotCA = xCA*xCA + yCA*yCA + zCA*zCA; // Per definition "1"!
double BCdotBC = xBin*xBin + yBin*yBin + zBin*zBin;
double BCdotCA = xBin*xCA + yBin*yCA + zBin*zCA;
double cos2PhiRelevant;
if (AngleTrans > Trans_max) {
//cout<<"Outside: "<<(AngleTrans - Trans_max)*c_Deg<<endl;
cos2PhiRelevant = cosPhiMax;
} else {
//cout<<"Inside: "<<(AngleTrans - Trans_min)*c_Deg<<endl;
cos2PhiRelevant = cosPhiMin;
}
cos2PhiRelevant *= cos2PhiRelevant;
double a = xCA*xCA - CAdotCA*cos2PhiRelevant;
double b = 2*(BCdotCA*xCA - xBin*CAdotCA*cos2PhiRelevant);
double c = BCdotCA*BCdotCA - BCdotBC*CAdotCA*cos2PhiRelevant;
double Diskriminante = b*b-4*a*c; // This has to be identical on one x-axis since the distance between
//axis intersection stays the same
//cout<<"Diskriminante: "<<Diskriminante<<":"<<a<<":"<<b<<":"<<c<<":"<<CAdotCA<<":"<<BCdotBC<<":"<<BCdotCA
//<<":"<<cos2PhiRelevant<<endl;
if (Diskriminante <= 0) { // We ignore the "academic" solution of a tangential point, it’s simply not
//relevant
//cout<<"No intersection with cone"<<endl;
break;
}
double Lambda1 = (-b-sqrt(Diskriminante))/(2*a);
double Lambda2 = (-b+sqrt(Diskriminante))/(2*a);
// Find the closest solution in x-direction (attention: we are in a coordiante system realtive to cone
//center!):
double xNext = 0.0;
//cout<<"xBC: "<<m_xBinCenter[x]<<" xBin: "<<xBin<<" l1:"<<Lambda1<<" l2:"<<Lambda2<<" dist: "<<Lambda1
//-Lambda2<<endl;
if (Lambda1 > 0 && Lambda2 > 0) {
if (Lambda1 < Lambda2) {
xNext = Lambda1;
} else {
xNext = Lambda2;
}
} else if (Lambda1 > 0) {
xNext = Lambda1;
} else if (Lambda2 > 0) {
xNext = Lambda2;
} else {
//cout<<"Beyond intersection"<<endl;
break;
}
//cout<<"Chosen lamda: "<<xNext<<endl;
// Now skip ahead:
// x+= (int) floor(xNext/m_x1IntervalLength);
//if (x >= m_x1NBins) break; // Done
//cout<<"New x: "<<m_xBinCenter[x]<<endl;
continue;
}
}


L = sqrt(xBin*xBin + yBin*yBin + zBin*zBin);
ConeRadius = fabs(tanPhi*Acos(AngleTrans)*L);
if (HasTrack == true) {
// angle between two planes spanned by ...
xn1 = yBin*zCA - zBin*yCA;
yn1 = (zBin*xCA - xBin*zCA);
zn1 = xBin*yCA - yBin*xCA;
xn2 = (yOrigin*zCA - zOrigin*yCA);
yn2 = (zOrigin*xCA - xOrigin*zCA);
zn2 = (xOrigin*yCA - yOrigin*xCA);
AngleLong = Acos((xn1*xn2+yn1*yn2+zn1*zn2)/sqrt((xn1*xn1+yn1*yn1+zn1*zn1)*(xn2*xn2+yn2*yn2+zn2*zn2)));
//cout<<i<<": "<<ConeRadius<<"!"<<tanPhi<<"!"<<AngleTrans<<"!"<<L<<"-->"<<m_Response->GetResponse(AngleTrans
//, AngleLong)/ConeRadius<<endl;
// Sample the 2d-Gauss-function: m_AreaBin[t] is missing ---> new geometry is missing!!!
Content = m_Response->GetComptonResponse(AngleTrans, AngleLong)/ConeRadius;
} else {
Content = m_Response->GetComptonResponse(AngleTrans)/ConeRadius;
}


// calculation of the Line of Response (LOR)
bx = m_xBinCenter[x] - X1Position;
by = m_yBinCenter[y] - Y1Position;
bz = m_zBinCenter[z] - Z1Position;
Distance = sqrt( ((ay*bz-az*by)*(ay*bz-az*by)
+
(az*bx-ax*bz)*(az*bx-ax*bz)
+
(ax*by-ay*bx)*(ax*by-ay*bx))
/ Deno );
// for simulation only!! (gaussian distribution of a LOR with 0.5 mm FWHM)
ContentLine = (1/( 0.02128*sqrt(2*c_Pi)) )*exp(-0.5*(Distance/ 0.02128)*(Distance/ 0.02128)); // LOR 0.5mm
//FWHM
// Intersection of Compton Cone and LOR
if (ContentLine > 0.0 && Content > 0.0) {
//if (ContentLine > 0.0 ) {
ContentLine = ContentLine * Content; // spatial conicidence
// search the highest pixel...
if (ContentLine > Maximum) {Maximum = ContentLine;}
InnerSum += ContentLine;
Image[NUsedBins] = ContentLine;
Bins[NUsedBins] = i;
++NUsedBins;
} else {

// Due to acos-rounding error protection, we might get a few here.
//cout<<"Event "<<m_Event->GetId()<<": I seem to still calculate image bins with no content: x="
// <<m_xBinCenter[x]<<" y="<<m_yBinCenter[y]<<" z="<<m_yBinCenter[z]<<endl;
}
} // end x
} // end y
} // end z
// <--------- time critical <---------
// If one of the above computed numbers has been NaN
// InnerSum is also NaN. Then we have to reject this event.

if (InRange(InnerSum) == false) {
cout<<"Event "<<m_Event->GetId()<<": Catched a NaN!"<<endl;
cout<<m_C->ToString()<<endl;
NUsedBins = 0;
Maximum = 0;
return false;
}
// If the image does not contain any content, return also false
// This case should not really happen due to protections before, thus the error message
if (InnerSum == 0 && NUsedBins != 0) {
cout<<"Event "<<m_Event->GetId()<<": The image seems to be empty..."<<endl;
NUsedBins = 0;
Maximum = 0;
return false;
}
return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionFarField::BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  //cout<<"Now backprojecting a pair-event..."<<endl;
  Maximum = 0.0;
  NUsedBins = 0;

  // The origin of the event on the sky:
  double xOrigin = m_P->m_IncomingGammaDirection.X()*m_x3BinCenter[0];
  double yOrigin = m_P->m_IncomingGammaDirection.Y()*m_x3BinCenter[0];
  double zOrigin = m_P->m_IncomingGammaDirection.Z()*m_x3BinCenter[0];
  RotateDetectorSystemImagingSystem(xOrigin, yOrigin, zOrigin);
  // --> The rotation should be performed during assimilation

  //double Integral = 1.0/m_Response->GetPairIntegral();

  double Content = 0.0;
  double InnerSum = 0.0;
  double AngleTrans;
  unsigned int i;

  // Let's fit a double gausshaped pair-event
  for (unsigned int x2 = 0; x2 < m_x2NBins; x2++) { // x2 == theta
    for (unsigned int x1 = 0; x1 < m_x1NBins; x1++) { // x1 == phi
      i = x1+x2*m_x1NBins;
      
      // ad A.:
      // the angle we search is the angle between the center of the cone and the bin center 
      // minus the angle of the cone 
      // Therefore its the (smallest) angle between the bin center and the eventcircle:
      AngleTrans = Angle(m_xBin[i], m_yBin[i], m_zBin[i], xOrigin, yOrigin, zOrigin); 

      
      Content = m_Response->GetPairResponse(AngleTrans); //*m_AreaBin[x2]*Integral;

      if (Content > 0.0) {
        if (Maximum < Content) Maximum = Content;
        InnerSum += Content;

        Image[NUsedBins] = Content;
        Bins[NUsedBins] = i;
        ++NUsedBins;
      }
    }
  }
  
  if (InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }

  return true;
}


// MBackprojectionFarField: the end...
////////////////////////////////////////////////////////////////////////////////
