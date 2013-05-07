/*
 * MEarthHorizon.cxx
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
// MEarthHorizon
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEarthHorizon.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MFile.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEarthHorizon)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MEarthHorizon::c_NoTest           = 0;
const int MEarthHorizon::c_IntersectionTest = 1;
const int MEarthHorizon::c_ProbabilityTest  = 2;

const int MEarthHorizon::c_Min = 0;
const int MEarthHorizon::c_Max = 2;


////////////////////////////////////////////////////////////////////////////////


MEarthHorizon::MEarthHorizon()
{
  // Construct an instance of MEarthHorizon

  m_TestType = c_NoTest;
  m_PositionEarth = MVector(0, 0, -1);
  m_HorizonAngle = 90*c_Rad;
  m_MaxProbability = 1.0;
  m_ValidPairResponse = false;
  m_ValidComptonResponse = false;

//   // Create a dummy Compton response file:
//   vector<float> AxisEHCEpsilonDiff;
//   AxisEHCEpsilonDiff.push_back(0); 
//   AxisEHCEpsilonDiff.push_back(180);

//   vector<float> AxisEHCElectronEnergy;
//   AxisEHCElectronEnergy.push_back(1);
//   AxisEHCElectronEnergy.push_back(1000000);

//   MResponseMatrixO2 CR("EHC", AxisEHCEpsilonDiff, AxisEHCElectronEnergy);
//   CR.SetAxisNames("SPD [#circ]", "Measured electron energy [keV]");  
//   CR.Add(90, 1000, 1);

//   m_ComptonResponse = CR;
//   m_ValidComptonResponse = true;

  mimp<<"Pair-response ignored in earth horizon test!"<<show;
}


////////////////////////////////////////////////////////////////////////////////


MEarthHorizon::MEarthHorizon(const MEarthHorizon& EH)
{
  // Construct an instance of MEarthHorizon

  m_TestType = EH.m_TestType;
  m_PositionEarth = EH.m_PositionEarth;
  m_HorizonAngle = EH.m_HorizonAngle;
  m_MaxProbability = EH.m_MaxProbability;
  m_ComptonResponse = EH.m_ComptonResponse;
  m_PairResponse = EH.m_PairResponse;
  m_ValidComptonResponse = EH.m_ValidComptonResponse;
  m_ValidPairResponse = EH.m_ValidPairResponse;
}


////////////////////////////////////////////////////////////////////////////////


MEarthHorizon::~MEarthHorizon()
{
  // Delete this instance of MEarthHorizon
}


////////////////////////////////////////////////////////////////////////////////


const MEarthHorizon& MEarthHorizon::operator=(const MEarthHorizon& EH)
{
  m_TestType = EH.m_TestType;
  m_PositionEarth = EH.m_PositionEarth;
  m_HorizonAngle = EH.m_HorizonAngle;
  m_MaxProbability = EH.m_MaxProbability;
  m_ComptonResponse = EH.m_ComptonResponse;
  m_PairResponse = EH.m_PairResponse;
  m_ValidComptonResponse = EH.m_ValidComptonResponse;
  m_ValidPairResponse = EH.m_ValidPairResponse;

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Do not apply any earth horizon tests
bool MEarthHorizon::SetNoTest()
{
  m_TestType = c_NoTest;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Reject all events whose cone is below or intersects the horizon
bool MEarthHorizon::SetIntersectionTest()
{
  m_TestType = c_IntersectionTest;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Reject all events who have a certain probability to come from earth.  
bool MEarthHorizon::SetProbabilityTest(const double MaxProbability,
                                       const MString ComptonResponseFileName,
                                       const MString PairResponseFileName)
{
  double Return = false;

  m_MaxProbability = MaxProbability;

  m_ValidComptonResponse = false;
  m_ValidPairResponse = false;
  
  if (ComptonResponseFileName != g_StringNotDefined && ComptonResponseFileName != "") {
    if (MFile::FileExists(ComptonResponseFileName) == true) {
      if (m_ComptonResponse.Read(ComptonResponseFileName) == false) {
        mgui<<"Cannot read response file: \""<<ComptonResponseFileName<<"\""<<show;
      } else {
        // Normalize the response matrix
        int e_max = m_ComptonResponse.GetAxisBins(2);
        int p_max = m_ComptonResponse.GetAxisBins(1);
        for (int e = 0; e < e_max; ++e) {
          /*
            mimp<<"Own response !!!!!!!!"<<endl;
            for (int p = 0; p < p_max; ++p) {
            if (m_ComptonResponse.GetAxisBinCenter(p, 1) < 10) {
            m_ComptonResponse.SetBinContent(p, e, 1);
            } else {
            m_ComptonResponse.SetBinContent(p, e, 0);
            }
            m_ComptonResponse.SetBinContent(p, e, TMath::Gaus(m_ComptonResponse.GetAxisBinCenter(p, 1), 0, 30));
            }
          */
          
          double Sum = 0.0;
          for (int p = 0; p < p_max; ++p) {
            Sum += m_ComptonResponse.GetBinContent(p, e);
          }
          // Normalize to 1
          if (Sum != 0) {
            for (int p = 0; p < p_max; ++p) {
              m_ComptonResponse.SetBinContent(p, e, m_ComptonResponse.GetBinContent(p, e)/Sum);
            }
          }
          // Generate sum:
          Sum = 0.0;
          for (int p = 0; p < p_max; ++p) {
            Sum += m_ComptonResponse.GetBinContent(p, e);
            m_ComptonResponse.SetBinContent(p, e, Sum);
          }
        }
        
        m_ValidComptonResponse = true;
      }
    } else {
      mgui<<"File not found: \""<<ComptonResponseFileName<<"\""<<show;
    }

    //m_ComptonResponse.Show(MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, false);
  }


  m_TestType = c_ProbabilityTest;
  return Return;
}  


