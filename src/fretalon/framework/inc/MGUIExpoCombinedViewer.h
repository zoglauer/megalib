/*
 * MGUIExpoCombinedViewer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoCombinedViewer__
#define __MGUIExpoCombinedViewer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <vector>
using namespace std;

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>
#include <TGTab.h>

// MEGAlib libs
#include "MTimer.h"
#include "MGUIDialog.h"
#include "MGUIExpo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Call sequence
//! MGUIExpoCombinedViewer* E = new MGUIExpoCombinedViewer();
//! E->Create();
//! E->AddExpos...
//! E->ShowExpos();
class MGUIExpoCombinedViewer : public MGUIDialog
{
  // Public members:
 public:
  //! Default constructor
  MGUIExpoCombinedViewer();
  //! Default destructor
  virtual ~MGUIExpoCombinedViewer();
  
  //! Create the GUI
  virtual void Create();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! Set a new expo tab
  void AddExpo(MGUIExpo* Expo) { m_Expos.push_back(Expo); }
  //! Set a new expo tab
  void AddExpos(vector<MGUIExpo*> Expos) { m_Expos.insert(m_Expos.end(), Expos.begin(), Expos.end()); }

  //! Return the number of expo's
  unsigned int GetNExpos() const { return m_Expos.size(); }
  
  //! Remove all expos
  void RemoveExpos();

  //! Show all expos
  void ShowExpos();
 
  //! Return true if we need an update
  bool NeedsUpdate();

  //! Handle the close window event
  virtual void CloseWindow();

  //! Handle the update button event
  virtual bool OnUpdate();
  //! Handle the reset button event
  virtual bool OnReset();
  //! Handle the print button event
  virtual bool OnPrint();

  
  // protected members:
 protected:



  // private members:
 private:
  //! All the expo modules
  vector<MGUIExpo*> m_Expos;
  //! The main tab...
  TGTab* m_MainTab;
  //! ... and its layout
  TGLayoutHints* m_MainTabLayout;

  //! The update timer
  MTimer m_Timer;

  // IDs:
  static const int c_Update      =   201;
  static const int c_Reset       =   202;
  static const int c_Print       =   203;
  

#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoCombinedViewer, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
