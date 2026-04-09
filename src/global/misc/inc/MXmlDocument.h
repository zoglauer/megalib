/*
 * MXmlDocument.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MXmlDocument__
#define __MXmlDocument__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A lightweight XML document wrapper for MEGAlib-written XML files
class MXmlDocument : public MXmlNode
{
  // public interface:
 public:
  //! Default constructor
  MXmlDocument();
  //! Constructor with the document root name
  MXmlDocument(const MString& Name);
  //! Default destructor
  virtual ~MXmlDocument();

  //! Load an XML document
  bool Load(MString FileName);
  //! Save an XML document
  bool Save(MString FileName);

  // protected methods:
 protected:
  //MXmlDocument() {};
  //MXmlDocument(const MXmlDocument& XmlDocument) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MXmlDocument, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
