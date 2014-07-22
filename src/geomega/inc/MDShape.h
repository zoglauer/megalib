/*
 * MDShape.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShape__
#define __MDShape__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TShape.h>
#include <TGeoShape.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MTokenizer.h"
#include "MDDebugInfo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Base class for all shapes
class MDShape
{
  // public interface:
 public:
  //! Standard constructor requiring a name
  MDShape(const MString& Name);
  //! Default destructor
  virtual ~MDShape();

  //! Return the unique name of this shape
  MString GetName() const { return m_Name; }
  
  //! Validate the shape
  virtual bool Validate() { return false; }
  
  //! Parse some tokenized text
  virtual bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);
  
  virtual MVector GetSize() = 0;
  virtual MString ToString() = 0;
  
  //! Return the representation as ROOT shape
  TGeoShape* GetRootShape() { return m_Geo; }

  virtual MString GetGeant3DIM(MString ShortName) = 0;
  virtual MString GetGeant3DATA(MString ShortName) = 0;
  virtual MString GetGeant3ShapeName() = 0;
  virtual int GetGeant3NumberOfParameters() = 0;
  virtual MString GetMGeantDATA(MString ShortName) = 0;
  virtual MString GetGeomega() const = 0;
  virtual MString GetType();

	virtual vector<MVector> CreateSurfacePattern(const unsigned int Detail = 0) const;
  virtual bool IsInside(const MVector& Pos, const double Tolerance = 0, const bool PreferOutside = false);
  virtual double DistanceInsideOut(const MVector& Pos, const MVector& Dir, double Tolerance = 0);
  virtual double DistanceOutsideIn(const MVector& Pos, const MVector& Dir, double Tolerance = 0);

  virtual double GetVolume() = 0;
  virtual MVector GetRandomPositionInside(); 

  //! Scale this shape by Factor
  virtual void Scale(const double Scale);
  //! Get the current scale factor
  double GetScaler() const { return m_Scaler; }

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const = 0;

  //! Return the number of subshapes
  unsigned int GetNSubShapes() { return m_SubShapes.size(); }
  
  //! Return a specific subshape - if there are no subshapes or if i is out of bound return 0
  MDShape* GetSubShape(unsigned int i);
  
  void SetColor(int Color);
  int GetColor();

  static const double c_NoIntersection;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The name of the shape
  MString m_Name; 

  //! The type of the shape
  MString m_Type;
  
  //! The color of the shape
  int m_Color;

  //! The representation of this shape as ROOT TGeoShape
  TGeoShape* m_Geo;

  //! The scale factor
  double m_Scaler;
  
  //! If this shape consists of other shapes, they are stored here
  vector<MDShape*> m_SubShapes;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MDShape, 0) // virtual base class for all shapes
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
