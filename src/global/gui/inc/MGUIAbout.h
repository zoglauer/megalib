/*
 * MGUIAbout.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIAbout__
#define __MGUIAbout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TObjArray.h"
#include "TGIcon.h"
#include "TGLabel.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"

// Forward declarations:
class MGUIEText;
class MGUIButton;

////////////////////////////////////////////////////////////////////////////////


class MGUIAbout : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIAbout(const TGWindow* Parent, const TGWindow* Main);
  virtual ~MGUIAbout();

  void SetProgramName(MString Name) { m_ProgramName = Name; }
  void SetIconPath(MString IconPath) { m_IconPath = IconPath; }
  void SetLeadProgrammer(MString Lead) { m_LeadProgrammer = Lead; }
  void SetProgrammers(MString Programmers) { m_Programmers = Programmers; }
  void SetAdditionalProgrammers(MString Add) { m_AdditionalProgrammers = Add; }
  void SetUpdates(MString Updates) { m_Updates = Updates; } 
  void SetCopyright(MString Copyright) { m_Copyright = Copyright; } 
  void SetMasterReference(MString Reference) { m_MasterReference = Reference; } 
  void SetReference(MString Topic, MString Reference) { 
    m_References.push_back(Reference); 
    m_Topics.push_back(Topic);
  } 

  virtual void Create();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_ProgramName; 
  MString m_IconPath; 
  MString m_LeadProgrammer;
  MString m_Programmers;
  MString m_AdditionalProgrammers;
  MString m_Email; 
  MString m_Updates; 
  MString m_Copyright;
  MString m_MasterReference;
  vector<MString> m_References;
  vector<MString> m_Topics;


#ifdef ___CINT___
 public:
  ClassDef(MGUIAbout, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
