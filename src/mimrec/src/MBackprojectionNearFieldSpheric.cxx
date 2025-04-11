/*
 * MBackprojectionNearFieldSpheric.cxx
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
// MBackprojectionNearFieldSpheric
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionNearFieldSpheric.h"

// Standard libs:
#include <iomanip>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackprojectionNearFieldSpheric)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionNearFieldSpheric::MBackprojectionNearFieldSpheric(MCoordinateSystem CoordinateSystem) : MBackprojection(CoordinateSystem)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionNearFieldSpheric::~MBackprojectionNearFieldSpheric()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearFieldSpheric::Assimilate(MPhysicalEvent* Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray,
  // and here: the intersection of the cone axis with the sphere.
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  if (MBackprojection::Assimilate(Event) == false) {
    return false;
  }

  // Those are the event types we handle:
  if (Event->GetType() == MPhysicalEvent::c_Compton ||
    Event->GetType() == MPhysicalEvent::c_Pair ||
    Event->GetType() == MPhysicalEvent::c_Photo ||
    Event->GetType() == MPhysicalEvent::c_PET ||
    Event->GetType() == MPhysicalEvent::c_Multi) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearFieldSpheric::Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray
  // if possible, the center of the cone.

  if (MBackprojectionNearFieldSpheric::Assimilate(Event) == false) return false;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    return BackprojectionCompton(Image, Bins, NUsedBins, Maximum);
  //} else if (Event->GetType() == MPhysicalEvent::c_Pair) {
  //  return BackprojectionPair(Image, Bins, NUsedBins, Maximum);
  //} else if (Event->GetType() == MPhysicalEvent::c_PET) {
  //  return BackprojectionPET(Image, Bins, NUsedBins, Maximum);
  //} else if (Event->GetType() == MPhysicalEvent::c_Multi) {
  //  return BackprojectionMulti(Image, Bins, NUsedBins, Maximum);
  } else  {
    cout<<"Near-field backprojection spheric does not yet work for this event type: "<<Event->GetTypeString()<<endl;
  }

  return false;
}


///////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearFieldSpheric::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins,
                                                    double x2Min, double x2Max, unsigned int x2NBins,
                                                    double x3Min, double x3Max, unsigned int x3NBins,
                                                    MVector xAxis, MVector zAxis)
{
  // We cannot handle a viewport rotation here, so set it to the defaults
  return MViewPort::SetDimensions(x1Min, x1Max, x1NBins,
                                  x2Min, x2Max, x2NBins,
                                  x3Min, x3Max, x3NBins);
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionNearFieldSpheric::PrepareBackprojection()
{
  // Make some initial computations, compute the center of each bin,
  // create an image with the sensitivities
  //
  // Reminder: x = phi, y = theta, z = radius

  // Calculate bin centers
  delete [] m_xBin;
  m_xBin = new double[m_NImageBins];
  delete [] m_yBin;
  m_yBin = new double[m_NImageBins];
  delete [] m_zBin;
  m_zBin = new double[m_NImageBins];

  for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // phi
    for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // theta
      for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // radius
        ToCartesean(m_x2BinCenter[x2], m_x1BinCenter[x1], m_x3BinCenter[x3],
                    m_xBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins], m_yBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins], m_zBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins]);
        //cout<<m_x2BinCenter[x2]<<":"<<m_x1BinCenter[x1]<<":"<<m_x3BinCenter[x3]<<endl;
        //cout<<m_xBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins]<<":"<<m_yBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins]<<":"<<m_zBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins]<<endl;
      }
    }
  }


  // Volume per bin normalization...
  delete [] m_VolumeBin;
  m_VolumeBin = new double[m_x1NBins * m_x2NBins * m_x3NBins];

  double TotalVolume = 0;
  for (unsigned int x1 = 0; x1 < m_x1NBins; ++x1) { // phi
    double x1EdgeLow = m_x1Min + x1*m_x1IntervalLength;
    double x1EdgeHigh = m_x1Min + (x1+1)*m_x1IntervalLength;
    for (unsigned int x2 = 0; x2 < m_x2NBins; ++x2) { // theta
      double x2EdgeLow = m_x2Min + x2*m_x2IntervalLength;
      double x2EdgeHigh = m_x2Min + (x2+1)*m_x2IntervalLength;
      for (unsigned int x3 = 0; x3 < m_x3NBins; ++x3) { // radius
        double x3EdgeLow = m_x3Min + x3*m_x3IntervalLength;
        double x3EdgeHigh = m_x3Min + (x3+1)*m_x3IntervalLength;

        m_VolumeBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins] =
          (1.0 / 3.0) * (x3EdgeHigh * x3EdgeHigh * x3EdgeHigh - x3EdgeLow * x3EdgeLow * x3EdgeLow) * (cos(x2EdgeLow) - cos(x2EdgeHigh)) * (x1EdgeHigh - x1EdgeLow);


        TotalVolume += m_VolumeBin[x1+x2*m_x1NBins+x3*m_x2NBins*m_x1NBins];
      }
    }
  }

  cout<<"Total volume = "<<TotalVolume<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearFieldSpheric::BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit
  // NABoveLimit      :

  static int Firsts = 0;
  //if (Firsts > 10) return false;

  //cout<<endl<<"Event "<<m_Event->GetId()<<endl;

  Maximum = 0.0;
  NUsedBins = 0;

  // Let the response get the event data:
  m_Response->AnalyzeEvent(m_C);  // <- not the correct place...

  unsigned int x, y, z;
  double AngleTrans, AngleLong;
  double InnerSum = 0;
  double ConeRadius = 0;

  double Content;

  // Cone axis vector (reverse direction of scattered gamma ray)
  double xCA = -m_C->Dg().X();
  double yCA = -m_C->Dg().Y();
  double zCA = -m_C->Dg().Z();
  Rotate(xCA, yCA, zCA);

  // Position of first interaction:
  double xCC = m_C->C1().X();
  double yCC = m_C->C1().Y();
  double zCC = m_C->C1().Z();
  RotateAndTranslate(xCC, yCC, zCC);

  MVector vCC(xCC, yCC, zCC);
  //cout<<"COSI location: "<<vCC.Mag()<<endl;

  //cout<<"Cone axis: "<<setprecision(20)<<xCA<<":"<<yCA<<":"<<zCA<<endl;
  //cout<<"Cone center: "<<setprecision(20)<<xCC<<":"<<yCC<<":"<<zCC<<endl;

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
  for (z = 0; z < m_x3NBins; z++) {
    i_z = z*m_x1NBins*m_x2NBins;
    for (y = 0; y < m_x2NBins; y++) {
      i_yz = y*m_x1NBins + i_z;
      for (x = 0; x < m_x1NBins; x++) {
        i = x + i_yz; // We fill each x-row

        //xBin = m_x1BinCenter[x]-xCC;
        //yBin = m_x2BinCenter[y]-yCC;
        //zBin = m_x3BinCenter[z]-zCC;

        xBin = m_xBin[i]-xCC;
        yBin = m_yBin[i]-yCC;
        zBin = m_zBin[i]-zCC;

        AngleTrans = Angle(xBin, yBin, zBin, xCA, yCA, zCA) - Phi;
        //cout<<x<<", "<<y<<", "<<z<<": "<<AngleTrans*c_Deg<<endl;
        //cout<<"   "<<m_xBin[i]<<", "<<m_yBin[i]<<", "<<m_zBin[i]<<endl;

        /*
        L = sqrt(xBin*xBin + yBin*yBin + zBin*zBin);
        if (m_ApproximatedMaths == true) {
          ConeRadius = fabs(tanPhi*MFastMath::acos(AngleTrans)*L);
        } else {
          ConeRadius = fabs(tanPhi*acos(AngleTrans)*L);
        }
        */

        // Normalize by distance:
        double Normalizer =  (MVector(xCC, yCC, zCC).Mag() - m_x3Min) / MVector(xBin, yBin, zBin).Mag();
        Normalizer = fabs(Normalizer);
        //cout<<"Normalizer: "<<Normalizer<<":"<<m_x3Min<<":"<<MVector(xCC, yCC, zCC).Mag()<<":"<<MVector(xBin  - xCC, yBin - yCC, zBin - zCC).Mag()<<endl;

        // Cut off large angles
        double AngleOffAxis = Angle(xBin, yBin, zBin, xCC, yCC, zCC);
        //Normalizer *= cos(AngleOffAxis);
        if (AngleOffAxis*c_Deg > 60) Normalizer = 0.0;


        Content = m_Response->GetComptonResponse(AngleTrans) * Normalizer;

        if (Content > 0.0 && InRange(InnerSum)) {
          // search the highest pixel...
          if (Content > Maximum) {
            Maximum = Content;
          }
          InnerSum += Content;

          Image[NUsedBins] = Content;
          Bins[NUsedBins] = i;
          ++NUsedBins;
        }

        if (InRange(Content) == false) {
          // Due to acos-rounding error protection, we might get a few here.
          cout<<"Event "<<m_Event->GetId()<<": Content is: "<<Content<<":"<<ConeRadius<<":"<<AngleTrans*c_Deg<<":"<<acos(AngleTrans)<<":"<<m_Response->GetComptonResponse(AngleTrans)/ConeRadius<<endl;
          cout<<"Approx? "<<((m_ApproximatedMaths == true) ? "true" : "false")<<endl;
          if (NUsedBins > 0) cout<<" Last content: "<<Image[NUsedBins-1]<<endl;
          cout<<"Event "<<m_Event->GetId()<<": I seem to still calculate image bins with no content: x="
           <<m_x1BinCenter[x]<<" y="<<m_x2BinCenter[y]<<" z="<<m_x2BinCenter[z]<<endl;
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
    cout<<"dPhi: "<<m_C->dPhi()*c_Deg<<endl;

    NUsedBins = 0;
    Maximum = 0;
    return false;
  }

  /*
  // If we have an efficiency the use it:
  if (m_Efficiency != nullptr) {
    Maximum = 0.0;
    InnerSum = 0.0;
    for (int i = 0; i < NUsedBins; ++i) {
      int xBin = Bins[i] % m_x1NBins;
      int yBin = ((Bins[i] - xBin) / m_x1NBins) % m_x1NBins;
      int zBin = (Bins[i] - xBin - m_x1NBins*yBin) / (m_x1NBins*m_x2NBins);

      double x = m_x1BinCenter[xBin];
      double y = m_x1BinCenter[yBin];
      double z = m_x1BinCenter[zBin];

      //RotateImagingSystemDetectorSystem(x, y, z);
      MVector D(x, y, z);

      double Efficiency = 0.0;
      if (m_ApproximatedMaths == false) {
        Efficiency = m_Efficiency->GetNearField(x, y, z);
      } else {
        Efficiency = m_Efficiency->GetNearField(x, y, z);
      }
      Image[i] *= Efficiency;
      InnerSum += Image[i];

      if (Image[i] > Maximum) Maximum = Image[i];
    }
  }
  */

  // If the image does not contain any content, return also false
  // This case should not really happen due to protections before, thus the error message
  if (InnerSum == 0) {
    cout<<"Event "<<m_Event->GetId()<<": The image seems to be empty..."<<endl;
    NUsedBins = 0;
    Maximum = 0;
    return false;
  }

  if (InnerSum > 0) {
    cout<<"Adding"<<endl;
    Firsts++;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearFieldSpheric::BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit
  // NABoveLimit      :

  // Tronsform the gamma direction to spherical coordinates:
  //double theta, phi, radius;

  Maximum = 0.0;
  NUsedBins = 0;

  double Content;

  double xOrigin = m_P->m_IncomingGammaDirection.X();
  double yOrigin = m_P->m_IncomingGammaDirection.Y();
  double zOrigin = m_P->m_IncomingGammaDirection.Z();
  Rotate(xOrigin, yOrigin, zOrigin);

  double xIA = m_P->GetPairCreationIA().X();
  double yIA = m_P->GetPairCreationIA().Y();
  double zIA = m_P->GetPairCreationIA().Z();
  Rotate(xIA, yIA, zIA);

  double InnerSum = 0.0;
  double AngleTrans;
  unsigned int i;

  // Let's fit a double gausshaped pair-event
  for (unsigned int z = 0; z < m_x3NBins; ++z) {
    for (unsigned int y = 0; y < m_x2NBins; ++y) {
      for (unsigned int x = 0; x < m_x1NBins; ++x) {
        i = x+y*m_x1NBins+z*m_x1NBins*m_x2NBins; // We fill each x-row

        AngleTrans = Angle(m_x1BinCenter[x]-xIA,
                           m_x2BinCenter[y]-yIA,
                           m_x3BinCenter[z]-zIA,
                           xOrigin, yOrigin, zOrigin); //; - m_C->Phi();

        Content = m_Response->GetPairResponse(AngleTrans);

        if (Content > 0.0) {
          if (Maximum < Content) Maximum = Content;
          InnerSum += Content;

          Image[NUsedBins] = Content;
          Bins[NUsedBins] = i;
          ++NUsedBins;
        }
      }
    }
  }

  if (InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }

  if (InnerSum < 0.00001) {
    // Event not inside the viewport
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the origin probabilities for a PET event
bool MBackprojectionNearFieldSpheric::BackprojectionPET(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  //cout<<"Backprojecting PET"<<endl;
  
  MVector P1 = m_PET->GetPosition1();
  MVector P2 = m_PET->GetPosition2();
  
  
  unsigned int i = 0;
  double InnerSum = 0.0;
  double Content = 0.0;
  double Distance = 0.0;
  
  // Let's fit a double gausshaped pair-event
  for (unsigned int z = 0; z < m_x3NBins; ++z) {
    for (unsigned int y = 0; y < m_x2NBins; ++y) {
      for (unsigned int x = 0; x < m_x1NBins; ++x) {
        i = x+y*m_x1NBins+z*m_x1NBins*m_x2NBins; // We fill each x-row
        
        MVector P(m_x1BinCenter[x], m_x2BinCenter[y], m_x3BinCenter[z]);
        Distance = P.DistanceToLine(P1, P2);
        
        Content = m_Response->GetPETResponse(Distance);
                           
        if (Content > 0.0) {
          if (Maximum < Content) Maximum = Content;
          InnerSum += Content;
                             
          Image[NUsedBins] = Content;
          Bins[NUsedBins] = i;
          ++NUsedBins;
        }
      }
    }
  }
  
  if (InRange(InnerSum) == false) {
    merr<<"Catched a NaN!"<<endl;
    merr<<m_C->ToString()<<endl;
    return false;
  }
  
  //cout<<"InnerSum PET: "<<InnerSum<<endl;
  
  if (InnerSum < 0.00001) {
    // Event not inside the viewport
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the origin probabilities for a multi-event event
bool MBackprojectionNearFieldSpheric::BackprojectionMulti(double* SparseImage, int* SparseBins, int& NUsedBins, double& Maximum)
{
  cout<<"Backprojecting multi event with: ";
  for (unsigned int i = 0; i < m_Multi->GetNumberOfEvents(); ++i) {
    cout<<m_Multi->GetEvent(i)->GetTypeString()<<" "; 
  }
  cout<<endl;
  
  double* SparseBackprojectionImage = new double[m_NImageBins];
  int* SparseBackprojectionBins = new int[m_NImageBins];
  double* FullImage = new double[m_NImageBins];
  
  double InnerSum = 0;  
  
  for (unsigned int i = 0; i < m_NImageBins; ++i) {
    FullImage[i] = 0.0;
    //SparseImage[i] = 0.0;
    //SparseBins[i] = 0;
    //SparseBackprojectionImage[i] = 0.0;
    //SparseBackprojectionBins[i] = 0;
  }
  
  
  // Store the multi event since we want to assimilate the sub events one at a time
  MMultiEvent* Multi = m_Multi;
  
  bool First = true;
  for (unsigned int i = 0; i < Multi->GetNumberOfEvents(); ++i) {
    // It is not working for photo...
    if (Multi->GetEvent(i)->GetType() == MPhysicalEvent::c_Photo) continue;
    
    Assimilate(Multi->GetEvent(i));
    NUsedBins = 0;
    Backproject(Multi->GetEvent(i), SparseBackprojectionImage, SparseBackprojectionBins, NUsedBins, Maximum);
    
    // Now add / multiply:
    if (First == true) {
      InnerSum = 0;
      for (int b = 0; b < NUsedBins; ++b) {
        FullImage[SparseBackprojectionBins[b]] = SparseBackprojectionImage[b];
        InnerSum += FullImage[SparseBackprojectionBins[b]];
      }
      First = false;
    } else {
      double* AnotherFullImage = new double[m_NImageBins];
      for (unsigned int b = 0; b < m_NImageBins; ++b) {
        AnotherFullImage[b] = 0.0;
      }
      for (int b = 0; b < NUsedBins; ++b) {
        AnotherFullImage[SparseBackprojectionBins[b]] = SparseBackprojectionImage[b];
      }
      InnerSum = 0;
      for (unsigned int b = 0; b < m_NImageBins; ++b) {
        FullImage[b] *= AnotherFullImage[b];
        InnerSum += FullImage[b];
      }
    }
  }
  
  // Determine bins;
  NUsedBins = 0;
  Maximum = 0;
  InnerSum = 0;
  for (unsigned int i = 0; i < m_NImageBins; ++i) {
    if (FullImage[i] > 0) {
      if (Maximum < FullImage[i]) Maximum = FullImage[i];
      InnerSum += FullImage[i];
      
      SparseImage[NUsedBins] = FullImage[i];
      SparseBins[NUsedBins] = i;
      ++NUsedBins;
    }
  }
  
  // Now move back to the original event
  Assimilate(Multi);
  
  if (InnerSum < 0.00001) {
    //cout<<"Event not inside viewport"<<endl;
    // Event not inside the viewport
    return false;
  }
  
  //cout<<"Final inner sum: "<<InnerSum<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionNearFieldSpheric::Rotate(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coordinate system
  // In the special version, we use the the GPS coordiantes

  MVector P(x, y, z);
  P = m_C->m_GPSRotation * P;
  //P = m_C->m_GPSInverseRotation * P;

  x = P.X();
  y = P.Y();
  z = P.Z();
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionNearFieldSpheric::RotateAndTranslate(double &x, double &y, double &z)
{
  // Rotate the reconstruction-coordinate system
  // In the special version, we use the the GPS coordiantes

  MVector P(x, y, z);
  P = m_C->m_GPSRotation * P;
  //P = m_C->m_GPSInverseRotation * P;
  P += m_C->m_GPSTranslation;

  x = P.X();
  y = P.Y();
  z = P.Z();
}

// MBackprojectionNearFieldSpheric.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
