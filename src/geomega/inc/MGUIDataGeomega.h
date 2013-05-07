/*
 * MGUIDataGeomega.h
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDataGeomega__
#define __MGUIDataGeomega__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIData.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDataGeomega : public MGUIData
{
  // public Session:
 public:
  MGUIDataGeomega();
  virtual ~MGUIDataGeomega();

  void SetMGeantOutputMode(const int Mode) { m_MGeantOutputMode = Mode; }
  int GetMGeantOutputMode() { return m_MGeantOutputMode; }

  void SetMGeantFileName(const TString& Name) { m_MGeantFileName = Name; } 
  TString GetMGeantFileName() const { return m_MGeantFileName; } 

  void SetPosition(const MVector& Position) { m_Position = Position; }
  MVector GetPosition() const { return m_Position; }

  void SetStoreIAs(const bool& StoreIAs) { m_StoreIAs = StoreIAs; }
  bool GetStoreIAs() const { return m_StoreIAs; }

  void SetStoreVetoes(const bool& StoreVetoes) { m_StoreVetoes = StoreVetoes; }
  bool GetStoreVetoes() const { return m_StoreVetoes; }

  // protected members:
 protected:
  virtual void ReadDataLocal(FILE* File);
  virtual bool ReadDataLocal(MXmlDocument* Node);
  virtual bool SaveDataLocal(MXmlDocument* Node);

  // private members:
 private:
  int m_MGeantOutputMode;
  TString m_MGeantFileName;
  MVector m_Position;
  bool m_StoreIAs;
  bool m_StoreVetoes;

#ifdef ___CINT___
 public:
  ClassDef(MGUIDataGeomega, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
