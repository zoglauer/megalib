/*
 * MGUIEFlatToolBar.cxx
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
// MGUIEFlatToolBar
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEFlatToolBar.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>

// MEGAlib libs:
#include "MFile.h"
#include "MGUIEFlatToolBarButton.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEFlatToolBar)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEFlatToolBar::MGUIEFlatToolBar(const TGWindow* Parent, unsigned int Width, 
																	 unsigned int Height, unsigned int Options) :
	TGCompositeFrame(Parent, Width, Height, Options)
{
  // Construct an instance of MGUIEFlatToolBar

	m_Pictures = new TObjArray();
	m_Buttons = new TObjArray();
	m_Layouts = new TObjArray();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEFlatToolBar::~MGUIEFlatToolBar()
{
  // Delete this instance of MGUIEFlatToolBar

  if (MustCleanup() == kNoCleanup) {
    m_Pictures->Delete();
    m_Buttons->Delete();
    m_Layouts->Delete();
  }

	delete m_Pictures;
	delete m_Buttons;
	delete m_Layouts;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEFlatToolBar::Add(const TGWindow* Associate, MString IconFile, int Id, unsigned int Distance, MString ToolTip)
{
	// Add a button to the tool bar

	MFile::ExpandFileName(IconFile);

	TGPicture* Picture = (TGPicture*) fClient->GetPicture(IconFile);
	if (Picture == 0) {
		Error("bool MGUIEFlatToolBar::Add",
					"Picture \"%s\" not found!", IconFile.Data());
		return false;
	}
	MGUIEFlatToolBarButton* Button = new MGUIEFlatToolBarButton(this, Picture, Id);
	Button->SetFlat(true);
	Button->Associate(Associate);
	TGLayoutHints* Layout = new TGLayoutHints(kLHintsTop | kLHintsLeft, Distance, 0, 0, 0);
	AddFrame(Button, Layout);

	m_Pictures->AddLast(Picture);
	m_Buttons->AddLast(Button);
	m_Layouts->AddLast(Layout);

	return true;
}

// MGUIEFlatToolBar.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
