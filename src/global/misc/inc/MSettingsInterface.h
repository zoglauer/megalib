/*
 * MSettingsInterface.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsInterface__
#define __MSettingsInterface__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsInterface
{
  // public Session:
 public:
  //! Standard constructor, name of the XML master node is given
  MSettingsInterface();
  //! Default destructor
  virtual ~MSettingsInterface();
  
  //! Get the modification level of the settings 
  int GetModificationLevel(bool Reset = true);


  // protected methods:
 protected:
  //! Abstract read function --- read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node) = 0;
  //! Abstract write function --- writes all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node) = 0;

  //! Change the modification level - it can ONLY be increased
  void Modify(int m);

  //! Clean the path, i.e. exchange absolute with relative path $(MEGALIB)
  MString CleanPath(MString Path);

  // private methods:
 private:



  // protected members:
 protected:
  //! The modification level 
  int m_ModificationLevel;


#ifdef ___CINT___
 public:
  ClassDef(MSettingsInterface, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
