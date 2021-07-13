/*
 * MGUIEviewMain.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Pliease see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEviewMain__
#define __MGUIEviewMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs::
#include <list>
using std::list;

// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TGMenu.h>       
#include <TGIcon.h>
#include <TGPicture.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsEview.h"
#include "MSettingsRevan.h"
#include "MRawEventAnalyzer.h"
#include "MGeometryRevan.h"
#include "MGUIEEntryList.h"
#include "MGUIECBList.h"
#include "MGUIEFlatToolBar.h"
#include "MGUIEStatusBar.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEviewMain : public TGMainFrame
{
 private:
  static const int c_UseGuiValues = -9998;

  // Public Interface:
 public:
  MGUIEviewMain(const TGWindow* p, const TGWindow* main);
  virtual ~MGUIEviewMain();

  bool ParseCommandLine(int argc, char** argv);

  // protected methods:
 protected:
  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  void CloseWindow();


  // private methods:
 private:
  bool Open(MString FileName = "");
  bool LoadGeometry(MString FileName = "", MString StartVolumeName = "");
  void Save();
  using TGFrame::Print;
  void Print();
  void Next();
  void Back();
  void View3D();
  void Rotate();
  void ApplyRotation(double Theta = c_UseGuiValues, double Phi = c_UseGuiValues);
  void ApplyZoomMove(double X, double Y, double Z, double In);
  void Exit();
  void Loop();
  void ExitLoop();
  void About();

  bool SelectAndDraw(MRERawEvent* RE);
  bool ShowGeometry(bool Redraw = false);
  void SetOptions(bool Update = false);
  void RotationLoop();

  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;

  MSettingsEview* m_Settings;

  MRawEventAnalyzer* m_Analyzer;
  MGeometryRevan* m_Geometry;

  // Flags:
  bool m_IsLooping;
  bool m_ExitLoop;
  bool m_IsRotating;

  // Image shortcuts:
  TCanvas* m_Canvas;
  TView* m_View;

  // Temporary event storage:
  int m_SizeHistory;
  int m_PositionHistory;
  list<MRERawEvent*> m_EventHistory;


  // Menu:
  TGLayoutHints* m_MenuBarLayout;
  TGMenuBar* m_MenuBar;
  
  TGLayoutHints* m_MenuBarItemLayout; 
  TGPopupMenu* m_MenuFile;
  TGPopupMenu* m_MenuReconstruction;
  TGPopupMenu* m_MenuInfo;

  // Toolbar:
  MGUIEFlatToolBar* m_ToolBar;
  TGLayoutHints* m_ToolBarLayout;

  // Center:
  TGCompositeFrame* m_CenterFrame;
  TGLayoutHints* m_CenterFrameLayout;

  // Status:
  TGLayoutHints* m_StatusBarLayout;
  MGUIEStatusBar* m_StatusBarEvent;

  // All the options:
  TGCompositeFrame* m_OptionsFrame;
  TGLayoutHints* m_OptionsFrameLayout;

  TGLayoutHints* m_OptionsLayout;

  // - Title
  const TGPicture* m_TitlePicture;
  TGLayoutHints* m_TitleIconLayout;
  TGIcon* m_TitleIcon;


  // - Display
  MGUIECBList* m_Display;
  
  // - Angle
  MGUIEEntryList* m_Angles;
  TGCompositeFrame* m_AngleButtonFrame;
  TGLayoutHints* m_AngleButtonFrameLayout;
  TGLayoutHints* m_AngleButtonsLayout;
  TGTextButton* m_AngleSide;
  TGTextButton* m_AngleTop;
  TGTextButton* m_AngleFront;
  TGTextButton* m_AngleApply;

  // - Zoom:
  TGLabel* m_Zoom;
  TGLayoutHints* m_ZoomLayout;
  TGCompositeFrame* m_ZoomButtonFrame;
  TGLayoutHints* m_ZoomButtonFrameLayout;
  TGLayoutHints* m_ZoomButtonsLayout;
  TGTextButton* m_ZoomXPos;
  TGTextButton* m_ZoomXNeg;
  TGTextButton* m_ZoomYPos;
  TGTextButton* m_ZoomYNeg;
  TGTextButton* m_ZoomZPos;
  TGTextButton* m_ZoomZNeg;
  TGTextButton* m_ZoomIn;
  TGTextButton* m_ZoomOut;

  // - Rotation:
  MGUIEEntryList* m_Rotations;

  // - Energy:
  MGUIEEntryList* m_Energy;
  
  // - Hits:
  MGUIEEntryList* m_MinHits;

  // and Image...
  TGCompositeFrame* m_RightFrame;
  TGLayoutHints* m_RightFrameLayout;

  TRootEmbeddedCanvas* m_ImageCanvas;
  TGLayoutHints* m_ImageCanvasLayout;


  enum ButtonIDs { c_Open = 100, 
                   c_LoadGeometry, 
                   c_Exit, 
                   c_StoreImage, 
                   c_EventReconstruction, 
                   c_Coincidence, 
                   c_Clustering, 
                   c_Tracking, 
                   c_Sequencing, 
                   c_EventReconstructionSelection, 
                   c_DumpEvent, c_View3D, c_Loop, c_Stop, c_Next, c_Back, c_Rotate,
                   c_Side, c_Top, c_Front, c_ApplyRotation, c_Print,
                   c_XNeg, c_XPos, c_YNeg, c_YPos, c_ZNeg, c_ZPos, c_In, c_Out, c_About };

#ifdef ___CLING___
 public:
  ClassDef(MGUIEviewMain, 0) // GUI window: for the display of one event...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
