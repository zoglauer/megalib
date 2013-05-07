/*
 * MForwardProjection.cxx
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
// MForwardProjection.cxx
//
////////////////////////////////////////////////////////////////////////////////


#include "MForwardProjection.h"

#ifdef ___CINT___
ClassImp(MForwardProjection)
#endif


////////////////////////////////////////////////////////////////////////////////


MForwardProjection::MForwardProjection() : MProjection()
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MForwardProjection::~MForwardProjection()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


void MForwardProjection::SetSpaceBins(int NBinsPsi, int NBinsEta, 
				      int NBinsPhi, int NBinsTheta)
{
  m_NBinsPsi = NBinsPsi;
  m_NBinsEta = NBinsEta; 
  m_NBinsPhi = NBinsPhi;
  m_NBinsTheta = NBinsTheta;
}


////////////////////////////////////////////////////////////////////////////////


bool MForwardProjection::ForwardProject(MPhysicalEvent *Event, MFPDataPoint *Point)
{
  if (Assimilate(Event) == false) return false;
  
  if (m_EventType != 1) return false;

  if (m_C->HasTrack() == false) return false;


  
  // Let's compute all the angles - the question is which angles...
  //double Radius, Eta, Psi;

  /*
  // Eta: Angle between the z-Axis and the comptonaxisvector, projected on the z,x-plane
  // Eta element {-pi;+pi}
  Eta = m_Maths.AngleBetweenTwoVectors(0, 0, 1, m_xComptonAxis, 0, m_zComptonAxis);

  if (m_Maths.AngleBetweenTwoVectors(1, 0, 0, m_xComptonAxis, 0, m_zComptonAxis) > pi/2)
    Eta *= -1;

  if (Eta < 0)
    Eta += 2*pi;

  // Psi: Angle between the plane z,x and the ComptonAxisVector
  // Psi e {-pi/2; +pi/2}
  Psi = pi/2 - m_Maths.AngleBetweenTwoVectors(0, 1, 0, m_xComptonAxis, m_yComptonAxis, m_zComptonAxis);
  */
  //if (m_Maths.AngleBetweenTwoVectors(0, 0, 1, m_xComptonAxis, m_yComptonAxis, m_zComptonAxis) > pi/2)
  //Psi = - Psi + pi;
  
  /*
  // Eta: Angle between the z-Axis and the comptonaxisvector, projected on the z,x-plane
  // Eta element {-pi;+pi}
  Eta = m_Maths.AngleBetweenTwoVectors(0, 0, 1, m_C->m_xElectronDirection, 0, m_C->m_zElectronDirection);

  if (m_Maths.AngleBetweenTwoVectors(1, 0, 0, m_C->m_xElectronDirection, 0, m_C->m_zElectronDirection) > pi/2)
    Eta *= -1;

  if (Eta < 0)
    Eta += 2*pi;

  // Psi: Angle between the plane z,x and the ElectronDirectionVector
  // Psi e {-pi/2; +pi/2}
  Psi = pi/2 - m_Maths.AngleBetweenTwoVectors(0, 1, 0, m_C->m_xElectronDirection, m_C->m_yElectronDirection, m_C->m_zElectronDirection);
  





  Point->SetEta(Eta);
  Point->SetPsi(Psi);

  // Some hints:
  // If psi -> +/- pi/2 then eta blurs


  */



  /*

  m_Maths.CarteseanToSpheric(m_xComptonAxis, m_yComptonAxis, m_zComptonAxis,
			     Eta, Psi, Radius);

  double m_xRotation = 0, m_yRotation = -Eta, m_zRotation = -Psi;
  double m_xx, m_xy, m_xz, m_yx, m_yy, m_yz, m_zx, m_zy, m_zz;

  m_xx = cos(m_zRotation)*cos(m_yRotation);
  m_xy = -sin(m_zRotation)*cos(m_xRotation)+cos(m_zRotation)*sin(m_yRotation)*sin(m_xRotation);
  m_xz = sin(m_zRotation)*sin(m_xRotation)+cos(m_zRotation)*sin(m_yRotation)*cos(m_xRotation);
  m_yx = sin(m_zRotation)*cos(m_yRotation);
  m_yy = cos(m_zRotation)*cos(m_xRotation)+sin(m_zRotation)*sin(m_yRotation)*sin(m_xRotation);
  m_yz = -cos(m_zRotation)*sin(m_xRotation)+sin(m_zRotation)*sin(m_yRotation)*cos(m_xRotation);
  m_zx = -sin(m_yRotation);
  m_zy = cos(m_yRotation)*sin(m_xRotation);
  m_zz = cos(m_yRotation)*cos(m_xRotation);



  // Compute angle between electron and scattered gamma:
  double alpha = 
    m_Maths.AngleBetweenTwoVectors(m_xComptonAxis, m_yComptonAxis, m_zComptonAxis, 
				    m_xElectronDirection, m_yElectronDirection, 
				    m_zElectronDirection);

  double xShortGamma, yShortGamma, zShortGamma;
  xShortGamma = m_xComptonAxis * cos(alpha);
  yShortGamma = m_yComptonAxis * cos(alpha);
  zShortGamma = m_zComptonAxis * cos(alpha);

  double xEN, yEN, zEN;

  xEN = m_xElectronDirection - xShortGamma;
  yEN = m_yElectronDirection - yShortGamma;
  zEN = m_zElectronDirection - zShortGamma;

  // And now rotate the this vector:

  double Rotx, Roty, Rotz;
  Rotx = m_xx * xEN + m_yx * yEN + m_zx * zEN;
  Roty = m_xy * xEN + m_yy * yEN + m_zy * zEN;
  Rotz = m_xz * xEN + m_yz * yEN + m_zz * zEN;

  // ... and finally compute the vectors phi angle...
  m_Maths.CarteseanToSpheric(Rotx, 
			     Roty, 
			     Rotz, 
			     Eta, Psi, Radius);

  */

  /*
  Psi = m_Maths.AngleBetweenTwoVectors(m_C->m_xComptonAxis, m_C->m_yComptonAxis, m_C->m_zComptonAxis, 
				       m_C->m_xElectronDirection, m_C->m_yElectronDirection, 
				       m_C->m_zElectronDirection) - m_C->m_ComptonAngle;


  if (m_Maths.InRange(Psi) == false) return false;
  

  Point->SetPhi(Psi);

  Point->SetPhi(m_C->m_ElectronAngle);
  Point->SetTheta(m_C->m_ComptonAngle);

  cout<<m_C->m_ComptonAngle*grad<<endl;
  */
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MForwardProjection::Assimilate(MPhysicalEvent *Event)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray, 
  // the intersection of the cone axis with the sphere. 
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton effect, we return false.

  return MProjection::Assimilate(Event);
}


// MForwardProjection: the end...
////////////////////////////////////////////////////////////////////////////////
