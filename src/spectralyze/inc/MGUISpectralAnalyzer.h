/*
 * MGUISpectralAnalyzer.h
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISpectralAnalyzer__
#define __MGUISpectralAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TObjArray.h>
#include <TGTab.h>
#include <TGListBox.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsSpectralOptions.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"
#include "MGUIEFileSelector.h"
#include "MDGeometryQuest.h"


////////////////////////////////////////////////////////////////////////////////


class MGUISpectralAnalyzer : public MGUIDialog
{
  // Public Interface:
 public:
  // Blocking constructor
  MGUISpectralAnalyzer(const TGWindow* p, const TGWindow* main, MSettingsSpectralOptions* Data, bool* OkPressed = 0);
  virtual ~MGUISpectralAnalyzer();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
	virtual bool OnApply();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The GUI data 
  MSettingsSpectralOptions* m_GUIData;
  //! Reference containing the information if the OKButton has been pressed
  bool* m_OkPressed;
    
  
  TGTab* m_MainTab;

		//  MGUIEEntry* m_HistBinsSpectralyzer;
	MGUIEEntry* m_SignaltoNoiseRatio;
	MGUIEEntry* m_PoissonLimit;	
//	MGUIEEntry* m_DeconvolutionLimit;
  MGUIEFileSelector* m_IsotopeFile;
  MGUIEEntry* m_EnergyWindow;
  
  
  
  enum ButtenIds { c_TBD = 140 };


#ifdef ___CINT___
 public:
  ClassDef(MGUISpectralAnalyzer, 0) // gui window for basic event selections
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
