/*
 * MGUIMain.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIMain__
#define __MGUIMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TApplication.h>
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>


// MEGAlib libs
#include "MGlobal.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"

// Forward declarations
class MInterface;
class MSettings;


////////////////////////////////////////////////////////////////////////////////


class MGUIMain : public TGMainFrame
{
  // Public Session:
 public:
  MGUIMain(unsigned int Width, unsigned int Height, MInterface* Interface, MSettings* Settings);
  virtual ~MGUIMain();

  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual void CloseWindow();

  virtual void SetConfiguration(MSettings* Cfg);

  // Updates all configuration data, the status bars as well as the histories
  virtual void UpdateConfiguration();

  // protected members:
 protected:
  virtual void Open() = 0;
  virtual void Exit();
  virtual void About() = 0;

  // This adds the geometry section to the file menu --- geomega overwrites this to do nothing
  virtual void AddGeometryToFileMenu();

  virtual bool ShowGeometryDialog(); 
  virtual bool ShowLoadConfigurationDialog();
  virtual bool ShowSaveConfigurationDialog();

  virtual void UpdateFileHistory();
  virtual void UpdateGeometryHistory();

  // protected members:
 protected:
  // Menu:
  TGLayoutHints* m_MenuBarLayout;
  TGMenuBar* m_MenuBar;
  
  TGLayoutHints* m_MenuBarItemLayoutLeft; 
  TGLayoutHints* m_MenuBarItemLayoutRight; 
  TGPopupMenu* m_MenuFile;
  TGPopupMenu* m_MenuGeometryHistory;
  TGPopupMenu* m_MenuInfo;

  // Toolbar:
  TGLayoutHints* m_ToolBarLayout;
  MGUIEFlatToolBar* m_ToolBar;

  // Status:
  TGLayoutHints* m_StatusBarLayout;
  MGUIEStatusBar* m_StatusBarFile;
  MGUIEStatusBar* m_StatusBarGeo;

  // The data:
  MSettings* m_BaseData;

  // The interface:
  MInterface* m_BaseInterface;

  enum ButtonIDs { c_Open = 70, 
                   c_LoadGeometry, 
                   c_LoadConfig, 
                   c_SaveConfig, 
                   c_Exit, 
                   c_About,
                   c_FileHistory0, 
                   c_FileHistory1, 
                   c_FileHistory2, 
                   c_FileHistory3, 
                   c_FileHistory4, 
                   c_FileHistory5, 
                   c_FileHistory6, 
                   c_FileHistory7, 
                   c_FileHistory8, 
                   c_FileHistory9, 
                   c_FileHistory10, 
                   c_FileHistory11, 
                   c_FileHistory12, 
                   c_FileHistory13, 
                   c_FileHistory14, 
                   c_FileHistory15, 
                   c_FileHistory16, 
                   c_FileHistory17, 
                   c_FileHistory18, 
                   c_FileHistory19, 
                   c_GeometryHistory0, 
                   c_GeometryHistory1, 
                   c_GeometryHistory2, 
                   c_GeometryHistory3, 
                   c_GeometryHistory4, 
                   c_GeometryHistory5, 
                   c_GeometryHistory6, 
                   c_GeometryHistory7, 
                   c_GeometryHistory8, 
                   c_GeometryHistory9, 
                   c_GeometryHistory10, 
                   c_GeometryHistory11, 
                   c_GeometryHistory12, 
                   c_GeometryHistory13, 
                   c_GeometryHistory14, 
                   c_GeometryHistory15, 
                   c_GeometryHistory16, 
                   c_GeometryHistory17, 
                   c_GeometryHistory18, 
                   c_GeometryHistory19,
                   c_LastMainButtonID }; 

#ifdef ___CINT___
 public:
  ClassDef(MGUIMain, 0) // Main frame for the MEGAlib Guis
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
