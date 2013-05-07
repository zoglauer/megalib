/*
 * MBackprojectionCartesian.cxx
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
// MBackprojectionCartesian
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackprojectionCartesian.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBackprojectionCartesian)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCartesian::MBackprojectionCartesian(int CoordinateSystem) : MBackprojection(CoordinateSystem)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackprojectionCartesian::~MBackprojectionCartesian()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCartesian::Assimilate(MPhysicalEvent* Event)
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

  if (Event->GetType() == MPhysicalEvent::c_Compton || 
      Event->GetType() == MPhysicalEvent::c_Pair) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCartesian::SetViewportDimensions(double x1Min, double x1Max, int x1NBins, 
                                                  double x2Min, double x2Max, int x2NBins, 
                                                  double x3Min, double x3Max, int x3NBins,
                                                  MVector xAxis, MVector zAxis)
{
  // Set the dimensions of the viewport

  MBackprojection::SetViewportDimensions(x1Min, x1Max, x1NBins, 
                                         x2Min, x2Max, x2NBins,
                                         x3Min, x3Max, x3NBins);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MBackprojectionCartesian::Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray 
  // if possible, the center of the cone. 

  if (MBackprojectionCartesian::Assimilate(Event) == false) return false;

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    return BackprojectionCompton(Image, Bins, NUsedBins, Maximum);
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return BackprojectionPair(Image, Bins, NUsedBins, Maximum);
  }
  else  {
    cout<<"Cartesian::Backproject only works for Comptons and pairs."<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MBackprojectionCartesian::PrepareBackprojection()
{
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


bool MBackprojectionCartesian::BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum)
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

  int x, y, z;
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
        

        xBin = m_xBinCenter[x]-xCC;
        yBin = m_yBinCenter[y]-yCC;
        zBin = m_zBinCenter[z]-zCC;
        

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
            //cout<<"xBC: "<<m_xBinCenter[x]<<"  xBin: "<<xBin<<"  l1:"<<Lambda1<<"  l2:"<<Lambda2<<" dist: "<<Lambda1-Lambda2<<endl;
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
    cout<<"dPhi: "<<m_C->dPhi()*c_Deg<<endl;
    
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


bool MBackprojectionCartesian::BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum)
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


// MBackprojectionCartesian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
