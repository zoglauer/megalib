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


#ifdef ___CINT___
ClassImp(MBackprojectionFarField)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionFarField::MBackprojectionFarField(MCoordinateSystem CoordinateSystem) : MBackprojection(CoordinateSystem)
{
  // Initialize a MBackprojectionFarField object

  m_x1BinCenter = 0;  
  m_x2BinCenter = 0;    
  m_x3BinCenter = 0;

  m_AreaBin = 0;

  m_xBin = 0;
  m_yBin = 0;
  m_zBin = 0;
  
  // We only have one bin in x3 (= radius) dimension - in a galaxy far, far away (32.4 Mio pc) ;-)
  m_x3Min = 0.99*c_FarAway;
  m_x3Max = 1.00*c_FarAway;
  m_x3NBins = 1;

  // We can initialize x3 here since it is definitely only one bin...
  m_x3IntervalLength = (m_x3Max - m_x3Min)/m_x3NBins;
  delete [] m_x3BinCenter;
  m_x3BinCenter  = new double[m_x3NBins];
  for (int k = 0; k < m_x3NBins; ++k) {
    m_x3BinCenter[k] = m_x3Min + (0.5+k)*m_x3IntervalLength;
  }

  m_XAxis.SetXYZ(1.0, 0.0, 0.0);
  m_ZAxis.SetXYZ(0.0, 0.0, 1.0);

  m_Rotation.ResizeTo(3,3);
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionFarField::~MBackprojectionFarField()
{
  // Delete a MBackprojectionFarField object

  delete [] m_x1BinCenter;
  delete [] m_x2BinCenter;
  delete [] m_x3BinCenter;
  delete [] m_AreaBin;

  delete [] m_xBin;
  delete [] m_yBin;
  delete [] m_zBin;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionFarField::SetViewportDimensions(double x1Min, double x1Max, int x1NBins, 
                                                  double x2Min, double x2Max, int x2NBins, 
                                                  double x3Min, double x3Max, int x3NBins,
                                                  MVector xAxis, MVector zAxis)
{
  // Set the dimensions of the viewport (minimum and maximum x and y-values, 
  // number of bins) - ignore the z-values
  // x1 = phi, x2 = theta, x3 = radius

  massert(x1Max > x1Min);
  //massert(x2Max > x2Min);
  massert(x3Max > x3Min);

  massert(x1NBins > 0);
  massert(x2NBins > 0);
  massert(x3NBins > 0);


  m_x1Min = x1Min;
  m_x1Max = x1Max;
  m_x1NBins = x1NBins;
  m_x2Min = x2Min;
  m_x2Max = x2Max;
  m_x2NBins = x2NBins;


  m_x1IntervalLength = (m_x1Max - m_x1Min)/m_x1NBins;
  m_x2IntervalLength = (m_x2Max - m_x2Min)/m_x2NBins;

  //mout<<"BS: x1: "<<m_x1Min*c_Deg<<"-"<<m_x1Max*c_Deg<<" in "<<m_x1NBins<<" --> "<<m_x1IntervalLength*c_Deg<<endl;
  //mout<<"BS: x2: "<<m_x2Min*c_Deg<<"-"<<m_x2Max*c_Deg<<" in "<<m_x2NBins<<" --> "<<m_x2IntervalLength*c_Deg<<endl;

  m_NImageBins = m_x1NBins*m_x2NBins*m_x3NBins;

  // Now compute for all bins their center:
  delete [] m_x1BinCenter;
  m_x1BinCenter = new double[m_x1NBins];
  for (int k = 0; k < m_x1NBins; ++k) {
    m_x1BinCenter[k] = m_x1Min + (0.5+k)*m_x1IntervalLength;
  }

  delete [] m_x2BinCenter;
  m_x2BinCenter  = new double[m_x2NBins];
  for (int k = 0; k < m_x2NBins; ++k) {
    m_x2BinCenter[k] = m_x2Min + (0.5+k)*m_x2IntervalLength;
  }

  
  // Create Rotation:
  m_XAxis = xAxis;
  m_ZAxis = zAxis;


  // First compute the y-Axis vector:
  MVector yAxis = zAxis.Cross(xAxis);
  
  //cout<<"RotVector x: "<<m_RotationXAxis.X()<<"!"<<m_RotationXAxis.Y()<<"!"<<m_RotationXAxis.Z()<<endl;
  //cout<<"RotVector y: "<<m_RotationYAxis.X()<<"!"<<m_RotationYAxis.Y()<<"!"<<m_RotationYAxis.Z()<<endl;
  //cout<<"RotVector z: "<<m_RotationZAxis.X()<<"!"<<m_RotationZAxis.Y()<<"!"<<m_RotationZAxis.Z()<<endl;

  m_Rotation(0,0) = xAxis.X();
  m_Rotation(1,0) = xAxis.Y();
  m_Rotation(2,0) = xAxis.Z();
  m_Rotation(0,1) = yAxis.X();
  m_Rotation(1,1) = yAxis.Y();
  m_Rotation(2,1) = yAxis.Z();
  m_Rotation(0,2) = zAxis.X();
  m_Rotation(1,2) = zAxis.Y();
  m_Rotation(2,2) = zAxis.Z();

  return;
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

  for (int x1 = 0; x1 < m_x1NBins; ++x1) { // phi!
    for (int x2 = 0; x2 < m_x2NBins; ++x2) { //theta!
      ToCartesean(m_x2BinCenter[x2], m_x1BinCenter[x1], m_x3BinCenter[0], 
                  m_xBin[x1+x2*m_x1NBins], m_yBin[x1+x2*m_x1NBins], m_zBin[x1+x2*m_x1NBins]);
    }
  }

  m_InvSquareDist = 1.0/(m_x3BinCenter[0]*m_x3BinCenter[0]);


  // Steradian per bin normalization...
  delete [] m_AreaBin;
  m_AreaBin = new double[m_x2NBins];
  double TotalArea = 0.0;
  for (int x2 = 0; x2 < m_x2NBins; x2++) { // theta
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


void MBackprojectionFarField::Rotate(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coodinate system

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

  int index;
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
  Rotate(xCC, yCC, zCC);
  ToSpherical(xCC, yCC, zCC, m_ThetaConeCenter, m_PhiConeCenter, m_RadiusConeCenter);

  // The origin of the event on the sky:
  double xOrigin = m_C->DiOnCone().X()*m_x3BinCenter[0];
  double yOrigin = m_C->DiOnCone().Y()*m_x3BinCenter[0];
  double zOrigin = m_C->DiOnCone().Z()*m_x3BinCenter[0];
  Rotate(xOrigin, yOrigin, zOrigin);

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

  //double InvIntegral = 1.0/m_Response->GetComptonIntegral(Phi);
  double InvIntegral = 1.0;

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

  for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
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
    for (int x1 = 0; x1 < m_x1NBins; ++x1) {
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
        Temp = Sin(Sum-AngleA)*Sin(Sum-AngleB)/(Sin(Sum)*Sin(Sum-AngleC));
        AngleLong = 2 * atan(sqrt(fabs(Temp)));

        // Sample the 2d-Gauss-function
        Content = m_Response->GetComptonResponse(AngleTrans, AngleLong)*InvIntegral; //*m_AreaBin[x2];
      }


      if (Content > 0.0) {

        // Now take into account absorption probabilities:
        if (m_UseAbsorptions == true) {
          double rx = m_xBin[index];
          double ry = m_yBin[index];
          double rz = m_zBin[index];
          Rotate(rx, ry, rz);
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
        if (x1 == x1End2) {
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

  int i;
  double InnerSum = 0.0;

  // We are going over all bins here...
  for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
    for (int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
      i = x1+x2*m_x1NBins; // image bin i
      
      Image[NUsedBins] = 1; //m_Photo->GetWeight();
      Bins[NUsedBins] = i;

      if (m_Geometry != 0) {
        double rx = m_xBin[i]-Position.X();
        double ry = m_yBin[i]-Position.Y();
        double rz = m_zBin[i]-Position.Z();
        //cout<<m_x2BinCenter[x2]*c_Deg<<":"<<m_x1BinCenter[x1]*c_Deg<<":  "<<m_Photo->GetPosition()<<":"<<MVector(rx, ry, rz)<<endl;
        Rotate(rx, ry, rz);
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
  Rotate(xOrigin, yOrigin, zOrigin);
  // --> The rotation should be performed during assimilation

  //double Integral = 1.0/m_Response->GetPairIntegral();

  double Content = 0.0;
  double InnerSum = 0.0;
  double AngleTrans;
  int i;

  // Let's fit a double gausshaped pair-event
  for (int x2 = 0; x2 < m_x2NBins; x2++) { // x2 == theta
    for (int x1 = 0; x1 < m_x1NBins; x1++) { // x1 == phi
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
