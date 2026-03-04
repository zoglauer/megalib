/*
 * MSettingsResolutions.cxx
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
// MSettingsResolutions.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsResolutions.h"

// Standard libs:
#include <limits>
using namespace std;
#include <iomanip>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MEarthHorizon.h"
#include "MProjection.h"
#include "MLMLAlgorithms.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsResolutions)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsResolutions::MSettingsResolutions() : MSettingsInterface()
{
  // default constructor

  // Test positions for ARM's etc.
  m_TPUse = false;
  m_TPTheta = 0;
  m_TPPhi = 180;
  m_TPGalLongitude = 0;
  m_TPGalLatitude = 180;
  m_TPX = 0;
  m_TPY = 0;
  m_TPZ = 100;
  m_TPDistanceTrans = 10;
  m_TPDistanceLong = 30;

  // log binning for spectra
  m_LogBinningSpectrum = false;
  
  // Bin sizes for certain histograms:
  m_HistBinsARMGamma = 181;
  m_HistBinsARMElectron = 181;
  m_HistBinsSpectrum = 200;

  // More options for the ARM
  m_OptimizeBinningARMGamma = false;
  m_NumberOfFitsARMGamma = 0;
  m_FitFunctionIDARMGamma = 5;
  m_UseUnbinnedFittingARMGamma = false;
}


////////////////////////////////////////////////////////////////////////////////


MSettingsResolutions::~MSettingsResolutions()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsResolutions::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MXmlNode* NodeLevel1 = nullptr;
  MXmlNode* NodeLevel2 = nullptr;
  MXmlNode* NodeLevel3 = nullptr;

  NodeLevel1 = new MXmlNode(Node, "Resolutions");

  // Menu ARM Gamma
  NodeLevel2 = new MXmlNode(NodeLevel1, "TestPositions");
  new MXmlNode(NodeLevel2, "Use", m_TPUse);
  NodeLevel3 = new MXmlNode(NodeLevel2, "CoordinateSystemSpherical");
  new MXmlNode(NodeLevel3, "Theta", m_TPTheta);
  new MXmlNode(NodeLevel3, "Phi", m_TPPhi);
  NodeLevel3 = new MXmlNode(NodeLevel2, "CoordinateSystemGalactic");
  new MXmlNode(NodeLevel3, "Longitude", m_TPGalLongitude);
  new MXmlNode(NodeLevel3, "Latitude", m_TPGalLatitude);
  NodeLevel3 = new MXmlNode(NodeLevel2, "CoordinateSystemCartesian");
  new MXmlNode(NodeLevel3, "X", m_TPX);
  new MXmlNode(NodeLevel3, "Y", m_TPY);
  new MXmlNode(NodeLevel3, "Z", m_TPZ);
  new MXmlNode(NodeLevel2, "DistanceTrans", m_TPDistanceTrans);
  new MXmlNode(NodeLevel2, "DistanceLong", m_TPDistanceLong);

  // Bin sizes for certain histograms
  NodeLevel2 = new MXmlNode(NodeLevel1, "LogBinning");
  new MXmlNode(NodeLevel2, "Spectrum", m_LogBinningSpectrum);

  // Bin sizes for certain histograms
  NodeLevel2 = new MXmlNode(NodeLevel1, "HistogramBins");
  new MXmlNode(NodeLevel2, "ARMGamma", m_HistBinsARMGamma);
  new MXmlNode(NodeLevel2, "ARMElectron", m_HistBinsARMElectron);
  new MXmlNode(NodeLevel2, "Spectrum", m_HistBinsSpectrum);

  // ARM Gamma additional options
  NodeLevel2 = new MXmlNode(NodeLevel1, "ARMGammaExtra");
  new MXmlNode(NodeLevel2, "OptimizeBinning", m_OptimizeBinningARMGamma);
  new MXmlNode(NodeLevel2, "NumberOfFits", m_NumberOfFitsARMGamma);
  new MXmlNode(NodeLevel2, "FitFunctionID", m_FitFunctionIDARMGamma);
  new MXmlNode(NodeLevel2, "UseUnbinnedFitting", m_UseUnbinnedFittingARMGamma);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsResolutions::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree

  MXmlNode* NodeLevel1 = nullptr;
  MXmlNode* NodeLevel2 = nullptr;
  MXmlNode* NodeLevel3 = nullptr;
  MXmlNode* NodeLevel4 = nullptr;

  if ((NodeLevel1 = Node->GetNode("Resolutions")) != nullptr) {

    if ((NodeLevel2 = NodeLevel1->GetNode("TestPositions")) != nullptr) {
      if ((NodeLevel3 = NodeLevel2->GetNode("Use")) != nullptr) {
        m_TPUse = NodeLevel3->GetValueAsBoolean();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("CoordinateSystemSpherical")) != nullptr) {
        if ((NodeLevel4 = NodeLevel3->GetNode("Theta")) != nullptr) {
          m_TPTheta = NodeLevel4->GetValueAsDouble();
        }
        if ((NodeLevel4 = NodeLevel3->GetNode("Phi")) != nullptr) {
          m_TPPhi = NodeLevel4->GetValueAsDouble();
        }
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("CoordinateSystemGalactic")) != nullptr) {
        if ((NodeLevel4 = NodeLevel3->GetNode("Longitude")) != nullptr) {
          m_TPGalLongitude = NodeLevel4->GetValueAsDouble();
        }
        if ((NodeLevel4 = NodeLevel3->GetNode("Latitude")) != nullptr) {
          m_TPGalLatitude = NodeLevel4->GetValueAsDouble();
        }
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("CoordinateSystemCartesian")) != nullptr) {
        if ((NodeLevel4 = NodeLevel3->GetNode("X")) != nullptr) {
          m_TPX = NodeLevel4->GetValueAsDouble();
        }
        if ((NodeLevel4 = NodeLevel3->GetNode("Y")) != nullptr) {
          m_TPY = NodeLevel4->GetValueAsDouble();
        }
        if ((NodeLevel4 = NodeLevel3->GetNode("Z")) != nullptr) {
          m_TPZ = NodeLevel4->GetValueAsDouble();
        }
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("DistanceTrans")) != nullptr) {
        m_TPDistanceTrans = NodeLevel3->GetValueAsDouble();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("DistanceLong")) != nullptr) {
        m_TPDistanceLong = NodeLevel3->GetValueAsDouble();
      }
    }


    if ((NodeLevel2 = NodeLevel1->GetNode("LogBinning")) != nullptr) {
      if ((NodeLevel3 = NodeLevel2->GetNode("Spectrum")) != nullptr) {
        m_LogBinningSpectrum = NodeLevel3->GetValueAsBoolean();
      }
    }

    if ((NodeLevel2 = NodeLevel1->GetNode("HistogramBins")) != nullptr) {
      if ((NodeLevel3 = NodeLevel2->GetNode("ARMGamma")) != nullptr) {
        m_HistBinsARMGamma = NodeLevel3->GetValueAsInt();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("ARMElectron")) != nullptr) {
        m_HistBinsARMElectron = NodeLevel3->GetValueAsInt();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("Spectrum")) != nullptr) {
        m_HistBinsSpectrum = NodeLevel3->GetValueAsInt();
      }
    }

    if ((NodeLevel2 = NodeLevel1->GetNode("ARMGammaExtra")) != nullptr) {
      if ((NodeLevel3 = NodeLevel2->GetNode("OptimizeBinning")) != nullptr) {
        m_OptimizeBinningARMGamma = NodeLevel3->GetValueAsBoolean();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("NumberOfFits")) != nullptr) {
        m_NumberOfFitsARMGamma = NodeLevel3->GetValueAsUnsignedInt();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("FitFunctionID")) != nullptr) {
        m_FitFunctionIDARMGamma = NodeLevel3->GetValueAsUnsignedInt();
      }
      if ((NodeLevel3 = NodeLevel2->GetNode("UseUnbinnedFitting")) != nullptr) {
        m_UseUnbinnedFittingARMGamma = NodeLevel3->GetValueAsBoolean();
      }
    }
  }

  return true;
}


// MSettingsResolutions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
