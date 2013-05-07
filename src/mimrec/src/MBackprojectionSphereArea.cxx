/*
 * MBackprojectionSphereArea.cxx
 *
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
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
// MBackprojectionSphereArea
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionSphereArea.h"

// Standard libs:
#include <iostream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBackprojectionSphereArea)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionSphereArea::MBackprojectionSphereArea()
{
  // standard constructor
  
  // Set ll pointers to zero
  m_AreaBin = 0;
  m_xBin = 0;
  m_yBin = 0;
  m_zBin = 0;
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionSphereArea::~MBackprojectionSphereArea()
{
  // Delete a MBackprojectionSphereArea-object

  delete [] m_AreaBin;
  delete [] m_xBin;
  delete [] m_yBin;
  delete [] m_zBin;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionSphereArea::PrepareBackprojection()
{
  // Make some initial computations to prepare the backprojection:

  MBackprojectionSphere::PrepareBackprojection();

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
  mout<<"Image Area: "<<TotalArea*m_x1NBins<<" sr"<<endl;
  mout<<"Average Area: "<<TotalArea/m_x2NBins<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::Backproject(MPhysicalEvent* Event, 
                                              double* Image)
{
  cout<<"MBackprojectionSphereArea::Backproject(const MPhysicalEventData" 
    "*EventData, double *Image): "
    "No longer implemented!"<<endl;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::Backproject(MPhysicalEvent* Event, 
                                            double* Image, 
                                            double& Limit, 
                                            int& NAboveLimit)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone. 

  cout<<"Event: "<<Event->GetId()<<":"<<m_EventType<<endl;

  if (Assimilate(Event) == false) return false;

  mout<<"Non uniform event type definitions..."<<show;
  if (m_EventType == 1) {
    return BackprojectionCompton(Image, Limit, NAboveLimit);
  } else if (m_EventType == 2) {
    return BackprojectionPair(Image, Limit, NAboveLimit);
  } else if (m_EventType == 3) {
    return BackprojectionPhoto(Image, Limit, NAboveLimit);
  } else {
    merr<<"Unknown event type for imaging: "<<m_EventType<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::BackprojectionCompton(double* Image, 
                                                      double& Limit, 
                                                      int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  // Obviously in the beginning no bins are above the threshold to use the bin...
  NAboveLimit = 0;

  int i, j, n;
  double Temp;

  // Direction to the cone center towards the sky:
  double xCC = -m_C->Dg().X()*m_x3BinCenter[0];
  double yCC = -m_C->Dg().Y()*m_x3BinCenter[0];
  double zCC = -m_C->Dg().Z()*m_x3BinCenter[0]; 
  //   xCC = 0*m_x3BinCenter[0]; // for debugging detector rotations
  //   yCC = 0*m_x3BinCenter[0];
  //   zCC = 1*m_x3BinCenter[0];
  Rotate(xCC, yCC, zCC);

  // The origin of the event on the sky:
  double xOrigin = m_C->DiOnCone().X()*m_x3BinCenter[0];
  double yOrigin = m_C->DiOnCone().Y()*m_x3BinCenter[0];
  double zOrigin = m_C->DiOnCone().Z()*m_x3BinCenter[0];
  Rotate(xOrigin, yOrigin, zOrigin);

  // The Compton scatter angle:
  double Phi = m_C->Phi();
  //   Phi = 0.01; // for debugging detector rotations

  double MaxValue = 0.0;


  // Determine the maximum of the gauss-function on the circle:

  // Min and max Trans-angle above threshold:
  double Trans_min = m_Response->GetComptonTransversalMin();
  double Trans_max = m_Response->GetComptonTransversalMax();


  // Now determine for all Bins
  // A. the smallest angle between the center of the bin and the circle
  // B. the smallest angle between the nearest point on the circle and the 
  //    maximum of the gauss-function on the circle
  double AngleLong, AngleTrans;
  double AngleA, AngleB, AngleC;
  double Sum = 0;
  double InnerSum = 0.0;

  // Later we need the angle between the cone center and the gauss-maximum:
  AngleA = m_Maths.AngleBetweenTwoVectors(xOrigin, yOrigin, zOrigin,
                                          xCC, yCC, zCC);

  // Precompute some correction factors:
  
  // The minimum electron angle error
  //double EAngleError = fabs(AngleA - m_C->Phi());

  double InvIntegral = 1.0/m_Response->GetComptonIntegral(Phi);
  //cout<<"Integral calc: "<<1.0/Integral<<endl;

  //mimp<<"InvIntegral is only part of the response!"<<show;

  // ---------> time critical --------->

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

  if (m_C->HasTrack() == false) {

    // Without track
    for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
      for (int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
        i = x1+x2*m_x1NBins;   // Loop over all image bins:

        // ad A.:
        // the angle we search is the angle between the center of the cone and the bin center 
        // minus the angle of the cone 
        // Therefore its the (smallest) angle between the bin center and the eventcircle:
        AngleTrans = Angle(m_xBin[i], m_yBin[i], m_zBin[i], xCC, yCC, zCC) - Phi;
        
        // Skip empty pixels
        if (AngleTrans > Trans_max) {
          n = int((AngleTrans - Trans_max)/m_x1IntervalLength);
          for (j = 0; j < n; j++) {
            Image[i+j] = 0;
            ++x1;
            if (x1 == m_x1NBins) break;
          }
          continue;
        } else if (AngleTrans < Trans_min) {
          n = int((Trans_min - AngleTrans)/m_x1IntervalLength);
          for (j = 0; j < n; j++) {
            Image[i+j] = 0;
            ++x1;
            if (x1 == m_x1NBins) break;
          }
          continue;
        } 
        
        Image[i] = m_Response->GetComptonResponse(AngleTrans)*InvIntegral; //*m_AreaBin[x2];
        //Image[i] = m_AreaBin[x2];

        // Now take into account absorption probabilities:
        if (m_Geometry != 0) {
          double rx = m_xBin[i];
          double ry = m_yBin[i];
          double rz = m_zBin[i];
          Rotate(rx, ry, rz);
          double Abs = m_Geometry->GetAbsorptionProbability(m_C->C1(), -MVector(rx, ry, rz), m_C->Ei());
          Image[i] *= (1-Abs); 
        }

        //if (MaxValue < Image[i]) MaxValue = Image[i];
        
        InnerSum += Image[i];
      }
    }
  } else {
    // With track
    for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
      for (int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
        i = x1+x2*m_x1NBins; // image bin i

        // ad A.:
        // the angle we search is the angle between the center of the cone and the bin center 
        // minus the angle of the cone 
        AngleB = Angle(m_xBin[i], m_yBin[i], m_zBin[i], xCC, yCC, zCC);
        AngleTrans = AngleB - Phi;

        // Skip empty pixels
        if (AngleTrans > Trans_max) {
          n = (int) ((AngleTrans - Trans_max)/m_x1IntervalLength);
          for (j = 0; j < n; ++j) {
            Image[i+j] = 0;
            x1++;
            if (x1 == m_x1NBins) break;
          }
          continue;
        } else if (AngleTrans < Trans_min) {
          n = (int) ((Trans_min - AngleTrans)/m_x1IntervalLength);
          for (j = 0; j < n; ++j) {
            Image[i+j] = 0;
            x1++;
            if (x1 == m_x1NBins) break;
          }
          continue;
        } 
        // A skip empty pixel via AngleLong is still missing
        
        // ad B.:
        // the angle we search is the angle between these points:
        // center of the cone, center of the bin and gauss-maximum
        // This is a spherical triangle and the angle can be solved with the "Halbwinkelsatz"

        AngleC = Angle(m_xBin[i], m_yBin[i], m_zBin[i], xOrigin, yOrigin, zOrigin);
        Sum = (AngleA + AngleB + AngleC)*0.5;
        Temp = sin(Sum-AngleA)*sin(Sum-AngleB)/(sin(Sum)*sin(Sum-AngleC));
        AngleLong = 2 * atan(sqrt(fabs(Temp)));
        
        // Sample the 2d-Gauss-function
        Image[i] = m_Response->GetComptonResponse(AngleTrans, AngleLong)*InvIntegral; //*m_AreaBin[x2];

        // Apply absorption probabilities:
        if (m_Geometry != 0) {
          double rx = m_xBin[i];
          double ry = m_yBin[i];
          double rz = m_zBin[i];
          Rotate(rx, ry, rz);
          double Abs = m_Geometry->GetAbsorptionProbability(m_C->C1(), -MVector(rx, ry, rz), m_C->Ei());
          Image[i] *= (1-Abs); 
        }

        InnerSum += Image[i];
      }
    }
  }

  // cout<<"InnerSum:"<<InnerSum<<" compared to : "<<m_Response->GetComptonIntegral(Phi)<<endl;

  // If one of the above computed number has been NaN
  // InnerSum is also NaN. Then we have to reject this event.
  if (m_Maths.InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }

  //cout<<"Integral summed: "<<InnerSum<<":"<<Phi<<"  Error:"<<1/InnerSum<<endl;

  // Save our results: Fill the matrix:
  double Corr = 1;
  
 
  // Not yet perfect: *m_AreaBin[0]; not ideal
  MaxValue = m_Response->GetComptonMaximum()*m_Response->GetThreshold(); // *m_AreaBin[0];


  // Test if Image can be saved as a sparse matrix:
  NAboveLimit = 0;
  Limit = MaxValue * (1.0 - 0.894141);
  // 1.0 sigma :  * (1.0 - 0.632215); 
  // 1.5 sigma :  * (1.0 - 0.894141);
  // 2.0 sigma :  * (1.0 - 0.981645);
  // 2.5 sigma :  * (1.0 - 0.998066);
  // 3.0 sigma :  * (1.0 - 0.999873);
  // 3.5 sigma :  * (1.0 - 0.999971);

  Limit *= Corr;
  //Limit = 0.0;

  // We only store floats, thus that's the real limits
  if (Limit < numeric_limits<float>::min()) {
    Limit = 0;
    NAboveLimit =0;
    return false;
  }

  // Now the first bin is the waste bin:
  for (i = 0; i < m_NImageBins; ++i) {
    if (Image[i] >= Limit) {
      ++NAboveLimit;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MBackprojectionSphereArea::Angle(double u, double v, double w, 
                                        double x, double y, double z)
{
 
  double Nenner = m_InvSquareDist*(u*x + v*y + w*z);

  if (Nenner > 1.0) Nenner = 1.0;
  if (Nenner < -1.0) Nenner = -1.0;

  return acos(Nenner);
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::BackprojectionPhoto(double* Image, 
                                                    double& Limit, 
                                                    int& NAboveLimit)
{
  // Backprojection of a photo events

  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  mout<<"Never tested and stupid..."<<endl;

//   int i;
//   double MaxValue = 0.0;
//   double InnerSum = 0.0;

//   // Let's fit a double gausshaped pair-event
//   for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
//     for (int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
//       i = x1+x2*m_x1NBins; // image bin i
      
//       // Sample the 2d-Gauss-function
//       //Image[i] = m_ResponsePhoto->GetResponse(m_x1BinCenter[p], m_x2BinCenter[t]);
//       Image[i] = m_ResponsePhoto->GetResponse(m_x2BinCenter[x2], m_x1BinCenter[x1]);
      

//       if (MaxValue < Image[i]) MaxValue = Image[i];
      
//       InnerSum += Image[i];
//     }
//   }
  
// //   if (InnerSum < 0.00001) {
// //     // Event not inside the viewport
// //     return false;
// //   }

// //   // Test if Image can be saved as a sparse matrix:
// //   NAboveLimit = 0;
// //   Limit = MaxValue * (1.0 - 0.981645);
// //   // 1.0 sigma :  * (1.0 - 0.632215); 
// //   // 1.5 sigma :  * (1.0 - 0.894141);
// //   // 2.0 sigma :  * (1.0 - 0.981645);
// //   // 2.5 sigma :  * (1.0 - 0.998066);
// //   // 3.0 sigma :  * (1.0 - 0.999873);
// //   // 3.5 sigma :  * (1.0 - 0.999971);

//   for (i = 0; i < m_NImageBins; ++i) {
//     if (Image[i] >= Limit) {
//       NAboveLimit++;
//     }
//   }

  double Energy = m_Photo->GetEnergy();
  Energy = 50;

  MVector Position = m_Photo->GetPosition();
//   Position[0] += gRandom->Rndm()-0.5;
//   Position[1] += gRandom->Rndm()-0.5;
  Position[2] += 0.5;

  MVector SkyDir;

  int t, i;
  double MaxValue = 0.0;
  double InnerSum = 0.0;

  // Let's fit a double gausshaped pair-event
  for (int x2 = 0; x2 < m_x2NBins; ++x2) { // x2 == theta
    for (int x1 = 0; x1 < m_x1NBins; ++x1) { // x1 == phi
      i = x1+x2*m_x1NBins; // image bin i
      
      Image[i] = 1; //m_Photo->GetWeight();
      if (m_Geometry != 0) {
        double rx = m_xBin[i];
        double ry = m_yBin[i];
        double rz = m_zBin[i];
        //cout<<m_x2BinCenter[x2]*c_Deg<<":"<<m_x1BinCenter[x1]*c_Deg<<":  "<<m_Photo->GetPosition()<<":"<<MVector(rx, ry, rz)<<endl;
        Rotate(rx, ry, rz);
        //cout<<m_x2BinCenter[x2]*c_Deg<<":"<<m_x1BinCenter[x1]*c_Deg<<":  "<<m_Photo->GetPosition()<<":"<<MVector(rx, ry, rz)<<endl;

        SkyDir = -MVector(rx, ry, rz) - Position;
        SkyDir.Unitize();

        double Abs = m_Geometry->GetAbsorptionProbability(Position, Position + 100*SkyDir, Energy);
        //cout<<"Abs: "<<Abs<<":"<<Position<<":"<<-MVector(rx, ry, rz)<<endl;
        Image[i] *= (1-Abs); 
      } else {
        cout<<"Error: Geometry needed!"<<endl;
      }
      
      if (MaxValue < Image[i]) MaxValue = Image[i];
      
      InnerSum += Image[i];
    }
  }
  
//   if (InnerSum < 0.00001) {
//     // Event not inside the viewport
//     return false;
//   }

  // Test if Image can be saved as a sparse matrix:
  NAboveLimit = 0;
  //Limit = MaxValue * (1.0 - 0.981645);
  Limit = 0.0;
  // 1.0 sigma :  * (1.0 - 0.632215); 
  // 1.5 sigma :  * (1.0 - 0.894141);
  // 2.0 sigma :  * (1.0 - 0.981645);
  // 2.5 sigma :  * (1.0 - 0.998066);
  // 3.0 sigma :  * (1.0 - 0.999873);
  // 3.5 sigma :  * (1.0 - 0.999971);

  for (t = 0; t < m_NImageBins; t++) {
    if (Image[t] >= Limit) NAboveLimit++;
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::BackprojectionPair(double* Image, 
                                                   double& Limit, 
                                                   int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  //cout<<"Now backprojecting a pair-event..."<<endl;

 // The origin of the event on the sky:
  double xOrigin = m_P->m_IncomingGammaDirection.X()*m_x3BinCenter[0];
  double yOrigin = m_P->m_IncomingGammaDirection.Y()*m_x3BinCenter[0];
  double zOrigin = m_P->m_IncomingGammaDirection.Z()*m_x3BinCenter[0];
  Rotate(xOrigin, yOrigin, zOrigin);
  // --> The rotation should be performed during assimilation

  //double Integral = 1.0/m_Response->GetPairIntegral();

  double MaxValue = 0.0;
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
      AngleTrans = 
        m_Maths.AngleBetweenTwoVectors(m_xBin[i], m_yBin[i], m_zBin[i], 
                                       xOrigin, yOrigin, zOrigin); 

      
      Image[i] = m_Response->GetPairResponse(AngleTrans); //*m_AreaBin[x2]*Integral;

      if (MaxValue < Image[i]) MaxValue = Image[i];
      
      InnerSum += Image[i];
    }
  }
  
  if (m_Maths.InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }

  // Test if Image can be saved as a sparse matrix:
  NAboveLimit = 0;
  Limit = MaxValue * (1.0 - 0.981645);
  // 1.0 sigma :  * (1.0 - 0.632215); 
  // 1.5 sigma :  * (1.0 - 0.894141);
  // 2.0 sigma :  * (1.0 - 0.981645);
  // 2.5 sigma :  * (1.0 - 0.998066);
  // 3.0 sigma :  * (1.0 - 0.999873);
  // 3.5 sigma :  * (1.0 - 0.999971);

  for (i = 0; i < m_NImageBins; ++i) {
    if (Image[i] >= Limit) {
      ++NAboveLimit;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionSphereArea::Assimilate(MPhysicalEvent *Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray, 
  // the intersection of the cone axis with the sphere. 
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  return MBackprojectionSphere::Assimilate(Event);
}


// MBackprojectionSphereArea: the end...
////////////////////////////////////////////////////////////////////////////////
