/*
 * MSettingsEview.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsEview__
#define __MSettingsEview__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"
#include "MSettingsEventReconstruction.h"
// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsEview : public MSettings, public MSettingsEventReconstruction
{
  // public Session:
 public:
  MSettingsEview(bool AutoLoad = true);
  ~MSettingsEview();

  double GetAngleTheta() { return m_AngleTheta; } 
  void SetAngleTheta(double AngleTheta) { m_AngleTheta = AngleTheta; } 
  double GetAnglePhi() { return m_AnglePhi; } 
  void SetAnglePhi(double AnglePhi) { m_AnglePhi = AnglePhi; } 

  double GetRotationTheta() { return m_RotationTheta; } 
  void SetRotationTheta(double RotationTheta) { m_RotationTheta = RotationTheta; } 
  double GetRotationPhi() { return m_RotationPhi; } 
  void SetRotationPhi(double RotationPhi) { m_RotationPhi = RotationPhi; } 

  bool GetReconstruct() { return m_Reconstruct; }
  void SetReconstruct(bool Reconstruct) { m_Reconstruct = Reconstruct; } 
  bool GetDisplaySequence() { return m_DisplaySequence; }
  void SetDisplaySequence(bool DisplaySequence) { m_DisplaySequence = DisplaySequence; } 
  bool GetDisplayOrigin() { return m_DisplayOrigin; }
  void SetDisplayOrigin(bool DisplayOrigin) { m_DisplayOrigin = DisplayOrigin; } 
  bool GetDisplayOnlyGoodEvents() { return m_DisplayOnlyGoodEvents; }
  void SetDisplayOnlyGoodEvents(bool DisplayOnlyGoodEvents) { m_DisplayOnlyGoodEvents = DisplayOnlyGoodEvents; } 

  double GetEnergyMin() { return m_EnergyMin; } 
  void SetEnergyMin(double EnergyMin) { m_EnergyMin = EnergyMin; } 
  double GetEnergyMax() { return m_EnergyMax; } 
  void SetEnergyMax(double EnergyMax) { m_EnergyMax = EnergyMax; } 

  double GetPositionXNeg() { return m_PositionXNeg; }
  void SetPositionXNeg(double PositionXNeg) { m_PositionXNeg = PositionXNeg; } 
  double GetPositionXPos() { return m_PositionXPos; }
  void SetPositionXPos(double PositionXPos) { m_PositionXPos = PositionXPos; } 
  double GetPositionYNeg() { return m_PositionYNeg; }
  void SetPositionYNeg(double PositionYNeg) { m_PositionYNeg = PositionYNeg; } 
  double GetPositionYPos() { return m_PositionYPos; }
  void SetPositionYPos(double PositionYPos) { m_PositionYPos = PositionYPos; } 
  double GetPositionZNeg() { return m_PositionZNeg; }
  void SetPositionZNeg(double PositionZNeg) { m_PositionZNeg = PositionZNeg; } 
  double GetPositionZPos() { return m_PositionZPos; }
  void SetPositionZPos(double PositionZPos) { m_PositionZPos = PositionZPos; } 

  int GetMinHitsTotal() { return m_MinHitsTotal; } 
  void SetMinHitsTotal(int MinHitsTotal) { m_MinHitsTotal = MinHitsTotal; } 
  int GetMinHitsD1() { return m_MinHitsD1; } 
  void SetMinHitsD1(int MinHitsD1) { m_MinHitsD1 = MinHitsD1; } 
  int GetMinHitsD2() { return m_MinHitsD2; } 
  void SetMinHitsD2(int MinHitsD2) { m_MinHitsD2 = MinHitsD2; } 


  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:
  double m_AngleTheta;
  double m_AnglePhi;

  double m_RotationTheta;
  double m_RotationPhi;

  double m_PositionXPos;
  double m_PositionXNeg;
  double m_PositionYPos;
  double m_PositionYNeg;
  double m_PositionZPos;
  double m_PositionZNeg;

  bool m_Reconstruct;
  bool m_DisplaySequence;
  bool m_DisplayOrigin;
  bool m_DisplayOnlyGoodEvents;

  double m_EnergyMin;
  double m_EnergyMax;

  int m_MinHitsTotal;
  int m_MinHitsD1;
  int m_MinHitsD2;

#ifdef ___CINT___
 public:
  ClassDef(MSettingsEview, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
