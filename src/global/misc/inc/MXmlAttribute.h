/*
 * MXmlAttribute.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MXmlAttribute__
#define __MXmlAttribute__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlData.h"

// Forward declarations:
class MXmlNode;


////////////////////////////////////////////////////////////////////////////////


//! An XML attribute used by the lightweight MEGAlib XML writer/reader
class MXmlAttribute : public MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlAttribute();
  //! Constructor without a value
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name);
  //! Constructor with a string value
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, const MString& Value);
  //! Constructor -- content is integer
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, int Value);
  //! Constructor -- content is unsigned integer
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, unsigned int Value);
  //! Constructor -- content is double
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, double Value);
  //! Constructor -- content is a boolean
  MXmlAttribute(MXmlNode* MotherNode, const MString& Name, bool Value);
  //! Default destructor
  virtual ~MXmlAttribute();

  //! Return the XML text
  virtual MString ToString();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlAttribute, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
