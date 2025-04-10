/*
 * MGUIARMExtended.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIARMExtended__
#define __MGUIARMExtended__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TApplication.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>
#include <TGComboBox.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMimrec.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! This is the UI window for the extended ARM fitting dialog
class MGUIARMExtended : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIARMExtended(const TGWindow* Parent, const TGWindow* Main, MSettingsMimrec* Data, bool& OkPressed);
  //! Default destructor
  virtual ~MGUIARMExtended();

  // protected methods:
 protected:
  //! Create the window
  virtual void Create();
  //! Process all messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  //! Save the data when OK was pressed
  virtual bool OnApply();


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The user settings data
  MSettingsMimrec* m_Settings;

  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! The number of fits to use
  MGUIEEntry* m_NumberOfFits;

  //! Check button indicating if we do want to use optimized binninf
  TGCheckButton* m_OptimizedBinning;
  const int m_OptimizedBinningID = 141;
  
  //! Entry for the theta / latitude / X value
  MGUIEEntry* m_ThetaIsX;
  //! Entry fir the phi / longitude / Y value
  MGUIEEntry* m_PhiIsY;
  //! Entry for the radius / Z value
  MGUIEEntry* m_RadiusIsZ;

  //! Entry for the ARM window
  MGUIEEntry* m_Distance;

  //! Enetry for the number of bins
  MGUIEEntry* m_Bins;

  //! The fit function combo box
  TGComboBox* m_FitFunctions;

  //! Check button indicating if we do want to use unbinned fitting
  TGCheckButton* m_UseUnbinnedFitting;
  const int m_UseUnbinnedFittingID = 142;

#ifdef ___CLING___
 public:
  ClassDef(MGUIARMExtended, 0) // Dialogbox: Select the ARM-parameters
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
