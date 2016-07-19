/*
 * MGUIHistogramData.cxx
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
// MGUIHistogramData
//
//
// Dialog box, which provides entry-boxes for HistogramData-parameter (theta, phi, radius)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIHistogramData.h"

// Standard libs:
#include <sstream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIHistogramData)
#endif


////////////////////////////////////////////////////////////////////////////////


const double MGUIHistogramData::c_Radius = -numeric_limits<double>::max()/2;


////////////////////////////////////////////////////////////////////////////////


MGUIHistogramData::MGUIHistogramData(const TGWindow* Parent, 
                                     const TGWindow* Main, 
                                     bool& OkPressed, 
                                     unsigned int Width, 
                                     unsigned int Height)
  : MGUIDialog(Parent, Main, Width, Height), m_OkPressed(OkPressed)
{
  // standard constructor

  m_Title = "Title";
  m_SubTitle = "Subtitle";

  m_PositionSphericalLabel = 0;
  m_PositionSphericalName = g_StringNotDefined;
  m_PositionSphericalUnits = g_StringNotDefined;
  m_ThetaPosition = 0;
  m_ThetaEntry = 0;
  m_PhiPosition = 0;
  m_PhiEntry = 0;
  
  m_PositionCartesianLabel = 0;
  m_PositionCartesianName = g_StringNotDefined;
  m_PositionCartesianUnits = g_StringNotDefined;
  m_XPosition = 0;
  m_XPositionEntry = 0;
  m_YPosition = 0;
  m_YPositionEntry = 0;
  m_ZPosition = 0;
  m_ZPositionEntry = 0;

  m_XLabel = 0;
  m_XName = g_StringNotDefined;
  m_XUnits = g_StringNotDefined;
  m_XNBins = 100;
  m_XBinsEntry = 0;
  m_XMin = -1;
  m_XMax = 1;
  m_XMinMaxEntry = 0;
  m_XMinMaxRadius = 0;

  m_YLabel = 0;
  m_YName = g_StringNotDefined;
  m_YUnits = g_StringNotDefined;
  m_YNBins = 100;
  m_YBinsEntry = 0;
  m_YMin = -1;
  m_YMax = 1;
  m_YMinMaxEntry = 0;
  m_YMinMaxRadius = 0;

  m_ZLabel = 0;
  m_ZName = g_StringNotDefined;
  m_ZUnits = g_StringNotDefined;
  m_ZNBins = 100;
  m_ZBinsEntry = 0;
  m_ZMin = -1;
  m_ZMax = 1;
  m_ZMinMaxEntry = 0;
  m_ZMinMaxRadius = 0;

  m_OkPressed = false;
}


////////////////////////////////////////////////////////////////////////////////


MGUIHistogramData::~MGUIHistogramData()
{
  // standard destructor

  delete m_PositionSphericalLabel;
  delete m_ThetaEntry;
  delete m_PhiEntry;
  
  delete m_PositionCartesianLabel;
  delete m_XPositionEntry;
  delete m_YPositionEntry;
  delete m_ZPositionEntry;

  delete m_XLabel;
  delete m_XBinsEntry;
  delete m_XMinMaxEntry;
  delete m_XMinMaxRadius;

  delete m_YLabel;
  delete m_YBinsEntry;
  delete m_YMinMaxEntry;
  delete m_YMinMaxRadius;

  delete m_ZLabel;
  delete m_ZBinsEntry;
  delete m_ZMinMaxEntry;
  delete m_ZMinMaxRadius;

  delete m_LabelLayout;
  delete m_MainLayout;

  delete m_LabelGraphics;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIHistogramData::Create()
{
  // Create the main window

  if (m_Title != g_StringNotDefined) {
    SetWindowName(m_Title);
  }
  if (m_SubTitle != g_StringNotDefined) {
    AddSubTitle(m_SubTitle);
  }

  FontStruct_t LabelFont = 
    gClient->GetFontByName("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1"); 
  m_LabelGraphics = new TGGC(TGLabel::GetDefaultGC());
  m_LabelGraphics->SetFont(gVirtualX->GetFontHandle(LabelFont));

  m_LabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 15, 2);
  m_MainLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 2, 2);

  if (m_PositionSphericalName != g_StringNotDefined) {
    m_PositionSphericalLabel = new TGLabel(this, m_PositionSphericalName, m_LabelGraphics->GetGC(), LabelFont);
    AddFrame(m_PositionSphericalLabel, m_LabelLayout);
    ostringstream ot;
    ot<<"Theta ["<<m_XUnits<<"]: ";
    m_ThetaEntry = new MGUIEEntry(this, ot.str().c_str(), false, m_ThetaPosition);
    AddFrame(m_ThetaEntry, m_MainLayout);
    ostringstream os;
    os<<"Phi ["<<m_XUnits<<"]: ";
    m_PhiEntry = new MGUIEEntry(this, os.str().c_str(), false, m_PhiPosition);
    AddFrame(m_PhiEntry, m_MainLayout);
  }

  if (m_PositionCartesianName != g_StringNotDefined) {
    m_PositionCartesianLabel = new TGLabel(this, m_PositionCartesianName, m_LabelGraphics->GetGC(), LabelFont);
    AddFrame(m_PositionCartesianLabel, m_LabelLayout);
    ostringstream ox;
    ox<<"x ["<<m_PositionCartesianUnits<<"]: ";
    m_XPositionEntry = new MGUIEEntry(this, ox.str().c_str(), false, m_XPosition);
    ostringstream oy;
    oy<<"y ["<<m_PositionCartesianUnits<<"]: ";
    m_YPositionEntry = new MGUIEEntry(this, oy.str().c_str(), false, m_YPosition);
    ostringstream oz;
    oz<<"z ["<<m_PositionCartesianUnits<<"]: ";
    m_ZPositionEntry = new MGUIEEntry(this, oz.str().c_str(), false, m_ZPosition);
    AddFrame(m_XPositionEntry, m_MainLayout);
    AddFrame(m_YPositionEntry, m_MainLayout);
    AddFrame(m_ZPositionEntry, m_MainLayout);
  }

  if (m_XName !=  g_StringNotDefined) {
    m_XLabel = new TGLabel(this, m_XName, m_LabelGraphics->GetGC(), LabelFont);
    AddFrame(m_XLabel, m_LabelLayout);
    m_XBinsEntry = new MGUIEEntry(this, "Bins: ", false, m_XNBins);
    AddFrame(m_XBinsEntry, m_MainLayout);
    if (m_XMax == c_Radius) {
      ostringstream o;
      o<<"Radius around 0 ["<<m_XUnits<<"]: ";
      m_XMinMaxRadius = new MGUIEEntry(this, o.str().c_str(), false, m_XMin);
      AddFrame(m_XMinMaxRadius, m_MainLayout);    
    } else {
      ostringstream o;
      o<<"Min/Max ["<<m_XUnits<<"]: ";
      m_XMinMaxEntry = new MGUIEMinMaxEntry(this, o.str().c_str(), false, m_XMin, m_XMax);
      AddFrame(m_XMinMaxEntry, m_MainLayout);    
    }
  }

  if (m_YName !=  g_StringNotDefined) {
    m_YLabel = new TGLabel(this, m_YName, m_LabelGraphics->GetGC(), LabelFont);
    AddFrame(m_YLabel, m_LabelLayout);
    m_YBinsEntry = new MGUIEEntry(this, "Bins: ", false, m_YNBins);
    AddFrame(m_YBinsEntry, m_MainLayout);
    if (m_YMax == c_Radius) {
      ostringstream o;
      o<<"Radius around 0 ["<<m_YUnits<<"]: ";
      m_YMinMaxRadius = new MGUIEEntry(this, o.str().c_str(), false, m_YMin);
      AddFrame(m_YMinMaxRadius, m_MainLayout);    
    } else {
      ostringstream o;
      o<<"Min/Max ["<<m_YUnits<<"]: ";
      m_YMinMaxEntry = new MGUIEMinMaxEntry(this, o.str().c_str(), false, m_YMin, m_YMax);
      AddFrame(m_YMinMaxEntry, m_MainLayout);    
    }
  }

  if (m_ZName !=  g_StringNotDefined) {
    m_ZLabel = new TGLabel(this, m_ZName, m_LabelGraphics->GetGC(), LabelFont);
    AddFrame(m_ZLabel, m_LabelLayout);
    m_ZBinsEntry = new MGUIEEntry(this, "Bins: ", false, m_ZNBins);
    AddFrame(m_ZBinsEntry, m_MainLayout);
    if (m_ZMax == c_Radius) {
      ostringstream o;
      o<<"Radius around 0 ["<<m_ZUnits<<"]: ";
      m_ZMinMaxRadius = new MGUIEEntry(this, o.str().c_str(), false, m_ZMin);
      AddFrame(m_ZMinMaxRadius, m_MainLayout);    
    } else {
      ostringstream o;
      o<<"Min/Max ["<<m_ZUnits<<"]: ";
      m_ZMinMaxEntry = new MGUIEMinMaxEntry(this, o.str().c_str(), false, m_ZMin, m_ZMax);
      AddFrame(m_ZMinMaxEntry, m_MainLayout);    
    }
  }


  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();

  fClient->WaitForUnmap(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIHistogramData::OnOk()
{

  // Transfer data to member variables
  if (m_ThetaEntry != 0) {
    m_ThetaPosition = m_ThetaEntry->GetAsDouble();
  }
  if (m_PhiEntry != 0) {
    m_PhiPosition = m_PhiEntry->GetAsDouble();
  }
  
  if (m_XPositionEntry != 0) {
    m_XPosition = m_XPositionEntry->GetAsDouble();
  }
  if (m_YPositionEntry != 0) {
    m_YPosition = m_YPositionEntry->GetAsDouble();
  }
  if (m_ZPositionEntry != 0) {
    m_ZPosition = m_ZPositionEntry->GetAsDouble();
  }

  if (m_XBinsEntry != 0) {
    m_XNBins = m_XBinsEntry->GetAsInt();
  }
  if (m_XMinMaxEntry != 0) {
    m_XMin = m_XMinMaxEntry->GetMinValue();
    m_XMax = m_XMinMaxEntry->GetMaxValue();
  }
  if (m_XMinMaxRadius != 0) {
    m_XMin = -m_XMinMaxRadius->GetAsDouble();
    m_XMax = m_XMinMaxRadius->GetAsDouble();
  }

  if (m_YBinsEntry != 0) {
    m_YNBins = m_YBinsEntry->GetAsInt();
  }
  if (m_YMinMaxEntry != 0) {
    m_YMin = m_YMinMaxEntry->GetMinValue();
    m_YMax = m_YMinMaxEntry->GetMaxValue();
  }
  if (m_YMinMaxRadius != 0) {
    m_YMin = -m_YMinMaxRadius->GetAsDouble();
    m_YMax = m_YMinMaxRadius->GetAsDouble();
  }

  if (m_ZBinsEntry != 0) {
    m_ZNBins = m_ZBinsEntry->GetAsInt();
  }
  if (m_ZMinMaxEntry != 0) {
    m_ZMin = m_ZMinMaxEntry->GetMinValue();
    m_ZMax = m_ZMinMaxEntry->GetMaxValue();
  }
  if (m_ZMinMaxRadius != 0) {
    m_ZMin = -m_ZMinMaxRadius->GetAsDouble();
    m_ZMax = m_ZMinMaxRadius->GetAsDouble();
  }

  m_OkPressed = true;
  
  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIHistogramData::OnCancel()
{
  m_OkPressed = false;
  
  UnmapWindow();

  return true;
}

// MGUIHistogramData: the end...
////////////////////////////////////////////////////////////////////////////////
