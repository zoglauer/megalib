/*
 * MBackprojectionNearField.cxx
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
// MBackprojectionNearField
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionNearField.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackprojectionNearField)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionNearField::MBackprojectionNearField(MCoordinateSystem CoordinateSystem) : MBackprojection(CoordinateSystem)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionNearField::~MBackprojectionNearField()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearField::Assimilate(MPhysicalEvent* Event)
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


bool MBackprojectionNearField::Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray
  // if possible, the center of the cone.

  if (MBackprojectionNearField::Assimilate(Event) == false) return false;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    return BackprojectionCompton(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return BackprojectionPair(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_PET) {
    return BackprojectionPET(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_Multi) {
    return BackprojectionMulti(Image, Bins, NUsedBins, Maximum);
  } else  {
    cout<<"Near-field backprojection does not work for this event type: "<<Event->GetTypeString()<<endl;
  }

  return false;
}


///////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearField::SetDimensions(double x1Min, double x1Max, unsigned int x1NBins,
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


void MBackprojectionNearField::PrepareBackprojection()
{
  // Nothing ...
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionNearField::BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Compton-Backprojection-algorithm:
  // The event expands to a double-gausshaped banana
  //
  // Image            :  the produced binned image - must have the correct dimensions
  // Limit            :  the limit
  // NABoveLimit      :

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
  for (z = 0; z < m_x3NBins; z++) {
    i_z = z*m_x1NBins*m_x2NBins;
    for (y = 0; y < m_x2NBins; y++) {
      i_yz = y*m_x1NBins + i_z;
      for (x = 0; x < m_x1NBins; x++) {
        i = x + i_yz; // We fill each x-row


        xBin = m_x1BinCenter[x]-xCC;
        yBin = m_x2BinCenter[y]-yCC;
        zBin = m_x3BinCenter[z]-zCC;


        AngleTrans = Angle(xBin, yBin, zBin, xCA, yCA, zCA) - Phi;


        // Skip pixels below threshold:
        if (x < m_x1NBins - 1) { // We just do the optimization if there are bins left in x-direction:
          // We now calculate the intersection point between this axis and the cone
          // The cone is defined by (cone center is origin):
          // 0 = (X dot CA)^2 - (X dot X) * (CA dot CA) * sin phi  (CA: cone axis)
          // The axis is defined by:
          // X = P + l*(1, 0, 0) where P is the bin center relative to the cone center, i.e. xBin, yBin, zBin
          // Then we determine a, b, c of the Mitternachtsformel and calculate lambda1,2

          // Outside outer ring
          if (AngleTrans > Trans_max + 0.002 || AngleTrans < Trans_min - 0.002) {
            //cout<<m_x1BinCenter[x]<<":"<<m_x2BinCenter[y]<<endl;
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

            double Diskriminante = b*b-4*a*c; // This has to be identical on one x-axis since the distance between axis intersection stays the same
            //cout<<"Diskriminante: "<<Diskriminante<<":"<<a<<":"<<b<<":"<<c<<":"<<CAdotCA<<":"<<BCdotBC<<":"<<BCdotCA<<":"<<cos2PhiRelevant<<endl;

            if (Diskriminante <= 0) { // We ignore the "academic" solution of a tangential point, it's simply not relevant
                //cout<<"No intersection with cone"<<endl;
              break;
            }
            double Lambda1 = (-b-sqrt(Diskriminante))/(2*a);
            double Lambda2 = (-b+sqrt(Diskriminante))/(2*a);

            // Find the closest solution in x-direction (attention: we are in a coordiante system realtive to cone center!):
            double xNext = 0.0;
            //cout<<"xBC: "<<m_x1BinCenter[x]<<"  xBin: "<<xBin<<"  l1:"<<Lambda1<<"  l2:"<<Lambda2<<" dist: "<<Lambda1-Lambda2<<endl;
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
            x+= (int) floor(xNext/m_x1IntervalLength);
            if (x >= m_x1NBins) break; // Done

            //cout<<"New x: "<<m_x1BinCenter[x]<<endl;
            continue;
          }
        }

        L = sqrt(xBin*xBin + yBin*yBin + zBin*zBin);
        if (m_ApproximatedMaths == true) {
          ConeRadius = fabs(tanPhi*MFastMath::acos(AngleTrans)*L);
        } else {
          ConeRadius = fabs(tanPhi*acos(AngleTrans)*L);
        }

        if (HasTrack == true) {
          // angle between two planes spanned by ...
          xn1 = yBin*zCA - zBin*yCA;
          yn1 = (zBin*xCA - xBin*zCA);
          zn1 = xBin*yCA - yBin*xCA;

          xn2 = (yOrigin*zCA - zOrigin*yCA);
          yn2 = (zOrigin*xCA - xOrigin*zCA);
          zn2 = (xOrigin*yCA - yOrigin*xCA);

          if (m_ApproximatedMaths == true) {
            AngleLong = MFastMath::acos((xn1*xn2+yn1*yn2+zn1*zn2)/sqrt((xn1*xn1+yn1*yn1+zn1*zn1)*(xn2*xn2+yn2*yn2+zn2*zn2)));
          } else {
            AngleLong = acos((xn1*xn2+yn1*yn2+zn1*zn2)/sqrt((xn1*xn1+yn1*yn1+zn1*zn1)*(xn2*xn2+yn2*yn2+zn2*zn2)));
          }

          //cout<<i<<": "<<ConeRadius<<"!"<<tanPhi<<"!"<<AngleTrans<<"!"<<L<<"-->"<<m_Response->GetResponse(AngleTrans, AngleLong)/ConeRadius<<endl;

          // Sample the 2d-Gauss-function: m_AreaBin[t] is missing ---> new geometry is missing!!!
          Content = m_Response->GetComptonResponse(AngleTrans, AngleLong)/ConeRadius;
        } else {
          Content = m_Response->GetComptonResponse(AngleTrans)/ConeRadius;
        }

        if (Content > 0.0 && InRange(InnerSum)) {
          // search the highest pixel...
          if (Content > Maximum) {
            Maximum = Content;
          }
          InnerSum += Content;

          Image[NUsedBins] = Content;
          Bins[NUsedBins] = i;
          ++NUsedBins;
        } else {
          // Due to acos-rounding error protection, we might get a few here.
          //cout<<"Content is: "<<Content<<":"<<ConeRadius<<":"<<AngleTrans*c_Deg<<":"<<acos(AngleTrans)<<":"<<m_Response->GetComptonResponse(AngleTrans)/ConeRadius<<endl;
          //cout<<fabs(tanPhi*Acos(AngleTrans)*L)<<":"<<tanPhi<<":"<<AngleTrans<<":"<<L<<":"<<Acos(AngleTrans)<<endl;
          //cout<<"Approx? "<<((m_ApproximatedMaths == true) ? "true" : "false")<<endl;
          //if (NUsedBins > 0) cout<<" Last content: "<<Image[NUsedBins-1]<<endl;
          //cout<<"Event "<<m_Event->GetId()<<": I seem to still calculate image bins with no content: x="
          // <<m_x1BinCenter[x]<<" y="<<m_x2BinCenter[y]<<" z="<<m_x2BinCenter[z]<<endl;
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

  // If we have an efficiency the use it:
  if (m_Efficiency != nullptr) {
    Maximum = 0.0;
    InnerSum = 0.0;
    for (int i = 0; i < NUsedBins; ++i) {
      int xBin = Bins[i] % m_x1NBins;
      int yBin = ((Bins[i] - xBin) / m_x1NBins) % m_x2NBins;
      int zBin = (Bins[i] - xBin - m_x1NBins*yBin) / (m_x1NBins*m_x2NBins);

      double x = m_x1BinCenter[xBin];
      double y = m_x2BinCenter[yBin];
      double z = m_x3BinCenter[zBin];

      //RotateImagingSystemDetectorSystem(x, y, z);
      MVector D(x, y, z);

      double Efficiency = 0.0;
      if (m_ApproximatedMaths == false) {
        Efficiency = m_Efficiency->GetNearField(x, y, z, xCC, yCC, zCC);
      } else {
        Efficiency = m_Efficiency->GetNearField(x, y, z, xCC, yCC, zCC);
      }
      Image[i] *= Efficiency;
      InnerSum += Image[i];

      if (Image[i] > Maximum) Maximum = Image[i];
    }
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


bool MBackprojectionNearField::BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum)
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
bool MBackprojectionNearField::BackprojectionPET(double* Image, int* Bins, int& NUsedBins, double& Maximum)
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
bool MBackprojectionNearField::BackprojectionMulti(double* SparseImage, int* SparseBins, int& NUsedBins, double& Maximum)
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


// MBackprojectionNearField.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
