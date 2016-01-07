/*
 * MGUIEReadOutElementView.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEReadOutElementView__
#define __MGUIEReadOutElementView__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TObjArray.h>
#include <TArrayI.h>
#include <TGCanvas.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"
#include "MGUIEReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The view of all the read-out elements
class MGUIEReadOutElementView : public TGCanvas
{
  // public interface:
 public:
  //! Standard constructor
  MGUIEReadOutElementView(const TGWindow* Parent);
  //! Default destructor
  virtual ~MGUIEReadOutElementView();

  //! Create or re-create the view
  void Create();  
  
  //! Clear all the read-out elements
  void ClearReadOutElements();
  
  //! Add a new read-out element
  void Add(const MReadOutElement& ROE, unsigned int ID) { m_ROEs.push_back(ROE.Clone()); m_ROEIDs.push_back(ID); }
  
  //! Set the quality of an element
  void SetQuality(const MReadOutElement& ROE, double Quality);
  
  //! Associate the button click to this window
  void Associate(TGWindow* Associate);
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // protected methods:
 protected:


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The parent frame
  TGWindow* m_Parent; 
  //! The window we are associated with
  TGWindow* m_Associate;
  //! The container in which all elements are created
  TGCompositeFrame* m_Container;
  
  //! A list of read out elements
  vector<MReadOutElement*> m_ROEs;

  //! A list of read out element button IDs
  vector<unsigned int> m_ROEIDs;

  //! A list of read out element GUI's
  vector<MGUIEReadOutElement*> m_ROEButtons;
  
  //! Element height
  double m_ROEButtonHeight;
  
#ifdef ___CINT___
 public:
  ClassDef(MGUIEReadOutElementView, 0) // Basic GUI element: a check button list
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
