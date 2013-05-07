/*
 * MSettingsSivan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsSivan__
#define __MSettingsSivan__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsSivan : public MSettings
{
  // public Session:
 public:
  MSettingsSivan(bool AutoLoad = true);
  ~MSettingsSivan();

  void SetRealism(const int Realism);
  int GetRealism() const;

  void SetNInitializationEvents(const int NEvents) { m_NInitializationEvents = NEvents; }
  unsigned int GetNInitializationEvents() const{ return m_NInitializationEvents; }


  //! Set the special GUI mode (this is not saved to file!)
  void SetSpecialMode(const bool SpecialMode) { m_SpecialMode = SpecialMode; }
  //! Get the special GUI mode (this is not saved to file!)
  bool GetSpecialMode() const { return m_SpecialMode; }

  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:
  //! Set the realism, i.e. is noising on or off
  int m_Realism;
  //! Number of events which are are used to initialize the initial size of dimensional hitsograms
  int m_NInitializationEvents;

  //! The special GUI mode flag
  bool m_SpecialMode;


#ifdef ___CINT___
 public:
  ClassDef(MSettingsSivan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
