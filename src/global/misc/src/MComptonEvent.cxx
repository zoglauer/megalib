/*
 * MComptonEvent.cxx
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
// MComptonEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MComptonEvent.h"

// Standard libs:
#include <cstdlib>
#include <iostream>
using namespace std;

// ROOT libs:
#include <TMath.h>
#include <TRandom.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MComptonEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MComptonEvent::MComptonEvent() : MPhysicalEvent(), m_CoincidenceWindow(0)
{
  // standard constructor

  m_EventType = c_Compton;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MComptonEvent::~MComptonEvent()
{
  //standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::IsKinematicsOK() const
{
  // Sometimes we have events, whose kinematics, i.e. whose energies are 
  // incompatible with a Compton-effect. If this is the case return false.

  return MComptonEvent::IsKinematicsOK(m_Ee, m_Eg);
}

////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::IsKinematicsOK(const double Ee, const double Eg)
{
  // Test if we have kinematically correct Compton event

  double TestValue; 

  // First test if the deposited energies allow the computation of the 
  // Compton-Angle
  TestValue = 1 - c_E0 * (1/Eg - 1/(Ee + Eg));

  if (TestValue <= -1 || TestValue >= 1) {
    //mout<<"cos(phi) = "<<TestValue<<endl;
    return false;
  }

  // Then test if the electron-scatter-angle can be computed
  TestValue = Ee*(Ee+Eg+c_E0) / ((Ee+Eg)* sqrt(Ee*(Ee + 2*c_E0)));
  if (TestValue <= -1 || TestValue >= 1) {
    //mout<<"cos(epsilon) = "<<TestValue<<endl;
    return false;
  }

  // Finally test if the total scatter angle can be computed: 
  TestValue = (Ee*(Eg-c_E0)) / (Eg*sqrt(Ee*(Ee + 2*c_E0)));
  if (TestValue <= -1 || TestValue >= 1) {
    //mout<<"cos(alpha) = "<<TestValue<<endl;
    return false;
  }

  // Now the kinematics of this event is ok.
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculatePhi()
{
  // Compute the compton scatter angle due to the standard equation
  // i.e neglect the movement of the electron,
  // which would lead to a Doppler-broadening
  //
  // Attention, make sure to test before you call this method,
  //            that the Compton-kinematics is correct

  double Value = 1 - c_E0 * (1/m_Eg - 1/(m_Ee + m_Eg));

  if (Value <= -1 || Value >= 1) {
    return false;
  }

  m_Phi = acos(Value);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculatePhiViaTheta()
{
  double Alpha = CalculateThetaViaAngles();

  //  if (m_Eg + m_Ee < 1900) return false;

  m_Eg =  c_E0 /(1 - cos(Alpha) * sqrt(1+2* c_E0/m_Ee));

  double Value = 1 - c_E0 * (1/m_Eg - 1/(m_Ee + m_Eg));

  if (Value <= -1 || Value >= 1) return false;

  m_Phi = acos(Value);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculateEpsilon()
{
  // Compute the electron angle via th energies:

  double Value = m_Ee*(m_Ee + 2*c_E0);
  if (Value <= 0) return false;

  m_Epsilon = acos(m_Ee*(m_Ee+m_Eg+c_E0) / ((m_Ee+m_Eg)*sqrt(Value)));

  return true;
} 


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::CalculateThetaViaAngles() const
{
  return m_De.Angle(m_C2 - m_C1);
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculateThetaViaEnergies()
{
  // Compute the scattering angle of the electron

  double Value = (m_Ee*(m_Eg-c_E0))/
    (m_Eg*sqrt(m_Ee*(m_Ee+2*c_E0)));

  if (Value <= -1 || Value >= 1) return false;

  m_Theta = acos(Value);

  return true;
} 


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculateDeltaTheta()
{
  // Compute the difference between the total scatter angle calculated by energy and by geometry
  // Return g_DoubleNotDefined in case of non tracked event or bad kinematics

  m_DeltaTheta = fabs(m_Theta - CalculateThetaViaAngles());

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::CalculateDi()
{
  // Compute origin-direction of the gamma-ray

  if (m_HasTrack == false) {
    m_Di = MVector(0.0, 0.0, 0.0);
    m_DiOnCone = MVector(0.0, 0.0, 0.0);
    return true;
  }

  double cImpulsElectron; 
  cImpulsElectron = sqrt(m_Ee*m_Ee + 2*m_Ee*c_E0);

  m_Di = (-1.0/(m_Eg  + m_Ee) * 
    (cImpulsElectron * m_De + m_Eg * m_Dg)).Unit();
  

  // Calculate Di as a projetion on the cone axis:
  double Phi = m_Phi;
  MVector Dg = m_Dg;
  if (Phi > c_Pi/2.0) {
    Phi = c_Pi - Phi;
  } else {
    Dg *= -1; // We need to point towards the sky, the original pointing is the flight direction
  }

  MVector Connector = (m_Di - Dg).Unit(); 
  double LengthConnector = Dg.Mag()*sin(Phi)/sin(c_Pi-Phi-(c_Pi-Connector.Angle(Dg)));
  m_DiOnCone = (Dg+LengthConnector*Connector).Unit();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MComptonEvent::ToString() const
{
  // Display the compton-data of this event

  ostringstream S;

  S<<endl;
  S<<"Compton event "<<m_Id<<":"<<endl;
  S<<"  Time: "<<m_Time<<endl; 
  S<<"  Energy of scattered gamma-ray: "<<m_Eg<<" +- "<<m_dEg<<endl; 
  S<<"  Energy of recoil electron: "<<m_Ee<<" +- "<<m_dEe<<endl; 
  S<<"  Position of first Compton IA: "<<m_C1.X()<<", "<<m_C1.Y()<<", "<<m_C1.Z()<<endl;
  S<<"  Position of second Compton IA: "<<m_C2.X()<<", "<<m_C2.Y()<<", "<<m_C2.Z()<<endl;
  if (m_HasTrack == true) {
    S<<"  Electron direction: "<<m_De.X()<<", "<<m_De.Y()<<", "<<m_De.Z()<<endl;
    S<<"  Incoming direction: "<<m_Di.X()<<", "<<m_Di.Y()<<", "<<m_Di.Z()<<endl;
  } else {
    S<<"  Event has no track!"<<endl;
  }

  S<<"  Detector rotation: x("
     <<m_DetectorRotationXAxis.X()<<", "<<m_DetectorRotationXAxis.Y()<<", "<<m_DetectorRotationXAxis.Z()
     <<") z("<<m_DetectorRotationZAxis.X()<<", "<<m_DetectorRotationZAxis.Y()<<", "<<m_DetectorRotationZAxis.Z()<<")"<<endl;

  S<<"  Compton scatter angle phi: "<<m_Phi*c_Deg<<endl;
  S<<"  Electron scatter angle phi: "<<m_Epsilon*c_Deg<<endl;
  S<<"  Total scatter angle theta (energies): "<<m_Theta*c_Deg<<endl;
  S<<"  Total scatter angle theta (geo): "<<CalculateThetaViaAngles()*c_Deg<<endl;
  S<<"  First lever arm: "<<(m_C2-m_C1).Mag()<<endl; 
  S<<"  Minimum lever arm in sequence: "<<m_LeverArm<<endl; 
  S<<"  Sequence Length: "<<m_SequenceLength<<endl; 
  S<<"  Clustering quality factor: "<<m_ClusteringQualityFactor<<endl; 
  S<<"  Compton quality factor 1: "<<m_ComptonQualityFactor1<<endl; 
  S<<"  Compton quality factor 2: "<<m_ComptonQualityFactor2<<endl; 
  S<<"  Track quality factor 1: "<<m_TrackQualityFactor1<<endl; 
  S<<"  Track quality factor 2: "<<m_TrackQualityFactor2<<endl; 
  if (IsDecay() == true) {
    S<<"  Probably decay..."<<endl;
  }
  S<<endl;

  return S.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetKleinNishina() const
{
  // Return the Klein-Nishina value of this event

  return GetKleinNishina(m_Ei, m_Phi);
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetKleinNishina(const double Ei, const double phi)
{
  // Return the value of the Klein-Nishina function

  if (Ei <= 0) {
    merr<<"Ei = "<<Ei<<" > 0 not fullfilled!"<<endl;
    return 0;
  }
  if (phi < 0 || phi > TMath::Pi()) {
    merr<<"0 < phi="<<phi<<" < pi not fullfilled!"<<endl;
    return 0;
  }

  static const double Radius = 2.8E-15; // m

  const double sinphi = sin(phi);
  const double Eg = -c_E0*Ei/(cos(phi)*Ei-Ei-c_E0);

  return 0.5*Radius*Radius*Eg*Eg/Ei/Ei*(Eg/Ei+Ei/Eg-sinphi*sinphi)*sinphi; 
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetKleinNishinaNormalized(const double Ei, const double phi)
{
  // Return the value of the Klein-Nishina function, 
  // where its area is normalized to 1

  if (Ei <= 0) {
    merr<<"Ei="<<Ei<<" > 0 not fullfilled!"<<endl;
    return 0;
  }
  if (phi < 0 || phi > TMath::Pi()) {
    merr<<"0 < phi="<<phi<<" < pi not fullfilled!"<<endl;
    return 0;
  }

  mimp<<"Normalization not done analytically!"<<show;

  double max = 0;
  for (double dphi = 0; dphi < c_Pi; dphi += 0.01) {
    if (GetKleinNishina(Ei, dphi) > max) {
      max = GetKleinNishina(Ei, dphi);
    } else {
      for (double ddphi = max; ddphi > max-0.01; ddphi -= 0.001) {
        if (GetKleinNishina(Ei, ddphi) > max) {
          max = GetKleinNishina(Ei, ddphi);
        } else {
          break;
        }
      }
    }
  }

  double result = GetKleinNishina(Ei, phi)/max;
  if (result > 1.0) result = 1.0;
  if (result < 0.0) result = 0.0;   

  return result;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetKleinNishinaNormalizedByArea(double Ei, double phi)
{
  // Return the value of the Klein-Nishina function, 
  // where its area is normalized to 1

  if (Ei <= 0) {
    merr<<"Ei="<<Ei<<" > 0 not fullfilled!"<<endl;
    return 0;
  }
  if (phi < 0 || phi > TMath::Pi()) {
    merr<<"0 < phi="<<phi<<" < pi not fullfilled!"<<endl;
    return 0;
  }

  static const double Radius = 2.8E-15; // m

  double Normalization = 0.5*c_E0*c_E0*Radius*Radius*
    (2*Ei*(Ei*Ei*Ei + 9*Ei*Ei*c_E0 + 8*Ei*c_E0*c_E0 + 2*c_E0*c_E0*c_E0) - 
     (2*Ei+c_E0)*(2*Ei+c_E0)*(Ei*Ei - 2*Ei*c_E0 - 2*c_E0*c_E0)*
     (log(c_E0/(Ei + c_E0)) - log((2*Ei + c_E0)/(Ei + c_E0))))
    /(Ei*Ei*Ei*c_E0*(2*Ei+c_E0)*(2*Ei+c_E0));

  if (Normalization > 0) {
    return GetKleinNishina(Ei, phi)/Normalization;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetRandomPhi(const double Ei)
{
  // Return a randomly sampled Compton scatter angle using the Klein-Nishina eqaution for the given
  // incidence photon energy (algorithm: Butcher & Messel: Nuc Phys 20(1960), 15)
  
  double Ei_m = Ei / c_E0;

  double Epsilon = 0.0;
  double EpsilonSquare = 0.0;
  double OneMinusCosPhi = 0.0;
  double SinPhiSquared = 0.0;

  double Epsilon0 = 1.0/(1.0 + 2.0*Ei_m);
  double Epsilon0Square = Epsilon0*Epsilon0;
  double Alpha1 = -log(Epsilon0);
  double Alpha2 = 0.5*(1.0 - Epsilon0Square);

  double Reject = 0.0;

  do {
    if (Alpha1/(Alpha1 + Alpha2) > gRandom->Rndm()) {
      Epsilon = exp(-Alpha1*gRandom->Rndm());
      EpsilonSquare = Epsilon*Epsilon; 
    } else {
      EpsilonSquare = Epsilon0Square + (1.0 - Epsilon0Square)*gRandom->Rndm();
      Epsilon = sqrt(EpsilonSquare);
    }

    OneMinusCosPhi = (1.0 - Epsilon)/(Epsilon*Ei_m);
    SinPhiSquared = OneMinusCosPhi*(2.0 - OneMinusCosPhi);
    Reject = 1.0 - Epsilon*SinPhiSquared/(1.0 + EpsilonSquare);

  } while (Reject < gRandom->Rndm());
 
  return acos(1.0 - OneMinusCosPhi);
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetARMGamma(const MVector& Position, const MCoordinateSystem& CS) 
{
  // The ARM value for the scattered gamma-ray is the minimum angle between 
  // the gamma-cone-surface and the line connecting the cone-apex with the 
  // test position
  // Values < 0 means phi is smaller than expected
  // Phi smaller means cos Phi larger means Ee is smaller than real


  // Rotate/translate the position into event coordinates
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorInverseRotationMatrix()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic && m_HasGalacticPointing == true) RotPosition = GetGalacticPointingInverseRotationMatrix()*RotPosition;

  return (m_C1 - m_C2).Angle(RotPosition - m_C1) - m_Phi;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetARMElectron(const MVector& Position, const MCoordinateSystem& CS) 
{
  // The ARM value for the recoil electron is the minimum angle between 
  // the elctron-cone-surface and the line connecting the cone-apex with the 
  // (test-) position

  // Rotate the position into event coordinates
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorRotationMatrix().Invert()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic && m_HasGalacticPointing == true) RotPosition = GetGalacticPointingInverseRotationMatrix()*RotPosition;
  
  return (-m_De).Angle(RotPosition - m_C1) - m_Epsilon;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::GetSPDElectron(const MVector& Position, const MCoordinateSystem& CS)
{
  // The SPD value for the recoil electron is the minimum angle between 
  // the electron-cone-surface and the line connecting the cone-apex with the 
  // (test-) position

  // Rotate the position into event coordinates
  MVector RotPosition = Position;
  if (m_HasDetectorRotation == true) RotPosition = GetDetectorRotationMatrix().Invert()*RotPosition;
  if (CS == MCoordinateSystem::c_Galactic && m_HasGalacticPointing == true) RotPosition = GetGalacticPointingInverseRotationMatrix()*RotPosition;

  return ((m_C1 - m_C2).Cross(RotPosition - m_C1)).Angle((m_C1 - m_C2).Cross(m_Di));
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Assimilate(MComptonEvent* Compton)
{
  // Take over all the necessary event data and perform some elementary computations:
  // the compton angle, the cone axis, the most probable origin of the gamma ray...
  //
  // If an error occures, normally because the event data is so bad that the event
  // can hardly be caused by compton scattering, we return false.
  
  Reset();
  
  // Get only the basic data and calculate the rest:
  MPhysicalEvent::Assimilate(dynamic_cast<MPhysicalEvent*>(Compton));

  m_Eg = Compton->Eg();       
  m_dEg = Compton->dEg();
       
  m_Ee = Compton->Ee();     
  m_dEe = Compton->dEe();      

  m_C1 = Compton->C1();    
  m_dC1 = Compton->dC1();    
  m_C2 = Compton->C2();   
  m_dC2 = Compton->dC2();   
  m_De = Compton->De();   
  m_dDe = Compton->dDe();   
  m_ToF = Compton->ToF();   
  m_dToF = Compton->dToF();   
  
  m_ClusteringQualityFactor = Compton->ClusteringQualityFactor();

  m_SequenceLength = Compton->SequenceLength();
  m_ComptonQualityFactor1 = Compton->ComptonQualityFactor1();
  m_ComptonQualityFactor2 = Compton->ComptonQualityFactor2();
  m_LeverArm = Compton->LeverArm();
  
  m_TrackLength = Compton->TrackLength();
  m_TrackInitialDeposit = Compton->TrackInitialDeposit();
  m_TrackQualityFactor1 = Compton->TrackQualityFactor1();
  m_TrackQualityFactor2 = Compton->TrackQualityFactor2();

  m_CoincidenceWindow = Compton->CoincidenceWindow();
  
  if (Validate() == false) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Assimilate(MPhysicalEvent* Event)
{
  // Simply Call: MComptonEvent::Assimilate(const MComptonEventData *ComptonEventData)

  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    return Assimilate(dynamic_cast<MComptonEvent*>(Event));
  } else {
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Assimilate(const MVector& C1, const MVector& C2, 
                               const MVector& De, const double Ee, 
                               const double Eg)
{
  //

  Reset();
  
  // Get only the basic data and calculate the rest:
  m_Eg = Eg;       
  m_Ee = Ee;     
  m_C1 = C1;    
  m_C2 = C2;   
  m_De = De;   
  
  if (Validate() == false) return false;
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Assimilate(char* LineBuffer)
{
  // Assimilate an event from a text-line

  Reset();

  double xFirstComptonIA, yFirstComptonIA, zFirstComptonIA;
  double xSecondComptonIA, ySecondComptonIA, zSecondComptonIA;
  double xElectronDirection, yElectronDirection, zElectronDirection;

  // 'C': Only accept comptons:
  if (LineBuffer[0] == 'C') {
    if (sscanf(LineBuffer, 
               "C;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf\n", 
               &xFirstComptonIA, 
               &yFirstComptonIA, 
               &zFirstComptonIA, 
               &xSecondComptonIA, 
               &ySecondComptonIA, 
               &zSecondComptonIA, 
               &xElectronDirection, 
               &yElectronDirection, 
               &zElectronDirection,
               &m_Ee, 
               &m_Eg, 
               &m_dEe, 
               &m_dEg) == 13) {

      m_C1 = 
        MVector(xFirstComptonIA, yFirstComptonIA, zFirstComptonIA);
      m_C2 = 
        MVector(xSecondComptonIA, ySecondComptonIA, zSecondComptonIA);
      m_De = 
        MVector(xElectronDirection, yElectronDirection, zElectronDirection)
        .Unit();
    } else {
      return false;
    }
  } else { 
    return false;
  } 

  if (Validate() == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MComptonEvent::Duplicate()
{
  // Duplicate this event

  MPhysicalEvent* Event = new MComptonEvent();
  Event->Assimilate(this);

  return dynamic_cast<MPhysicalEvent*>(Event);
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Validate()
{
  // Do some initial computations after assimilation:
  // Test if the event is ok, compute the scatter angles and the 
  // compton-axis-Vector, etc.

  // Sometimes the real data is so false that we are unable to compute a compton-angle: 
  if (IsKinematicsOK() == false) {
    return false;
  }

  // energy of the inital gamma
  m_Ei = m_Ee + m_Eg;

  // Direction of the second gamma-ray:
  m_Dg = (m_C2 - m_C1).Unit();


  // Compute the three angles:
  CalculatePhi();
  CalculateEpsilon();
  CalculateThetaViaEnergies();
  CalculateDeltaTheta();

  // Compute the most probable origin of the gamma-ray, if possible
  if (m_De.X() == 0.0 && 
      m_De.Y() == 0.0 && 
      m_De.Z() == 0.0) {
    m_HasTrack = false;
  } else {
    m_HasTrack = true;
    CalculateDi();
  } 

  if (m_LeverArm == 0) {
    m_LeverArm = (m_C2 - m_C1).Mag();
  }

  //cout<<ToString()<<endl;

  //cout<<m_Phi<<"!"<<m_MinComptonAngle<<"!"<<m_MaxComptonAngle<<endl;

  // Check allowed compton angle:
  /*
  if (m_Phi < m_MinComptonAngle ||
      m_Phi > m_MaxComptonAngle) {
    return false;
  }
  */
  
  //cout<<"ComptonEvent modified"<<endl;
  //if (m_C2.Z() < -38) return false;  // < Seite,  > Boden

  /*
  // Distance between first and second interaction:
  //cout<<(m_C2 - m_C1).Mag()<<endl;
  if ((m_C2 - m_C1).Mag() < 0 || 
      (m_C2 - m_C1).Mag() > 20) return false; 
  */

  /*
  if (TestEventRestrictions() == false) {
    return false; 
  }
  */

  m_IsGoodEvent = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputeEeViaThetaEg(const double Theta, const double Eg)
{
  double costheta = cos(Theta);

  if (Theta < 0.5*c_Pi) {
    if (Eg >= c_E0/(1+costheta) && Eg <= c_E0/(1-costheta) ) {
      // Actually no solution exists...
      return 9.99E+99;
    }
  } else {
    if (Eg >= c_E0/(1-costheta) && Eg <= c_E0/(1+costheta)) {
      // Actually no solution exists...
      return 9.99E+99;
    }
  }

  return -2*c_E0*costheta*costheta*Eg*Eg/(costheta*costheta*Eg*Eg-(Eg-c_E0)*(Eg-c_E0));  
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputeEgViaThetaEe(const double Theta, const double Ee)
{
  double Value = -cos(Theta)*sqrt(Ee*Ee+2*Ee*c_E0)+Ee;
  
  if (Value <= 0) {
    merr<<"MComptonEvent::ComputeEg: Devision through zero or negative energy: "
        <<sqrt(Ee*Ee+2*Ee*c_E0)<<"!"<<Value<<", theta="
        <<-cos(Theta)*sqrt(Ee*Ee+2*Ee*c_E0)<<", Ee="<<Ee<<show;
    return 9.99E+99;
  }

  return Ee*c_E0/Value;  
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputePhiViaEeEg(const double Ee, const double Eg)
{
  double Value = 1 - c_E0*(1/Eg - 1/(Ee + Eg));
  
  if (Value <= -1) {
    //cerr<<"MComptonEvent::ComputePhi: NaN ("<<Ee<<", "<<Eg<<") ---> "<<Value<<endl;
    Value = -1;
  } else if (Value >= 1) {
    //cerr<<"MComptonEvent::ComputePhi: NaN ("<<Ee<<", "<<Eg<<") ---> "<<Value<<endl;
    Value = 1;
  }
  
  return acos(Value);
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputeCosPhiViaEeEg(const double Ee, const double Eg)
{
  return 1 - c_E0*(1/Eg - 1/(Ee + Eg));
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputeEeViaPhiEg(const double Phi, const double Eg)
{
  double Value = cos(Phi)*Eg-Eg+c_E0;

  if (Value != 0) {
    return Eg*Eg*(1-cos(Phi))/Value;
  } 

  merr<<"Denominator is zero! Returning zero"<<endl;

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputeEeViaPhiEi(const double Phi, const double Ei)
{
  double Value = cos(Phi)*Ei-Ei-c_E0;

  if (Value != 0) {
    return Ei*Ei*(cos(Phi)-1)/Value;
  } 

  merr<<"Denominator is zero! Returning zero"<<endl;

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////

  
double MComptonEvent::dPhi()
{
  //! Return the Compton scatter angle uncertainty

  // dPhi is not always calculated, but only when not calculated before...
  if (m_dPhi > 0) return m_dPhi;
  
  // Maple calculated dPhiE
  // Don't look at dCosPhiE here, because one cannot do acos(dCosPhiE), because it is just an error!!!
  double CosPhiE = 1-c_E0/m_Eg+c_E0/m_Ei;
  double dPhiE = sqrt(pow(c_E0/m_Eg/m_Eg-c_E0/m_Ei/m_Ei, 2)/(1-pow(CosPhiE,2))*m_dEg*m_dEg + c_E0*c_E0/(m_Ei*m_Ei*m_Ei*m_Ei)/(1-pow(CosPhiE,2))*m_dEe*m_dEe);
  
  // dPhiA is the Gaussian error propagation of all (averaged) positions error components (=x,y,z) of all two positions
  double dPhiA = 0;
  double dPhiAp = 0;
  double dPhiAm = 0;
  MVector dC1 = m_C1;
  MVector dC2 = m_C2;
  
  for (unsigned int i = 0; i < 3; ++i) {
    dC1 = m_C1;
    dC1[i] += m_dC1[i];
    dPhiAp = (m_C1 - m_C2).Angle(dC1 - m_C2);
    dC1 = m_C1;
    dC1[i] -= m_dC1[i];
    dPhiAm = (m_C1 - m_C2).Angle(dC1 - m_C2);
    dPhiA += pow(0.5*(dPhiAp+dPhiAm), 2);
  }
  
  for (unsigned int i = 0; i < 3; ++i) {
    dC2 = m_C2;
    dC2[i] += m_dC2[i];
    dPhiAp = (m_C2 - m_C1).Angle(dC2 - m_C1);
    dC2 = m_C2;
    dC2[i] -= m_dC2[i];
    dPhiAm = (m_C2 - m_C1).Angle(dC2 - m_C1);
    dPhiA += pow(0.5*(dPhiAp+dPhiAm), 2);
  }
  dPhiA = sqrt(dPhiA);
     
  // Boggs' method - gives almost identical results
  /*
  double dx = sqrt(m_dC1[0]*m_dC1[0] + m_dC2[0]*m_dC2[0]);
  double dy = sqrt(m_dC1[1]*m_dC1[1] + m_dC2[1]*m_dC2[1]);
  double dz = sqrt(m_dC1[2]*m_dC1[2] + m_dC2[2]*m_dC2[2]);
  
  double Mag = (m_C1 - m_C2).Mag();
  double dPhix = dx/Mag*sqrt(1-(m_C2[0]-m_C1[0])*(m_C2[0]-m_C1[0])/Mag/Mag);
  double dPhiy = dy/Mag*sqrt(1-(m_C2[1]-m_C1[1])*(m_C2[1]-m_C1[1])/Mag/Mag);
  double dPhiz = dz/Mag*sqrt(1-(m_C2[2]-m_C1[2])*(m_C2[2]-m_C1[2])/Mag/Mag);
  
  double dPhiA_Boggs = sqrt(dPhix*dPhix + dPhiy*dPhiy + dPhiz*dPhiz);
  
  cout<<dPhiA<<" vs. Boggs: "<<dPhiA_Boggs<<endl;
  */
  
  m_dPhi = sqrt(dPhiE*dPhiE + dPhiA*dPhiA);

  return m_dPhi;
}

  
////////////////////////////////////////////////////////////////////////////////

  
bool MComptonEvent::SwitchDirection()
{
  //! Switch direction: Reverse the Compton path

  double Temp;

  Temp = m_Eg;
  m_Eg = m_Ee;
  m_Ee = Temp;

  Temp = m_dEg;
  m_dEg = m_dEe;
  m_dEe = Temp;

  MVector VTemp;
  
  VTemp = m_C1;
  m_C1 = m_C2;
  m_C2 = VTemp;
  
  VTemp = m_dC1;
  m_dC1 = m_dC2;
  m_dC2 = VTemp;

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


void MComptonEvent::Reset() 
{
  // Reset all values to zero:
  
  MPhysicalEvent::Reset();

  m_Eg = 0.0;
  m_dEg = 0.0;
  m_Ee = 0.0;
  m_dEe = 0.0;
  
  m_C1.Clear();
  m_dC1.Clear();

  m_C2.Clear();
  m_dC2.Clear();

  m_De.Clear();
  m_dDe.Clear();

  m_ToF = 0.0;
  m_dToF = 0.0;

  m_Ei = 0.0;
  m_dEi = 0.0;

  m_Phi = 0.0;
  m_dPhi = 0.0;
  m_Epsilon = 0.0;
  m_sEpsilon = 0.0;
  m_Theta = 0.0;
  m_dTheta = 0.0;
  
  m_DeltaTheta = 0.0;

  m_Dg.Clear();
  m_dDg.Clear();

  m_Di.Clear();
  m_dDi.Clear();

  m_HasTrack = false;

  m_IsGoodEvent = false;

  m_ClusteringQualityFactor = 0;

  m_SequenceLength = 2;
  m_ComptonQualityFactor1 = 0;
  m_ComptonQualityFactor2 = 0;

  m_TrackLength = 1;
  m_TrackInitialDeposit = 0;
  m_TrackQualityFactor1 = 0;
  m_TrackQualityFactor2 = 0;

  m_LeverArm = 0;

  m_CoincidenceWindow = 0;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MComptonEvent::Data() 
{
  // Return a pointer to this event
 
  return (MPhysicalEvent *) this; 
} 


////////////////////////////////////////////////////////////////////////////////


MString MComptonEvent::ToBasicString() const
{
  // Transform the data to one line of text
  char LineBuffer[1000];

  sprintf(LineBuffer, "C;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f\n",
    m_C1.X(), m_C1.Y(), m_C1.Z(), 
    m_C2.X(), m_C2.Y(), m_C2.Z(), 
    m_De.X(), m_De.Y(), m_De.Z(), 
    m_Ee, m_Eg, 
    m_dEe, m_dEg);

  return MString(LineBuffer);
  
  ostringstream S;

  //S<<"<?>"<<LineBuffer<<"</?>"<<endl;


  // Let's try xml:
  // <Event>
  //  <TIME> 8392.90527</TIME>
  //  <HD1><X> 0.495</X><Y>-0.014</Y><Z> 0.000</Z><E> 5095.66</E></HD1>
  //  <HD2><X>-0.231</X><Y> 0.071</Y><Z>-1.577</Z><E> 9319.43</E></HD2>
  // </Event>

  
  S<<"<Event>"<<endl;
  S<<"<HD1><X>"<<m_C1.X()
     <<"</X><Y>"<<m_C1.Y()
     <<"</Y><Z>"<<m_C1.Z()
     <<"</Z><E>"<<m_Ee<<"</E></HD1>"<<endl;
  S<<"<HD2><X>"<<m_C2.X()
     <<"</X><Y>"<<m_C2.Y()
     <<"</Y><Z>"<<m_C2.Z()
     <<"</Z><E>"<<m_Eg<<"</E></HD2>"<<endl;
  S<<"<ED><X>"<<m_De.X()
     <<"</X><Y>"<<m_De.Y()
     <<"</Y><Z>"<<m_De.Z()<<"</Z></ED>"<<endl;
  S<<"</Event>"<<endl<<endl;

  return S.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::Stream(MFile& File, int Version, bool Read, bool Fast, bool ReadDelayed)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...
  // Rearely used options which are zero and default to zero are not streamed!
  
  bool Return = MPhysicalEvent::Stream(File, Version, Read, Fast, ReadDelayed);

  if (Read == false) {
    // Write Compton specific infos:
    
    ostringstream S;
    
    if (m_ClusteringQualityFactor != 0) S<<"PQ "<<m_ClusteringQualityFactor<<endl;
    S<<"SQ "<<m_SequenceLength<<endl;
    S<<"CT "<<m_ComptonQualityFactor1<<" "<<m_ComptonQualityFactor2<<endl;
    S<<"TL "<<m_TrackLength<<endl;
    S<<"TE "<<m_TrackInitialDeposit<<endl;
    if (m_TrackQualityFactor1 != 0 || m_TrackQualityFactor2 != 0) S<<"TQ "<<m_TrackQualityFactor1<<" "<<m_TrackQualityFactor2<<endl;
    S<<"CE "<<m_Eg<<" "<<m_dEg<<"   "<<m_Ee<<" "<<m_dEe<<endl;
    S<<"CD "<<m_C1[0]<<" "<<m_C1[1]<<" "<<m_C1[2]<<"   ";
    S<<m_dC1[0]<<" "<<m_dC1[1]<<" "<<m_dC1[2]<<"   ";
    S<<m_C2[0]<<" "<<m_C2[1]<<" "<<m_C2[2]<<"   ";
    S<<m_dC2[0]<<" "<<m_dC2[1]<<" "<<m_dC2[2]<<"   ";
    S<<m_De[0]<<" "<<m_De[1]<<" "<<m_De[2]<<"   ";
    S<<m_dDe[0]<<" "<<m_dDe[1]<<" "<<m_dDe[2]<<endl;
    if (m_ToF != 0 || m_dToF != 0) S<<"TF "<<m_ToF<<" "<<m_dToF<<endl;
    S<<"LA "<<m_LeverArm<<endl;
    if (m_CoincidenceWindow != 0) S<<"CW "<<m_CoincidenceWindow<<endl;
    for (unsigned int h = 0; h < m_Hits.size(); ++h) {
      S<<"CH "<<h
      <<" "<<m_Hits[h].GetPosition().GetX()
      <<" "<<m_Hits[h].GetPosition().GetY()
      <<" "<<m_Hits[h].GetPosition().GetZ()
      <<" "<<m_Hits[h].GetEnergy()
      <<" "<<m_Hits[h].GetTime().GetAsDouble()
      <<" "<<m_Hits[h].GetPositionUncertainty().GetX()
      <<" "<<m_Hits[h].GetPositionUncertainty().GetY()
      <<" "<<m_Hits[h].GetPositionUncertainty().GetZ()
      <<" "<<m_Hits[h].GetEnergyUncertainty()
      <<" "<<m_Hits[h].GetTimeUncertainty().GetAsDouble()<<endl;
    }
    
    File.Write(S);
    File.Flush();
  }

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


int MComptonEvent::ParseLine(const char* Line, bool Fast)
{
  // Return  0, if the line got correctly parsed
  // Return  1, if the line got not correctly parsed
  // Return  2, if the line got not parsed
  // Return -1, if the end of event has been reached

  int Ret = MPhysicalEvent::ParseLine(Line, Fast);
  if (Ret != 2) {
    return Ret;
  }

  Ret = 0;

  if (Line[0] == 'C' && Line[1] == 'E') {
    if (Fast == true) {
      char* p;
      m_Eg = strtod(Line+3, &p);
      m_dEg = strtod(p, &p);
      m_Ee = strtod(p, &p);
      m_dEe = strtod(p, NULL);
    } else {
      if (sscanf(Line, "CE %lf %lf %lf %lf", &m_Eg, &m_dEg, &m_Ee, &m_dEe) != 4) {
        mout<<"Unable to parse CE of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'C' && Line[1] == 'D') {
    if (Fast == true) {
      char* p;
      m_C1[0] = strtod(Line+3, &p);
      m_C1[1] = strtod(p, &p);
      m_C1[2] = strtod(p, &p);
      m_dC1[0] = strtod(p, &p);
      m_dC1[1] = strtod(p, &p);
      m_dC1[2] = strtod(p, &p);
      m_C2[0] = strtod(p, &p);
      m_C2[1] = strtod(p, &p);
      m_C2[2] = strtod(p, &p);
      m_dC2[0] = strtod(p, &p);
      m_dC2[1] = strtod(p, &p);
      m_dC2[2] = strtod(p, &p);
      m_De[0] = strtod(p, &p);
      m_De[1] = strtod(p, &p);
      m_De[2] = strtod(p, &p);
      m_dDe[0] = strtod(p, &p);
      m_dDe[1] = strtod(p, &p);
      m_dDe[2] = strtod(p, NULL);
    } else {
      if (sscanf(Line, "CD %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
                 &m_C1[0], &m_C1[1], &m_C1[2], &m_dC1[0], &m_dC1[1], &m_dC1[2], 
                 &m_C2[0], &m_C2[1], &m_C2[2], &m_dC2[0], &m_dC2[1], &m_dC2[2], 
                 &m_De[0], &m_De[1], &m_De[2], &m_dDe[0], &m_dDe[1], &m_dDe[2]) != 18) {
        cout<<"Unable to parse CD of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'C' && Line[1] == 'H') {
    if (Fast == true) {
      char* p;
      unsigned long Index = strtoul(Line+3, &p, 10);
      double P0 = strtod(p, &p);
      double P1 = strtod(p, &p);
      double P2 = strtod(p, &p);
      double E = strtod(p, &p);
      double T = strtod(p, &p);
      double dP0 = strtod(p, &p);
      double dP1 = strtod(p, &p);
      double dP2 = strtod(p, &p);
      double dE = strtod(p, &p);
      double dT = strtod(p, NULL);
      
      MPhysicalEventHit Hit;
      Hit.Set(MVector(P0, P1, P2), MVector(dP0, dP1, dP2), E, dE, MTime(T), MTime(dT));
      if (m_Hits.size() <= Index) m_Hits.resize(Index+1);
      m_Hits[Index] = Hit;
    } else {
      unsigned long Index = 0;
      double P0 = 0;
      double P1 = 0;
      double P2 = 0;
      double E = 0;
      double T = 0;
      double dP0 = 0;
      double dP1 = 0;
      double dP2 = 0;
      double dE = 0;
      double dT = 0;
      
      if (sscanf(Line, "CH %lu %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &Index, &P0, &P1, &P2, &E, &T, &dP0, &dP1, &dP2, &dE, &dT) != 11) {
        mout<<"Unable to parse CH of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      } else {
        MPhysicalEventHit Hit;
        Hit.Set(MVector(P0, P1, P2), MVector(dP0, dP1, dP2), E, dE, MTime(T), MTime(dT));
        if (m_Hits.size() <= Index) m_Hits.resize(Index+1);
        m_Hits[Index] = Hit;        
      }
    }
  } else if (Line[0] == 'P' && Line[1] == 'Q') {
    if (Fast == true) {
      m_ClusteringQualityFactor = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "PQ %lf", &m_ClusteringQualityFactor) != 1) {
        mout<<"Unable to parse PQ of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'S' && Line[1] == 'Q') {
    if (Fast == true) {
      m_SequenceLength = strtol(Line+3, NULL, 10);
    } else {
      if (sscanf(Line, "SQ %u", &m_SequenceLength) != 1) {
        mout<<"Unable to parse SQ of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'C' && Line[1] == 'T') {
    if (Fast == true) {
      char* p;
      m_ComptonQualityFactor1 = strtod(Line+3, &p);
      if (p != NULL) {
        m_ComptonQualityFactor2 = strtod(p, NULL);
      }
    } else {
      if (sscanf(Line, "CT %lf %lf", &m_ComptonQualityFactor1, &m_ComptonQualityFactor2) != 2) {
        if (sscanf(Line, "CT %lf", &m_ComptonQualityFactor1) != 1) {
          mout<<"Unable to parse CT of event "<<m_Id<<"!"<<endl;
          Ret = 1;
        } else {
          m_ComptonQualityFactor2 = 1;
        }
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'F') {
    if (Fast == true) {
      char* p;
      m_ToF = strtod(Line+3, &p);
      m_dToF = strtod(p, NULL);
    } else {
      if (sscanf(Line, "TF %lf %lf", &m_ToF, &m_dToF) != 2) {
        mout<<"Unable to parse TF of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'Q') {
    if (Fast == true) {
      char* p;
      m_TrackQualityFactor1 = strtod(Line+3, &p);
      if (p != NULL) {
        m_TrackQualityFactor2 = strtod(p, NULL);
      }
    } else {
      if (sscanf(Line, "TQ %lf %lf", &m_TrackQualityFactor1, &m_TrackQualityFactor2) != 2) {
        if (sscanf(Line, "TQ %lf", &m_TrackQualityFactor1) != 1) {
          mout<<"Unable to parse TQ of event "<<m_Id<<"!"<<endl;
          Ret = 1;
        } else {
          m_TrackQualityFactor2 = 0;
        }
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'L') {
    if (Fast == true) {
      m_TrackLength = strtol(Line+3, NULL, 10);
    } else {
      if (sscanf(Line, "TL %u", &m_TrackLength) != 1) {
        mout<<"Unable to parse TL of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'T' && Line[1] == 'E') {
    if (Fast == true) {
      m_TrackInitialDeposit = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "TE %lf", &m_TrackInitialDeposit) != 1) {
        mout<<"Unable to parse TE of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'L' && Line[1] == 'A') {
    if (Fast == true) {
      m_LeverArm = strtod(Line+3, NULL);
    } else {
      if (sscanf(Line, "LA %lf", &m_LeverArm) != 1) {
        mout<<"Unable to parse LA of event "<<m_Id<<"!"<<endl;
        Ret = 1;
      }
    }
  } else if (Line[0] == 'C' && Line[1] == 'W') {
    if (Fast == true) {
      if (m_CoincidenceWindow.Set(Line) == false) {
        Ret = 1;
      }
    } else {
      if (m_CoincidenceWindow.Set(MString(Line), 3) == false) {
        Ret = 1;
      }
    }
  } else {
    Ret = 2;
  }

  return Ret;
}



////////////////////////////////////////////////////////////////////////////////
/*
bool MComptonEvent::ZenitDistance(double& theta)
{
  // Compute the distance of the most probable origin of the gamma-ray and the zenit
  // of the standard coordinate system

  if (m_IsGoodEvent == false) {
    Fatal("MComptonEvent::ZenitDistance(double& theta):\n\n",
          "No successfully assimilated event available!\n"
          "Test with MComptonEvent::AssimilationCompleted()");  
  }

  if (m_HasTrack == false) return false;

  theta = m_Di.Angle(MVector(0.0, 0.0, 1.0));

  return true;
}
*/

////////////////////////////////////////////////////////////////////////////////

/*
double MComptonEvent::TA_Ee(double E2, double Alpha)
{
  return -2*E2*E2*cos(Alpha)*cos(Alpha)*c_E0/
    (cos(Alpha)*cos(Alpha)*E2*E2 -
     (E2 - c_E0)*(E2 - c_E0));
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::TA_E2(double Ee, double Alpha)
{
  return Ee*c_E0/(Ee-cos(Alpha)*sqrt(Ee*(Ee+2*c_E0)));
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::TA_GradientEe(double E2, double Alpha)
{
  double Denom = cos(Alpha)*cos(Alpha)*E2*E2 - (E2 - c_E0) * (E2 - c_E0);

  return 8*c_E0*cos(Alpha)*sin(Alpha)*E2/Denom -
    4*c_E0*cos(Alpha)*sin(Alpha)*E2*E2*(2*cos(Alpha)*cos(Alpha)*E2-2*E2+2*c_E0)/(Denom*Denom) -
    16*c_E0*cos(Alpha)*cos(Alpha)*cos(Alpha)*E2*E2*E2*sin(Alpha)/(Denom*Denom) +
    8*c_E0*cos(Alpha)*cos(Alpha)*cos(Alpha)*E2*E2*E2*E2*sin(Alpha)*
    (2*cos(Alpha)*cos(Alpha)*E2-2*E2+2*c_E0)/(Denom*Denom*Denom);
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::TA_GradientE2(double Ee, double Alpha)
{
  double S = sqrt(Ee*(Ee+2*c_E0));
  double Cos = cos(Alpha);

  return c_E0*c_E0*sin(Alpha)*(Ee+S*Cos)/
    (Ee*(Ee*S-3*Ee*Ee*Cos-6*Ee*Cos*c_E0+
    3*Cos*Cos*S*(Ee+2*c_E0) -
    Cos*Cos*Cos*(Ee*Ee + 4*Ee*c_E0+4*c_E0*c_E0)));
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::TA_GradientTA(double Ee, double E2)
{

  return c_E0*c_E0/(E2*E2*(Ee+2*c_E0)*sqrt(Ee*(Ee+2*c_E0)));
}


////////////////////////////////////////////////////////////////////////////////


double MComptonEvent::ComputedEnergyScatteredGamma()
{ 
  return TA_E2(m_Ee, TotalAngleViaIAs()); 
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::DistanceBetweenHits(double& Distance)
{

  Distance = (m_C1 - m_C2).Mag();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MComptonEvent::SeemsToBeAGoodEvent()
{


  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MComptonEvent::SetEventRestrictions(double MinComptonAngle, 
                                           double MaxComptonAngle)
{
  // Set the allowed range of Compton-angles:
  // ---> muss nach EventLoader!!!!

  m_MinComptonAngle = MinComptonAngle;
  m_MaxComptonAngle = MaxComptonAngle;
}
*/

// MComptonEvent: the end...
////////////////////////////////////////////////////////////////////////////////
