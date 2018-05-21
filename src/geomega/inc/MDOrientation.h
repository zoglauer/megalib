/*
 * MDOrientation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDOrientation__
#define __MDOrientation__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std; 

// ROOT libs:
#include "TRotMatrix.h"
#include "TGeoMatrix.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRotation.h"
#include "MTokenizer.h"
#include "MDDebugInfo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDOrientation
{
  // This is a named orientation consiting of a vector and a rotation matrix

  // public interface:
 public:
  //! Standard constructor with the name of the orientation
  MDOrientation(const MString Name);
  //! Standard copy constructor
  MDOrientation(const MDOrientation& V);
  //! Default destructor
  virtual ~MDOrientation();

  //! Return the name of the orientation
  MString GetName() const { return m_Name; }
  //! Set the name of the orientation
  void SetName(const MString& Name) { m_Name = Name; }

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  //! Return the ROOT matrix
  TGeoMatrix* GetRootMatrix() { return m_RootMatrix; }
  
  //! Set the translation
  void SetPosition(MVector Position);
  //! Set the translation
  MVector GetPosition() const { return m_Position; }


  //! Set the rotation
  void SetRotation(double x, double y, double z);
  //! Set the rotation 
  void SetRotation(MRotation Rotation);
  //! Set the rotation x{row number}{column number}
  void SetRotation(double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33);
  //! Set the rotation
  void SetRotation(MRotation RotationMatrix, int RotID);
  //! Set the rotation in Euler notarion   
  void SetRotation(double theta1, double phi1, 
                   double theta2, double phi2, 
                   double theta3, double phi3);
  //! Return as a rotation matrix
  MRotation GetRotationMatrix() const { return m_RotMatrix; }
  //! Return the inverted rotation matrix
  MRotation GetInvRotationMatrix() const { return m_InvertedRotMatrix; }
  //! The if we have a rotation
  bool IsRotated() const;

  //! Dump content into a string
  MString ToString() const;
  
  // protected methods:
 protected:

   
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Name of the vector
  MString m_Name;
  
  //! The translation of the volume 
  MVector m_Position;

  //! True is this volume is rotated
  bool m_IsRotated;
  //! The rotation matrix
  MRotation m_RotMatrix;
  //! The inverted rotation matrix
  MRotation m_InvertedRotMatrix;
  //! Rotation of the new x-Axis in MCS (Mother Coordinate System)
  double m_Theta1, m_Phi1;   
  //! Rotation of the new y-Axis in MCS
  double m_Theta2, m_Phi2;
  //! Rotation of the new z-Axis in MCS
  double m_Theta3, m_Phi3;

  //! The whole thing as ROOT matrix
  TGeoMatrix* m_RootMatrix;

#ifdef ___CLING___
 public:
  ClassDef(MDOrientation, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
