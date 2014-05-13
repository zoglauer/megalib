/*
 * MSettingsGlobal.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsGlobal__
#define __MSettingsGlobal__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsGlobal
{
  // public Session:
 public:
  //! The default constructor
  MSettingsGlobal();
  //! The default destructor
  virtual ~MSettingsGlobal();

  //! Read all data from the default/stored file
  virtual bool Read();
  //! Write all data to the default/stored file
  virtual bool Write();

  //! Set the license hash
  void SetLicenseHash(long LicenseHash) { m_LicenseHash = LicenseHash; }
  //! Get the license hash
  long GetLicenseHash() const { return m_LicenseHash; }

  //! Set the changelog hash
  void SetChangeLogHash(long ChangeLogHash) { m_ChangeLogHash = ChangeLogHash; }
  //! Get the changelog hash
  long GetChangeLogHash() const { return m_ChangeLogHash; }

  //! Set the font scaler
  void SetFontScaler(MString FontScaler) { m_FontScaler = FontScaler; }
  //! Get the font scaler
  MString GetFontScaler() const { return m_FontScaler; }



  // protected methods:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Writes all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private methods:
 private:



  // protected members:
 protected:
  //! Name of the master node of the document
  MString m_NameMasterNode;
  //! The settings file name
  MString m_SettingsFileName;

  //! The hash of the change-log file
  long m_ChangeLogHash;
  //! The hash of the license file
  long m_LicenseHash;
  //! The font scaling type: one of normal, large, huge, gigantic
  MString m_FontScaler;



#ifdef ___CINT___
 public:
  ClassDef(MSettingsGlobal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
