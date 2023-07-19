/*
 * MSettingsMelinator.cxx
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
// MSettingsMelinator.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsMelinator.h"

// Standard libs:
#include <algorithm>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSettingsMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The default constructor
MSettingsMelinator::MSettingsMelinator(bool AutoLoad) : MSettings("MelinatorConfigurationFile")
{
  m_DefaultSettingsFileName = "~/.melinator.cfg";
  m_SettingsFileName = m_DefaultSettingsFileName;
  
  m_HistogramMin = 0;
  m_HistogramMax = 1024;
  m_HistogramBinningMode = 0;
  m_HistogramBinningModeValue = 100;
  m_HistogramLogX = false;
  m_HistogramLogY = false;

  m_PeakHistogramBinningMode = 0;
  m_PeakHistogramBinningModeValue = 100;

  m_PeakParametrizationMethod = 0;

  m_PeakParametrizationMethodFittingBackgroundModel = 0;
  m_PeakParametrizationMethodFittingEnergyLossModel = 0;
  m_PeakParametrizationMethodFittingPeakShapeModel = 1;

  m_CalibrationModelZeroCrossing = true;
  m_CalibrationModelDeterminationMethod = 0;
  m_CalibrationModelDeterminationMethodFittingEnergyModel = 3;
  m_CalibrationModelDeterminationMethodFittingFWHMModel = 1;
  
  m_SaveAsFileName = "Out.ecal";
  
  m_SelectedDetectorID = -1;
  m_SelectedDetectorSide = -1;
  
  m_MinimumTemperature = -numeric_limits<double>::max();
  m_MaximumTemperature = +numeric_limits<double>::max();
  
  if (AutoLoad == true) {
    Read();
  }
}


////////////////////////////////////////////////////////////////////////////////


MSettingsMelinator::~MSettingsMelinator()
{
  // default destructor

  Write();
}


////////////////////////////////////////////////////////////////////////////////


//! Set all calibration files, group IDs, and Isotopes at once
void MSettingsMelinator::SetAllCalibrationFiles(const vector<MString>& Files, 
                                                const vector<unsigned int> GroupIDs, 
                                                const vector<vector<MString>>& Isotopes)
{
  size_t Size = Files.size();
  if (GroupIDs.size() != Size) {
    cout<<"Error: The size of the calibration group ID is not identical with the number of files."<<endl;
    Size = min(Size, GroupIDs.size());
  }
  if (Isotopes.size() != Size) {
    cout<<"Error: The size of the isotopes vector is not identical with the number of files or group IDs."<<endl;
    Size = min(Size, Isotopes.size());
  }
  
  m_CalibrationFiles.clear();
  m_CalibrationGroupIDs.clear();
  m_CalibrationIsotopes.clear();  
  for (unsigned int i = 0; i < Size; ++i) {
    m_CalibrationFiles.push_back(Files[i]);
    m_CalibrationGroupIDs.push_back(GroupIDs[i]);
    m_CalibrationIsotopes.push_back(Isotopes[i]);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMelinator::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MSettings::WriteXml(Node);

  MXmlNode* N = new MXmlNode(Node, "CalibrationFiles");
  for (unsigned int f = 0; f < m_CalibrationFiles.size(); ++f) {
    MXmlNode* M = new MXmlNode(N, "CalibrationFile");
    new MXmlNode(M, "CalibrationFileName", m_CalibrationFiles[f]);
    new MXmlNode(M, "CalibrationFileGroupID", m_CalibrationGroupIDs[f]);
    MXmlNode* I = new MXmlNode(M, "Isotopes");
    for (unsigned int i = 0; i < m_CalibrationIsotopes[f].size(); ++i) {
      new MXmlNode(I, "Isotope", m_CalibrationIsotopes[f][i]);
    }
  }
  
  new MXmlNode(Node, "HistogramMin", m_HistogramMin);
  new MXmlNode(Node, "HistogramMax", m_HistogramMax);
  new MXmlNode(Node, "HistogramBinningMode", m_HistogramBinningMode);
  new MXmlNode(Node, "HistogramBinningModeValue", m_HistogramBinningModeValue);
  new MXmlNode(Node, "HistogramLogX", m_HistogramLogX);
  new MXmlNode(Node, "HistogramLogY", m_HistogramLogY);

  new MXmlNode(Node, "PeakHistogramBinningMode", m_PeakHistogramBinningMode);
  new MXmlNode(Node, "PeakHistogramBinningModeValue", m_PeakHistogramBinningModeValue);

  new MXmlNode(Node, "PeakParametrizationMethod", m_PeakParametrizationMethod);
  new MXmlNode(Node, "PeakParametrizationMethodFittingBackgroundModel", m_PeakParametrizationMethodFittingBackgroundModel);
  new MXmlNode(Node, "PeakParametrizationMethodFittingEnergyLossModel", m_PeakParametrizationMethodFittingEnergyLossModel);
  new MXmlNode(Node, "PeakParametrizationMethodFittingPeakShapeModel", m_PeakParametrizationMethodFittingPeakShapeModel);

  new MXmlNode(Node, "CalibrationModelZeroCrossing", m_CalibrationModelZeroCrossing);
  new MXmlNode(Node, "CalibrationModelDeterminationMethod", m_CalibrationModelDeterminationMethod);
  new MXmlNode(Node, "CalibrationModelDeterminationMethodFittingEnergyModel", m_CalibrationModelDeterminationMethodFittingEnergyModel);
  new MXmlNode(Node, "CalibrationModelDeterminationMethodFittingFWHMModel", m_CalibrationModelDeterminationMethodFittingFWHMModel);
  
  new MXmlNode(Node, "SaveAsFileName", m_SaveAsFileName);
  
  // Not stored, since command line options:
  // new MXmlNode(Node, "SelectedDetectorID", m_SelectedDetectorID);
  // new MXmlNode(Node, "SelectedDetectorSide", m_SelectedDetectorSide);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsMelinator::ReadXml(MXmlNode* Node)
{  
  // Retrieve the content from an XML tree

  MSettings::ReadXml(Node);
  
  MXmlNode* aNode = 0;

  m_CalibrationFiles.clear();
  m_CalibrationGroupIDs.clear();
  m_CalibrationIsotopes.clear();  
  if ((aNode = Node->GetNode("CalibrationFiles")) != 0) {
    for (unsigned int f = 0; f < aNode->GetNNodes(); ++f) {
      MXmlNode* bNode = aNode->GetNode(f);
      if (bNode->GetName() == "CalibrationFile") {
        MXmlNode* cNode = 0;
        if ((cNode = bNode->GetNode("CalibrationFileName")) != 0) {
          m_CalibrationFiles.push_back(cNode->GetValueAsString()); 
        }        
        if ((cNode = bNode->GetNode("CalibrationFileGroupID")) != 0) {
          m_CalibrationGroupIDs.push_back(cNode->GetValueAsUnsignedInt()); 
        }        
        if ((cNode = bNode->GetNode("Isotopes")) != 0) {
          vector<MString> Isotopes;
          for (unsigned int i = 0; i < cNode->GetNNodes(); ++i) {
            MXmlNode* dNode = cNode->GetNode(i);
            if (dNode->GetName() == "Isotope") {
              Isotopes.push_back(dNode->GetValueAsString()); 
            }
          }
          m_CalibrationIsotopes.push_back(Isotopes);
        }
      }
    }
  }


  if ((aNode = Node->GetNode("HistogramMin")) != 0) {
    m_HistogramMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("HistogramMax")) != 0) {
    m_HistogramMax = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("HistogramBinningMode")) != 0) {
    m_HistogramBinningMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("HistogramBinningModeValue")) != 0) {
    m_HistogramBinningModeValue = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("HistogramLogX")) != 0) {
    m_HistogramLogX = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("HistogramLogY")) != 0) {
    m_HistogramLogY = aNode->GetValueAsBoolean();
  }
 
  if ((aNode = Node->GetNode("PeakHistogramBinningMode")) != 0) {
    m_PeakHistogramBinningMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakHistogramBinningModeValue")) != 0) {
    m_PeakHistogramBinningModeValue = aNode->GetValueAsDouble();
  }

  if ((aNode = Node->GetNode("PeakParametrizationMethod")) != 0) {
    m_PeakParametrizationMethod = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakParametrizationMethodFittingBackgroundModel")) != 0) {
    m_PeakParametrizationMethodFittingBackgroundModel = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakParametrizationMethodFittingEnergyLossModel")) != 0) {
    m_PeakParametrizationMethodFittingEnergyLossModel = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakParametrizationMethodFittingPeakShapeModel")) != 0) {
    m_PeakParametrizationMethodFittingPeakShapeModel = aNode->GetValueAsUnsignedInt();
  }

  if ((aNode = Node->GetNode("CalibrationModelZeroCrossing")) != 0) {
    m_CalibrationModelZeroCrossing = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("CalibrationModelDeterminationMethod")) != 0) {
    m_CalibrationModelDeterminationMethod = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("CalibrationModelDeterminationMethodFittingEnergyModel")) != 0) {
    m_CalibrationModelDeterminationMethodFittingEnergyModel = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("CalibrationModelDeterminationMethodFittingFWHMModel")) != 0) {
    m_CalibrationModelDeterminationMethodFittingFWHMModel = aNode->GetValueAsUnsignedInt();
  }
  
  if ((aNode = Node->GetNode("PeakHistogramBinningMode")) != 0) {
    m_PeakHistogramBinningMode = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Node->GetNode("PeakHistogramBinningModeValue")) != 0) {
    m_PeakHistogramBinningModeValue = aNode->GetValueAsDouble();
  }

  if ((aNode = Node->GetNode("SaveAsFileName")) != 0) {
    m_SaveAsFileName = aNode->GetValueAsString();
  }
 
  /*
  // Not stored since command line options
  if ((aNode = Node->GetNode("SelectedDetectorID")) != 0) {
    m_SelectedDetectorID = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("SelectedDetectorSide")) != 0) {
    m_SelectedDetectorSide = aNode->GetValueAsInt();
  }
  */
 
  return true;
}


// MSettingsMelinator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
