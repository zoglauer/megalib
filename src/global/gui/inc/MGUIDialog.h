/*
 * MGUIDialog.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDialog__
#define __MGUIDialog__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDialog : public TGTransientFrame
{
  // public Session:
 public:
  //! Default constructor
  MGUIDialog();
  //! Standard constructor
  MGUIDialog(const TGWindow* Parent, const TGWindow* Main, 
             unsigned int w = 320, unsigned int h = 240, 
             unsigned int options = kMainFrame | kVerticalFrame);
  //! Default destructor
  virtual ~MGUIDialog();

  //! Close the window
  virtual void CloseWindow();

	enum Buttons { c_Ok = BIT(0), c_Cancel = BIT(1), c_Apply  = BIT(2)};

  // protected methods:
 protected:
  //! Position the windoe on the screen
  void PositionWindow(int a, int b, bool AllowResize = true);

  //! Create the window
  virtual void Create() = 0;
  //! Create the basic layout of the window
  virtual void CreateBasics();
  //! Perform some default mapping and positioning
  virtual void FinalizeCreate(int Width = -1, int Height = -1, bool AllowResize = false);

  //! Process all messages
  virtual bool ProcessMessage(long msg, long param1, long);
  //! Handle key press events
	virtual bool HandleKey(Event_t *event);

  //! Set the subtitle text
  void SetSubTitleText(MString SubTitleText) { m_SubTitleText = SubTitleText; } 
  //! Add the subtitle, etc. --- one of the subtitle function should be called during the derived Create() 
  void AddSubTitle(MString Label);
  //! Add the subtitle, etc. --- one of the subtitle function should be called during the derived Create() 
	void AddSubTitle(const char *Format, ...);

  //! Add some buttons
  void AddButtons(int Buttons = c_Ok | c_Cancel, bool Centered = true, int TopOffset = 30);
  //! Add some buttons
  void AddOKCancelButtons() { AddButtons(c_Ok | c_Cancel); } 

  //! Called when the OK button is pressed
	virtual bool OnOk();
  //! Called when the cancel button has been pressed
	virtual bool OnCancel();
  //! Called when the applyt button has been pressed
	virtual bool OnApply();

  //! Convert a number to a string in a smart way
  MString MakeSmartString(double Number);
  //! Check if the string is a number
  bool IsNumber(MString Str);
  //! Convert a string to a number
  double MakeNumber(MString Str);


  // protected members:
 protected:
  //! The window which called this class
  TGFrame* m_ParentWindow;              

  //! The main panel of the class where all user data should be added
  TGVerticalFrame* m_MainPanel;              
  //! The layout of the main panel
  TGLayoutHints* m_MainPanelLayout;   

  //! Frame for the OK- and Cancel-button
  TGHorizontalFrame* m_ButtonFrame;    
  //! its layout
  TGLayoutHints* m_ButtonFrameLayout;   
  //! the cancel button
  TGTextButton* m_CancelButton;
  //! The Apply button
  TGTextButton* m_ApplyButton; 
  //! the OK-Button
  TGTextButton* m_OKButton;
  //! Left most buttons layout
  TGLayoutHints* m_LeftButtonLayout;
  //! Layout of right button
  TGLayoutHints* m_RightButtonLayout;
  //! true, if the buttons have been added
  bool m_ButtonsAdded;

  //! Font which is in some way emphasized (e.g. bold)
  FontStruct_t m_EmphasizedFont;
  //! Standrad font but italic
  FontStruct_t m_ItalicFont;

  //! Types of Buttons which have been added
	int m_ButtonTypes; 

	enum BasicButtonIDs { e_Ok = 1, e_Cancel, e_Apply };

  // private members:
 private:
  //! The subtitle text
  MString m_SubTitleText;
  //! Frame for the subtitle
  TGVerticalFrame* m_LabelFrame;
  //! Layout for this frame
  TGLayoutHints* m_LabelFrameLayout;
  //! Graphics mode for the subtitle
	TGGC* m_SubTitleGraphics;
  //! Layout of the subtitles
  TGLayoutHints* m_SubTitleFirstLayout;
  //! Layout of the subtitles
  TGLayoutHints* m_SubTitleMiddleLayout;
  //! Layout of the subtitles
  TGLayoutHints* m_SubTitleLastLayout;
  //! Layout of the subtitles
  TGLayoutHints* m_SubTitleOnlyLayout;
  //! Array of the subtitle rows
  TObjArray* m_SubTitleLabel;
  //! True, if a subtitle has been added
  bool m_SubTitleAdded;

};

#endif


////////////////////////////////////////////////////////////////////////////////
