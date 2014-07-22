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

  //!
  MVector GetSize() { return MVector(); }

  //! Dump the content as a string
  MString ToString();
  //! Dump the content in geomega format
  MString GetGeomega() const { return ""; };
  
  // DUMMY -  Geant3/MGGPOD interface
  MString GetGeant3DIM(MString) { merr<<"MDShapeUnion does not support the Geant3/MGGPOD interface"<<error; return ""; }
  MString GetGeant3DATA(MString) { merr<<"MDShapeUnion does not support the Geant3/MGGPOD interface"<<error; return ""; }
  MString GetGeant3ShapeName() { merr<<"MDShapeUnion does not support the Geant3/MGGPOD interface"<<error; return ""; }
  int GetGeant3NumberOfParameters() { merr<<"MDShapeUnion does not support the Geant3/MGGPOD interface"<<error; return 0; }
  MString GetMGeantDATA(MString) { merr<<"MDShapeUnion does not support the Geant3/MGGPOD interface"<<error; return ""; }

  
  
  //! Return the augend (first number)
  MDShape* GetAugend() const { return m_SubShapes[0]; }
  //! Return the addend (second number)
  MDShape* GetAddend() const { return m_SubShapes[1]; }
  //! Return the orientation of the addend
  MDOrientation* GetOrientationAddend() const { return m_Orientation; }

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

  
#ifdef ___CINT___
 public:
  ClassDef(MDShapeUnion, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
