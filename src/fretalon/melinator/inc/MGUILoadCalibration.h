/*
 * MGUILoadCalibration.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUILoadCalibration__
#define __MGUILoadCalibration__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMelinator.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MIsotopeStore.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! GUI dialog showing the options for loading ans classifying the calibration file
class MGUILoadCalibration : public MGUIDialog
{
  // Public Interface:
 public:
  MGUILoadCalibration(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings);
  virtual ~MGUILoadCalibration();

  
  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();

  //! Add the isotopes to the combo box
  void AddIsotopes(TGComboBox* ComboBox, MString Select);


  // IDs
  static const int c_Isotope11        =   801;
  static const int c_Isotope12        =   802;
  static const int c_Isotope13        =   803;
  static const int c_Isotope21        =   804;
  static const int c_Isotope22        =   805;
  static const int c_Isotope23        =   806;
  static const int c_Isotope31        =   807;
  static const int c_Isotope32        =   808;
  static const int c_Isotope33        =   809;
  static const int c_Isotope41        =   810;
  static const int c_Isotope42        =   811;
  static const int c_Isotope43        =   812;
  static const int c_Isotope51        =   813;
  static const int c_Isotope52        =   814;
  static const int c_Isotope53        =   815;
  static const int c_Isotope61        =   816;
  static const int c_Isotope62        =   817;
  static const int c_Isotope63        =   818;
  static const int c_Isotope71        =   819;
  static const int c_Isotope72        =   820;
  static const int c_Isotope73        =   821;

  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;

  MSettingsMelinator* m_Settings;

  MGUIEFileSelector* m_FileName1;
  TGComboBox* m_Isotope11;
  TGComboBox* m_Isotope12;
  TGComboBox* m_Isotope13;
  MGUIEEntry* m_GroupID1; 

  MGUIEFileSelector* m_FileName2;
  TGComboBox* m_Isotope21;
  TGComboBox* m_Isotope22;
  TGComboBox* m_Isotope23;
  MGUIEEntry* m_GroupID2; 

  MGUIEFileSelector* m_FileName3;
  TGComboBox* m_Isotope31;
  TGComboBox* m_Isotope32;
  TGComboBox* m_Isotope33;
  MGUIEEntry* m_GroupID3; 

  MGUIEFileSelector* m_FileName4;
  TGComboBox* m_Isotope41;
  TGComboBox* m_Isotope42;
  TGComboBox* m_Isotope43;
  MGUIEEntry* m_GroupID4; 

  MGUIEFileSelector* m_FileName5;
  TGComboBox* m_Isotope51;
  TGComboBox* m_Isotope52;
  TGComboBox* m_Isotope53;
  MGUIEEntry* m_GroupID5; 

  MGUIEFileSelector* m_FileName6;
  TGComboBox* m_Isotope61;
  TGComboBox* m_Isotope62;
  TGComboBox* m_Isotope63;
  MGUIEEntry* m_GroupID6; 

  MGUIEFileSelector* m_FileName7;
  TGComboBox* m_Isotope71;
  TGComboBox* m_Isotope72;
  TGComboBox* m_Isotope73;
  MGUIEEntry* m_GroupID7; 

  //! The isotope store from which we load the isotopes
  MIsotopeStore m_IsotopeStore;
  

#ifdef ___CINT___
 public:
  ClassDef(MGUILoadCalibration, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
