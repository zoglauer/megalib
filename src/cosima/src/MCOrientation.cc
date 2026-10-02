/*
 * MCOrientation.cc
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
  m_EarthAlt.clear();
  m_EarthLat.clear();
  m_EarthLong.clear();
  
  m_Translations.clear();
  
  m_Rotations.clear();
  m_RotationsInvers.clear();
  
  m_IsLooping = true;
  m_LastIndex = 0;
  
  m_IsOriented = false;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse some tokenized text
bool MCOrientation::Parse(const MTokenizer& Tokenizer)
{
  // Parse some tokenized text - a rejected text must not leave a partially filled orientation behind

  Clear();
  if (ParseTokens(Tokenizer) == false) {
    Clear();
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse some tokenized text, the content may be partially filled if the text is rejected
bool MCOrientation::ParseTokens(const MTokenizer& Tokenizer)
{
  
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
    mlog<<"   ***  Error  ***  Unknown coordinate system in orientation: "<<Tokenizer.GetTokenAtAsString(2)<<endl;
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
      if (m_XThetaLat.back() > c_Pi/2 + 1E-6 || m_XThetaLat.back() < -c_Pi/2 - 1E-6) {
        mlog<<"   ***  Error  ***  Latitude value for X axis not within [-90, 90]: "<<m_XThetaLat.back()/deg<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > c_Pi/2 + 1E-6 || m_ZThetaLat.back() < -c_Pi/2 - 1E-6) {
        mlog<<"   ***  Error  ***  Latitude value for Z axis not within [-90, 90]: "<<m_ZThetaLat.back()/deg<<endl;
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
      MRotation Rotation;
      if (CalculateRotation(m_XThetaLat.back(), m_XPhiLong.back(), m_ZThetaLat.back(), m_ZPhiLong.back(), Rotation) == false) return false;
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));
      m_Rotations.push_back(Rotation);
      m_RotationsInvers.push_back(Rotation.GetInvers());
    } 
    else if (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Local) {
      // A fixed local orientation has exactly the axes x = (90 deg, 0) and z = (0, 0): the identity, which is not mirrored (nothing is oriented)
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));
      m_Rotations.push_back(MRotation());
      m_RotationsInvers.push_back(MRotation());
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
      if (m_CoordianteSystem != MCOrientationCoordinateSystem::c_Galactic) {
        mlog<<"   ***  Error  ***  OG lines are only allowed in a Galactic orientation file"<<endl;
        return false;
      }
      if (T->GetNTokens() != 6 && T->GetNTokens() != 9 &&(m_CoordianteSystem == MCOrientationCoordinateSystem::c_Galactic || m_CoordianteSystem == MCOrientationCoordinateSystem::c_Local) ) {
        mlog<<"   ***  Error  ***  Number of tokens for OG keyword must be 6 or 9"<<endl;
        return false;          
      }
      
      m_Times.push_back(T->GetTokenAtAsDouble(1)*s);
      m_XThetaLat.push_back(T->GetTokenAtAsDouble(2)*deg);
      m_XPhiLong.push_back(T->GetTokenAtAsDouble(3)*deg);
      m_ZThetaLat.push_back(T->GetTokenAtAsDouble(4)*deg);
      m_ZPhiLong.push_back(T->GetTokenAtAsDouble(5)*deg);
      
      if (T->GetNTokens() == 9){
        m_EarthAlt.push_back(T->GetTokenAtAsDouble(6)*km);
        m_EarthLat.push_back(T->GetTokenAtAsDouble(7)*deg);
        m_EarthLong.push_back(T->GetTokenAtAsDouble(8)*deg);
      }
       
      
      if (m_XThetaLat.back() > c_Pi/2 + 1E-6 || m_XThetaLat.back() < -c_Pi/2 - 1E-6) {
        mlog<<"   ***  Error  ***  Latitude value for X axis not within [-90, 90]: "<<m_XThetaLat.back()/deg<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > c_Pi/2 + 1E-6 || m_ZThetaLat.back() < -c_Pi/2 - 1E-6) {
        mlog<<"   ***  Error  ***  Latitude value for Z axis not within [-90, 90]: "<<m_ZThetaLat.back()/deg<<endl;
        return false;
      }
      
      MRotation Rotation;
      if (CalculateRotation(m_XThetaLat.back(), m_XPhiLong.back(), m_ZThetaLat.back(), m_ZPhiLong.back(), Rotation) == false) return false;
      m_Translations.push_back(MVector(0.0, 0.0, 0.0));
      m_Rotations.push_back(Rotation);
      m_RotationsInvers.push_back(Rotation.GetInvers());
      
    } else if (T->IsTokenAt(0, "OL") == true) {
      if (m_CoordianteSystem != MCOrientationCoordinateSystem::c_Local) {
        mlog<<"   ***  Error  ***  OL lines are only allowed in a Local orientation file"<<endl;
        return false;
      }
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
    
      if (m_XThetaLat.back() > c_Pi + 1E-6 || m_XThetaLat.back() < -1E-6) {
        mlog<<"   ***  Error  ***  Theta value for X axis not within [0, 180]: "<<m_XThetaLat.back()/deg<<endl;
        return false;
      }
      if (m_ZThetaLat.back() > c_Pi + 1E-6 || m_ZThetaLat.back() < -1E-6) {
        mlog<<"   ***  Error  ***  Theta value for Z axis not within [0, 180]: "<<m_ZThetaLat.back()/deg<<endl;
        return false;
      }
    
      // Local orientations are currently only applied to sources: OrientationSky and OrientationDetector accept Local Fixed only
      MRotation Rotation;
      if (CalculateRotation(m_XThetaLat.back(), m_XPhiLong.back(), m_ZThetaLat.back(), m_ZPhiLong.back(), Rotation) == false) return false;
      m_Rotations.push_back(Rotation);
      m_RotationsInvers.push_back(Rotation.GetInvers());
    }
  }
 
  // Sanity check that the times are increasing
  for (unsigned int i = 1; i < m_Times.size(); ++i) {
    if (m_Times[i] <= m_Times[i-1]) {
      mlog<<"   ***  Error  ***  The times in the ori file are not in increasing order: "<<m_Times[i]<<" (index: "<<i<<") not larger than "<<m_Times[i-1]<<""<<endl;
      return false; 
    }
  }

  // Earth coordinates are given either in all or in none of the lines
  if (m_EarthAlt.size() != 0 && m_EarthAlt.size() != m_Times.size()) {
    mlog<<"   ***  Error  ***  The Earth coordinates (altitude, latitude, longitude) must be given in all or none of the OG lines"<<endl;
    return false;
  }
 
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the rotation from the angles of the x- and z-axis (latitudes for Galactic, theta angles for local orientations).
//! The frame is always mirrored, i.e., y = -(z cross x), following the convention of MRotationInterface.
//! The axes are checked to be at a right angle and orthonormalized. The angles of an axis which had to be corrected are updated,
//! so that they describe the rotation. Nothing is changed and false is returned if the axes are invalid
bool MCOrientation::CalculateRotation(double& XThetaLat, double& XPhiLong, double& ZThetaLat, double& ZPhiLong, MRotation& Rotation) const
{
  // Galactic angles are latitudes, local angles are theta angles
  const double Offset = (m_CoordianteSystem == MCOrientationCoordinateSystem::c_Galactic) ? c_Pi/2 : 0.0;

  MVector X;
  X.SetMagThetaPhi(1.0, Offset + XThetaLat, XPhiLong);
  MVector Z;
  Z.SetMagThetaPhi(1.0, Offset + ZThetaLat, ZPhiLong);

  // Verify that x and z axis are at right angle:
  if (fabs(X.Angle(Z) - c_Pi/2.0) > 0.001) {
    mlog<<"   ***  Error  ***  The orientation axes are not at right angle, but: "<<X.Angle(Z)/deg<<" deg"<<endl;
    mlog<<"  Input: x: "<<XThetaLat/deg<<" "<<XPhiLong/deg<<"  vs. z: "<<ZThetaLat/deg<<" "<<ZPhiLong/deg<<endl;
    return false;
  }

  MVector Y = Z.Cross(X);
  // The oriented frame is left-handed on purpose (y = -(z cross x)), following the convention of MRotationInterface
  Y *= -1;

  MRotation Result(X.X(), Y.X(), Z.X(),
                   X.Y(), Y.Y(), Z.Y(),
                   X.Z(), Y.Z(), Z.Z());
  if (Result.Orthonormalize() == false) {
    mlog<<"   ***  Error  ***  The orientation axes cannot be orthonormalized"<<endl;
    return false;
  }

  // The stored angles are the reported pointing and have to be those of the transformation:
  // Update an axis only if it was corrected, thus exact input stays unchanged (also for axes at the poles, where the longitude is arbitrary)
  auto Update = [Offset](const MVector& Input, const MVector& Corrected, double& ThetaLat, double& PhiLong) {
    if ((Corrected - Input).Mag() < 1E-12) return;
    ThetaLat = Corrected.Theta() - Offset;
    double Phi = Corrected.Phi();
    PhiLong = Phi + 2*c_Pi*round((PhiLong - Phi)/(2*c_Pi));
  };
  Update(X, Result.GetX(), XThetaLat, XPhiLong);
  Update(Z, Result.GetZ(), ZThetaLat, ZPhiLong);

  Rotation = Result;

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
  // The closest entry for a time within [front, back]
  auto Closest = [this](double T) -> unsigned int {
    unsigned int Index = lower_bound(m_Times.begin(), m_Times.end(), T) - m_Times.begin();
    if (Index == 0) return 0;
    if (Index >= m_Times.size()) return m_Times.size() - 1;
    return (T - m_Times[Index-1] <= m_Times[Index] - T) ? Index - 1 : Index;
  };

  if (m_IsLooping == true) {
    if (m_Times.size() == 1) {
      return 0;
    } else if (m_Times.size() == 0) {
      throw MExceptionEmptyArray("m_Times");
      return 0;
    } else {
      // Wrap the time into [front, back)
      double Span = m_Times.back() - m_Times.front();
      double Relative = fmod(Time - m_Times.front(), Span);
      if (Relative < 0) Relative += Span;
      return Closest(m_Times.front() + Relative);
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
    return Closest(Time);
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


//! Get the Earth coordinates of the current spacecraft orbit position 
bool MCOrientation::GetEarthCoordinate(double Time, double& Alt, double& Lat, double& Long) const
{
  if (m_EarthAlt.size() != m_Times.size() || m_EarthLat.size() != m_Times.size() || m_EarthLong.size() != m_Times.size()) {
    mlog<<"   ***  Error  ***  The orientation file does not contain Earth coordinates (altitude, latitude, longitude) for all entries"<<endl;
    return false;
  }

  if (InRange(Time) == true) {
    unsigned int Index = FindClosestIndex(Time);
    Alt = m_EarthAlt[Index];
    Lat = m_EarthLat[Index];
    Long = m_EarthLong[Index];
    
    return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the orientation for the given time from local to oriented coordinate system
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


//! Perfrom the inverted orientation for the given time from oriented to local coordiante system
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
