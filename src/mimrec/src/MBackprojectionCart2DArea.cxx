/*
 * MBackprojectionCart2DArea.cxx
 *
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
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
// MBackprojectionCart2DArea
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionCart2DArea.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBackprojectionCart2DArea)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart2DArea::MBackprojectionCart2DArea()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCart2DArea::~MBackprojectionCart2DArea()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart2DArea::Backproject(MPhysicalEvent* Event, double *Image, 
                                              double &Limit, int &NAboveLimit)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone. 

  if (MBackprojectionCart2D::Assimilate(Event) == false) return false;

  if (m_EventType == 1) {
    return BackprojectionCompton(Image, Limit, NAboveLimit);
  } else if (m_EventType == 2) {
    return BackprojectionPair(Image, Limit, NAboveLimit);
  }
  else if (m_EventType == 5) {
    Fatal("bool MBackprojectionCart2DArea::Backproject(...)",
          "Cart2DArea only works for Comptons.");
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCart2DArea::PrepareBackprojection()
{
  MBackprojectionCart2D::PrepareBackprojection();

  // Now compute for all bins their center:
  m_xBinCenter = new double[m_x1NBins];
  for (int k = 0; k < m_x1NBins; ++k) {
    m_xBinCenter[k] = m_x1Min + (0.5+k)*m_x1IntervalLength;
    //cout<<m_xBinCenter[k]<<endl;
  }

  m_yBinCenter = new double[m_x2NBins];
  for (int k = 0; k < m_x2NBins; ++k) {
    m_yBinCenter[k] = m_x2Min + (0.5+k)*m_x2IntervalLength;
    //cout<<m_yBinCenter[k]<<endl;
  }

  m_zBinCenter = new double[m_x3NBins];
  for (int k = 0; k < m_x3NBins; ++k) {
    m_zBinCenter[k] = m_x3Min + (0.5+k)*m_x3IntervalLength;
    //cout<<m_yBinCenter[k]<<endl;
  }

  //m_zBinCenter = 0.5*(m_x3Min + m_x3Max);
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart2DArea::BackprojectionCompton(double *Image, double& Limit, int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  // Let the response get the event data:
  m_Response->AnalyzeEvent(m_C);  // <- not the correct place...

  int x, y, z, i, j, n;
  double AngleTrans, AngleLong;
  double InnerSum = 0;
  double ConeRadius = 0;

  double Maximum = 0;
  
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

  // The origin of the gamma ray:
  double xOrigin = -m_C->De().X();
  double yOrigin = -m_C->De().Y();
  double zOrigin = -m_C->De().Z();
  Rotate(xOrigin, yOrigin, zOrigin);

  double xBin, yBin, zBin;
  double xn1, yn1, zn1, xn2, yn2, zn2;


  // Important pre-computations
  double tanphi = tan(m_C->Phi());

  //double Threshold = 0.001;
  
  // Min and max Trans-angle above threshold:
  double Trans_max = m_Response->GetComptonTransversalMax();
  double Trans_min = m_Response->GetComptonTransversalMin();
  
  double L;

  // Later we need the angle between the cone center and the gauss-maximum:
  //double AngleA = m_Maths.AngleBetweenTwoVectors(xOrigin, yOrigin, zOrigin,
  //                                        xCA, yCA, zCA);


  // Precompute some correction factors:
  
  // The minimum electron angle error
  //double EAngleError = fabs(AngleA - m_C->Phi());

  //cout<<m_C->ToString()<<endl;


  // ---------> time critical --------->

  // Start the backprojections
  //if (true) { //m_C->HasTrack() == false) {
  if (m_C->HasTrack() == false) {

    // Without track:
    for (z = 0; z < m_x3NBins; z++) {
      for (y = 0; y < m_x2NBins; y++) {
        for (x = 0; x < m_x1NBins; x++) {
          i = x+y*m_x1NBins+z*m_x1NBins*m_x2NBins; // We fill each x-row
        
          L = sqrt((m_xBinCenter[x]-xCC)*(m_xBinCenter[x]-xCC) + 
                   (m_yBinCenter[y]-yCC)*(m_yBinCenter[y]-yCC) +
                   (m_zBinCenter[z]-zCC)*(m_zBinCenter[z]-zCC));

          AngleTrans = Angle(m_xBinCenter[x]-xCC, m_yBinCenter[y]-yCC, m_zBinCenter[z]-zCC, xCA, yCA, zCA)- m_C->Phi();
          

          // Skip pixels below threshold:
          if (AngleTrans > Trans_max) {
            n = (int) floor(L*sin(AngleTrans - Trans_max)/m_x1IntervalLength);
            for (j = 0; j < n; j++) {
              Image[i+j] = 0;
              x++;
              if (x == m_x1NBins) break;
            }
            continue;
          } else if (AngleTrans < Trans_min) {
            n = (int) floor(L*sin(Trans_min - AngleTrans)/m_x1IntervalLength);
            for (j = 0; j < n; j++) {
              Image[i+j] = 0;
              x++;
              if (x == m_x1NBins) break;
            }
            continue;
          }
        
          ConeRadius = fabs(tanphi*cos(AngleTrans)*L);

          Image[i] = m_Response->GetComptonResponse(AngleTrans)/ConeRadius;

          // search the highest pixel...
          if (Image[i] > Maximum) {
            Maximum = Image[i];
          }
          InnerSum += Image[i];
        } // end x
      } // end y
    } // end z
  } else { // Has track...
    // With track
    for (z = 0; z < m_x3NBins; z++) {
      for (y = 0; y < m_x2NBins; y++) {
        for (x = 0; x < m_x1NBins; x++) {
          i = x+y*m_x1NBins+z*m_x1NBins*m_x2NBins; // We fill each x-row
    
          xBin = m_xBinCenter[x]-xCC;
          yBin = m_yBinCenter[y]-yCC;
          zBin = m_zBinCenter[z]-zCC;
          
          L = sqrt(xBin*xBin + yBin*yBin + zBin*zBin);
          
          //AngleTrans = Angle(xBin, yBin, zBin, xCA, yCA, zCA);
          AngleTrans = Angle(xBin, yBin, zBin, xCA, yCA, zCA) - m_C->Phi();
          
          /*
          // Skip pixels below threshold:
          if (AngleTrans > Trans_max) {
          n = (int) floor(L*sin(AngleTrans - Trans_max)/m_x1IntervalLength);
          for (j = 0; j < n; j++) {
          Image[i+j] = 0;
          x++;
          if (x == m_x1NBins) break;
          }
          continue;
          } else if (AngleTrans < Trans_min) {
          n = (int) floor(L*sin(Trans_min - AngleTrans)/m_x1IntervalLength);
          for (j = 0; j < n; j++) {
          Image[i+j] = 0;
          x++;
          if (x == m_x1NBins) break;
          }
          continue;
          }
          */
          
          // Can be zero:
          //ConeRadius = fabs(tanphi*cos(AngleTrans-m_C->Phi())*L);
          ConeRadius = fabs(tanphi*cos(AngleTrans)*L);
          
          // angle between two planes spanned by ...
          xn1 = yBin*zCA - zBin*yCA;
          yn1 = (zBin*xCA - xBin*zCA);
          zn1 = xBin*yCA - yBin*xCA;
    
          xn2 = (yOrigin*zCA - zOrigin*yCA);
          yn2 = (zOrigin*xCA - xOrigin*zCA);
          zn2 = (xOrigin*yCA - yOrigin*xCA);
          
          AngleLong = acos((xn1*xn2+yn1*yn2+zn1*zn2)/sqrt((xn1*xn1+yn1*yn1+zn1*zn1)*(xn2*xn2+yn2*yn2+zn2*zn2)));
    
          //cout<<i<<": "<<ConeRadius<<"!"<<tanphi<<"!"<<AngleTrans<<"!"<<L<<"-->"<<m_Response->GetResponse(AngleTrans, AngleLong)/ConeRadius<<endl;

          // Sample the 2d-Gauss-function: m_AreaBin[t] is missing ---> new geometry is missing!!!
          Image[i] = m_Response->GetComptonResponse(AngleTrans, AngleLong)/ConeRadius;

          /*
            if (AngleLong > EAngleError) {
            Image[i] = m_Response->GetResponse(AngleTrans, AngleLong - EAngleError)/ConeRadius;
            } else {
            Image[i] = m_Response->GetResponse(AngleTrans, 0)/ConeRadius;
            }        
          */
          if (Image[i] > Maximum) {
            Maximum = Image[i];
          }
          InnerSum += Image[i];
        }
      }
    }
  }
    
  // <--------- time critical <---------

  // If one of the above computed numbers has been NaN
  // InnerSum is also NaN. Then we have to reject this event.
  if (m_Maths.InRange(InnerSum) == false) {
    // This error occurs when we have an error in Sivan ...
    // ---> To be solved!!!
    //Warning("bool MBackprojectionSphereArea::BackprojectionCompton(...)",
    //   "NaN");
    return false;
  }
  //cout<<"Innersum: "<<InnerSum<<endl;

  // Probleme wegen des /ConeRadius !!
  Limit = 0.01*Maximum; //Threshold*m_Response->GetMaximum();


  // We only store floats, thus that's the real limits
  if (Limit < numeric_limits<float>::min()) {
    Limit = 0;
    NAboveLimit =0;
    return false;
  }

  for (i = 0; i < m_NImageBins; i++) {
    //Image[t] *= Corr;
    //Image[i] /= InnerSum;
    if (Image[i] > Limit) NAboveLimit++;
  }

  if (NAboveLimit == 0) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCart2DArea::BackprojectionPair(double *Image, double& Limit, int& NAboveLimit)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit 
  // NABoveLimit      :

  // Tronsform the gamma direction to spherical coordinates:
  //double theta, phi, radius;

  double xOrigin = m_P->m_IncomingGammaDirection.X();
  double yOrigin = m_P->m_IncomingGammaDirection.Y();
  double zOrigin = m_P->m_IncomingGammaDirection.Z();
  Rotate(xOrigin, yOrigin, zOrigin);

  double xIA = m_P->GetPairCreationIA().X();
  double yIA = m_P->GetPairCreationIA().Y();
  double zIA = m_P->GetPairCreationIA().Z();
  Rotate(xIA, yIA, zIA);

  double MaxValue = 0.0;
  double InnerSum = 0.0;
  double AngleTrans;
  int i;

  // Let's fit a double gausshaped pair-event
  for (int z = 0; z < m_x3NBins; ++z) {
    for (int y = 0; y < m_x2NBins; ++y) {
      for (int x = 0; x < m_x1NBins; ++x) {
        i = x+y*m_x1NBins+z*m_x1NBins*m_x2NBins; // We fill each x-row

        AngleTrans = Angle(m_xBinCenter[x]-xIA, 
                           m_yBinCenter[y]-yIA, 
                           m_zBinCenter[z]-zIA, 
                           xOrigin, yOrigin, zOrigin); //; - m_C->Phi();
        
        Image[i] = m_Response->GetPairResponse(AngleTrans);

        if (MaxValue < Image[i]) MaxValue = Image[i];
        
        InnerSum += Image[i];
      }
    }
  }
  
  if (m_Maths.InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }
  
  if (InnerSum < 0.00001) {
    // Event not inside the viewport
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


bool MBackprojectionCart2DArea::Assimilate(MPhysicalEvent *Event)
{
  return MBackprojectionCart2D::Assimilate(Event);
}



////////////////////////////////////////////////////////////////////////////////


double MBackprojectionCart2DArea::Angle(double u, double v, double w, 
                                        double x, double y, double z)
{
  double Nenner = sqrt((u*u + v*v + w*w)*(x*x + y*y + z*z));

  if (Nenner <= 0.0) {
    return 0.0;
  } else {
    Nenner = (u*x + v*y + w*z)/Nenner;
    if (Nenner > 1.0) Nenner = 1.0;
    if (Nenner < -1.0) Nenner = -1.0;
    return acos(Nenner);
  }
}

// MBackprojectionCart2DArea.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
