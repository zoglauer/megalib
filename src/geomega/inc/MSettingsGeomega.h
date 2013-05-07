/*
 * MSettingsGeomega.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsGeomega__
#define __MSettingsGeomega__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MSettings.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsGeomega : public MSettings
{
  // public Session:
 public:
  MSettingsGeomega(bool AutoLoad = true);
  virtual ~MSettingsGeomega();

  void SetMGeantOutputMode(const int Mode) { m_MGeantOutputMode = Mode; }
  int GetMGeantOutputMode() { return m_MGeantOutputMode; }

  void SetMGeantFileName(const MString& Name) { m_MGeantFileName = Name; } 
  MString GetMGeantFileName() const { return m_MGeantFileName; } 

  void SetPosition(const MVector& Position) { m_Position = Position; }
  MVector GetPosition() const { return m_Position; }

  void SetStoreIAs(const bool& StoreIAs) { m_StoreIAs = StoreIAs; }
  bool GetStoreIAs() const { return m_StoreIAs; }

  void SetStoreVetoes(const bool& StoreVetoes) { m_StoreVetoes = StoreVetoes; }
  bool GetStoreVetoes() const { return m_StoreVetoes; }


  void SetSpecialMode(const bool SpecialMode) { m_SpecialMode = SpecialMode; }
  bool GetSpecialMode() const { return m_SpecialMode; }

// protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);

  // private members:
 private:
  int m_MGeantOutputMode;
  MString m_MGeantFileName;
  MVector m_Position;
  bool m_StoreIAs;
  bool m_StoreVetoes;

  //! The special mode flag
  bool m_SpecialMode;

#ifdef ___CINT___
 public:
  ClassDef(MSettingsGeomega, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
