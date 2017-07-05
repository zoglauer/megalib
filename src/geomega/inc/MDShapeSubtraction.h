/*
 * MDShapeSubtraction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeSubtraction__
#define __MDShapeSubtraction__


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
class MDShapeSubtraction : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeSubtraction(const MString& Name);
  //! Default destructor
  virtual ~MDShapeSubtraction();

  //! Set the all parameters of the shape
  bool Set(MDShape* Minuend, MDShape* Subtrahend, MDOrientation* Orientation);

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

  
  
  //! Return the minuend (first number)
  MDShape* GetMinuend() const { return m_SubShapes[0]; }
  //! Return the subtrahend (second number)
  MDShape* GetSubtrahend() const { return m_SubShapes[1]; }
  //! Return the orientation of the subtrahend
  MDOrientation* GetOrientationSubtrahend() const { return m_Orientation; }

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
  //! The orientation of the subtrahend
  MDOrientation* m_Orientation;

  //! Almost unique position
  MVector m_AlmostUniquePosition;
  
#ifdef ___CINT___
 public:
  ClassDef(MDShapeSubtraction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
