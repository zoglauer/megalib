/*
 * MSettingsImaging.cxx
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
// MSettingsImaging.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsImaging.h"

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


#ifdef ___CINT___
ClassImp(MSettingsImaging)
#endif


///////////////////////////////////////////////////////////////////////////////


MSettingsImaging::MSettingsImaging() : MSettingsInterface()
{
  // default constructor

  m_BackprojectionModified = true;
  m_LikelihoodModified = true;

  m_StoreImages = false;

  m_CoordinateSystem = MProjection::c_Spheric;
  m_LHAlgorithm = MLMLAlgorithms::c_ClassicEM;
  m_OSEMSubSets = 4;
  m_LHStopCriteria = MLMLAlgorithms::c_StopAfterIterations;
  m_LHIncrease = 0.0001;
  m_Penalty = 0;
  m_NIterations = 5;
  m_PenaltyAlpha = 0;




  // Dimensions spherical
  m_ImageRotationXAxis = MVector(1.0, 0.0, 0.0);
  m_ImageRotationZAxis = MVector(0.0, 0.0, 1.0);

  m_ThetaMin = 0;
  m_ThetaMax = 180;
  m_PhiMin = -180;
  m_PhiMax = 180;
  m_BinsTheta = 40;
  m_BinsPhi = 80;

  // Dimensions galactic
  m_GalLatitudeMin = -10;
  m_GalLatitudeMax = 10;
  m_GalLongitudeMin = 170;
  m_GalLongitudeMax = 190;
  m_BinsGalLatitude = 40;
  m_BinsGalLongitude = 40;

  // Dimensions Cartesean
  m_XMin = -5;
  m_XMax = 5;
  m_BinsX = 50;
  m_YMin = -5;
  m_YMax = 5;
  m_BinsY = 50;
  m_ZMin = 9.99;
  m_ZMax = 10.01;
  m_BinsZ = 1;

  // Image drawing options 
  m_ImageDrawMode = 0;
  m_ImagePalette = 0;
  m_ImageSourceCatalog = "";

  // Animation options
  m_AnimationMode = 1;
  m_AnimationFrameTime = 10; //sec
  m_AnimationFileName = "MyAnimated.gif";
  
  // Response
  m_ResponseType = 0;

  m_FitParameterComptonLongSphere = 20;
  m_FitParameterComptonTransSphere = 2;
  m_FitParameterPair = 2;
  m_UseAbsorptions = false;
  m_Gauss1DCutOff = 2.5;


  m_ImagingResponseComptonTransversalFileName = g_StringNotDefined;
  m_ImagingResponseComptonLongitudinalFileName = g_StringNotDefined;
  m_ImagingResponsePairRadialFileName = g_StringNotDefined;


  // Memory management
  m_RAM = 1500;
  m_Swap = 128;
  m_MemoryExhausted = 2;
  m_Bytes = 1;
  m_ApproximatedMaths = false;
  m_FastFileParsing = false;
  m_NThreads = 1;

  // Significance
  m_SignificanceMapRadius = 5.2;
  m_SignificanceMapDistance = 20.;

}


////////////////////////////////////////////////////////////////////////////////


MSettingsImaging::~MSettingsImaging()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsImaging::WriteXml(MXmlNode* Node)
{
   // Write content to an XML tree

  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;

  // Section image algorithms:
  //////////////////////

  aNode = new MXmlNode(Node, "ImageReconstructionAlgorithms");

  bNode = new MXmlNode(aNode, "ListMode");
  // Menu algorithm
  new MXmlNode(bNode, "Algorithm", m_LHAlgorithm);
  new MXmlNode(bNode, "OSEMSubSets", m_OSEMSubSets);
  new MXmlNode(bNode, "StopCriteria", m_LHStopCriteria);
  new MXmlNode(bNode, "Increase", m_LHIncrease);
  new MXmlNode(bNode, "NIterations", m_NIterations);

  // Menu penalty
  new MXmlNode(bNode, "PenaltyType", m_Penalty);
  new MXmlNode(bNode, "PenaltyAlpha", m_PenaltyAlpha);

  // Menu response selection:
  new MXmlNode(bNode, "ResponseType", m_ResponseType);

  // Menu Fitparameter:
  new MXmlNode(bNode, "ResponseParameter1DGaussianComptonLong", m_FitParameterComptonLongSphere);
  new MXmlNode(bNode, "ResponseParameter1DGaussianComptonTrans", m_FitParameterComptonTransSphere);
  new MXmlNode(bNode, "ResponseParameter1DGaussianPair", m_FitParameterPair);
  new MXmlNode(bNode, "ResponseParameter1DUseAbsorptions", m_UseAbsorptions);
  new MXmlNode(bNode, "ResponseParameter1DGaussianCutOff", m_Gauss1DCutOff);

  new MXmlNode(bNode, "ResponseParameterFilesComptonLong", CleanPath(m_ImagingResponseComptonLongitudinalFileName));
  new MXmlNode(bNode, "ResponseParameterFilesComptonTrans", CleanPath(m_ImagingResponseComptonTransversalFileName));
  new MXmlNode(bNode, "ResponseParameterFilesPair", CleanPath(m_ImagingResponsePairRadialFileName));


  // Menu Sensitivity:
  new MXmlNode(bNode, "SensitivityFileName", CleanPath(m_SensitivityFile));
  new MXmlNode(bNode, "UseSensitivity", m_UseSensitivityFile);



  // Menu Coordinate-system
  aNode = new MXmlNode(Node, "CoordinateSystem");

  new MXmlNode(aNode, "Type", m_CoordinateSystem);

  // Menu Dimensions - Spherical
  new MXmlNode(aNode, "SphericalRotationAxisX", m_ImageRotationXAxis); 
  new MXmlNode(aNode, "SphericalRotationAxisZ", m_ImageRotationZAxis);
  new MXmlNode(aNode, "SphericalTheta", m_ThetaMin, m_ThetaMax);
  new MXmlNode(aNode, "SphericalThetaBins", m_BinsTheta);
  new MXmlNode(aNode, "SphericalPhi", m_PhiMin, m_PhiMax);
  new MXmlNode(aNode, "SphericalPhiBins", m_BinsPhi);

  // Menu Dimensions - Galactic
  new MXmlNode(aNode, "GalacticLatitude", m_GalLatitudeMin, m_GalLatitudeMax);
  new MXmlNode(aNode, "GalacticLatitudeBins", m_BinsGalLatitude);
  new MXmlNode(aNode, "GalacticLongitude", m_GalLongitudeMin, m_GalLongitudeMax);
  new MXmlNode(aNode, "GalacticLongitudeBins", m_BinsGalLongitude);

  // Menu Dimensions - Cartesean
  new MXmlNode(aNode, "CartesianX", m_XMin, m_XMax);
  new MXmlNode(aNode, "CartesianXBins", m_BinsX);
  new MXmlNode(aNode, "CartesianY", m_YMin, m_YMax);
  new MXmlNode(aNode, "CartesianYBins", m_BinsY);
  new MXmlNode(aNode, "CartesianZ", m_ZMin, m_ZMax);
  new MXmlNode(aNode, "CartesianZBins", m_BinsZ);

  // Menu Image drawing options
  aNode = new MXmlNode(Node, "ImageDrawingOptions");
  new MXmlNode(aNode, "ImageDrawMode", m_ImageDrawMode);
  new MXmlNode(aNode, "ImagePalette", m_ImagePalette);
  new MXmlNode(aNode, "ImageSourceCatalog", CleanPath(m_ImageSourceCatalog));

  // Menu Image animation options
  aNode = new MXmlNode(Node, "AnimationOptions");
  new MXmlNode(aNode, "AnimationMode", m_AnimationMode);
  new MXmlNode(aNode, "AnimationFrameTime", m_AnimationFrameTime);
  new MXmlNode(aNode, "AnimationFileName", CleanPath(m_AnimationFileName));

  // Menu Image significance map options
  aNode = new MXmlNode(Node, "SignificanceMap");
  new MXmlNode(aNode, "SignificanceMapRadius", m_SignificanceMapRadius);
  new MXmlNode(aNode, "SignificanceMapDistance", m_SignificanceMapDistance);

  // Menu memory exhausted:
  aNode = new MXmlNode(Node, "MemoryManagement");
  new MXmlNode(aNode, "RAM", m_RAM);
  new MXmlNode(aNode, "Swap", m_Swap);
  new MXmlNode(aNode, "MemoryExhausted", m_MemoryExhausted);
  new MXmlNode(aNode, "NBytes", m_Bytes);
  new MXmlNode(aNode, "ApproximatedMaths", m_ApproximatedMaths);
  new MXmlNode(aNode, "FastFileParsing", m_FastFileParsing);
  new MXmlNode(aNode, "NThreads", m_NThreads);
  

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsImaging::ReadXml(MXmlNode* Node)
{
  // Retrieve the content from an XML tree
  
  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;
  MXmlNode* cNode = 0;
  
  if ((aNode = Node->GetNode("ImageReconstructionAlgorithms")) != 0) {
    if ((bNode = aNode->GetNode("ListMode")) != 0) {
      if ((cNode = bNode->GetNode("Algorithm")) != 0) {
        m_LHAlgorithm = cNode->GetValueAsUnsignedInt();
      }
      if ((cNode = bNode->GetNode("OSEMSubSets")) != 0) {
        m_OSEMSubSets = cNode->GetValueAsUnsignedInt();
      }
      if ((cNode = bNode->GetNode("StopCriteria")) != 0) {
        m_LHStopCriteria = cNode->GetValueAsUnsignedInt();
      }
      if ((cNode = bNode->GetNode("Increase")) != 0) {
        m_LHIncrease = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("NIterations")) != 0) {
        m_NIterations = cNode->GetValueAsUnsignedInt();
      }
      if ((cNode = bNode->GetNode("PenaltyType")) != 0) {
        m_Penalty = cNode->GetValueAsInt();
      }
      if ((cNode = bNode->GetNode("PenaltyAlpha")) != 0) {
        m_PenaltyAlpha = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("ResponseType")) != 0) {
        m_ResponseType = cNode->GetValueAsInt();
      }
      if ((cNode = bNode->GetNode("ResponseParameter1DGaussianComptonLong")) != 0) {
        m_FitParameterComptonLongSphere = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("ResponseParameter1DGaussianComptonTrans")) != 0) {
        m_FitParameterComptonTransSphere = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("ResponseParameter1DGaussianPair")) != 0) {
        m_FitParameterPair = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("ResponseParameter1DUseAbsorptions")) != 0) {
        m_UseAbsorptions = cNode->GetValueAsBoolean();
      }
      if ((cNode = bNode->GetNode("ResponseParameter1DGaussianCutOff")) != 0) {
        m_Gauss1DCutOff = cNode->GetValueAsDouble();
      }
      if ((cNode = bNode->GetNode("ResponseParameterFilesComptonLong")) != 0) {
        m_ImagingResponseComptonLongitudinalFileName = cNode->GetValue();
      }
      if ((cNode = bNode->GetNode("ResponseParameterFilesComptonTrans")) != 0) {
        m_ImagingResponseComptonTransversalFileName = cNode->GetValue();
      }
      if ((cNode = bNode->GetNode("ResponseParameterFilesPair")) != 0) {
        m_ImagingResponsePairRadialFileName = cNode->GetValue();
      }
      if ((cNode = bNode->GetNode("SensitivityFileName")) != 0) {
        m_SensitivityFile = cNode->GetValue();
      }
      if ((cNode = bNode->GetNode("UseSensitivity")) != 0) {
        m_UseSensitivityFile = cNode->GetValueAsBoolean();
      }
    }
  }
  
  
  if ((aNode = Node->GetNode("CoordinateSystem")) != 0) {
    if ((bNode = aNode->GetNode("Type")) != 0) {
      m_CoordinateSystem = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("SphericalRotationAxisX")) != 0) {
      m_ImageRotationXAxis = bNode->GetValueAsVector();
    }
    if ((bNode = aNode->GetNode("SphericalRotationAxisZ")) != 0) {
      m_ImageRotationZAxis = bNode->GetValueAsVector();
    }
    if ((bNode = aNode->GetNode("SphericalTheta")) != 0) {
      m_ThetaMin = bNode->GetMinValueAsDouble();
      m_ThetaMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("SphericalThetaBins")) != 0) {
      m_BinsTheta = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("SphericalPhi")) != 0) {
      m_PhiMin = bNode->GetMinValueAsDouble();
      m_PhiMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("SphericalPhiBins")) != 0) {
      m_BinsPhi = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("GalacticLatitude")) != 0) {
      m_GalLatitudeMin = bNode->GetMinValueAsDouble();
      m_GalLatitudeMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("GalacticLatitudeBins")) != 0) {
      m_BinsGalLatitude = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("GalacticLongitude")) != 0) {
      m_GalLongitudeMin = bNode->GetMinValueAsDouble();
      m_GalLongitudeMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("GalacticLongitudeBins")) != 0) {
      m_BinsGalLongitude = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("CartesianX")) != 0) {
      m_XMin = bNode->GetMinValueAsDouble();
      m_XMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("CartesianXBins")) != 0) {
      m_BinsX = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("CartesianY")) != 0) {
      m_YMin = bNode->GetMinValueAsDouble();
      m_YMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("CartesianYBins")) != 0) {
      m_BinsY = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("CartesianZ")) != 0) {
      m_ZMin = bNode->GetMinValueAsDouble();
      m_ZMax = bNode->GetMaxValueAsDouble();
    }
    if ((bNode = aNode->GetNode("CartesianZBins")) != 0) {
      m_BinsZ = bNode->GetValueAsInt();
    }
  }
  
  
  if ((aNode = Node->GetNode("ImageDrawingOptions")) != 0) {
    if ((bNode = aNode->GetNode("ImageDrawMode")) != 0) {
      m_ImageDrawMode = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("ImagePalette")) != 0) {
      m_ImagePalette = bNode->GetValueAsInt();
    }      
    if ((bNode = aNode->GetNode("ImageSourceCatalog")) != 0) {
      m_ImageSourceCatalog = bNode->GetValueAsString();
    }      
  }
  
  if ((aNode = Node->GetNode("AnimationOptions")) != 0) {
    if ((bNode = aNode->GetNode("AnimationMode")) != 0) {
      m_AnimationMode = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("AnimationFrameTime")) != 0) {
      m_AnimationFrameTime = bNode->GetValueAsDouble();
    }      
    if ((bNode = aNode->GetNode("AnimationFileName")) != 0) {
      m_AnimationFileName = bNode->GetValueAsString();
    }      
  }
  
  if ((aNode = Node->GetNode("SignificanceMap")) != 0) {
    if ((bNode = aNode->GetNode("SignificanceMapDistance")) != 0) {
      m_SignificanceMapDistance = bNode->GetValueAsDouble(); 
    }
    if ((bNode = aNode->GetNode("SignificanceMapRadius")) != 0) {
      m_SignificanceMapRadius = bNode->GetValueAsDouble();
    }
  }
  
  if ((aNode = Node->GetNode("MemoryManagement")) != 0) {
    if ((bNode = aNode->GetNode("RAM")) != 0) {
      m_RAM = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("Swap")) != 0) {
      m_Swap = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("MemoryExhausted")) != 0) {
      m_MemoryExhausted = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("NBytes")) != 0) {
      m_Bytes = bNode->GetValueAsInt();
    }
    if ((bNode = aNode->GetNode("ApproximatedMaths")) != 0) {
      m_ApproximatedMaths = bNode->GetValueAsBoolean();
    }
    if ((bNode = aNode->GetNode("FastFileParsing")) != 0) {
      m_FastFileParsing = bNode->GetValueAsBoolean();
    }
    if ((bNode = aNode->GetNode("NThreads")) != 0) {
      m_NThreads = bNode->GetValueAsInt();
    }
  }
  
  return true;
}


// MSettingsImaging.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
