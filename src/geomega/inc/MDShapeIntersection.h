/*
 * MDShapeIntersection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeIntersection__
#define __MDShapeIntersection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MStreams.h"
#include "MDShape.h"
#include "MDOrientation.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a boolean shape constructed by a subtraction
class MDShapeIntersection : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeIntersection(const MString& Name);
  //! Default destructor
  virtual ~MDShapeIntersection();

  //! Set the all parameters of the shape
  bool Set(MDShape* ShapeA, MDShape* ShapeB, MDOrientation* Orientation);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  //!
  MVector GetSize() { return MVector(); }

  //! Dump the content as a string
  MString ToString();
  //! Dump the content in geomega format
  MString GetGeomega() const { return ""; };
  
  
  //! Return the first shape
  MDShape* GetShapeA() const { return m_SubShapes[0]; }
  //! Return the second shape
  MDShape* GetShapeB() const { return m_SubShapes[1]; }
  //! Return the orientation of the second shape
  MDOrientation* GetOrientationShapeB() const { return m_Orientation; }

  //! Get the volume of this shape
  double GetVolume();

  //! Scale this shape by Factor
  virtual void Scale(const double Factor);

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The orientation of the second shape
  MDOrientation* m_Orientation;

  //! Almost unique position
  MVector m_AlmostUniquePosition;
  
#ifdef ___CINT___
 public:
  ClassDef(MDShapeIntersection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
