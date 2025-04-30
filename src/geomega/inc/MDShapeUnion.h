/*
 * MDShapeUnion.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeUnion__
#define __MDShapeUnion__


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
class MDShapeUnion : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeUnion(const MString& Name);
  //! Default destructor
  virtual ~MDShapeUnion();

  //! Set the all parameters of the shape
  bool Set(MDShape* Augend, MDShape* Addend, MDOrientation* Orientation);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  //! Not implemented
  MVector GetSize() { return MVector(); }

  //! Dump the content as a string
  MString ToString();
  //! Dump the content in geomega format
  MString GetGeomega() const { return ""; };


  //! Return the augend (first number)
  MDShape* GetAugend() const { return m_SubShapes[0]; }
  //! Return the addend (second number)
  MDShape* GetAddend() const { return m_SubShapes[1]; }
  //! Return the orientation of the addend
  MDOrientation* GetOrientationAddend() const { return m_Orientation; }

  //! Get the volume of this shape
  //! Attention: For boolean volumes it is sampled thus only correct to >1%
  double GetVolume();

  //! Scale the axes given in Axes by a factor Scaler
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");

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

  
#ifdef ___CLING___
 public:
  ClassDef(MDShapeUnion, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
