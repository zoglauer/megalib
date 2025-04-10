/*
 * MDOrientation.cxx
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
// MDOrientation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDOrientation.h"

// Standard libs:

// ROOT libs:
#include "TGeoMatrix.h"

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDOrientation)
#endif


////////////////////////////////////////////////////////////////////////////////


MDOrientation::MDOrientation(const MString Name): m_Name(Name)
{
  // Construct an instance of MDOrientation
  
  m_RotMatrix.SetIdentity();
  m_InvertedRotMatrix.SetIdentity();
  m_Theta1 = 90.0;
  m_Phi1 = 0.0;
  m_Theta2 = 90.0;
  m_Phi2 = 90.0;
  m_Theta3 = 0.0;
  m_Phi3 = 0.0;
  m_IsRotated = false;
}


////////////////////////////////////////////////////////////////////////////////


MDOrientation::~MDOrientation()
{
  // Delete this instance of MDOrientation
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetPosition(MVector Position)
{
  // Set the position of this volume

  m_Position = Position;
}


////////////////////////////////////////////////////////////////////////////////


bool MDOrientation::Validate()
{
  // Correctly initialize this shape

  // delete m_RootMatrix; // --> If it has been handed over to the geometry I lost track of it...
  
  TGeoTranslation T0(m_Position[0], m_Position[1], m_Position[2]);
  TGeoRotation R0("A rotation", m_Theta1, m_Phi1, m_Theta2, m_Phi2, m_Theta3, m_Phi3);
  m_RootMatrix = new TGeoCombiTrans(T0, R0);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDOrientation::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info) 
{ 
  // Parse some tokenized text
  
  if (Tokenizer.IsTokenAt(1, "Position") == true || Tokenizer.IsTokenAt(1, "Translation") == true) {
    if (Tokenizer.GetNTokens() == 5) {
      SetPosition(MVector(Tokenizer.GetTokenAtAsDouble(2), 
                          Tokenizer.GetTokenAtAsDouble(3),
                          Tokenizer.GetTokenAtAsDouble(4)));
    } else {
      Info.Error("You have not correctly defined your translation. It is defined by 3 parameters (x, y, z)");
      return false;
    }
  } else if (Tokenizer.IsTokenAt(1, "Rotation") == true || Tokenizer.IsTokenAt(1, "Rotate") == true) {
    if (Tokenizer.GetNTokens() == 5) {
      SetRotation(Tokenizer.GetTokenAtAsDouble(2), 
                  Tokenizer.GetTokenAtAsDouble(3),
                  Tokenizer.GetTokenAtAsDouble(4));
    } else if (Tokenizer.GetNTokens() == 8) {
      SetRotation(Tokenizer.GetTokenAtAsDouble(2), 
                  Tokenizer.GetTokenAtAsDouble(3),
                  Tokenizer.GetTokenAtAsDouble(4),
                  Tokenizer.GetTokenAtAsDouble(5),
                  Tokenizer.GetTokenAtAsDouble(6),
                  Tokenizer.GetTokenAtAsDouble(7));
    } else {
      Info.Error("You have not correctly defined your translation. It is defined by either 3 parameters (around x, y, z) or by 6 parameters (Euler notation)");
      return false;
    }
  } else {
    Info.Error("Unhandled parameters for orientation!");
    return false;
  }
  
  return true; 
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetRotation(double theta1, double phi1, 
                                double theta2, double phi2, 
                                double theta3, double phi3)
{
  // Set Geant3 type rotations
  // 
  // This function only converts the rotation matrix in another format and then
  // calls another SetRotation() function which handles the rotation IDs

  MVector xcolumn, ycolumn, zcolumn;
  xcolumn.SetMagThetaPhi(1.0, theta1*c_Rad, phi1*c_Rad);
  ycolumn.SetMagThetaPhi(1.0, theta2*c_Rad, phi2*c_Rad);
  zcolumn.SetMagThetaPhi(1.0, theta3*c_Rad, phi3*c_Rad);


  // Set the columns of the rotation matrix: correct!
  MRotation Matrix;
  Matrix.SetXX(xcolumn[0]);
  Matrix.SetXY(ycolumn[0]);
  Matrix.SetXZ(zcolumn[0]);

  Matrix.SetYX(xcolumn[1]);
  Matrix.SetYY(ycolumn[1]);
  Matrix.SetYZ(zcolumn[1]);

  Matrix.SetZX(xcolumn[2]);
  Matrix.SetZY(ycolumn[2]);
  Matrix.SetZZ(zcolumn[2]);


  // We have to do some sanity checks, if the axis are orthogonal:
  bool Error = false;
  double tol = 1E-6;
  double u1u2 = xcolumn.Dot(ycolumn);
  double f12  = fabs(u1u2);
  if ( f12 > tol) {
    mout<<"   *** Error *** in orientation "<<m_Name<<endl;
    mout<<"The columns X and Y supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f12<<endl;
    Error = true;
  }
  double u1u3 = xcolumn.Dot(zcolumn);
  double f13  = fabs(u1u3);
  if ( f13 > tol) {
    mout<<"   *** Error *** in orientation "<<m_Name<<endl;
    mout<<"The columns X and Z supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f12<<endl;
    Error = true;
  }
  double u2u3 = ycolumn.Dot(zcolumn);
  double f23  = fabs(u2u3);
  if ( f23 > tol) {
    mout<<"   *** Error *** in orientation "<<m_Name<<endl;
    mout<<"The columns Y and Z supplied for rotation are not close to orthogonal: "
        <<"cos = "<<f23<<endl;
    Error = true;
  }

  if ( (f12 <= f13) && (f12 <= f23) ) {
    if (xcolumn.Cross((ycolumn - u1u2*xcolumn).Unit()).Dot(zcolumn) < 0) {
      mout<<"   *** Error *** in orientation "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"x res = "<<xcolumn.Cross((ycolumn - u1u2*xcolumn).Unit()).Dot(zcolumn)<<endl;
      Error = true;
    }
  } else if ( f13 <= f23 ) {
    if (zcolumn.Cross((xcolumn - u1u3*zcolumn).Unit()).Dot(ycolumn) < 0) {
      mout<<"   *** Error *** in orientation "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"z res = "<<zcolumn.Cross((xcolumn - u1u3*zcolumn).Unit()).Dot(ycolumn)<<endl;
      Error = true;
    }
  } else {
    if (ycolumn.Cross((zcolumn - u2u3*ycolumn).Unit()).Dot(xcolumn) < 0) {
      mout<<"   *** Error *** in orientation "<<m_Name<<endl;
      mout<<"The columns X Y and Z are closer to a reflection than a rotation: "
          <<"y res = "<<ycolumn.Cross((zcolumn - u2u3*ycolumn).Unit()).Dot(xcolumn)<<endl;
      Error = true;
    }
  }

  if (Error == false) {
    SetRotation(Matrix);
  } else {
    mout<<"Ignoring the rotation"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetRotation(double x, double y, double z)
{
  // Set the rotation of this volume
  // x,y,z are the counterclockwise rotations around x, y and z in c_Deg
  // 
  // Then call the standard SetRotation() which handles all IDs

  double sinx, siny, sinz, cosx, cosy, cosz;
  sinx = sin(x*c_Rad);
  siny = sin(y*c_Rad);
  sinz = sin(z*c_Rad);
  cosx = cos(x*c_Rad);
  cosy = cos(y*c_Rad);
  cosz = cos(z*c_Rad);

  MVector xcolumn, ycolumn, zcolumn;
  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);
  
  MRotation Matrix;
  Matrix.SetXX(  cosz*cosy);
  Matrix.SetXY(- sinz*cosx + cosz*siny*sinx);
  Matrix.SetXZ(  sinz*sinx + cosz*siny*cosx);
 
  Matrix.SetYX(  sinz*cosy);
  Matrix.SetYY(  cosz*cosx + sinz*siny*sinx);
  Matrix.SetYZ(- cosz*sinx + sinz*siny*cosx);

  Matrix.SetZX(- siny);
  Matrix.SetZY(  cosy*sinx);
  Matrix.SetZZ(  cosy*cosx);

  SetRotation(Matrix);
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetRotation(double x11, double x21, double x31, 
                                double x12, double x22, double x32, 
                                double x13, double x23, double x33)
{
  // Set the rotation x{row number}{column number}

  MRotation Matrix;
  Matrix.SetXX(x11);
  Matrix.SetXY(x21);
  Matrix.SetXZ(x31);
 
  Matrix.SetYX(x12);
  Matrix.SetYY(x22);
  Matrix.SetYZ(x32);

  Matrix.SetZX(x13);
  Matrix.SetZY(x23);
  Matrix.SetZZ(x33);

  SetRotation(Matrix);
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetRotation(MRotation Rotation)
{
  // Set the rotation of this volume and handle all IDs

  MVector xcolumn, ycolumn, zcolumn;

  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);

  // I never figuered out, why we have to invert it here...
  m_RotMatrix = Rotation.Invert();
  
  xcolumn = m_RotMatrix*xcolumn;
  ycolumn = m_RotMatrix*ycolumn;
  zcolumn = m_RotMatrix*zcolumn; 

  m_Theta1 = xcolumn.Theta()*c_Deg;
  m_Phi1 = xcolumn.Phi()*c_Deg;
  
  m_Theta2 = ycolumn.Theta()*c_Deg;
  m_Phi2 = ycolumn.Phi()*c_Deg;
  
  m_Theta3 = zcolumn.Theta()*c_Deg;
  m_Phi3 = zcolumn.Phi()*c_Deg;
  
  // Make in any case a new rotation ID, otherwise gmega is wrong for virtual volumes
  //m_RotID = m_RotIDCounter++;
  m_IsRotated = true;

  // Now invert back...
  m_InvertedRotMatrix = m_RotMatrix;
  m_RotMatrix.Invert();
}


////////////////////////////////////////////////////////////////////////////////


void MDOrientation::SetRotation(MRotation RotationMatrix, int RotID)
{
  // Set the rotation of this volume as a rotation matrix

  m_RotMatrix = RotationMatrix;
  m_InvertedRotMatrix = RotationMatrix;
  m_InvertedRotMatrix.Invert();
  //m_RotID = RotID;
  
  // Calculate rotation for Geant3
  MVector xcolumn, ycolumn, zcolumn;

  xcolumn = MVector(1.,0.,0.);
  ycolumn = MVector(0.,1.,0.);
  zcolumn = MVector(0.,0.,1.);

  MRotation RM = RotationMatrix;
  RM.Invert();

  xcolumn = RM*xcolumn;
  ycolumn = RM*ycolumn;
  zcolumn = RM*zcolumn; 
  
  m_Theta1 = xcolumn.Theta()*c_Deg;
  m_Phi1 = xcolumn.Phi()*c_Deg;
  
  m_Theta2 = ycolumn.Theta()*c_Deg;
  m_Phi2 = ycolumn.Phi()*c_Deg;
  
  m_Theta3 = zcolumn.Theta()*c_Deg;
  m_Phi3 = zcolumn.Phi()*c_Deg;

  m_IsRotated = true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDOrientation::Scale(const double Scale, const MString Axes)
{
  // Scale this shape by a factor of Scale

  if (IsScaled() == true) {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
    mout<<"The shape is already scaled."<<endl;
    return false;
  }

  for (unsigned int c = 0; c < Axes.Length(); ++c) {
    char C = Axes[c];
    if (C != 'X' && C != 'Y' && C != 'Z') {
      mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
      mout<<"The scaling axis can only contaion X, Y, or Z, and not "<<C<<"."<<endl;
      return false;
    }
  }

  if (Scale == 1.0) return true;

  if (IsRotated() == true && Axes != "XYZ") {
    mout<<"   ***  Error  ***  in shape "<<m_Name<<endl;
    mout<<"This is a rotated volumes. The only allowed scaling is \"XYZ\"."<<endl;
    return false;
  }

  m_Scaler = Scale;
  m_ScalingAxis = Axes;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! True if we are scaled
bool MDOrientation::IsScaled() const
{
  if (m_ScalingAxis != "") {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MDOrientation::ToString() const
{
  //! Dump content into a string

  ostringstream out;
  
  out<<"    Pos: "<<m_Position;
  
  return out.str();
}


// MDOrientation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
