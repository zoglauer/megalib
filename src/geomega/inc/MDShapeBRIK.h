/*
 * MDShapeBRIK.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeBRIK__
#define __MDShapeBRIK__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MDShape.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a box shape
class MDShapeBRIK : public MDShape
{
  // public interface:
 public:
  //! Default constructor
  MDShapeBRIK(const MString& Name);
  //! Default destructor
  virtual ~MDShapeBRIK();

  //! Set the shape data
  bool Set(double x, double y, double z);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();

  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false);

  MString ToString();
  MString GetGeomega() const;

  // Old Geant3/MGGPOD interface
  MString GetGeant3DIM(MString ShortName);
  MString GetGeant3DATA(MString ShortName);
  MString GetGeant3ShapeName();
  int GetGeant3NumberOfParameters();
  MString GetMGeantDATA(MString ShortName);


  double GetSizeX();
  double GetSizeY();
  double GetSizeZ();

  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  //! Return a random position withn this volume
  virtual MVector GetRandomPositionInside(); 

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Dx;
  double m_Dy;
  double m_Dz;


#ifdef ___CINT___
 public:
  ClassDef(MDShapeBRIK, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
