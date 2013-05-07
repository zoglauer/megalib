/*
 * MGUIDialog.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUIDialog
//
//
// This is the base class for most of the other Dialogs.
// It provides some methods all other dialogs need
//
// Important info:
// Always call a derived class via
// MDerived *d = new MDerived(...)
// and never use MDerived d(..) because it will simply always and ever crash.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDialog.h"

// Standard libs:
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

// ROOT libs:
#include <KeySymbols.h>
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


//#ifdef ___CINT___
//ClassImp(MGUIDialog)
//#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDialog::MGUIDialog() 
  : TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 320, 240)
{
  // standard constructor

  m_ParentWindow = (TGFrame *) gClient->GetRoot();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDialog::MGUIDialog(const TGWindow* Root, const TGWindow* Parent, unsigned int w, 
                       unsigned int h, unsigned int options) 
  : TGTransientFrame(Root, Parent, w, h, options)
{
  m_ParentWindow = (TGFrame *) Parent;
  
  MString Path(g_MEGAlibPath + "/resource/icons/global/Icon.xpm");
  MFile::ExpandFileName(Path);
  SetIconPixmap(Path);

  m_SubTitleAdded = false;
  m_ButtonsAdded = false;

  m_SubTitleGraphics = 0;
  m_SubTitleLabel = 0;
  m_SubTitleFirstLayout = 0;
  m_SubTitleMiddleLayout = 0;
  m_SubTitleLastLayout = 0;
  m_SubTitleOnlyLayout = 0;
  
  m_LabelFrame = 0;
  m_LabelFrameLayout = 0;
  
  m_MainPanel = 0;
  m_MainPanelLayout = 0;

  m_OKButton = 0;
  m_CancelButton = 0;
  m_ApplyButton = 0;

  m_LeftButtonLayout = 0;
  m_RightButtonLayout = 0;
    
  m_ButtonFrame = 0;
  m_ButtonFrameLayout = 0;    

  const TGFont* Font = gClient->GetFont("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
  if (!Font) Font = gClient->GetResourcePool()->GetDefaultFont();
  m_EmphasizedFont = Font->GetFontStruct();

  Font = gClient->GetFont("-*-helvetica-medium-o-*-*-12-*-*-*-*-*-iso8859-1");
  if (!Font) Font = gClient->GetResourcePool()->GetDefaultFont();
  m_ItalicFont = Font->GetFontStruct();

	// Catch four keys: Esc, Ret, Ent, a
//   BindKey(this, gVirtualX->KeysymToKeycode(kKey_a), kAnyModifier);
//   BindKey(this, gVirtualX->KeysymToKeycode(kKey_A), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Escape), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Return), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Enter), kAnyModifier);
}


////////////////////////////////////////////////////////////////////////////////


MGUIDialog::~MGUIDialog()
{
  m_ParentWindow = 0;

  // Backward Compatibility:
  if (MustCleanup() == kNoCleanup) {
    delete m_SubTitleGraphics;
    delete m_SubTitleLabel;
    delete m_SubTitleFirstLayout;
    delete m_SubTitleMiddleLayout;
    delete m_SubTitleLastLayout;
    delete m_SubTitleOnlyLayout;
    
    delete m_LabelFrame;
    delete m_LabelFrameLayout;
    
    delete m_MainPanel;
    delete m_MainPanelLayout;
    
    
    delete m_OKButton;
    delete m_CancelButton;
    delete m_ApplyButton;
    
    delete m_LeftButtonLayout;
    delete m_RightButtonLayout;
    
    delete m_ButtonFrame;
    delete m_ButtonFrameLayout;    
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::CloseWindow()
{
  // When the x is pressed, this function is called.

  DeleteWindow();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::HandleKey(Event_t *event)
{
  // Here we handle all keys...

  char   tmp[10];
  unsigned int keysym;


  // Test if we have a key release:
  if (event->fType != kKeyRelease) return false;
  
  //mout<<"Hmmm... think someone had the foolish idea to press a key..."<<endl;

  // First we get the key...
  gVirtualX->LookupString(event, tmp, sizeof(tmp), keysym);
  //mout<<"Last key: "<<(int) keysym<<endl;
  
  // ... and than we do what we need to do...

  
  // The following keys need an initialized hardware
  switch ((EKeySym) keysym) {
  case kKey_Escape:
    OnCancel();
    break;
  case kKey_Return:
	case kKey_Enter:
    OnOk();
    break;
	case kKey_a:
	case kKey_A:
    OnApply();
    break;
  default:
    break;
  }

  // m_LastKey = keysym;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::PositionWindow(int Width, int Height, bool AllowResize)
{
  // This method positions the dialog window in the center of its parent window,
  // but (if possible) the whole window is shown on the screen.
  
  // First we compute the above-mentioned x and y coordinates ...
  int x = 0, y = 0;
  Window_t WindowDummy;
  gVirtualX->TranslateCoordinates(m_ParentWindow->GetId(), 
                                  GetParent()->GetId(),
																	(m_ParentWindow->GetWidth() - Width) >> 1,
																	(m_ParentWindow->GetHeight() - Height) >> 1,
                                  x, 
                                  y, 
                                  WindowDummy);


  // ... get the width and height of the display ...
  int xDisplay, yDisplay;
  unsigned int wDisplay, hDisplay;
  gVirtualX->GetGeometry(-1, xDisplay, yDisplay, wDisplay, hDisplay);


//   if (Width > 800-50 || Height > 600-50) {
//     merr<<"This GUI will probably not fit on a 800x600 screen: Width="<<Width<<" and Height="<<Height<<show;
//   }

  // ... make sure that the whole dialog window is shown on the screen ...
  if (Width > (int) wDisplay) {
    Width = wDisplay - 50;
  }
  if (Height > (int) hDisplay) {
    Height = hDisplay - 50;
  }


  if (x + Width > (int) wDisplay) {
    //x = wDisplay - Width - 8;
    x = wDisplay - Width;
	}
  if (y + Height > (int) hDisplay) {
    //y = hDisplay - Height - 28;
    y = hDisplay - Height;
	}

  if (x < 0) x = 0;
  if (y < 0) y = 0;


  // ... and bring the dialog to the calculated position with the given dimensions
  MoveResize(x, y, Width, Height);
  
  if (AllowResize == false) {
    SetWMSize(Width, Height);
    SetWMSizeHints(Width, Height, Width, Height, 0, 0);
  }
  
	// This stupid hack is required, so that the GUIs are shown on Windoof:
	// Don't remove unless you are sure that all windows are shown all the times
	// on all windows/root combinations.
	cout<<""<<flush;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIDialog::MakeSmartString(double Number)
{
  ostringstream out;
  out.precision(12); // Not too high to avoid the round-off errors
  out<<Number;
  
  MString S(out);
  S.StripBack('0');
  
  return S;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::IsNumber(MString Str)
{
  Str.ReplaceAll("1", "");
  Str.ReplaceAll("2", "");
  Str.ReplaceAll("3", "");
  Str.ReplaceAll("4", "");
  Str.ReplaceAll("5", "");
  Str.ReplaceAll("6", "");
  Str.ReplaceAll("7", "");
  Str.ReplaceAll("8", "");
  Str.ReplaceAll("9", "");
  Str.ReplaceAll("0", "");
  Str.ReplaceAll(".", "");
  Str.ReplaceAll("E", "");
  Str.ReplaceAll("e", "");
  Str.ReplaceAll("+", "");
  Str.ReplaceAll("-", "");

  return Str.IsEmpty();
}


////////////////////////////////////////////////////////////////////////////////


double MGUIDialog::MakeNumber(MString S)
{
  return S.ToDouble();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::CreateBasics()
{
  // Create the basic layout
  // Subtitle, main panel, and buttons

  if (m_SubTitleText != "") {
    AddSubTitle(m_SubTitleText);
  }

  m_MainPanel = new TGVerticalFrame(this, 100, 100);
  m_MainPanelLayout =
    new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);
  AddFrame(m_MainPanel, m_MainPanelLayout);

  AddButtons();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::FinalizeCreate(int Width, int Height, bool AllowResize)
{
  //! Perform some default mapping and positioning

  if (Width <= 0) Width = GetDefaultWidth();
  if (Height <= 0) Height = GetDefaultHeight();

  // Let's resize and position the window, 
  PositionWindow(Width, Height, AllowResize);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::AddSubTitle(const char *Format, ...)
{
  // Add a label to the top of the window:

  va_list args;
	va_start(args, Format);
	char SubTitle[1024];
	vsprintf(SubTitle, Format, args);
	AddSubTitle(MString(SubTitle));
	va_end(args);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::AddSubTitle(MString SubTitle)
{
  // Add a label to the top of the window:

 
  if (m_SubTitleAdded == false) {
    m_LabelFrame = new TGVerticalFrame(this, 100, 10, kRaisedFrame);
    m_LabelFrameLayout =
      new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 30);
    AddFrame(m_LabelFrame, m_LabelFrameLayout);
    
    m_SubTitleLabel = new TObjArray();
    m_SubTitleFirstLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 5, 1);
    m_SubTitleMiddleLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 1, 1);
    m_SubTitleLastLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 1, 5);
    m_SubTitleOnlyLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 5, 5);
    
    bool First = true;
    int LabelIndex = 0;
    MString SubString;
    
    // (start a new line for each "\n")
    while (SubTitle.Contains("\n") == true) {
      SubString = SubTitle;
      
      MString S = SubString.Remove(SubString.First('\n'), SubString.Length() - (SubString.First('\n')));
      TGLabel* Label = new TGLabel(m_LabelFrame, new TGString(S));
      Label->SetTextFont(m_EmphasizedFont);
      m_SubTitleLabel->AddAt(Label, LabelIndex++);

      if (First == true) {
        m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                               m_SubTitleFirstLayout);
        First = false;
      } else {
        m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                               m_SubTitleMiddleLayout);
      }
      
      SubTitle.Replace(0, SubTitle.First('\n')+1, "");
    }
  
    TGLabel* Label = new TGLabel(m_LabelFrame, new TGString(SubTitle));
    Label->SetTextFont(m_EmphasizedFont);
    m_SubTitleLabel->AddAt(Label, LabelIndex++);

    if (First == true) {
      m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                             m_SubTitleOnlyLayout);
    } else {
      m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                             m_SubTitleLastLayout);
    }
  } else {
    merr<<"SubTitle has already been added!"<<show;
  }

  m_SubTitleAdded = true;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDialog::AddButtons(int Buttons, bool Centered, int TopOffset)
{
  // Add the Ok- and Cancel-Buttons

  if (m_ButtonsAdded == true) {
    merr<<"The OK- and Cancel-button are already added!"<<show;
		return;
  }

	m_ButtonTypes = Buttons;

	// Frame around the buttons:
	m_ButtonFrame = new TGHorizontalFrame(this, 150, 25);
	m_ButtonFrameLayout = 
		new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
											5, 5, TopOffset, 8);
	AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
	
	// All layouts:
	if (Centered == true) {
		m_LeftButtonLayout = 
			new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 0, 0);
		m_RightButtonLayout = 
			new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 5, 5, 0, 0);
	} else {
		m_LeftButtonLayout = 
			new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 0, 0);
		m_RightButtonLayout = 
			new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 0, 0);
	}


	if (m_ButtonTypes & c_Ok) {
		m_OKButton = new TGTextButton(m_ButtonFrame, "      OK      ", e_Ok); 
		m_OKButton->Associate(this);
		m_ButtonFrame->AddFrame(m_OKButton, m_RightButtonLayout);
	}	

	if (m_ButtonTypes & c_Cancel) {
		m_CancelButton = new TGTextButton(m_ButtonFrame, "   Cancel   ", e_Cancel); 
		m_CancelButton->Associate(this);
		m_ButtonFrame->AddFrame(m_CancelButton, m_RightButtonLayout);
	}

	if (m_ButtonTypes & c_Apply) {
		m_ApplyButton = new TGTextButton(m_ButtonFrame, "   Apply   ", e_Apply); 
		m_ApplyButton->Associate(this);
		m_ButtonFrame->AddFrame(m_ApplyButton, m_RightButtonLayout);
	}

  m_ButtonsAdded = true;

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window

	bool Status = true;
	
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
				Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
				Status = OnApply();
        break;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::OnOk()
{
	// The Apply button has been pressed

	if (OnApply() == true) {
		CloseWindow();
		return true;
	}
	
	return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::OnCancel()
{
	// The Apply button has been pressed

	CloseWindow();

	return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDialog::OnApply()
{
	// The Apply button has been pressed

	// Add here...

	return true;
}


// MGUIDialog: the end...
////////////////////////////////////////////////////////////////////////////////
