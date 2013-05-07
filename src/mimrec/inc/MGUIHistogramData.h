/*
 * MGUIHistogramData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIHistogramData__
#define __MGUIHistogramData__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TApplication.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIHistogramData : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIHistogramData(const TGWindow* Parent, 
                    const TGWindow* Main, 
                    bool& OkPressed, 
                    unsigned int w = 320, 
                    unsigned int h = 240);
  ~MGUIHistogramData();

  void SetTitle(MString Title) { m_Title = Title; }
  void SetSubTitle(MString SubTitle) { m_SubTitle = SubTitle; }

  void AddPositionSpherical(MString Name, MString Units, 
                            double ThetaValue, double PhiValue) {
    m_PositionSphericalName = Name;
    m_PositionSphericalUnits = Units;
    m_ThetaPosition = ThetaValue;
    m_PhiPosition = PhiValue;
  }
  double GetThetaPosition() { return m_ThetaPosition; }
  double GetPhiPosition() { return m_PhiPosition; }

  void AddPositionCartesian(MString Name, MString Units, 
                            double XValue, double YValue, double ZValue) {
    m_PositionCartesianName = Name;
    m_PositionCartesianUnits = Units;
    m_XPosition = XValue;
    m_YPosition = YValue;
    m_ZPosition = ZValue;
  }
  double GetXPosition() { return m_XPosition; }
  double GetYPosition() { return m_YPosition; }
  double GetZPosition() { return m_ZPosition; }

  void AddXAxis(MString Name, MString Units, int NBins, double Min, double Max = c_Radius) {
    m_XName = Name; m_XUnits = Units; m_XNBins = NBins; m_XMin = Min; m_XMax = Max; }
  int GetXAxisNBins() { return m_XNBins; }
  double GetXAxisMin() { return m_XMin; }
  double GetXAxisMax() { return m_XMax; }

  void AddYAxis(MString Name, MString Units, int NBins, double Min, double Max = c_Radius) {
    m_YName = Name; m_YUnits = Units; m_YNBins = NBins; m_YMin = Min; m_YMax = Max; }
  int GetYAxisNBins() { return m_YNBins; }
  double GetYAxisMin() { return m_YMin; }
  double GetYAxisMax() { return m_YMax; }

  void AddZAxis(MString Name, MString Units, int NBins, double Min, double Max = c_Radius) {
    m_ZName = Name; m_ZUnits = Units; m_ZNBins = NBins; m_ZMin = Min; m_ZMax = Max; }
  int GetZAxisNBins() { return m_ZNBins; }
  double GetZAxisMin() { return m_ZMin; }
  double GetZAxisMax() { return m_ZMax; }

  void Create();

  static const double c_Radius;

  // protected methods:
 protected:

	bool OnOk();
	bool OnCancel();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;
  MString m_SubTitle;

	TGGC* m_LabelGraphics;

  TGLayoutHints* m_LabelLayout;
  TGLayoutHints* m_MainLayout;

  MString m_PositionSphericalName;
  TGLabel* m_PositionSphericalLabel;
  MString m_PositionSphericalUnits;
  double m_ThetaPosition;
  MGUIEEntry* m_ThetaEntry;
  double m_PhiPosition;
  MGUIEEntry* m_PhiEntry;
  
  MString m_PositionCartesianName;
  TGLabel* m_PositionCartesianLabel;
  MString m_PositionCartesianUnits;
  double m_XPosition;
  MGUIEEntry* m_XPositionEntry;
  double m_YPosition;
  MGUIEEntry* m_YPositionEntry;
  double m_ZPosition;
  MGUIEEntry* m_ZPositionEntry;

  MString m_XName;
  TGLabel* m_XLabel;
  MString m_XUnits;
  int m_XNBins;
  MGUIEEntry* m_XBinsEntry;
  double m_XMin;
  double m_XMax;
  MGUIEMinMaxEntry* m_XMinMaxEntry;
  MGUIEEntry* m_XMinMaxRadius;

  MString m_YName;
  TGLabel* m_YLabel;
  MString m_YUnits;
  int m_YNBins;
  MGUIEEntry* m_YBinsEntry;
  double m_YMin;
  double m_YMax;
  MGUIEMinMaxEntry* m_YMinMaxEntry;
  MGUIEEntry* m_YMinMaxRadius;

  MString m_ZName;
  TGLabel* m_ZLabel;
  MString m_ZUnits;
  int m_ZNBins;
  MGUIEEntry* m_ZBinsEntry;
  double m_ZMin;
  double m_ZMax;
  MGUIEMinMaxEntry* m_ZMinMaxEntry;
  MGUIEEntry* m_ZMinMaxRadius;

  bool& m_OkPressed;

#ifdef ___CINT___
 public:
  ClassDef(MGUIHistogramData, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