////////////////////////////////////////////////////////////////////////////////


//! Set the position of earth in detector coordinates and 
//! the azimuthal angle of the horizon realtive to this position
bool MEarthHorizon::SetEarthHorizon(const MVector& PositionEarth, 
                                    const double HorizonAngle)
{
  m_PositionEarth = PositionEarth.Unit();
  m_HorizonAngle = HorizonAngle;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the event come from earth
bool MEarthHorizon::IsEventFromEarth(MPhysicalEvent* Event, bool DumpOutput) const
{
  if (m_TestType == c_IntersectionTest) {
    return IsEventFromEarthByIntersectionTest(Event, DumpOutput);
  } else if (m_TestType == c_ProbabilityTest) {
    return IsEventFromEarthByProbabilityTest(Event, DumpOutput);
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the event originates from earth via intersection test
bool MEarthHorizon::IsEventFromEarthByIntersectionTest(MPhysicalEvent* Event, bool DumpOutput) const
{
  if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
    MComptonEvent* C = dynamic_cast<MComptonEvent*>(Event);

    double Phi = C->Phi();

    MVector ConeAxis = -C->Dg();
    
    // Rotate the ConeAxis into the Earth system:
    ConeAxis = C->GetHorizonPointingRotationMatrix()*ConeAxis;

    
    // Distance between the earth cone axis and the Compton cone axis:
    double AxisDist = m_PositionEarth.Angle(ConeAxis);
     
    if (fabs(AxisDist - Phi) > m_HorizonAngle) {
      return false;
    } else {
      if (DumpOutput == true) {
        mout<<"ID "<<Event->GetId()<<": Cone intersects earth: "<<AxisDist + Phi<<" < "<<m_HorizonAngle<<endl;
      }
      return true;
    }
  } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
    MPairEvent* P = dynamic_cast<MPairEvent*>(Event); 
    double AxisDist = m_PositionEarth.Angle(P->GetOrigin());
    
    if (AxisDist < m_HorizonAngle) {
      if (DumpOutput == true) {
        mout<<"ID "<<Event->GetId()<<": Origin inside earth: "<<AxisDist<<" < "<<m_HorizonAngle<<endl;
      }
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the event come from earth via probability test
bool MEarthHorizon::IsEventFromEarthByProbabilityTest(MPhysicalEvent* Event, bool DumpOutput) const
{
  massert(Event != 0);

  if (Event->GetEventType() == MPhysicalEvent::c_Compton) {
    MComptonEvent* C = dynamic_cast<MComptonEvent*>(Event);

    // Take care of scatter angles larger than 90 deg:
    double Phi = C->Phi();
    // cout<<"Phi: "<<Phi*c_Deg<<endl;
    MVector ConeAxis = C->Dg();
    // Rotate the ConeAxis into the Earth system:
    ConeAxis = C->GetHorizonPointingRotationMatrix()*ConeAxis;

    MVector Origin = C->DiOnCone();

    // That's the trick, but I don't remember what it means...
    if (Phi > c_Pi/2.0) {
      Phi = c_Pi - Phi;
    } else {
      ConeAxis *= -1;
    }

    // cout<<"Pos earth:"<<m_PositionEarth<<endl;
    // cout<<"ConeAxis:"<<ConeAxis<<endl;

    // Distance between the earth cone axis and the Compton cone axis:
    double EarthConeaxisDist = m_PositionEarth.Angle(ConeAxis);

		// Now determine both angles between the cone and earth (simpler now since phi always <= 90)
		// First the one towards the earth -- if it is smaller than 0 just use fabs
		double AngleEarthCone1 = EarthConeaxisDist - Phi;
		if (AngleEarthCone1 < 0) AngleEarthCone1 = fabs(AngleEarthCone1); // please don't simplify
		// Then the one away from Earth -- if it is > 180 deg then use the smaller angle
		double AngleEarthCone2 = EarthConeaxisDist + Phi;
		if (AngleEarthCone2 > c_Pi) AngleEarthCone2 -= 2*(AngleEarthCone2 - c_Pi);
		
    //mout << "ECAD:                 " << EarthConeaxisDist*180./c_Pi << endl;
    //mout << "Earth Horizon angle:  " << m_HorizonAngle*180./c_Pi << endl;
    //mout << "Phi:                  " << Phi*180./c_Pi << endl;
    //mout << "Inside earth?        (" << (EarthConeaxisDist+Phi)*180./c_Pi << " <? " << m_HorizonAngle*180./c_Pi << "  AND  " << (EarthConeaxisDist-Phi)*180./c_Pi << " >? " << -m_HorizonAngle*180./c_Pi  << ") OR (" << (EarthConeaxisDist-Phi)*180./c_Pi << " <? " << m_HorizonAngle*180./c_Pi << "  AND  " << (EarthConeaxisDist+Phi)*180./c_Pi << " >? " << 360.-m_HorizonAngle*180./c_Pi  << ")" << endl;
    //mout << "Outside earth?       (" << (EarthConeaxisDist-Phi)*180./c_Pi << " >? " << m_HorizonAngle*180./c_Pi << "  AND  " << (EarthConeaxisDist+Phi)*180./c_Pi << " <? " << 360.-m_HorizonAngle*180./c_Pi << ") OR (" << Phi*180./c_Pi << " >? " << (EarthConeaxisDist+m_HorizonAngle)*180./c_Pi << "  AND   " << Phi*180./c_Pi << " >? " << (m_HorizonAngle-EarthConeaxisDist)*180./c_Pi << ")" << endl;

    // Case a: Cone is completely inside earth
    //if (EarthConeaxisDist + Phi < m_HorizonAngle) {
    //if ( ((EarthConeaxisDist+Phi < m_HorizonAngle) && (EarthConeaxisDist-Phi > -m_HorizonAngle)) ||
		//		((EarthConeaxisDist+Phi > 2.*c_Pi-m_HorizonAngle) && (EarthConeaxisDist-Phi < m_HorizonAngle)) ) {
		if (AngleEarthCone1 < m_HorizonAngle && AngleEarthCone2 < m_HorizonAngle) {
      mdebug<<"EHC: Cone is completely inside earth"<<endl;
      if (m_MaxProbability < 1.0) {
        if (DumpOutput == true) {
          mout<<"ID "<<Event->GetId()<<": Cone inside earth"<<endl;
        }
        return true;
      } else {
				// If the probability is 1.0, we are OK with events from Earth
        return false;
      }
    }
    // Case b: Cone is completely outside earth
    // else if (EarthConeaxisDist - Phi > m_HorizonAngle || (EarthConeaxisDist == 0 && Phi > m_HorizonAngle)) {
    //else if ( ((EarthConeaxisDist+Phi > m_HorizonAngle) && (EarthConeaxisDist-Phi < -m_HorizonAngle))
	  //    || ((EarthConeaxisDist+Phi < 2.*c_Pi-m_HorizonAngle) && (EarthConeaxisDist-Phi > m_HorizonAngle)) ) {
		else if (AngleEarthCone1 > m_HorizonAngle && AngleEarthCone2 > m_HorizonAngle) {
			mdebug<<"EHC: Cone is completely outside earth"<<endl;
      //cout<<EarthConeaxisDist*c_Deg<<":"<<Phi*c_Deg<<":"<<m_HorizonAngle*c_Deg<<endl;
      return false;
    }
    // Case c: Cone intersects horizon
    else {
      // Determine the intersection points on the Compton cone:
      mdebug<<"EHC: Cone intersects horizon"<<endl;
      
      if (sin(EarthConeaxisDist) == 0 || sin(Phi) == 0) {
        merr<<"Numerical boundary: Scattered gamma-ray flew in direction of "
            <<"the earth axis and the Compton scatter angle is identical with "
            <<"horizon angle (horizon angle="<<m_HorizonAngle*c_Deg
            <<", earth-coneaxis-distance="<<EarthConeaxisDist*c_Deg
            <<") - or we have a Compton backscattering (180 deg) " 
            <<"or no scattering at all (phi="<<Phi*c_Deg<<")... "
            <<"Nevertheless, I am not rejecting event "<<Event->GetId()<<show;
        return false;
      }
      // Law of cosines for the sides of spherical triangles
      double EarthConeaxisIntersectionAngle = 
        acos((cos(m_HorizonAngle)-cos(EarthConeaxisDist)*cos(Phi))/(sin(EarthConeaxisDist)*sin(Phi)));
      //mout << "cos(ECAD):          " << (cos(m_HorizonAngle)-cos(EarthConeaxisDist)*cos(Phi))/(sin(EarthConeaxisDist)*sin(Phi)) << endl;
      //mout << "EarthConeaxisangle: " << EarthConeaxisIntersectionAngle*180./c_Pi << endl;

      if (C->HasTrack() == true && m_ValidComptonResponse == true) {
        // Now we have to determine the distance to the origin on the cone:

        double ConeaxisOriginDist = ConeAxis.Angle(Origin);
        double EarthOriginDist = m_PositionEarth.Angle(Origin);

        if (sin(ConeaxisOriginDist) == 0) {
          merr<<"Numerical boundary: The photon's origin is identical with the cone axis...!"
              <<" Nevertheless, I am not rejecting this event "<<Event->GetId()<<show;
          return false;
        }

        // Law of cosines for the sides of spherical triangles
        double EarthConeaxisOriginAngle = 
          acos((cos(EarthOriginDist) - cos(EarthConeaxisDist)*cos(ConeaxisOriginDist))/
               (sin(EarthConeaxisDist)*sin(ConeaxisOriginDist)));

        // The intersections on the cone are at EarthConeaxisOriginAngle +-EarthConeaxisIntersectionAngle
        // Let's figure out the probabilities:

        double Probability = 0;

        // Case A: "Maximum" on cone (not necessarily origin) is from *within* earth:
        if (EarthOriginDist < m_HorizonAngle) { 
          // If the intersection are in different Origin-Coneaxis-Hemispheres:
          if (EarthConeaxisOriginAngle + EarthConeaxisIntersectionAngle < c_Pi) {
            Probability = m_ComptonResponse.GetInterpolated((EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle)*c_Deg, C->Ee()) +
              m_ComptonResponse.GetInterpolated((EarthConeaxisIntersectionAngle + EarthConeaxisOriginAngle)*c_Deg, C->Ee());
            Probability = 0.5*Probability;

            massert(EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle >= 0);
            massert(EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle <= c_Pi);
            massert(EarthConeaxisIntersectionAngle + EarthConeaxisOriginAngle >= 0);
            massert(EarthConeaxisIntersectionAngle + EarthConeaxisOriginAngle <= c_Pi);

          } 
          // otherwise:
          else {
            Probability = m_ComptonResponse.GetInterpolated((2*c_Pi - EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle)*c_Deg, C->Ee()) -
              m_ComptonResponse.GetInterpolated((EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle)*c_Deg, C->Ee());
            Probability = 1-0.5*Probability;
          }
        } 
        // Case B: "Maximum" on cone (not necessarily origin) is from *outside* earth:
        else {
          // If the intersection are in different hemispheres Origin-Coneaxis-Hemispheres:
          if (EarthConeaxisOriginAngle + EarthConeaxisIntersectionAngle > c_Pi) {
            Probability = m_ComptonResponse.GetInterpolated((EarthConeaxisOriginAngle - EarthConeaxisIntersectionAngle)*c_Deg, C->Ee()) +
              m_ComptonResponse.GetInterpolated((c_Pi + EarthConeaxisIntersectionAngle - EarthConeaxisOriginAngle)*c_Deg, C->Ee());
            Probability = 1-0.5*Probability;
          } 
          // otherwise:
          else {
            Probability = m_ComptonResponse.GetInterpolated((EarthConeaxisOriginAngle + EarthConeaxisIntersectionAngle)*c_Deg, C->Ee()) -
              m_ComptonResponse.GetInterpolated((EarthConeaxisOriginAngle - EarthConeaxisIntersectionAngle)*c_Deg, C->Ee());
            Probability = 0.5*Probability;

          } 
        }

        if (Probability > m_MaxProbability) {
          if (DumpOutput == true) {
            mout<<"ID "<<Event->GetId()<<": Probability higher max probability: "<<Probability<<" > "<<m_MaxProbability<<endl;
          }
          return true;
        }
      } else {
        // The probability is simply determined by the length of the segment within earth
        if (EarthConeaxisIntersectionAngle/c_Pi > m_MaxProbability) {
          if (DumpOutput == true) {
            mout<<"ID "<<Event->GetId()<<": Probability higher max probability: "<<EarthConeaxisIntersectionAngle/c_Pi<<" > "<<m_MaxProbability<<endl;
          }
          return true;
        }
      }
    } 
  } else if (Event->GetEventType() == MPhysicalEvent::c_Pair) {
    MPairEvent* P = dynamic_cast<MPairEvent*>(Event); 
    double AxisDist = m_PositionEarth.Angle(P->GetOrigin());
    
    if (AxisDist < m_HorizonAngle) {
      if (DumpOutput == true) {
        mout<<"ID "<<Event->GetId()<<": Origin inside earth: "<<AxisDist<<" < "<<m_HorizonAngle<<endl;
      }
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const MEarthHorizon& E)
{
  os<<"Earth horizon test type: "<<E.m_TestType;
  if (E.m_TestType == MEarthHorizon::c_NoTest) {
    os<<" (no test)"<<endl;
  } else if (E.m_TestType == MEarthHorizon::c_IntersectionTest) {
    os<<" (intersection test: "<<E.m_HorizonAngle*c_Deg<<" deg)"<<endl;
  } else if (E.m_TestType == MEarthHorizon::c_ProbabilityTest) {
    os<<" (intersection test: "<<E.m_HorizonAngle*c_Deg<<" deg, p="<<E.m_MaxProbability<<")"<<endl;
  } else {
    merr<<"Not yet implemented!"<<endl;
    os<<endl;
  }
  return os;
}


// MEarthHorizon.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
