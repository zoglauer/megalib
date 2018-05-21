/*
 * MCOrientation.cxx
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
// MCOrientation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MCOrientation.hh"

// Standard libs:

// ROOT libs:
#include "TGeoMatrix.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MStreams.h"
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


MCOrientation::MCOrientation(/*const MCOrientationAxes& Axes*/)
{
  // Construct an instance of MCOrientation
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MCOrientation::~MCOrientation()
{
  // Delete this instance of MCOrientation
}


////////////////////////////////////////////////////////////////////////////////


//! Set everything to default
void MCOrientation::Clear()
{
  m_CoordianteSystem = MCOrientationCoordinateSystem::c_Local;
  
  m_Times.clear();
  
  m_XThetaLat.clear();
  m_XPhiLong.clear();
  m_ZThetaLat.clear();
  m_ZPhiLong.clear();
  
  m_Translations.clear();
  
  m_Rotations.clear();
  m_RotationsInvers.clear();
  
  m_IsLooping = true;
  m_LastIndex = 0;
  
  m_IsOriented = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MCOrientation::Parse(const MTokenizer& Tokenizer) 
{ 
  // Parse some tokenized text

  Clear();
  
  // Orientation is used for the run and the sources, thus the content aleays starts at position 2:
  
  if (Tokenizer.GetNTokens() < 4) {  //! True if we actually have an orientation
    mlog<<"   ***  Error  ***  You need at least 4 tokens to for an orientation"<<endl;
    return false;
  }

  if (Tokenizer.IsTokenAt(2, "Local") == true) {
    m_CoordianteSystem = MCOrientationCoordinateSystem::c_Local;
  } else if (Tokenizer.IsTokenAt(2, "Galactic") == true) {
    m_CoordianteSystem = MCOrientationCoordinateSystem::c_Galactic;
  } else {
    mlog<<"   ***  Error  ***  Unknown coordiante system in orientation: "<<Tokenizer.GetTokenAtAsString(2)<<endl;
    return false;
  }

  if (Tokenizer.IsTokenAt(3, "Fixed") == true) {
     m_IsLooping = true;
     if (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Local) {
      if (Tokenizer.GetNTokens() == 4) {
        m_Times.push_back(0);
        m_XThetaLat.push_back(90*deg);
        m_XPhiLong.push_back(0.0);
        m_ZThetaLat.push_back(0.0);
        m_ZPhiLong.push_back(0.0);
        m_IsLooping = true;
        m_IsOriented = false; // That's the standard here...
      } else {
        mlog<<"   ***  Error  ***  Fixed in Local coordinates cannot have more than 4 tokens"<<endl;
        return false;                         
      }
    } else if (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Galactic) {
        // Default we point towards the Galactic center
      if (Tokenizer.GetNTokens() == 4) {
        m_Times.push_back(0);
        m_XThetaLat.push_back(0.0);
        m_XPhiLong.push_back(90*deg);
        m_ZThetaLat.push_back(0.0);
        m_ZPhiLong.push_back(0.0);        
        m_IsLooping = true;
        m_IsOriented = true;
      } else if (Tokenizer.GetNTokens() == 6) {
        m_Times.push_back(0);
        m_ZThetaLat.push_back(Tokenizer.GetTokenAtAsDouble(4)*deg);
        m_ZPhiLong.push_back(Tokenizer.GetTokenAtAsDouble(5)*deg);

        // Create X component:
        MVector Z;
        Z.SetMagThetaPhi(1.0, c_Pi/2 + m_ZThetaLat.back(), m_ZPhiLong.back());
        MVector X = Z.Orthogonal();
        m_XThetaLat.push_back(X.Theta() - c_Pi/2);
        m_XPhiLong.push_back(X.Phi());
        if (m_XPhiLong.back() < 0.0) m_XPhiLong.back() += c_TwoPi;
        
        m_IsLooping = true;
        m_IsOriented = true;
      } else if (Tokenizer.GetNTokens() == 8) {
        m_Times.push_back(0);
        m_XThetaLat.push_back(Tokenizer.GetTokenAtAsDouble(4)*deg);
        m_XPhiLong.push_back(Tokenizer.GetTokenAtAsDouble(5)*deg);
        m_ZThetaLat.push_back(Tokenizer.GetTokenAtAsDouble(6)*deg);
        m_ZPhiLong.push_back(Tokenizer.GetTokenAtAsDouble(7)*deg);             
        m_IsLooping = true;
        m_IsOriented = true;
      } else {
        mlog<<"   ***  Error  ***  Fixed cannot have any additional options at the moment"<<endl;
        return false;                 
      }
      
      // Some sanity check:
      if (m_XThetaLat.back() > 90 || m_XThetaLat.back() < -90) {
        mlog<<"   ***  Error  ***  Latitude value for X axis not within [-90, 90]: "<<m_XThetaLat.back()<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > 90 || m_ZThetaLat.back() < -90) {
        mlog<<"   ***  Error  ***  Latitude value for X axis not within [-90, 90]: "<<m_ZThetaLat.back()<<endl;
        return false;
      }     
      
    } else {
      mlog<<"   ***  Error  ***  Unknown coordiante system in orientation: "<<Tokenizer.GetTokenAtAsString(2)<<endl;
      return false;     
    }
    
    // Now create the rotation
    
    if (m_Rotations.size() != m_Times.size() - 1) {
      throw MExceptionTestFailed("The rotation array is not one smaller than the time array", m_Rotations.size(), "!=", m_Times.size() - 1);
      return false;
    }
    
    if (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Galactic) {
      // First compute the y-Axis vector:
      MVector X;
      X.SetMagThetaPhi(1.0, c_Pi/2 + m_XThetaLat.back(), m_XPhiLong.back());
      MVector Z;
      Z.SetMagThetaPhi(1.0, c_Pi/2 + m_ZThetaLat.back(), m_ZPhiLong.back());
      
      // Verify that x and z axis are at right angle:
      if (fabs(X.Angle(Z) - c_Pi/2.0) > 0.001) {
        mlog<<"   ***  Error  ***  GalacticPointing axes are not at right angle, but: "<<X.Angle(Z)/deg<<" deg"<<endl;
        return false;
      }
      
      MVector Y = Z.Cross(X);
      // We need a minus here since the Galactic coordinate system in left-handed!!!!
      Y *= -1;
      
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));
      
      m_Rotations.push_back(MRotation(X.X(), Y.X(), Z.X(),
                                      X.Y(), Y.Y(), Z.Y(),
                                      X.Z(), Y.Z(), Z.Z()));
      m_RotationsInvers.push_back(m_Rotations.back().GetInvers());
    } 
    else if (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Local) {
      // First compute the y-Axis vector:
      MVector X;
      X.SetMagThetaPhi(1.0, m_XThetaLat.back(), m_XPhiLong.back());
      MVector Z;
      Z.SetMagThetaPhi(1.0, m_ZThetaLat.back(), m_ZPhiLong.back());
      
      // Verify that x and z axis are at right angle:
      if (fabs(X.Angle(Z) - c_Pi/2.0) > 0.001) {
        mlog<<"   ***  Error  ***  Pointing axes are not at right angle, but: "<<X.Angle(Z)/deg<<" deg"<<endl;
        return false;
      }
      
      MVector Y = Z.Cross(X);
      
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));
      
      m_Rotations.push_back(MRotation(X.X(), Y.X(), Z.X(),
                                      X.Y(), Y.Y(), Z.Y(),
                                      X.Z(), Y.Z(), Z.Z()));
      m_RotationsInvers.push_back(m_Rotations.back().GetInvers());
    }    

    
  } else if (Tokenizer.IsTokenAt(3, "File") == true) {
    if (Tokenizer.GetNTokens() != 6) {
      mlog<<"   ***  Error  ***  You need exactly 6 tokens to for an orientation read from file"<<endl;
      return false;
    }
    if (Tokenizer.IsTokenAt(4, "Loop") == true) {
      m_IsLooping = true;
    } else if (Tokenizer.IsTokenAt(4, "NoLoop") == true) {
      m_IsLooping = false;
    } else {
      mlog<<"   ***  Error  ***  The 5th token of an orientation read from file must be either Loop or NoLoop"<<endl;
      return false;        
    }
    
    if (Read(Tokenizer.GetTokenAtAsString(5)) == false) {
      mlog<<"   ***  Error  ***  Unable to read orientation file correctly: \""<<Tokenizer.GetTokenAtAsString(5)<<"\""<<endl;
      return false; 
    }
    
    if (m_Times.size() > 0) {
      m_IsOriented = true;
    } else {
      m_IsOriented = false; 
    }
    
  } else {
    mlog<<"   ***  Error  ***  Unknown orientation mode: "<<Tokenizer.GetTokenAtAsString(3)<<endl;
    return false;    
  }
   
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a list of orientations from file
bool MCOrientation::Read(MString FileName)
{
  MParser P;
  if (P.Open(FileName) == false) {
    mlog<<"   ***  Error  ***  Unable to open file \""<<FileName<<"\""<<endl;
    return false;      
  }
 
  for (unsigned int l = 0; l < P.GetNLines(); ++l) {
    MTokenizer* T = P.GetTokenizerAt(l);
    if (T->IsTokenAt(0, "OG") == true) {
      if (T->GetNTokens() != 6) {
        mlog<<"   ***  Error  ***  Number of tokens for OG keyword must be 6"<<endl;
        return false;          
      }
      
      m_Times.push_back(T->GetTokenAtAsDouble(1)*s);
      m_XThetaLat.push_back(T->GetTokenAtAsDouble(2)*deg);
      m_XPhiLong.push_back(T->GetTokenAtAsDouble(3)*deg);
      m_ZThetaLat.push_back(T->GetTokenAtAsDouble(4)*deg);
      m_ZPhiLong.push_back(T->GetTokenAtAsDouble(5)*deg); 
      
      if (m_XThetaLat.back() > 90 || m_XThetaLat.back() < -90) {
        mlog<<"   ***  Error  ***  Latitude value for X axis not within [-90, 90]: "<<m_XThetaLat.back()<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > 90 || m_ZThetaLat.back() < -90) {
        mlog<<"   ***  Error  ***  Latitude value for Z axis not within [-90, 90]: "<<m_ZThetaLat.back()<<endl;
        return false;
      }
      
      // First compute the y-Axis vector:
      MVector X;
      X.SetMagThetaPhi(1.0, c_Pi/2.0 + m_XThetaLat.back(), m_XPhiLong.back());
      MVector Z;
      Z.SetMagThetaPhi(1.0, c_Pi/2.0 + m_ZThetaLat.back(), m_ZPhiLong.back());
      
      // Verify that x and z axis are at right angle:
      if (fabs(X.Angle(Z) - c_Pi/2.0) > 0.001) {
        mlog<<"   ***  Error  ***  GalacticPointing axes are not at right angle, but: "<<X.Angle(Z)/deg<<" deg"<<endl;
        mlog<<"  Input: lat:"<<m_XThetaLat.back()/deg<<"  long:"<<m_XPhiLong.back()/deg<<"  vs. lat:"<<m_ZThetaLat.back()/deg<<" long:"<<m_ZPhiLong.back()/deg<<endl;
        return false;
      }
      
      MVector Y = Z.Cross(X);
      // We need a minus here since the Galactic coordinate system in left-handed!!!!
      Y *= -1;
      
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));

      m_Rotations.push_back(MRotation(X.X(), Y.X(), Z.X(),
                                      X.Y(), Y.Y(), Z.Y(),
                                      X.Z(), Y.Z(), Z.Z()));  
      m_RotationsInvers.push_back(m_Rotations.back().GetInvers());
      
    } else if (T->IsTokenAt(0, "OL") == true) {
      if (T->GetNTokens() != 9) {
        mlog<<"   ***  Error  ***  Number of tokens for OL keyword must be 9"<<endl;
        return false;          
      }
      
      m_Times.push_back(T->GetTokenAtAsDouble(1)*s);
      
      m_Translations.push_back(MVector(T->GetTokenAtAsDouble(2)*cm, T->GetTokenAtAsDouble(3)*cm, T->GetTokenAtAsDouble(4)*cm));
      
      m_XThetaLat.push_back(T->GetTokenAtAsDouble(5)*deg);
      m_XPhiLong.push_back(T->GetTokenAtAsDouble(6)*deg);
      m_ZThetaLat.push_back(T->GetTokenAtAsDouble(7)*deg);
      m_ZPhiLong.push_back(T->GetTokenAtAsDouble(8)*deg); 
    
      if (m_XThetaLat.back() > 180 || m_XThetaLat.back() < 0) {
        mlog<<"   ***  Error  ***  Theta value for X axis not within [0, 180]: "<<m_XThetaLat.back()<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > 180 || m_ZThetaLat.back() < 0) {
        mlog<<"   ***  Error  ***  Theta value for Z axis not within [0, 180]: "<<m_ZThetaLat.back()<<endl;
        return false;
      }
    
      // First compute the y-Axis vector:
      MVector X;
      X.SetMagThetaPhi(1.0, m_XThetaLat.back(), m_XPhiLong.back());
      MVector Z;
      Z.SetMagThetaPhi(1.0, m_ZThetaLat.back(), m_ZPhiLong.back());
    
      // Verify that x and z axis are at right angle:
      if (fabs(X.Angle(Z) - c_Pi/2.0) > 0.001) {
        mlog<<"   ***  Error  ***  LocalPointing axes are not at right angle, but: "<<X.Angle(Z)/deg<<" deg"<<endl;
        mlog<<"  Input: theta:"<<m_XThetaLat.back()/deg<<"  phi:"<<m_XPhiLong.back()/deg<<"  vs. theta:"<<m_ZThetaLat.back()/deg<<" phi:"<<m_ZPhiLong.back()/deg<<endl;
        return false;
      }
    
      MVector Y = Z.Cross(X);
      // We need a minus here since the Galactic coordinate system in left-handed!!!!
      Y *= -1;
    
      m_Rotations.push_back(MRotation(X.X(), Y.X(), Z.X(),
                                      X.Y(), Y.Y(), Z.Y(),
                                      X.Z(), Y.Z(), Z.Z()));  
      m_RotationsInvers.push_back(m_Rotations.back().GetInvers());
    }
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Check if Time is covered in the time array
bool MCOrientation::InRange(double Time) const
{
  if (m_Times.size() == 0) {
    return false;
  }
    
  if (m_IsLooping == false) {
    if (m_Times.front() > Time || m_Times.back() < Time) {
      return false;     
    }
  }  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////
  
  
//! Find the closest index, always check with InRange(Time) first to avoid exceptions!
unsigned int MCOrientation::FindClosestIndex(double Time) const
{
  if (m_IsLooping == true) {
    if (m_Times.size() == 1) {
      return 0;
    } else if (m_Times.size() == 0) {
      throw MExceptionEmptyArray("m_Times");
      return 0;
    } else {
      // Get it in range
      Time = fabs(fmod(Time - m_Times.front(), m_Times.back() - m_Times.front()));
      // Find and return the index
      return lower_bound(m_Times.begin(), m_Times.end(), Time) - m_Times.begin();
    }    
  } else {
    if (m_Times.size() == 0) {
      throw MExceptionEmptyArray("m_Times");
      return 0;
    }
    if (m_Times.front() > Time || m_Times.back() < Time) {
      throw MExceptionIndexOutOfBounds();
      return 0;     
    }
    return lower_bound(m_Times.begin(), m_Times.end(), Time) - m_Times.begin();    
  }
  
  
  throw MExceptionNeverReachThatLineOfCode();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the orientation
bool MCOrientation::GetOrientation(double Time, double& XThetaLat, double& XPhiLong, double& ZThetaLat, double& ZPhiLong) const
{
  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    XThetaLat = m_XThetaLat[Index];
    XPhiLong = m_XPhiLong[Index];
    ZThetaLat = m_ZThetaLat[Index];
    ZPhiLong = m_ZPhiLong[Index];
    
    return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Perfrom the orientation for the given time
bool MCOrientation::OrientPositionAndDirection(double Time, G4ThreeVector& Position, G4ThreeVector& Direction) const
{
  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    
    /*
     *    cout<<"Index: "<<Index<<" at t="<<Time/s<<endl;
     *    cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
     *    cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
     *    cout<<"Orient: T="<<m_Translations[Index]/cm<<" cm"<<endl;
     *    cout<<"Orient: R="<<m_Rotations[Index]<<endl;
     */
    
    MVector P(Position.x(), Position.y(), Position.z());
    MVector D(Direction.x(), Direction.y(), Direction.z());
    
    P = m_Rotations[Index]*P + m_Translations[Index];
    D = m_Rotations[Index]*D;
    Position.set(P.X(), P.Y(), P.Z());
    Direction.set(D.X(), D.Y(), D.Z());
    
    //cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
    //cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
  } else {
    mlog<<"   ***  Error  ***  The time is out of bounds!"<<endl;
    return false;
  }    
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perfrom the inverted orientation for the given time
bool MCOrientation::OrientPositionAndDirectionInvers(double Time, G4ThreeVector& Position, G4ThreeVector& Direction) const
{
  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    
    /*
     *    cout<<"Index: "<<Index<<" at t="<<Time/s<<endl;
     *    cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
     *    cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
     *    cout<<"Orient: T="<<m_Translations[Index]/cm<<" cm"<<endl;
     *    cout<<"Orient: R="<<m_Rotations[Index]<<endl;
     */
    
    MVector P(Position.x(), Position.y(), Position.z());
    MVector D(Direction.x(), Direction.y(), Direction.z());
    P = m_RotationsInvers[Index]*(P - m_Translations[Index]);
    D = m_RotationsInvers[Index]*D;
    Position.set(P.X(), P.Y(), P.Z());
    Direction.set(D.X(), D.Y(), D.Z());
    
    //cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
    //cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
  } else {
    mlog<<"   ***  Error  ***  The time is out of bounds!"<<endl;
    return false;
  }    
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perfrom the orientation for the given time
bool MCOrientation::OrientDirection(double Time, G4ThreeVector& Direction) const
{
  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    
    /*
     *    cout<<"Index: "<<Index<<" at t="<<Time/s<<endl;
     *    cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
     *    cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
     *    cout<<"Orient: T="<<m_Translations[Index]/cm<<" cm"<<endl;
     *    cout<<"Orient: R="<<m_Rotations[Index]<<endl;
     */
    

    MVector D(Direction.x(), Direction.y(), Direction.z());
    D = m_Rotations[Index]*D;
    Direction.set(D.X(), D.Y(), D.Z());
    
    //cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
    //cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
  } else {
    mlog<<"   ***  Error  ***  The time is out of bounds!"<<endl;
    return false;
  }    
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perfrom the inverted orientation for the given time
bool MCOrientation::OrientDirectionInvers(double Time, G4ThreeVector& Direction) const
{
  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    
    /*
     *    cout<<"Index: "<<Index<<" at t="<<Time/s<<endl;
     *    cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
     *    cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
     *    cout<<"Orient: T="<<m_Translations[Index]/cm<<" cm"<<endl;
     *    cout<<"Orient: R="<<m_Rotations[Index]<<endl;
     */
    
    MVector D(Direction.x(), Direction.y(), Direction.z());
    D = m_RotationsInvers[Index]*D;
    Direction.set(D.X(), D.Y(), D.Z());
    
    //cout<<"Orient: P="<<Position/cm<<" cm"<<endl;
    //cout<<"Orient: D="<<Direction/cm<<" cm"<<endl;
  } else {
    mlog<<"   ***  Error  ***  The time is out of bounds!"<<endl;
    return false;
  }    
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the start time or zero if there is none
double MCOrientation::GetStartTime() const
{
  if (m_IsOriented == false || m_IsLooping == true || m_Times.size() == 0) {
    return 0;
  }
  
  return m_Times[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Return the start time or zero if there is none
double MCOrientation::GetStopTime() const
{
  if (m_IsOriented == false || m_IsLooping == true || m_Times.size() == 0) {
    return 0;
  }
  
  return m_Times.back();
}


////////////////////////////////////////////////////////////////////////////////


//! Write the coordinate system integer
std::ostream& operator<<(std::ostream& os, MCOrientationCoordinateSystem C) {
  return os<<static_cast<int>(C); 
}


// MCOrientation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
