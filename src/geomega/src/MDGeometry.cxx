/*
 * MDGeometry.cxx
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
// MDGeometry
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGeometry.h"

// Standard libs:
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>
#include <iterator>
using namespace std;

// ROOT libs:
#include <TNode.h>
#include <TGeometry.h>
#include <TCanvas.h>
#include <TView.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TObjString.h>
#include <TMath.h>
#include <TGeoOverlap.h>
#include <TObjArray.h>
#include <TMD5.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"
#include "MTokenizer.h"
#include "MDShape.h"
#include "MDShapeBRIK.h"
#include "MDShapeTRD1.h"
#include "MDShapeTRD2.h"
#include "MDShapeSPHE.h"
#include "MDShapeTUBS.h"
#include "MDShapeCONE.h"
#include "MDShapeCONS.h"
#include "MDShapeTRAP.h"
#include "MDShapeGTRA.h"
#include "MDShapePCON.h"
#include "MDShapePGON.h"
#include "MDShapeSubtraction.h"
#include "MDShapeUnion.h"
#include "MDShapeIntersection.h"
#include "MDCalorimeter.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDStrip3DDirectional.h"
#include "MDACS.h"
#include "MDDriftChamber.h"
#include "MDAngerCamera.h"
#include "MDVoxel3D.h"
#include "MDGuardRing.h"
#include "MDSystem.h"
#include "MDTrigger.h"
#include "MDTriggerBasic.h"
#include "MDTriggerMap.h"
#include "MDGDMLImport.h"
#include "MTimer.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDGeometry)
#endif


////////////////////////////////////////////////////////////////////////////////


MDGeometry::MDGeometry()
{
  // default constructor

  m_GeometryScanned = false;
  m_WorldVolume = 0;
  m_StartVolume = "";
  m_ShowVolumes = true;
  m_DefaultColor = -1;
  m_DoSanityChecks = true;
  m_ComplexER = true;

  m_Name = "\"Geometry, which was not worth a name...\"" ;
  m_Version = "0.0";

  m_SurroundingSphereRadius = g_DoubleNotDefined;
  m_SurroundingSpherePosition = g_VectorNotDefined;
  m_SurroundingSphereDistance = g_DoubleNotDefined;
  m_SurroundingSphereShow = false;

  m_GeoView = 0;
  m_Geometry = 0;

  m_LastVolumes.clear();
  m_LastVolumePosition = 0;

  m_TriggerUnit = new MDTriggerUnit(this);
  m_System = new MDSystem("NoName");
  m_DetectorEffectsEngine = new MDDetectorEffectsEngine();
  m_DetectorEffectsEngine->SetGeometry(this);

  // Make sure we ignore the default ROOT geometry...
  // BUG: In case we are multi-threaded and some one interact with the geometry
  //      before the gGeoManager is reset during new, we will get a seg-fault!
  gGeoManager = 0;
  // ... before
  m_Geometry = new TGeoManager("Geomega geometry", "Geomega");

  m_LaunchedByGeomega = false;

  m_BlockedConstants = { "Volume", "Material", "System",
    "Trigger", "TriggerBasic", "TriggerMap"
    "Strip2D", "MDStrip2D",
    "Strip3D", "MDStrip3D",
    "Strip3DDirectional", "MDStrip3DDirectional",
    "DriftChamber", "MDDriftChamber",
    "AngerCamera", "MDAngerCamera",
    "Simple", "Scintillator", "ACS", "MDACS",
    "Calorimeter", "MDCalorimeter",
    "Voxel3D", "MDVoxel3D",
    "SurroundingVolume",
    "If", "Endif", "For", "Done", "Vector",
    "H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
    "Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
    "Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
    "Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
    "In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
    "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
    "Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
    "At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
    "Bk","Cf","Es","Fm","Md","No","Lr" };
}


////////////////////////////////////////////////////////////////////////////////


MDGeometry::~MDGeometry()
{
  // default destructor

  Reset();

  delete m_TriggerUnit;
  delete m_System;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::Reset()
{
  // Reset this class to default values

  for (unsigned int i = 0; i < m_VolumeList.size(); ++i) {
    delete m_VolumeList[i];
  }
  m_VolumeList.clear();

  for (unsigned int i = 0; i < m_MaterialList.size(); ++i) {
    delete m_MaterialList[i];
  }
  m_MaterialList.clear();

  for (unsigned int i = 0; i < m_DetectorList.size(); ++i) {
    delete m_DetectorList[i];
  }
  m_DetectorList.clear();

  m_NDetectorTypes.clear();
  m_NDetectorTypes.resize(MDDetector::c_MaxDetector+1);

  for (unsigned int i = 0; i < m_TriggerList.size(); ++i) {
    delete m_TriggerList[i];
  }
  m_TriggerList.clear();

  for (unsigned int i = 0; i < m_ShapeList.size(); ++i) {
    delete m_ShapeList[i];
  }
  m_ShapeList.clear();

  for (unsigned int i = 0; i < m_OrientationList.size(); ++i) {
    delete m_OrientationList[i];
  }
  m_OrientationList.clear();

  for (unsigned int i = 0; i < m_VectorList.size(); ++i) {
    delete m_VectorList[i];
  }
  m_VectorList.clear();

  m_ConstantList.clear();
  m_ConstantMap.clear();

  m_WorldVolume = 0;
  // m_StartVolume = ""; // This is a start option of geomega, so do not reset!

  m_IncludeList.clear();
  m_IncludeListHashes.clear();

  if (m_GeoView != 0) {
    if (gROOT->FindObject("MainCanvasGeomega") != 0) {
      delete m_GeoView;
    }
    m_GeoView = 0;
  }
  
  m_ViewValid = false;
  
  m_ViewPositionX = 0;
  m_ViewPositionY = 0;

  m_ViewPositionShiftX = 0;
  m_ViewPositionShiftY = 0;
  
  m_ViewSizeX = 0;
  m_ViewSizeY = 0;
  
  m_ViewRangeMin.resize(3);
  fill(m_ViewRangeMin.begin(), m_ViewRangeMin.end(), 0);
  m_ViewRangeMax.resize(3);
  fill(m_ViewRangeMax.begin(), m_ViewRangeMax.end(), 0);
  m_ViewRotationPhi = 0;
  m_ViewRotationTheta = 0;
  m_ViewRotationPsi = 0;

  m_ViewDistanceCOPtoCOV = 0;
  m_ViewDistanceCOPtoPL = 0;


  m_Name = "";

  m_SurroundingSphereRadius = g_DoubleNotDefined;
  m_SurroundingSpherePosition = g_VectorNotDefined;
  m_SurroundingSphereDistance = g_DoubleNotDefined;
  m_SurroundingSphereShow = false;

  // Create a new geometry
  // BUG: In case we are multi-threaded and someone interacts with the geometry
  //      before the gGeoManager is reset during new, we will get a seg-fault!
  if (gGeoManager != nullptr) {
    gGeoManager = 0;
    m_Geometry->ClearNavigators();
    delete m_Geometry;
  }
  m_Geometry = new TGeoManager("Give it a good name", "MissingName");

  delete m_System;
  m_System = new MDSystem("NoName");

  m_DoSanityChecks = true;
  m_ComplexER = true;
  m_VirtualizeNonDetectorVolumes = false;

  m_LastVolumes.clear();
  m_LastVolumePosition = 0;

  m_DetectorSearchTolerance = 0.000001;

  m_DefaultCrossSectionFileDirectory = g_MEGAlibPath + "/resource/examples/geomega/materials";
  // This one will be overwritten when we have the file name of the mass model
  m_CrossSectionFileDirectory = g_MEGAlibPath + "/resource/examples/geomega/materials";

  MDVolume::ResetIDs();
  MDDetector::ResetIDs();
  MDMaterial::ResetIDs();

  m_LastFoundVolume_GetRandomPositionInVolume = nullptr;
  //! The last found placements in GetRandomPositionInVolume
  m_LastFoundPlacements_GetRandomPositionInVolume.clear();

  m_GeometryScanned = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::IsScanned()
{
  // Return true if the geometry setup-file has been scanned/read successfully

  return m_GeometryScanned;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::ScanSetupFile(MString FileName, bool CreateNodes, bool VirtualizeNonDetectorVolumes, bool AllowCrossSectionCreation)
{
  // Scan the setup-file and create the geometry

  //if (IsScanned() == false) {
  //  mout<<"Loading geometry file: "<<FileName<<endl;
  //}

  bool DebugParsing = false;
  
  int Stage = 0;
  MTimer Timer;
  double TimeLimit = 0;
  bool FoundDeprecated = false;
  bool FoundSurroundingSphere = false;

  // First clean the geometry ...
  Reset();

  // Save if we want to virtualize non detector volumes for speed increase:
  // If the file contains such a statement, this one is overwritten
  m_VirtualizeNonDetectorVolumes = VirtualizeNonDetectorVolumes;

  // If the is set the visibility of everything but the sensitive volume to zero
  bool ShowOnlySensitiveVolumes = false;

  m_FileName = FileName;
  if (m_FileName == "") {
    mout<<"   *** Error: No geometry file name given"<<endl;
    return false;
  }

  if (MFile::ExpandFileName(m_FileName) == false) {
    mgui<<"Unable to expand file name: \""<<m_FileName<<"\""<<error;
    return false;
  }
  
  if (gSystem->IsAbsoluteFileName(m_FileName) == false) {
    m_FileName = gSystem->WorkingDirectory() + MString("/") + m_FileName;
  }

  if (gSystem->AccessPathName(m_FileName) == 1) {
    mgui<<"Geometry file \""<<m_FileName<<"\" does not exist. Aborting."<<error;
    return false;
  }

  m_CrossSectionFileDirectory = MFile::GetDirectoryName(m_FileName);
  m_CrossSectionFileDirectory += "/cross-sections";

  if (g_Verbosity >= c_Info) {
    mout<<"Started scanning of geometry... This may take a while..."<<endl;
  }

  MDMaterial* M = 0;
  MDMaterial* MCopy = 0;
  MDVolume* V = 0;
  MDVolume* VCopy = 0;
  MDDetector* D = 0;
  MDTrigger* T = 0;
  MDSystem* S = 0;
  MDVector* Vector = 0;
  MDShape* Shape = 0;
  MDOrientation* Orientation = 0;

  // For scaling some volumes:
  map<MDVolume*, double> ScaledVolumes;

  // Since the geometry-file can include other geometry files,
  // we have to store the whole file in memory

  list<MDDebugInfo> FileContent;
  if (AddFile(m_FileName, FileContent) == false) {
    mout<<"   *** Error reading included files. Aborting!"<<endl;
    return false;
  }

  // Now scan the data and search for "Include" files and add them
  // to the original stored file content

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) {
      continue;
    }
    
    if (Tokenizer.IsTokenAt(0, "Include") == true) {
      
      // Test for old material path
      if (Tokenizer.GetTokenAt(1).EndsWith("resource/geometries/materials/Materials.geo") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"You are using the old MEGAlib material path:"<<endl;
        mout<<m_DebugInfo.GetText()<<endl;
        mout<<"Please update to the new path now!"<<endl;
        mout<<"Change: resource/geometries To: resource/examples/geomega "<<endl;
        mout<<endl;
        FoundDeprecated = true;
      }
      
      MString FileName = Tokenizer.GetTokenAt(1);
      if (MFile::ExpandFileName(FileName, m_FileName) == false) {
        Typo("Unable to expand file name");
        return false;
      }
      
      if (MFile::Exists(FileName) == false) {
        mout<<"   *** Error finding file "<<FileName<<endl;
        Typo("File IO error");
        return false;
      }
      
      list<MDDebugInfo> AddFileContent;
      if (AddFile(FileName, AddFileContent) == false) {
        mout<<"   *** Error reading file "<<FileName<<endl;
        Typo("File IO error");
        return false;
      }
      
      (*ContentIter).SetText("");
      auto BackToStartIter = ContentIter;
      for (auto AddIter = AddFileContent.begin(); AddIter != AddFileContent.end(); ++AddIter) {
        ContentIter = FileContent.insert(next(ContentIter), (*AddIter));
      }
      ContentIter = BackToStartIter;
    }
    
    if (Tokenizer.IsTokenAt(0, "Import") == true) {
      
      if (Tokenizer.GetNTokens() != 3) {
        Typo("Line must contain three entries, e.g. \"Import GDML MyGDML.gdml\"");
        return false;
      }
      
      if (Tokenizer.IsTokenAt(1, "GDML") == true) {
        MString FileName = Tokenizer.GetTokenAt(2);
        if (MFile::ExpandFileName(FileName, m_FileName) == false) {
          mout<<"   *** Error expanding file "<<FileName<<endl;
          Typo("Unable to expand file name");
          return false;
        }
      
        if (MFile::Exists(FileName) == false) {
          mout<<"   *** Error finding file "<<FileName<<endl;
          Typo("File IO error");
          return false;
        }
      
        list<MDDebugInfo> AddFileContent;
        if (ImportGDML(FileName, AddFileContent) == false) {
          mout<<"   *** Error reading file "<<FileName<<endl;
          Typo("File IO error");
          return false;
        }
      
        (*ContentIter).SetText("");
        auto BackToStartIter = ContentIter;
        for (auto AddIter = AddFileContent.begin(); AddIter != AddFileContent.end(); ++AddIter) {
          ContentIter = FileContent.insert(next(ContentIter), (*AddIter));
        }
        ContentIter = BackToStartIter;
      } else {
        mout<<"   *** Error unknown import file type "<<Tokenizer.GetTokenAt(1)<<endl;
        Typo("Import error");
      }
    }
    
  }

  if (FileContent.size() == 0) {
    mgui<<"File is \""<<m_FileName<<"\" empty or binary!"<<error;
    return false;
  }
  
  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (reading of file(s)) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file reading *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  


  // Find lines which are continued in a second line by the "\\" keyword

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) continue;

    // Of course the real token is "\\"
    if (Tokenizer.IsTokenAt(Tokenizer.GetNTokens()-1, "\\\\") == true) {
      //cout<<"Found \\\\: "<<Tokenizer.ToString()<<endl;
      // Prepend this text to the next line
      if (next(ContentIter, 1) != FileContent.end()) {
        //cout<<"Next: "<<FileContent[i+1].GetText()<<endl;
        MString Prepend = "";
        for (unsigned int t = 0; t < Tokenizer.GetNTokens()-1; ++t) {
          Prepend += Tokenizer.GetTokenAt(t);
          Prepend += " ";
        }
        (*(next(ContentIter, 1))).Prepend(Prepend);
        (*ContentIter).SetText("");
      }
    }
  }



  // Find constants
  //

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) continue;

    // Constants
    if (Tokenizer.IsTokenAt(0, "Constant") == true) {
      if (Tokenizer.GetNTokens() != 3) {
        Typo("Line must contain three entries, e.g. \"Constant Distance 10.5\"");
        return false;
      }
      if (Tokenizer.GetTokenAt(1) == Tokenizer.GetTokenAt(2)) {
        Typo("The constant name and replacement are identical!");
        return false;
      }
      vector<MString>::iterator VIter = find(m_BlockedConstants.begin(), m_BlockedConstants.end(), Tokenizer.GetTokenAt(1));
      if (VIter != m_BlockedConstants.end()) {
        Typo("Constant has a reserved name, and thus cannot be used!");
        return false;
      }
      map<MString, MString>::iterator Iter = m_ConstantMap.find(Tokenizer.GetTokenAt(1));
      if (Iter != m_ConstantMap.end()) {
        if (m_ConstantMap[Tokenizer.GetTokenAt(1)] != Tokenizer.GetTokenAt(2)) {
          Typo("Constant has already been defined and both are not identical!");
          return false;
        }
      }
      m_ConstantMap[Tokenizer.GetTokenAt(1)] = Tokenizer.GetTokenAt(2);
      m_ConstantList.push_back(Tokenizer.GetTokenAt(1));
    }
  }

  // Take care of maths and constants containing constants, containing constants...
  bool ConstantChanged = true;
  while (ConstantChanged == true) {
    // Step 1: Solve maths:
    for (map<MString, MString>::iterator Iter1 = m_ConstantMap.begin();
         Iter1 != m_ConstantMap.end();
         ++Iter1) {
      if (MTokenizer::IsMaths((*Iter1).second) == true) {
        bool ContainsConstant = false;
        for (map<MString, MString>::iterator Iter2 = m_ConstantMap.begin();
             Iter2 != m_ConstantMap.end();
             ++Iter2) {
          if (ContainsReplacableConstant((*Iter1).second, (*Iter2).first) == true) {
            ContainsConstant = true;
            //cout<<"Replaceable constant: "<<(*Iter1).second<<" (namely:"<<(*Iter2).first<<")"<<endl;
            break;
          } else {
            //cout<<"No replaceable constant: "<<(*Iter1).second<<" (test:"<<(*Iter2).first<<")"<<endl;
          }
        }
        if (ContainsConstant == false) {
          MString Constant = (*Iter1).second;
          if ( MTokenizer::CheckMaths(Constant) == false) {
            mout<<"   *** Error ***"<<endl;
            mout<<"Maths in constant cannot be evaluated: "<<(*Iter1).first<<" "<<Constant<<endl;
            return false;
          }
          MTokenizer::EvaluateMaths(Constant);
          (*Iter1).second = Constant;
        }
      }
    }

    // Step 2: Replace constants in constants
    bool ConstantChangableWithMath = false;
    ConstantChanged = false;
    for (map<MString, MString>::iterator Iter1 = m_ConstantMap.begin(); Iter1 != m_ConstantMap.end(); ++Iter1) {
      //cout<<"Checking for replacement: "<<(*Iter1).first<<" with "<<(*Iter1).second<<endl;
      for (map<MString, MString>::iterator Iter2 = m_ConstantMap.begin(); Iter2 != m_ConstantMap.end(); ++Iter2) {
        //cout<<"Map size: "<<m_ConstantMap.size()<<endl;
        if (ContainsReplacableConstant((*Iter1).second, (*Iter2).first) == true) {
          //cout<<"   ---> "<<(*Iter2).first<<" - "<<(*Iter2).second<<endl;
          //cout<<(*Iter1).second<<" contains "<<(*Iter2).first<<endl;
          if (MTokenizer::IsMaths((*Iter2).second) == false) {
            MString Constant = (*Iter1).second;
            ReplaceWholeWords(Constant, (*Iter2).first, (*Iter2).second);
            (*Iter1).second = Constant;
            ConstantChanged = true;
          } else {
            ConstantChangableWithMath = true;
          }
        }
      }
    }

    if (ConstantChanged == false &&  ConstantChangableWithMath == true) {
      mout<<"   *** Error ***"<<endl;
      mout<<"Recursively defined constant found!"<<endl;
      return false;
    }
  }

  // Do the final replace:
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) continue;

    MString Init = Tokenizer.GetTokenAt(0);
    if (Init == "Volume" ||
      Init == "Material" ||
      Init == "Trigger" ||
      Init == "TriggerBasic" ||
      Init == "TriggerMap" ||
      Init == "System" ||
      Init == "Strip2D" ||
      Init == "MDStrip2D" ||
      Init == "Strip3D" ||
      Init == "MDStrip3D" ||
      Init == "Strip3DDirectional" ||
      Init == "MDStrip3DDirectional" ||
      Init == "DriftChamber" ||
      Init == "MDDriftChamber" ||
      Init == "AngerCamera" ||
      Init == "MDAngerCamera" ||
      Init == "Simple" ||
      Init == "Scintillator" ||
      Init == "ACS" ||
      Init == "MDACS" ||
      Init == "Calorimeter" ||
      Init == "MDCalorimeter" ||
      Init == "Voxel3D" ||
      Init == "MDVoxel3D") {
      continue;
    }

    for (map<MString, MString>::iterator Iter = m_ConstantMap.begin();
         Iter != m_ConstantMap.end(); ++Iter) {
      (*ContentIter).Replace((*Iter).first, (*Iter).second, true);
    }
  }


  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating constants and maths) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file evaluating constants and maths *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  

  // Check for Vectors FIRST since those are used in ForVector loops...
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) continue;

    if (Tokenizer.IsTokenAt(0, "Vector") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Vector MyMatrix\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddVector(new MDVector(Tokenizer.GetTokenAt(1)));
      continue;
    }
  }
  
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.GetNTokens() == 0) continue;

    if ((Vector = GetVector(Tokenizer.GetTokenAt(0))) != 0) {
      MTokenizer& Tokenizer2 = m_DebugInfo.GetTokenizer(true);  // Let's do some maths

      if (Tokenizer2.IsTokenAt(1, "Matrix") == true) {
        // We need at least 9 keywords:
        if (Tokenizer2.GetNTokens() < 9) {
          Typo("Vector.Matrix must contain at least nine keywords,"
               " e.g. \"MaskMatrix.Matrix  3 1.0  3 1.0  1 0.0  1 0 1 0 1 0 1 0 1\"");
          return false;
        }
        unsigned int x_max = Tokenizer2.GetTokenAtAsUnsignedInt(2);
        unsigned int y_max = Tokenizer2.GetTokenAtAsUnsignedInt(4);
        unsigned int z_max = Tokenizer2.GetTokenAtAsUnsignedInt(6);
        double dx = Tokenizer2.GetTokenAtAsDouble(3);
        double dy = Tokenizer2.GetTokenAtAsDouble(5);
        double dz = Tokenizer2.GetTokenAtAsDouble(7);

        // Now we know the real number of keywords:
        if (Tokenizer2.GetNTokens() != 8+x_max*y_max*z_max) {
          Typo("This version of Vector.Matrix does not contain the right amount of numbers\"");
          return false;
        }

        for (unsigned int z = 0; z < z_max; ++z) {
          for (unsigned int y = 0; y < y_max; ++y) {
            for (unsigned int x = 0; x < x_max; ++x) {
              Vector->Add(MVector(x*dx, y*dy, z*dz), Tokenizer2.GetTokenAtAsDouble(8 + x + y*x_max + z*x_max*y_max));
            }
          }
        }

      } else {
        Typo("Unrecognized vector option");
        return false;
      }
    }
  }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating vectors) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file evaluating vectors *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  
  // Check for "For"-loops as well as the special "ForVector"-loop
  int ForDepth = 0;
  int CurrentDepth = 0;
  
  
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); /* ++ContentIter erase */) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);

    if (Tokenizer.GetNTokens() == 0) {
      ++ContentIter;
      continue;
    }

    // For
    auto BackToStartIter = ContentIter;
    if (Tokenizer.IsTokenAt(0, "For") == true || Tokenizer.IsTokenAt(0, "for") == true) {
      MTokenizer& TokenizerMaths = m_DebugInfo.GetTokenizer(true); // redo for math's evaluation just here

      CurrentDepth = ForDepth;
      ForDepth++;
      if (TokenizerMaths.GetNTokens() != 5) {
        Typo("Line must contain five entries, e.g. \"For I 3 -11.0 11.0\"");
        return false;
      }

      MString Index = TokenizerMaths.GetTokenAt(1);
      if (TokenizerMaths.GetTokenAtAsInt(2) < 0 || TokenizerMaths.GetTokenAtAsInt(2) != TokenizerMaths.GetTokenAtAsDouble(2) || std::isnan(TokenizerMaths.GetTokenAtAsDouble(2))) { // std:: is required
        Typo("Loop number in for loop must be a positive integer");
        return false;
      }
      unsigned int Loops = TokenizerMaths.GetTokenAtAsUnsignedInt(2);
      double Start = TokenizerMaths.GetTokenAtAsDouble(3);
      double Step = TokenizerMaths.GetTokenAtAsDouble(4);

      // Erase the for line
      (*ContentIter).SetText("");

      // Store content of for loop:
      list<MDDebugInfo> ForLoopContent;
      for (; ContentIter != FileContent.end(); /* ++ContentIter erase */) {
        m_DebugInfo = (*ContentIter);
        MTokenizer& TokenizerFor = (*ContentIter).GetTokenizer(false);

        if (TokenizerFor.GetNTokens() == 0) {
          ContentIter++;
          continue;
        }
        if (TokenizerFor.IsTokenAt(0, "For") == true || TokenizerFor.IsTokenAt(0, "for") == true) {
          ForDepth++;
        }
        if (TokenizerFor.IsTokenAt(0, "Done") == true || TokenizerFor.IsTokenAt(0, "done") == true) {
          ForDepth--;
          if (ForDepth == CurrentDepth) {
            (*ContentIter).SetText("");
            break;
          }
        }

        ForLoopContent.push_back(m_DebugInfo);
        (*ContentIter).SetText("");
      }

      // Add new content at the same place:
      list<MDDebugInfo>::iterator LastIter = ContentIter;
      int Position = 0;
      for (unsigned int l = 1; l <= Loops; ++l) {
        MString LoopString;
        LoopString += l;
        MString ValueString;
        ValueString += (Start + (l-1)*Step);


        list<MDDebugInfo>::iterator ForIter;
        for (ForIter = ForLoopContent.begin();
             ForIter != ForLoopContent.end();
             ++ForIter) {
          m_DebugInfo = (*ForIter);
          m_DebugInfo.Replace(MString("%") + Index, LoopString);
          m_DebugInfo.Replace(MString("$") + Index, ValueString);

          LastIter = FileContent.insert(next(LastIter), m_DebugInfo);
          Position++;
        }
      }
      ContentIter = BackToStartIter; // Multiple fors -- have to go back where we started
            
      continue;
    }

    // ForVector
    BackToStartIter = ContentIter;
    if (Tokenizer.IsTokenAt(0, "ForVector") == true) {

      // Take care of nesting
      CurrentDepth = ForDepth;
      ForDepth++;


      if (Tokenizer.GetNTokens() != 6) {
        Typo("The ForVector-line must contain six entries, e.g. \"ForVector MyVector X Y Z V\"");
        return false;
      }

      // Retrieve data:
      Vector = GetVector(Tokenizer.GetTokenAt(1));
      if (Vector == 0) {
        Typo("ForVector-line: cannot find vector\"");
        return false;
      }

      MString XIndex = Tokenizer.GetTokenAt(2);
      MString YIndex = Tokenizer.GetTokenAt(3);
      MString ZIndex = Tokenizer.GetTokenAt(4);
      MString VIndex = Tokenizer.GetTokenAt(5);

      // Erase the for line
      (*ContentIter).SetText("");
      
      // Store content of ForVector loop:
      list<MDDebugInfo> ForLoopContent;
      for (; ContentIter != FileContent.end(); /* ++ContentIter erase */) {
        m_DebugInfo = (*ContentIter);
        MTokenizer& TokenizerFor = m_DebugInfo.GetTokenizer(false);

        if (TokenizerFor.GetNTokens() == 0) {
          ContentIter++;
          continue;
        }
        if (TokenizerFor.IsTokenAt(0, "ForVector") == true) {
          ForDepth++;
        }
        if (TokenizerFor.IsTokenAt(0, "DoneVector") == true) {
          ForDepth--;
          if (ForDepth == CurrentDepth) {
            (*ContentIter).SetText("");
            break;
          }
        }

        ForLoopContent.push_back(m_DebugInfo);
        (*ContentIter).SetText("");
      }

      // Add new content at the same place:
      list<MDDebugInfo>::iterator LastIter = ContentIter;
      int Position = 0;
      for (unsigned int l = 1; l <= Vector->GetSize(); ++l) {
        //cout<<"Vector loc: "<<l<<endl;
        
        MString LoopString;
        LoopString += l;
        MString XValueString;
        XValueString += Vector->GetPosition(l-1).X();
        MString YValueString;
        YValueString += Vector->GetPosition(l-1).Y();
        MString ZValueString;
        ZValueString += Vector->GetPosition(l-1).Z();
        MString ValueString;
        ValueString += Vector->GetValue(l-1);

        list<MDDebugInfo>::iterator ForIter;
        for (ForIter = ForLoopContent.begin();
             ForIter != ForLoopContent.end();
             ++ForIter) {
          m_DebugInfo = (*ForIter);
          m_DebugInfo.Replace(MString("%") + XIndex, LoopString);
          m_DebugInfo.Replace(MString("$") + XIndex, XValueString);
          m_DebugInfo.Replace(MString("%") + YIndex, LoopString);
          m_DebugInfo.Replace(MString("$") + YIndex, YValueString);
          m_DebugInfo.Replace(MString("%") + ZIndex, LoopString);
          m_DebugInfo.Replace(MString("$") + ZIndex, ZValueString);
          m_DebugInfo.Replace(MString("%") + VIndex, LoopString);
          m_DebugInfo.Replace(MString("$") + VIndex, ValueString);

          LastIter = FileContent.insert(next(LastIter), m_DebugInfo);
          Position++;
        }
      }
      ContentIter = BackToStartIter;
      continue;
    }
    ++ContentIter;
  }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating for loops) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file evaluating for loops *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  

  // Find random numbers
  TRandom3 R;
  R.SetSeed(11031879); // Do never modify!!!!
  
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    while ((*ContentIter).Contains("RandomDouble") == true) {
      //cout<<"Before: "<<(*ContentIter).GetText()<<endl;
      (*ContentIter).ReplaceFirst("RandomDouble", R.Rndm());
      //cout<<"after: "<<(*ContentIter).GetText()<<endl;
    }
  }


  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating random numbers) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file evaluating random numbers *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  
    
  // Do a final maths check:
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
    
    if (Tokenizer.CheckAllMaths() == false) {
      Typo("Cannot parse maths -- typo or unsupported maths function.");
      return false;
    }
  }
  
  // Check for "If"-clauses
  int IfDepth = 0;
  int ElseDepth = 0;
  int CurrentIfDepth = 0;
  bool InsideElse = false;
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    if ((*ContentIter).IsTokenizerValid() == false) {
      Typo("Parsing of the line failed.");
      return false;
    }
   
    if (Tokenizer.GetNTokens() == 0) {
      continue;
    }

    if (Tokenizer.IsTokenAt(0, "If") == true) {

      // Take care of nesting
      CurrentIfDepth = IfDepth;
      IfDepth++;

      // Take care of else
      InsideElse = false;
      
      if (Tokenizer.GetNTokens() != 2) {
        Typo("The If-line must contain two entries, the last one must be math, e.g. \"If { 1 == 2 } or If { $Value > 0 } \"");
        return false;
      }

      // Retrieve data:
      bool IfStatement = Tokenizer.GetTokenAtAsBoolean(1);

      // Clear the if line
      //cout<<"Erasing (if): "<<(*ContentIter).GetText()<<endl;
      (*ContentIter).SetText("");

      // Forward its endif:
      for (auto NewContentIter = next(ContentIter, 1); NewContentIter != FileContent.end(); ++NewContentIter) {
        MTokenizer& TokenizerIf = (*NewContentIter).GetTokenizer(false);
        if (TokenizerIf.GetNTokens() == 0) {
          continue;
        }
        if (TokenizerIf.IsTokenAt(0, "If") == true || TokenizerIf.IsTokenAt(0, "if") == true) {
          IfDepth++;
        }
        if (TokenizerIf.IsTokenAt(0, "Else") == true || TokenizerIf.IsTokenAt(0, "else") == true) {
          InsideElse = true;
          ElseDepth++;
          if (IfDepth == CurrentIfDepth && IfDepth == ElseDepth) {
            //cout<<"Erasing (else): "<<(*NewContentIter).GetText()<<endl;
            (*NewContentIter).SetText("");
          }
        }
        if (TokenizerIf.IsTokenAt(0, "EndIf") == true || TokenizerIf.IsTokenAt(0, "Endif") == true || TokenizerIf.IsTokenAt(0, "endif") == true) {
          IfDepth--;
          if (IfDepth == CurrentIfDepth) {
            //cout<<"Erasing (endif): "<<(*NewContentIter).GetText()<<endl;
            (*NewContentIter).SetText("");
            break;
          }
        }
        if (IfStatement == false && InsideElse == false) {
          //cout<<"Erasing (if is false): "<<(*NewContentIter).GetText()<<endl;
          (*NewContentIter).SetText("");
        }
        if (IfStatement == true && InsideElse == true) {
          //cout<<"Erasing (else is false): "<<(*NewContentIter).GetText()<<endl;
          (*NewContentIter).SetText("");
        }
      }
      // ContentIter is not changed since we stay at the same level to all subsequent if's
    } // Is if
  } // global loop

  // Clean empty lines:
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ) {
    if ((*ContentIter).GetText() == "") {
      ContentIter = FileContent.erase(ContentIter);
    } else {
      ++ContentIter;
    }
  }
  
  
  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating if clauses + initial maths evaluation) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }
  
  if (DebugParsing == true) {
    cout<<endl<<endl<<endl<<endl;
    cout<<"***** After file evaluating if clauses *****"<<endl;
    cout<<endl<<endl;
    for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
      MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false);
      cout<<Tokenizer.ToCompactString()<<endl;
    }
  }
  
  
  
  // All constants and for loops are expanded, let's print some text ;-)
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    if ((*ContentIter).IsTokenizerValid() == false) {
      Typo("Parsing of the line failed.");
      return false;
    }
        
    if (Tokenizer.GetNTokens() == 0) continue;

    if (Tokenizer.IsTokenAt(0, "Print") == true ||
        Tokenizer.IsTokenAt(0, "Echo") == true) {
      if (Tokenizer.GetNTokens() < 2) {
        Typo("Line must contain at least two entries, e.g. \"Print Me!\"");
        return false;
      }

      //mout<<"   *** User Info start ***"<<endl;
      mout<<"   *** User Info: ";
      for (unsigned int t = 1; t < Tokenizer.GetNTokens(); ++t) {
        mout<<Tokenizer.GetTokenAt(t)<<"  ";
      }
      mout<<endl;
      //mout<<"   *** User Info end ***"<<endl;
    }
  }


  // First loop:
  // Find the master keyword, volumes, material, detectors
  //

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    if ((*ContentIter).IsTokenizerValid() == false) {
      Typo("Parsing of the line failed.");
      return false;
    }
       
    if (Tokenizer.GetNTokens() == 0) continue;

    // Let's scan for first order keywords:
    // Here we have:
    // - Name
    // - Version
    // - Material
    // - Volume
    // - Detector
    // - Trigger
    // - System
    // - Vector
    // - Shape
    // - Orientation

    // Volume (Most frequent, so start with this one)
    if (Tokenizer.IsTokenAt(0, "Volume") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Volume D1Main\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddVolume(new MDVolume(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Name
    else if (Tokenizer.IsTokenAt(0, "Name") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Name MEGA\"");
        return false;
      }
      m_Name = Tokenizer.GetTokenAt(1);
      m_Geometry->SetNameTitle(m_Name, "A Geomega geometry");
      continue;
    }

    // Version
    else if (Tokenizer.IsTokenAt(0, "Version") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Version 1.1.123\"");
        return false;
      }
      m_Version = Tokenizer.GetTokenAt(1);
      continue;
    }

    // Surrounding sphere
    else if (Tokenizer.IsTokenAt(0, "SurroundingSphere") == true) {
      if (FoundSurroundingSphere == true) {
        Typo("You have multiple surrounding spheres defined in your code. The ones read later overwrite the original one. This is too error prone to be allowed.");
        return false;
      }

      if (Tokenizer.GetNTokens() != 6) {
        Typo("Line must contain five values: Radius, xPos, yPos, zPos of sphere center, Distance to sphere center");
        return false;
      }

      m_SurroundingSphereRadius = Tokenizer.GetTokenAtAsDouble(1);
      m_SurroundingSpherePosition = MVector(Tokenizer.GetTokenAtAsDouble(2),
                                 Tokenizer.GetTokenAtAsDouble(3),
                                 Tokenizer.GetTokenAtAsDouble(4));
      m_SurroundingSphereDistance = Tokenizer.GetTokenAtAsDouble(5);

      if (m_SurroundingSphereRadius != m_SurroundingSphereDistance) {
        Typo("Limitation: Concerning your surrounding sphere: The sphere radius must equal the distance to the sphere for the time being. Sorry.");
        return false;
      }

      FoundSurroundingSphere = true;

      continue;
    }

    // Show the surrounding sphere
    else if (Tokenizer.IsTokenAt(0, "ShowSurroundingSphere") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: ShowSurroundingSphere true/false");
        return false;
      }

      m_SurroundingSphereShow = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Show volumes
    else if (Tokenizer.IsTokenAt(0, "ShowVolumes") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: ShowVolumes false");
        return false;
      }

      m_ShowVolumes = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Show volumes
    else if (Tokenizer.IsTokenAt(0, "ShowOnlySensitiveVolumes") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: ShowOnlySensitiveVolumes false");
        return false;
      }

      ShowOnlySensitiveVolumes = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Do Sanity checks
    else if (Tokenizer.IsTokenAt(0, "DoSanityChecks") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: DoSanityChecks false");
        return false;
      }

      m_DoSanityChecks = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Virtualize all non detector volumes
    else if (Tokenizer.IsTokenAt(0, "VirtualizeNonDetectorVolumes") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: VirtualizeNonDetectorVolumes false");
        return false;
      }

      m_VirtualizeNonDetectorVolumes = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Complex event reconstruction
    else if (Tokenizer.IsTokenAt(0, "ComplexER") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: ComplexER false");
        return false;
      }

      m_ComplexER = Tokenizer.GetTokenAtAsBoolean(1);

      continue;
    }

    // Ignore short names
    else if (Tokenizer.IsTokenAt(0, "IgnoreShortNames") == true) {
      mout<<" *** Outdated *** "<<endl;
      mout<<"The \"IgnoreShortNames\" keyword is no longer supported!"<<endl;

      continue;
    }

    // Default color
    else if (Tokenizer.IsTokenAt(0, "DefaultColor") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: DefaultColor  3");
        return false;
      }

      m_DefaultColor = Tokenizer.GetTokenAtAsInt(1);

      continue;
    }

    // Detector search tolerance
    else if (Tokenizer.IsTokenAt(0, "DetectorSearchTolerance") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: DetectorSearchTolerance 0.000001");
        return false;
      }

      m_DetectorSearchTolerance = Tokenizer.GetTokenAtAsDouble(1);

      continue;
    }

    // General absorption file directory
    else if (Tokenizer.IsTokenAt(0, "AbsorptionFileDirectory") == true ||
             Tokenizer.IsTokenAt(0, "CrossSectionFilesDirectory") == true ||
             Tokenizer.IsTokenAt(0, "CrossSectionPath") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: CrossSectionPath auxiliary");
        return false;
      }

      // We have to use TString
      MString Name = Tokenizer.GetTokenAtAsString(1).Data();
      if (MFile::ExpandFileName(Name) == false) {
        Typo("Unable to expand file name");
        return false;
      }

      if (gSystem->IsAbsoluteFileName(Name) == false) {
        Name.Prepend("/");
        Name.Prepend(MFile::GetDirectoryName(m_FileName));

        if (MFile::ExpandFileName(Name) == false) {
          Typo("Unable to expand cross-section path to absolute path");
          return false;
        }
      }

      m_CrossSectionFileDirectory = Name;

      continue;
    }

    // Material
    else if (Tokenizer.IsTokenAt(0, "Material") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Material Aluminimum\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddMaterial(new MDMaterial(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Shape
    else if (Tokenizer.IsTokenAt(0, "Shape") == true) {
      if (Tokenizer.GetNTokens() != 3) {
        Typo("Line must contain three strings, e.g. \"Shape BOX RedBox\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(2)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(2)) == true) {
          return false;
        }
      }

      AddShape(Tokenizer.GetTokenAt(1), Tokenizer.GetTokenAt(2));

      continue;
    }


    // Orientation
    else if (Tokenizer.IsTokenAt(0, "Orientation") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Orientation RedBoxOrientation\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddOrientation(new MDOrientation(Tokenizer.GetTokenAt(1)));

      continue;
    }

    // Trigger
    else if (Tokenizer.IsTokenAt(0, "Trigger") == true ||
      Tokenizer.IsTokenAt(0, "TriggerBasic") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Trigger D1D2\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddTrigger(dynamic_cast<MDTrigger*>(new MDTriggerBasic(Tokenizer.GetTokenAt(1))));
      continue;
      }

    // Trigger
    else if (Tokenizer.IsTokenAt(0, "TriggerMap") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"TriggerMap T\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddTrigger(dynamic_cast<MDTrigger*>(new MDTriggerMap(Tokenizer.GetTokenAt(1))));
      continue;
    }

    // System
    else if (Tokenizer.IsTokenAt(0, "System") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"System D1D2\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      m_System = new MDSystem(Tokenizer.GetTokenAt(1));
      continue;
    }

    // Detectors: Strip2D
    else if (Tokenizer.IsTokenAt(0, "Strip2D") == true ||
             Tokenizer.IsTokenAt(0, "MDStrip2D") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Strip2D Tracker\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDStrip2D(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: Strip3D
    else if (Tokenizer.IsTokenAt(0, "Strip3D") == true ||
             Tokenizer.IsTokenAt(0, "MDStrip3D") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Strip3D Germanium\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDStrip3D(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: Strip3DDirectional
    else if (Tokenizer.IsTokenAt(0, "Strip3DDirectional") == true ||
             Tokenizer.IsTokenAt(0, "MDStrip3DDirectional") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Strip3DDirectional ThickSiliconWafer\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDStrip3DDirectional(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: DriftChamber
    else if (Tokenizer.IsTokenAt(0, "DriftChamber") == true ||
             Tokenizer.IsTokenAt(0, "MDDriftChamber") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"DriftChamber Xenon\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDDriftChamber(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: AngerCamera
    else if (Tokenizer.IsTokenAt(0, "AngerCamera") == true ||
             Tokenizer.IsTokenAt(0, "MDAngerCamera") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"AngerCamera Angers\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDAngerCamera(Tokenizer.GetTokenAt(1)));
      continue;
    }


    // Detectors: ACS
    else if (Tokenizer.IsTokenAt(0, "Scintillator") == true ||
             Tokenizer.IsTokenAt(0, "Simple") == true ||
             Tokenizer.IsTokenAt(0, "ACS") == true ||
             Tokenizer.IsTokenAt(0, "MDACS") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Scintillator ACS1\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDACS(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: Calorimeter
    else if (Tokenizer.IsTokenAt(0, "Calorimeter") == true ||
             Tokenizer.IsTokenAt(0, "MDCalorimeter") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Calorimeter athena\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDCalorimeter(Tokenizer.GetTokenAt(1)));
      continue;
    }

    // Detectors: Voxel3D
    else if (Tokenizer.IsTokenAt(0, "Voxel3D") == true ||
             Tokenizer.IsTokenAt(0, "MDVoxel3D") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two strings, e.g. \"Voxel3D MyVoxler\"");
        return false;
      }
      if (m_DoSanityChecks == true) {
        if (ValidName(Tokenizer.GetTokenAt(1)) == false) {
          return false;
        }
        if (NameExists(Tokenizer.GetTokenAt(1)) == true) {
          return false;
        }
      }

      AddDetector(new MDVoxel3D(Tokenizer.GetTokenAt(1)));
      continue;
    }

  }

  // Now we can do some basic evaluation of the input:

  if (m_SurroundingSphereRadius == DBL_MAX) {
    Typo("You have to define a surrounding sphere!");
    return false;
  }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (analyzing primary keywords) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }

  //
  // Second loop:
  // Search for copies/clones of different volumes and named detectors
  //
  //

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    
    if (Tokenizer.GetNTokens() < 3) continue;


    // Check for volumes with copies
    if (Tokenizer.IsTokenAt(1, "Copy") == true) {
      if ((V = GetVolume(Tokenizer.GetTokenAt(0))) != 0) {
        if (GetVolume(Tokenizer.GetTokenAt(2)) != 0) {
          Typo("Copy: A volume of this name already exists!");
          return false;
        }
        if (V->IsClone() == true) {
          Typo("You cannot create a copy of a copy...");
          return false;
        }

        VCopy = new MDVolume(Tokenizer.GetTokenAt(2));

        AddVolume(VCopy);
        V->AddClone(VCopy);
      } else if ((M = GetMaterial(Tokenizer.GetTokenAt(0))) != 0) {
        if (GetMaterial(Tokenizer.GetTokenAt(2)) != 0) {
          Typo("A material of this name already exists!");
          return false;
        }

        MCopy = new MDMaterial(Tokenizer.GetTokenAt(2));

        AddMaterial(MCopy);
        M->AddClone(MCopy);
      }
    } else if (Tokenizer.IsTokenAt(1, "NamedDetector") == true || Tokenizer.IsTokenAt(1, "Named") == true) {
      if ((D = GetDetector(Tokenizer.GetTokenAt(0))) != 0) {
        if (D->IsNamedDetector() == true) {
          Typo("You cannot add a named detector to a named detector!");
          return false;
        }
        if (Tokenizer.GetNTokens() != 3) {
         Typo("Line must contain three strings, e.g. MyStripDetector.Named Strip1. Are you using the old NamedDetector format?");
         return false;
        }

        if (m_DoSanityChecks == true) {
          if (ValidName(Tokenizer.GetTokenAt(2)) == false) {
            Typo("You don't have a valid detector name!");
            return false;
          }
          if (NameExists(Tokenizer.GetTokenAt(2)) == true) {
            Typo("The name alreday exists!");
            return false;
          }
        }
        MDDetector* Clone = D->Clone();
        Clone->SetName(Tokenizer.GetTokenAt(2));
        if (D->AddNamedDetector(Clone) == false) {
          Typo("Unable to add a named detector!");
          return false;
        }
        AddDetector(Clone);
      }
    }

    // Umdrehen: Copy gefunden, dann Volumen/ Detector zuordnen
    // --> Fehler falls kein Volumen vorhanden
  }


  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (analyzing \"Copies\") finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }




  ////////////////////////////////////////////////////////////////////////////////////////////
  // Third loop:
  // Fill the volumes, materials with life...

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    
    if (Tokenizer.GetNTokens() < 2) continue;

    // Now the first token is some kind of name, so we have to find the
    // according object and fill it

    // Check for Materials:
    if ((M = GetMaterial(Tokenizer.GetTokenAt(0))) != 0) {
      if (Tokenizer.IsTokenAt(1, "Density") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Alu.Density 2.77\"");
          return false;
        }
        M->SetDensity(Tokenizer.GetTokenAtAsDouble(2));
      } else if (Tokenizer.IsTokenAt(1, "RadiationLength") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Alu.RadiationLength 8.9\"");
          return false;
        }
        M->SetRadiationLength(Tokenizer.GetTokenAtAsDouble(2));
      } else if (Tokenizer.IsTokenAt(1, "Component") == true ||
                 Tokenizer.IsTokenAt(1, "ComponentByAtoms") == true) {
        if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Alu.ComponentByAtoms 27.0 13.0 1.0  --> but this is deprecated\""
               " or three string and one double\""
               " e.g. \"Alu.ComponentByAtoms Al 1.0\"");
          return false;
        }
        if (Tokenizer.GetNTokens() == 4) {
          if (M->SetComponentByAtomicWeighting(Tokenizer.GetTokenAtAsString(2),
                                               Tokenizer.GetTokenAtAsInt(3)) == false) {
            Typo("Element not found!");
            return false;
          }
        } else {
          M->SetComponentByAtomicWeighting(Tokenizer.GetTokenAtAsDouble(2),
                                           Tokenizer.GetTokenAtAsInt(3),
                                          Tokenizer.GetTokenAtAsInt(4));
          mout<<"   ***  Info  ***  "<<endl;
          mout<<"Remember to use a component description which contains the element name, if you want natural isotope composition during Geant4 simulations"<<endl;
          mout<<"e.g. \"Alu.ComponentByAtoms Al 1.0\""<<endl;
        }
      } else if (Tokenizer.IsTokenAt(1, "ComponentByMass") == true) {
       if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Alu.ComponentByMass 27.0 13.0 1.0  --> but this is deprecated\""
               " or three string and one double\""
               " e.g. \"Alu.ComponentByMass Al 1.0\"");
          return false;
        }
        if (Tokenizer.GetNTokens() == 4) {
          if (M->SetComponentByMassWeighting(Tokenizer.GetTokenAtAsString(2),
                                             Tokenizer.GetTokenAtAsDouble(3)) == false) {
            Typo("Element not found!");
            return false;
          }
        } else {
          M->SetComponentByMassWeighting(Tokenizer.GetTokenAtAsDouble(2),
                                         Tokenizer.GetTokenAtAsInt(3),
                                         Tokenizer.GetTokenAtAsDouble(4));
          mout<<"   ***  Info  ***  "<<endl;
          mout<<"Remember to use a component description which contains the element name, if you want natural isotope composition during Geant4 simulations"<<endl;
          mout<<"e.g. \"Alu.ComponentByMass Al 1.0\""<<endl;
        }
      } else if (Tokenizer.IsTokenAt(1, "Sensitivity") == true) {
        mout<<"   ***  Info  ***  "<<endl;
        mout<<"Sensitivity keyword is redundant"<<endl;
      } else if (Tokenizer.IsTokenAt(1, "Copy") == true) {
        // No warning...
      } else {
        Typo("Unrecognized material option");
        return false;
      }
    } // end materials



    // Check for orientations:
    else if ((Orientation = GetOrientation(Tokenizer.GetTokenAt(0))) != 0) {
      if (Orientation->Parse(Tokenizer, m_DebugInfo) == false) {
        Reset();
        return false;
      }
    } // end orientations



    // Check for shapes:
    else if ((Shape = GetShape(Tokenizer.GetTokenAt(0))) != 0) {
      if (Shape->GetType() == "Subtraction") {
        if (Tokenizer.IsTokenAt(1, "Parameters") == true) {
          if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
            Typo("The shape subtraction needs 4-5 parameters");
            return false;
          }
          MDShape* Minuend = GetShape(Tokenizer.GetTokenAt(2));
          MDShape* Subtrahend = GetShape(Tokenizer.GetTokenAt(3));
          MDOrientation* Orientation = 0;
          if (Tokenizer.GetNTokens() == 5) {
            Orientation = GetOrientation(Tokenizer.GetTokenAt(4));
            if (Orientation == 0) {
              Typo("The orientation was not found!");
              return false;
            }
          } else {
            Orientation = new MDOrientation(Shape->GetName() + "_Orientation");
            AddOrientation(Orientation);
          }

          if (Minuend == 0) {
            Typo("The minuend shape was not found!");
            return false;
          }
          if (Subtrahend == 0) {
            Typo("The subtrahend shape was not found!");
            return false;
          }

          if (dynamic_cast<MDShapeSubtraction*>(Shape)->Set(Minuend, Subtrahend, Orientation) == false) {
            Typo("Unable to parse the shape correctly");
            return false;
          }
        }
      } else if (Shape->GetType() == "Union") {
        if (Tokenizer.IsTokenAt(1, "Parameters") == true) {
          if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
            Typo("The shape Union needs 4-5 parameters");
            return false;
          }
          MDShape* Augend = GetShape(Tokenizer.GetTokenAt(2));
          MDShape* Addend = GetShape(Tokenizer.GetTokenAt(3));
          MDOrientation* Orientation = 0;
          if (Tokenizer.GetNTokens() == 5) {
            Orientation = GetOrientation(Tokenizer.GetTokenAt(4));
            if (Orientation == 0) {
              Typo("The orientation was not found!");
              return false;
            }
          } else {
            Orientation = new MDOrientation(Shape->GetName() + "_Orientation");
            AddOrientation(Orientation);
          }

          if (Augend == 0) {
            Typo("The augend shape was not found!");
            return false;
          }
          if (Addend == 0) {
            Typo("The addend shape was not found!");
            return false;
          }

          if (dynamic_cast<MDShapeUnion*>(Shape)->Set(Augend, Addend, Orientation) == false) {
            Typo("Unable to parse the shape Union correctly");
            return false;
          }
        }
      } else if (Shape->GetType() == "Intersection") {
        if (Tokenizer.IsTokenAt(1, "Parameters") == true) {
          if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
            Typo("The shape Intersection needs 4-5 parameters");
            return false;
          }
          MDShape* Left = GetShape(Tokenizer.GetTokenAt(2));
          MDShape* Right = GetShape(Tokenizer.GetTokenAt(3));
          MDOrientation* Orientation = 0;
          if (Tokenizer.GetNTokens() == 5) {
            Orientation = GetOrientation(Tokenizer.GetTokenAt(4));
            if (Orientation == 0) {
              Typo("The orientation was not found!");
              return false;
            }
          } else {
            Orientation = new MDOrientation(Shape->GetName() + "_Orientation");
            AddOrientation(Orientation);
          }

          if (Left == 0) {
            Typo("The left shape was not found!");
            return false;
          }
          if (Right == 0) {
            Typo("The right shape was not found!");
            return false;
          }

          if (dynamic_cast<MDShapeIntersection*>(Shape)->Set(Left, Right, Orientation) == false) {
            Typo("Unable to parse the shape Intersection correctly");
            return false;
          }
        }
      } else {
        if (Shape->Parse(Tokenizer, m_DebugInfo) == false) {
          Reset();
          return false;
        }
      }
    } // end shapes



    // Check for triggers:
    else if ((T = GetTrigger(Tokenizer.GetTokenAt(0))) != 0) {
      if (Tokenizer.IsTokenAt(1, "PositiveDetectorType") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"PositiveDetectorType\" keyword is no longer supported!"<<endl;
        mout<<"Replace it with:"<<endl;
        mout<<T->GetName()<<".Veto false"<<endl;
        mout<<T->GetName()<<".TriggerByChannel true"<<endl;
        mout<<T->GetName()<<".DetectorType "
            <<MDDetector::GetDetectorTypeName(Tokenizer.GetTokenAtAsInt(2))
            <<" "<<Tokenizer.GetTokenAtAsInt(3)<<endl;
        mout<<"Using the above..."<<endl;
        mout<<endl;
        FoundDeprecated = true;

        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 integer, e.g. \"D1D2Trigger.DetectorType 1 2\"");
          return false;
        }
        // Check if such a detector type exists:
        bool Found = false;
        for (unsigned int d = 0; d < GetNDetectors(); ++d) {
          if (GetDetectorAt(d)->GetType() == Tokenizer.GetTokenAtAsInt(2)) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          Typo("Line contains a not defined detector type!");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetVeto(false);
          dynamic_cast<MDTriggerBasic*>(T)->SetTriggerByChannel(true);
          dynamic_cast<MDTriggerBasic*>(T)->SetDetectorType(Tokenizer.GetTokenAtAsInt(2), Tokenizer.GetTokenAtAsInt(3));
        } else {
          Typo("Keyword PositiveDetectorType only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "TriggerByChannel") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean, e.g. \"D1D2Trigger.TriggerByChannel true\"");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetTriggerByChannel(Tokenizer.GetTokenAtAsBoolean(2));
        } else {
          Typo("Keyword TriggerByChannel only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "TriggerByDetector") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean,"
               " e.g. \"D1D2Trigger.TriggerByDetector true\"");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetTriggerByDetector(Tokenizer.GetTokenAtAsBoolean(2));
        } else {
          Typo("Keyword TriggerByDetector only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "Veto") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean,"
               " e.g. \"D1D2Trigger.Veto true\"");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetVeto(Tokenizer.GetTokenAtAsBoolean(2));
        } else {
          Typo("Keyword Veto only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "DetectorType") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
               " e.g. \"D1D2Trigger.DetectorType Strip3D 1\"");
          return false;
        }
        if (MDDetector::IsValidDetectorType(Tokenizer.GetTokenAtAsString(2)) == false) {
          Typo("Line must contain a valid detector type, e.g. Strip2D, DriftChamber, etc.");
          return false;
        }
        // Check if such a detector type exists:
        bool Found = false;
        for (unsigned int d = 0; d < GetNDetectors(); ++d) {
          if (GetDetectorAt(d)->GetTypeName() == Tokenizer.GetTokenAtAsString(2)) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          Typo("Line contains a not defined detector type!");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetDetectorType(MDDetector::GetDetectorType(Tokenizer.GetTokenAtAsString(2)),
                           Tokenizer.GetTokenAtAsInt(3));
        } else {
          Typo("Keyword DetectorType only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "Detector") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
               " e.g. \"D1D2Trigger.Detector MyStrip2D 1\"");
          return false;
        }
        MDDetector* TriggerDetector;
        if ((TriggerDetector = GetDetector(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("A detector of this name does not exist!");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetDetector(TriggerDetector, Tokenizer.GetTokenAtAsInt(3));
        } else {
          Typo("Keyword Detector only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "GuardRingDetectorType") == true || Tokenizer.IsTokenAt(1, "GuardringDetectorType") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
               " e.g. \"D1D2Trigger.GuardRingDetectorType Strip3D 1\"");
          return false;
        }
        if (MDDetector::IsValidDetectorType(Tokenizer.GetTokenAtAsString(2)) == false) {
          Typo("Line must contain a valid detector type, e.g. Strip2D, DriftChamber, etc.");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetGuardRingDetectorType(MDDetector::GetDetectorType(Tokenizer.GetTokenAtAsString(2)),
                                    Tokenizer.GetTokenAtAsInt(3));
        } else {
          Typo("Keyword GuardRingDetectorType only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "GuardRingDetector") == true || Tokenizer.IsTokenAt(1, "GuardringDetector") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
          " e.g. \"D1D2Trigger.GuardRingDetector MyStrip2D 1\"");
          return false;
        }
        MDDetector* TriggerDetector;
        if ((TriggerDetector = GetDetector(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("A detector of this name does not exist!");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Basic) {
          dynamic_cast<MDTriggerBasic*>(T)->SetGuardRingDetector(TriggerDetector, Tokenizer.GetTokenAtAsInt(3));
        } else {
          Typo("Keyword GuardRingDetector only supported for basic trigger");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "Map") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain three strings, e.g. \"MapTrigger.Map MyMap.trig\"");
          return false;
        }
        if (T->GetType() == MDTriggerType::c_Universal) {
          MString FileName = Tokenizer.GetTokenAtAsString(2);

          if (MFile::ExpandFileName(FileName, m_FileName) == false) {
            Typo("Unable to expand file name");
            return false;
          }
      
          if (gSystem->AccessPathName(FileName) == 1) {
            Typo("The file does not exist!");
            return false;
          }

          if (dynamic_cast<MDTriggerMap*>(T)->ReadTriggerMap(FileName) == false) {
            Typo("Unable to read trigger map");
            return false;
          }
        } else {
          Typo("Keyword Map only supported for map triggers");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "NegativeDetectorType") == true) {
        mout<<" *** Outdated *** "<<endl;
        mout<<"The \"NegativeDetectorType\" keyword is no longer supported!"<<endl;
        return false;

      } else {
        Typo("Unrecognized trigger option");
        return false;
      }
    }

    // Check for volumes
    else if ((S = GetSystem(Tokenizer.GetTokenAt(0))) != 0) {
      if (Tokenizer.IsTokenAt(1, "TimeResolution") == true) {
        if (Tokenizer.GetNTokens() >= 3) {
          MString Type = Tokenizer.GetTokenAt(2);
          Type.ToLower();
          if (Type == "ideal" || Type == "none" || Type == "no" || Type == "nix" || Type == "perfect") {
            m_System->SetTimeResolutionType(MDSystem::c_TimeResolutionTypeIdeal);
          } else if (Type == "gauss" || Type == "gaus") {
            if (Tokenizer.GetNTokens() == 4) {
              m_System->SetTimeResolutionType(MDSystem::c_TimeResolutionTypeGauss);
              m_System->SetTimeResolutionGaussSigma(Tokenizer.GetTokenAtAsDouble(3));
            } else {
              Typo("Line must contain three strings and one double,"
                   " e.g. \"MEGA.TimeResolution Gauss 2.0E-6\"");
              return false;
            }
          } else {
            Typo("Unrecognized time resolution type.");
            return false;
          }
        } else {
          Typo("Not enough tokens.");
          return false;
        }
      } else {
        Typo("Unrecognized system option");
        return false;
      }
    }

    // Check for volumes
    else if ((V = GetVolume(Tokenizer.GetTokenAt(0))) != 0) {
      if (Tokenizer.IsTokenAt(1, "Material") == true) {
        if ((M = GetMaterial(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("Unknown material found!");
          return false;
        }
        V->SetMaterial(M);
      } else if (Tokenizer.IsTokenAt(1, "Shape") == true) {
        if (Tokenizer.GetNTokens() < 3) {
          Typo("Not enough input parameters for this shape!");
          return false;
        }
        MString ShapeID = Tokenizer.GetTokenAtAsString(2);
        ShapeID.ToLowerInPlace();

        if (ShapeID.AreIdentical("brik") == true || ShapeID.AreIdentical("box") == true) {
          MDShapeBRIK* BRIK = new MDShapeBRIK(V->GetName() + "_Shape");
          if (BRIK->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(BRIK);
          AddShape(BRIK);

        } else if (ShapeID.AreIdentical("pcon") == true) {
          MDShapePCON* PCON = new MDShapePCON(V->GetName() + "_Shape");
          if (PCON->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(PCON);
          AddShape(PCON);

        } else if (ShapeID.AreIdentical("pgon") == true) {
          MDShapePGON* PGON = new MDShapePGON(V->GetName() + "_Shape");
          if (PGON->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(PGON);
          AddShape(PGON);
        }

        // Sphere:
        else if (ShapeID.AreIdentical("sphe") == true || ShapeID.AreIdentical("sphere") == true) {
          MDShapeSPHE* SPHE = new MDShapeSPHE(V->GetName() + "_Shape");
          if (SPHE->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(SPHE);
          AddShape(SPHE);
        }

        // Cylinder:
        else if (ShapeID.AreIdentical("tubs") == true || ShapeID.AreIdentical("tube") == true) {
          MDShapeTUBS* TUBS = new MDShapeTUBS(V->GetName() + "_Shape");
          if (TUBS->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(TUBS);
          AddShape(TUBS);
        }

        // Cone:
        else if (ShapeID.AreIdentical("cone") == true) {
          MDShapeCONE* CONE = new MDShapeCONE(V->GetName() + "_Shape");
          if (CONE->Parse(Tokenizer, m_DebugInfo) == false) {
            Typo("Shape of CONE not ok");
            return false;
          }
          V->SetShape(CONE);
          AddShape(CONE);
        }

        // CONS:
        else if (ShapeID.AreIdentical("cons") == true) {
          MDShapeCONS* CONS = new MDShapeCONS(V->GetName() + "_Shape");
          if (CONS->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(CONS);
          AddShape(CONS);
        }
        // General trapezoid:
        else if (ShapeID.AreIdentical("trap") == true) {
         MDShapeTRAP* TRAP = new MDShapeTRAP(V->GetName() + "_Shape");
          if (TRAP->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(TRAP);
          AddShape(TRAP);
        }
        // General twisted trapezoid:
        else if (ShapeID.AreIdentical("gtra") == true) {
          MDShapeGTRA* GTRA = new MDShapeGTRA(V->GetName() + "_Shape");
          if (GTRA->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(GTRA);
          AddShape(GTRA);
        }
        // Simple trapezoid
        else if (ShapeID.AreIdentical("trd1") == true) {
          MDShapeTRD1* TRD1 = new MDShapeTRD1(V->GetName() + "_Shape");
          if (TRD1->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(TRD1);
          AddShape(TRD1);
        }
        // Simple trapezoid
        else if (ShapeID.AreIdentical("trd2") == true) {
          MDShapeTRD2* TRD2 = new MDShapeTRD2(V->GetName() + "_Shape");
          if (TRD2->Parse(Tokenizer, m_DebugInfo) == false) {
            Reset();
            return false;
          }
          V->SetShape(TRD2);
          AddShape(TRD2);
        }
        // Simple union of two shapes
        else if (ShapeID.AreIdentical("union") == true) {
          MDShapeUnion* Union = new MDShapeUnion(V->GetName() + "_Shape");

          if (Tokenizer.GetNTokens() != 6) {
            Typo("The shape Union needs 6 parameters");
            return false;
          }
          MDShape* Augend = GetShape(Tokenizer.GetTokenAt(3));
          MDShape* Addend = GetShape(Tokenizer.GetTokenAt(4));
          MDOrientation* Orientation = GetOrientation(Tokenizer.GetTokenAt(5));

          if (Augend == 0) {
            Typo("The augend shape was not found!");
            return false;
          }
          if (Addend == 0) {
            Typo("The addend shape was not found!");
            return false;
          }
          if (Orientation == 0) {
            Typo("The orientation was not found!");
            return false;
          }

          if (Union->Set(Augend, Addend, Orientation) == false) {
            Typo("Unable to parse the shape Union correctly");
            return false;
          }

          V->SetShape(Union);
          AddShape(Union);
        }
        // Subtraction
        else if (ShapeID.AreIdentical("subtraction") == true) {
          MDShapeSubtraction* Subtraction = new MDShapeSubtraction(V->GetName() + "_Shape");

          if (Tokenizer.GetNTokens() != 6) {
            Typo("The shape subtraction needs 6 parameters");
            return false;
          }
          MDShape* Minuend = GetShape(Tokenizer.GetTokenAt(3));
          MDShape* Subtrahend = GetShape(Tokenizer.GetTokenAt(4));
          MDOrientation* Orientation = GetOrientation(Tokenizer.GetTokenAt(5));

          if (Minuend == 0) {
            Typo("The minuend shape was not found!");
            return false;
          }
          if (Subtrahend == 0) {
            Typo("The subtrahend shape was not found!");
            return false;
          }
          if (Orientation == 0) {
            Typo("The orientation was not found!");
            return false;
          }

          if (Subtraction->Set(Minuend, Subtrahend, Orientation) == false) {
            Typo("Unable to parse the shape correctly");
            return false;
          }

          V->SetShape(Subtraction);
          AddShape(Subtraction);
        }
        // Intersection
        else if (ShapeID.AreIdentical("intersection") == true) {
          MDShapeIntersection* Intersection = new MDShapeIntersection(V->GetName() + "_Shape");

          if (Tokenizer.GetNTokens() != 6) {
            Typo("The shape Intersection needs 6 parameters");
            return false;
          }
          MDShape* Left = GetShape(Tokenizer.GetTokenAt(3));
          MDShape* Right = GetShape(Tokenizer.GetTokenAt(4));
          MDOrientation* Orientation = GetOrientation(Tokenizer.GetTokenAt(5));

          if (Left == 0) {
            Typo("The left shape was not found!");
            return false;
          }
          if (Right == 0) {
            Typo("The right shape was not found!");
            return false;
          }
          if (Orientation == 0) {
            Typo("The orientation was not found!");
            return false;
          }

          if (Intersection->Set(Left, Right, Orientation) == false) {
            Typo("Unable to parse the shape Intersection correctly");
            return false;
          }

          V->SetShape(Intersection);
          AddShape(Intersection);
        }

        // Now check if it is in the list
        else {
          MDShape* Shape = GetShape(Tokenizer.GetTokenAt(2));
          if (Shape != 0) {
            V->SetShape(Shape);
          } else {
            Typo("Unknown shape found!");
            return false;
          }
        }


      } else if (Tokenizer.IsTokenAt(1, "Mother") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain three strings"
               " e.g. \"Triangle.Mother WorldVolume\"");
          return false;
        }

        if (Tokenizer.IsTokenAt(2, "0")) {
          V->SetWorldVolume();
          if (m_WorldVolume != 0) {
            Typo("You are only allowed to have one world volume!");
            return false;
          }
          m_WorldVolume = V;
        } else {
          if (GetVolume(Tokenizer.GetTokenAt(2)) == 0) {
            Typo("Mother: A volume of this name does not exist!");
            return false;
          }
          if (GetVolume(Tokenizer.GetTokenAt(2)) == V) {
            Typo("A volume can't be its own mother!");
            return false;
          }
          if (GetVolume(Tokenizer.GetTokenAt(2))->IsClone() == true) {
            Typo("You are not allowed to set a volume as mother which is created via \"Copy\". Use the original volume!");
            return false;
          }

          if (V->SetMother(GetVolume(Tokenizer.GetTokenAt(2))) == false) {
            Typo("Mother could not be set (Do you have some cyclic mother-relations defined?)");
            return false;
          }
        }
      } else if (Tokenizer.IsTokenAt(1, "Color") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one integer"
               " e.g. \"Triangle.Color 2\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsInt(2) < 0) {
          Typo("The color value needs to be positive");
          return false;
        }
        V->SetColor(Tokenizer.GetTokenAtAsInt(2));
      } else if (Tokenizer.IsTokenAt(1, "LineStyle") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one integer"
               " e.g. \"Triangle.LineStyle 1\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsInt(2) < 0) {
          Typo("The line style value needs to be positive");
          return false;
        }
        V->SetLineStyle(Tokenizer.GetTokenAtAsInt(2));
      } else if (Tokenizer.IsTokenAt(1, "LineWidth") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one integer"
               " e.g. \"Triangle.LineWidth 1\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsInt(2) < 0) {
          Typo("The line width value needs to be positive");
          return false;
        }
        V->SetLineWidth(Tokenizer.GetTokenAtAsInt(2));
      } else if (Tokenizer.IsTokenAt(1, "Scale") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one integer"
               " e.g. \"Triangle.Scale 2.0\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsDouble(2) <= 0) {
          Typo("The color value needs to be positive");
          return false;
        }
        ScaledVolumes[V] = Tokenizer.GetTokenAtAsDouble(2);
      } else if (Tokenizer.IsTokenAt(1, "Virtual") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean"
               " e.g. \"Triangle.Virtual true\"");
          return false;
        }
        V->SetVirtual(Tokenizer.GetTokenAtAsBoolean(2));
      } else if (Tokenizer.IsTokenAt(1, "Many") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean"
               " e.g. \"Triangle.Many true\"");
          return false;
        }
        V->SetMany(Tokenizer.GetTokenAtAsBoolean(2));
      } else if (Tokenizer.IsTokenAt(1, "Visibility") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one integer"
               " e.g. \"Triangle.Visibility 1\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsInt(2) < -1 || Tokenizer.GetTokenAtAsInt(2) > 3) {
          Typo("The visibility needs to be -1, 0, 1, 2, or 3");
          return false;
        }
        V->SetVisibility(Tokenizer.GetTokenAtAsInt(2));
      } else if (Tokenizer.IsTokenAt(1, "Absorptions") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"Absorptions\" keyword is no longer supported"<<endl;
        mout<<"All cross section files have fixed file names, and can be found in the directory given by"<<endl;
        mout<<"the keyword CrossSectionFilesDirectory or in \"auxiliary\" as default"<<endl;
        mout<<endl;
        FoundDeprecated = true;
      } else if (Tokenizer.IsTokenAt(1, "Orientation") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain three strings,"
               " e.g. \"Wafer.Orientation WaferOrientation\"");
          return false;
        }
        Orientation = GetOrientation(Tokenizer.GetTokenAtAsString(2));
        if (Orientation == 0) {
          Typo("Cannot find the requested orientation. Did you define it?");
          return false;
        }
        V->SetPosition(Orientation->GetPosition());
        V->SetRotation(Orientation->GetRotationMatrix());

      } else if (Tokenizer.IsTokenAt(1, "Position") == true) {
        if (Tokenizer.GetNTokens() != 5) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Wafer.Position 3.5 1.0 0.0\"");
          return false;
        }
        V->SetPosition(MVector(Tokenizer.GetTokenAtAsDouble(2),
                               Tokenizer.GetTokenAtAsDouble(3),
                               Tokenizer.GetTokenAtAsDouble(4)));
      } else if (Tokenizer.IsTokenAt(1, "Rotation") == true ||
                 Tokenizer.IsTokenAt(1, "Rotate") == true) {
        if (Tokenizer.GetNTokens() != 5 && Tokenizer.GetNTokens() != 8) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Wafer.Rotation 60.0 0.0 0.0\"");
          return false;
        }
        if (Tokenizer.GetNTokens() == 5) {
          V->SetRotation(Tokenizer.GetTokenAtAsDouble(2),
                         Tokenizer.GetTokenAtAsDouble(3),
                         Tokenizer.GetTokenAtAsDouble(4));
        } else {
          double theta1 = Tokenizer.GetTokenAtAsDouble(2);
          double theta2 = Tokenizer.GetTokenAtAsDouble(4);
          double theta3 = Tokenizer.GetTokenAtAsDouble(6);
          if (theta1 < 0 || theta1 > 180) {
            Typo("Theta1 of Rotation needs per definition to be within [0;180]");
            return false;
          }
          if (theta2 < 0 || theta2 > 180) {
            Typo("Theta2 of Rotation needs per definition to be within [0;180]");
            return false;
          }
          if (theta3 < 0 || theta3 > 180) {
            Typo("Theta3 of Rotation needs per definition to be within [0;180]");
            return false;
          }

          if (V->SetRotation(Tokenizer.GetTokenAtAsDouble(2),
                             Tokenizer.GetTokenAtAsDouble(3),
                             Tokenizer.GetTokenAtAsDouble(4),
                             Tokenizer.GetTokenAtAsDouble(5),
                             Tokenizer.GetTokenAtAsDouble(6),
                             Tokenizer.GetTokenAtAsDouble(7)) == false) {
            Typo("Unable to create the new rotation");
            return false;
          }
        }
      } else if (Tokenizer.IsTokenAt(1, "Copy") == true) {
        // No warning...
      } else {
        Typo("Unrecognized volume option!");
        return false;
      }
    } // end Volume
  } // end third loop...





  /////////////////////////////////////////////////////////////////////////////////////
  // Fourth loop:
  // Fill the detector not before everything else is done!

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    
    if (Tokenizer.GetNTokens() < 2) continue;

    // Check for detectors:
    if ((D = GetDetector(Tokenizer.GetTokenAt(0))) != 0) {
      // Check for global tokens

      // Check for simulation in voxels instead of a junk volume
      if (Tokenizer.IsTokenAt(1, "VoxelSimulation") == true) {
        mout<<" *** Outdated *** "<<endl;
        mout<<"The \"VoxelSimulation\" keyword is no longer supported!"<<endl;
      }
      // Check for sensitive volume
      else if (Tokenizer.IsTokenAt(1, "SensitiveVolume") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("SensitiveVolume cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        // Test if volume exists:
        if ((V = GetVolume(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("SensitiveVolume: A volume of this name does not exist!");
          return false;
        }
        D->AddSensitiveVolume(V);
      }
      // Check for detector volume
      else if (Tokenizer.IsTokenAt(1, "DetectorVolume") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("DetectorVolume cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        // Test if volume exists:
        if ((V = GetVolume(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("DetectorVolume: A volume of this name does not exist!");
          return false;
        }
        D->SetDetectorVolume(V);
      }
      // Check for NoiseThresholdEqualsTriggerThreshold
      else if (Tokenizer.IsTokenAt(1, "NoiseThresholdEqualsTriggerThreshold") == true ||
               Tokenizer.IsTokenAt(1, "NoiseThresholdEqualTriggerThreshold") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain one string and one bool,"
               " e.g. \"Wafer.NoiseThresholdEqualsTriggerThreshold true\"");
          return false;
        }
        D->SetNoiseThresholdEqualsTriggerThreshold(Tokenizer.GetTokenAtAsBoolean(2));
      }
      // Check for noise threshold
      else if (Tokenizer.IsTokenAt(1, "NoiseThreshold") == true) {
        if (Tokenizer.GetNTokens() < 3 || Tokenizer.GetNTokens() > 4) {
          Typo("Line must contain one string and 2 doubles,"
               " e.g. \"Wafer.NoiseThreshold 30 10\"");
          return false;
        }
        D->SetNoiseThreshold(Tokenizer.GetTokenAtAsDouble(2));
        if (Tokenizer.GetNTokens() == 4) {
          D->SetNoiseThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
        }
      }
      // Check for trigger threshold
      else if (Tokenizer.IsTokenAt(1, "TriggerThreshold") == true) {
        if (Tokenizer.GetNTokens() < 3 || Tokenizer.GetNTokens() > 4) {
          Typo("Line must contain one string and 2 double,"
               " e.g. \"Wafer.TriggerThreshold 30 10\"");
          return false;
        }
        D->SetTriggerThreshold(Tokenizer.GetTokenAtAsDouble(2));
        if (Tokenizer.GetNTokens() == 4) {
          D->SetTriggerThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
        }
      }
      // Check for failure rate
      else if (Tokenizer.IsTokenAt(1, "FailureRate") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain one string and 1 double,"
               " e.g. \"Wafer.FailureRate 0.01\"");
          return false;
        }
        D->SetFailureRate(Tokenizer.GetTokenAtAsDouble(2));
      }
      // Check for minimum and maximum overflow
      else if (Tokenizer.IsTokenAt(1, "Overflow") == true) {
        if (Tokenizer.GetNTokens() < 3 || Tokenizer.GetNTokens() > 4) {
          Typo("Line must contain one string and 2 doubles,"
               " e.g. \"Wafer.Overflow 350 100\"");
          return false;
        }
        D->SetOverflow(Tokenizer.GetTokenAtAsDouble(2));
        if (Tokenizer.GetNTokens() == 4) {
          D->SetOverflowSigma(Tokenizer.GetTokenAtAsDouble(3));
        } else {
          D->SetOverflowSigma(0.0);
        }
      }
      // Check for energy loss maps
      else if (Tokenizer.IsTokenAt(1, "EnergyLossMap") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("EnergyLossMap cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings,"
               " e.g. \"Wafer.EnergyLossMap MyEnergyLoss\"");
          return false;
        }
        MString FileName = Tokenizer.GetTokenAt(2);
        if (MFile::ExpandFileName(FileName, m_FileName) == false) {
          Typo("Unable to expand file name");
          return false;
        }
        if (MFile::Exists(FileName) == false) {
          Typo("File does not exist.");
          return false;
        }
        D->SetEnergyLossMap(FileName);
      }
      // Check for energy resolution
      else if (Tokenizer.IsTokenAt(1, "EnergyResolutionAt") == true ||
               Tokenizer.IsTokenAt(1, "EnergyResolution") == true) {
        if (Tokenizer.GetNTokens() < 3) {
          Typo("EnergyResolution keyword not correct.");
          return false;
        }
        MString Type = Tokenizer.GetTokenAt(2);
        char* Tester;
        double d = strtod(Type.Data(), &Tester); if (d != 0) d = 0; // Just to prevent the compiler from complaining
        if (Tester != Type.Data()) {
          // We have a number - do it the old way
          if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 6) {
            Typo("Line must contain one string and 2-4 doubles,"
                 " e.g. \"Wafer.EnergyResolutionAt 662   39  (20  -2)\"");
            return false;
          }
          mout<<" *** Deprectiated *** "<<endl;
          mout<<"The \"EnergyResolution\" keyword format has changed. Please see the geomega manual."<<endl;
          mout<<"Using a Gaussian resolution in compatibility mode."<<endl;
          mout<<endl;
          //FoundDeprecated = true;
          D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
          if (Tokenizer.GetNTokens() == 4) {
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                   Tokenizer.GetTokenAtAsDouble(2),
                                   Tokenizer.GetTokenAtAsDouble(3));
          } else if (Tokenizer.GetNTokens() == 5) {
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                   Tokenizer.GetTokenAtAsDouble(2),
                                   Tokenizer.GetTokenAtAsDouble(3));
          } else if (Tokenizer.GetNTokens() == 6) {
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                   Tokenizer.GetTokenAtAsDouble(2) + Tokenizer.GetTokenAtAsDouble(5),
                                   Tokenizer.GetTokenAtAsDouble(3));
          }
        } else {
          // New way:
          Type.ToLower();
          if (Type == "ideal" || Type == "perfect") {
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeIdeal);
          } else if (Type == "none" || Type == "no") {
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeNone);
          } else if (Type == "gauss" || Type == "gaus") {
            if (Tokenizer.GetNTokens() != 6 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolution Gauss 122 122 2.0\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeGauss &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(3),
                                   Tokenizer.GetTokenAtAsDouble(4),
                                   Tokenizer.GetTokenAtAsDouble(5));


          } else if (Type == "lorentz" || Type == "lorenz") {
            if (Tokenizer.GetNTokens() != 6 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolution Lorentz 122 122 2.0\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeLorentz &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeLorentz);
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(3),
                                   Tokenizer.GetTokenAtAsDouble(4),
                                   Tokenizer.GetTokenAtAsDouble(5));

          } else if (Type == "gauslandau" || Type == "gausslandau" || Type == "gauss-landau") {
            if (Tokenizer.GetNTokens() != 9 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolution GaussLandau 122 122 2.0 122 3.0 0.2\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeGaussLandau &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
            D->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(3),
                                   Tokenizer.GetTokenAtAsDouble(4),
                                   Tokenizer.GetTokenAtAsDouble(5),
                                   Tokenizer.GetTokenAtAsDouble(6),
                                   Tokenizer.GetTokenAtAsDouble(7),
                                   Tokenizer.GetTokenAtAsDouble(8));

          } else {
            Typo("Unknown EnergyResolution parameters.");
            return false;
          }
        }
      }
      // Check for energy resolution type
      else if (Tokenizer.IsTokenAt(1, "EnergyResolutionType") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings,"
               " e.g. \"Wafer.EnergyResolutionType Gauss\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsString(2) == "Gauss" ||
            Tokenizer.GetTokenAtAsString(2) == "Gaus") {
          D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
        } else if (Tokenizer.GetTokenAtAsString(2) == "Lorentz") {
          D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeLorentz);
        } else {
          Typo("Unkown energy resolution type!");
          return false;
        }
      }
      // Check for energy resolution type
      else if (Tokenizer.IsTokenAt(1, "EnergyCalibration") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings,"
               " e.g. \"Wafer.EnergyCalibration File.dat\"");
          return false;
        }
        MString FileName = Tokenizer.GetTokenAtAsString(2);
        if (MFile::ExpandFileName(FileName, m_FileName) == false) {
          Typo("Unable to expand file name");
          return false;
        }
        MFunction Calibration;
        if (Calibration.Set(FileName, "DP") == false) {
          Typo("Unable to read file");
          return false;
        }
        D->SetEnergyCalibration(Calibration);
      }
      // Check for time resolution
      else if (Tokenizer.IsTokenAt(1, "TimeResolution") == true ||
               Tokenizer.IsTokenAt(1, "TimeResolutionAt") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain one string and 2 doubles,"
               " e.g. \"Wafer.TimeResolutionAt 662  0.0000001\"");
          return false;
        }
        D->SetTimeResolution(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3));
      }
      // Check for pulse-form:
      else if (Tokenizer.IsTokenAt(1, "PulseShape") == true) {
        if (Tokenizer.GetNTokens() != 14) {
          Typo("Line must contain one string and 12 doubles,"
               " e.g. \"Wafer.PulseShape <10xFitParameter> FitStart FitStop\"");
          return false;
        }
        D->SetPulseShape(Tokenizer.GetTokenAtAsDouble(2),
                         Tokenizer.GetTokenAtAsDouble(3),
                         Tokenizer.GetTokenAtAsDouble(4),
                         Tokenizer.GetTokenAtAsDouble(5),
                         Tokenizer.GetTokenAtAsDouble(6),
                         Tokenizer.GetTokenAtAsDouble(7),
                         Tokenizer.GetTokenAtAsDouble(8),
                         Tokenizer.GetTokenAtAsDouble(9),
                         Tokenizer.GetTokenAtAsDouble(10),
                         Tokenizer.GetTokenAtAsDouble(11),
                         Tokenizer.GetTokenAtAsDouble(12),
                         Tokenizer.GetTokenAtAsDouble(13));
      }
      // Check for structural size
      else if (Tokenizer.IsTokenAt(1, "StructuralSize") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"StructuralSize\" keyword is no longer supported, since it contained redundant information"<<endl;
        mout<<endl;
        FoundDeprecated = true;
        //         if (Tokenizer.GetNTokens() != 5) {
        //           Typo("Line must contain one string and 3 doubles,"
        //                " e.g. \"Wafer.StructuralSize 0.0235, 3.008, 0.5\"");
        //           return false;
        //         }
        //         D->SetStructuralSize(MVector(Tokenizer.GetTokenAtAsDouble(2),
        //                                       Tokenizer.GetTokenAtAsDouble(3),
        //                                       Tokenizer.GetTokenAtAsDouble(4)));
      }
      // Check for structural offset
      else if (Tokenizer.IsTokenAt(1, "StructuralOffset") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("StructuralOffset cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        if (Tokenizer.GetNTokens() != 5) {
          Typo("Line must contain one string and 3 doubles,"
               " e.g. \"Wafer.StructuralOffset 0.142, 0.142, 0.0\"");
          return false;
        }
        D->SetStructuralOffset(MVector(Tokenizer.GetTokenAtAsDouble(2),
                                       Tokenizer.GetTokenAtAsDouble(3),
                                       Tokenizer.GetTokenAtAsDouble(4)));
      }
      // Check for structural pitch
      else if (Tokenizer.IsTokenAt(1, "StructuralPitch") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("StructuralPitch cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        if (Tokenizer.GetNTokens() != 5) {
          Typo("Line must contain one string and 3 doubles,"
               " e.g. \"Wafer.StructuralPitch 0.0, 0.0, 0.0\"");
          return false;
        }
        D->SetStructuralPitch(MVector(Tokenizer.GetTokenAtAsDouble(2),
                                      Tokenizer.GetTokenAtAsDouble(3),
                                      Tokenizer.GetTokenAtAsDouble(4)));
      }
      // Check for SiStrip specific tokens:
      else if (Tokenizer.IsTokenAt(1, "Width") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"Width\" keyword is no longer supported, since it contained redundant information"<<endl;
        mout<<endl;
        FoundDeprecated = true;
        //         if (D->GetType() != MDDetector::c_Strip2D &&
        //             D->GetType() != MDDetector::c_DriftChamber &&
        //             D->GetType() != MDDetector::c_Strip3D) {
        //           Typo("Option Width only supported for StripxD & DriftChamber");
        //           return false;
        //         }
        //         if (Tokenizer.GetNTokens() != 4) {
        //           Typo("Line must contain one string and 2 doubles,"
        //                " e.g. \"Wafer.Width 6.3, 6.3\"");
        //           return false;
        //         }
        //         dynamic_cast<MDStrip2D*>(D)->SetWidth(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3));
      } else if (Tokenizer.IsTokenAt(1, "Pitch") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"Pitch\" keyword is no longer supported, since it contained redundant information"<<endl;
        mout<<endl;
        FoundDeprecated = true;
        //         if (D->GetType() != MDDetector::c_Strip2D &&
        //             D->GetType() != MDDetector::c_Strip3D &&
        //             D->GetType() != MDDetector::c_DriftChamber) {
        //           Typo("Option Pitch only supported for StripxD & DriftChamber");
        //           return false;
        //         }

        //         if (Tokenizer.GetNTokens() != 4) {
        //           Typo("Line must contain one string and 2 doubles,"
        //                " e.g. \"Wafer.Pitch 0.047 0.047\"");
        //           return false;
        //         }

        //         dynamic_cast<MDStrip2D*>(D)->SetPitch(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3));
      } else if (Tokenizer.IsTokenAt(1, "Offset") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("Offset cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        if (D->GetType() != MDDetector::c_Strip2D &&
            D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_Voxel3D &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option Offset only supported for StripxD & DriftChamber");
          return false;
        }

        if (D->GetType() == MDDetector::c_Voxel3D) {
          if (Tokenizer.GetNTokens() != 5) {
            Typo("Line must contain one string and 3 doubles,"
                 " e.g. \"Voxler.Offset 0.2 0.2 0.2\"");
            return false;
          }

          dynamic_cast<MDVoxel3D*>(D)->SetOffset(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3), Tokenizer.GetTokenAtAsDouble(4));
        } else {
          if (Tokenizer.GetNTokens() != 4) {
            Typo("Line must contain one string and 2 doubles,"
                 " e.g. \"Wafer.Offset 0.2 0.2\"");
            return false;
          }

          dynamic_cast<MDStrip2D*>(D)->SetOffset(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3));
        }
      } else if (Tokenizer.IsTokenAt(1, "StripNumber") == true ||
                 Tokenizer.IsTokenAt(1, "Strip") == true ||
                 Tokenizer.IsTokenAt(1, "Strips") == true) {
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("StripNumber cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }
        if (D->GetType() != MDDetector::c_Strip2D &&
            D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option StripNumber only supported for StripxD & DriftChamber");
          return false;
        }

        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain one string and 2 doubles,"
               " e.g. \"Wafer.StripNumber 128 128\"");
          return false;
        }

        dynamic_cast<MDStrip2D*>(D)->SetNStrips(Tokenizer.GetTokenAtAsInt(2), Tokenizer.GetTokenAtAsInt(3));
      } else if (Tokenizer.IsTokenAt(1, "PixelNumber") == true ||
                 Tokenizer.IsTokenAt(1, "Pixels") == true ||
                 Tokenizer.IsTokenAt(1, "Pixel") == true ||
                 Tokenizer.IsTokenAt(1, "VoxelNumber") == true ||
                 Tokenizer.IsTokenAt(1, "Voxels") == true ||
                 Tokenizer.IsTokenAt(1, "Voxel") == true) {
        if (D->GetType() != MDDetector::c_Voxel3D) {
          Typo("Option Strip/Voxel number only supported for Voxel3D");
          return false;
        }
        // Check and reject named detector
        if (D->IsNamedDetector() == true) {
          Typo("Pixels/voxels cannot be used with a named detector! It's inherited from its template detector.");
          return false;
        }

        if (Tokenizer.GetNTokens() != 5) {
          Typo("Line must contain one string and 3 doubles,"
               " e.g. \"Voxler.StripNumber 128 128 128\"");
          return false;
        }

        dynamic_cast<MDVoxel3D*>(D)->SetNVoxels(Tokenizer.GetTokenAtAsInt(2), Tokenizer.GetTokenAtAsInt(3), Tokenizer.GetTokenAtAsInt(4));
      } else if (Tokenizer.IsTokenAt(1, "StripLength") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"StripLength\" keyword is no longer supported, since it contained redundant information"<<endl;
        mout<<endl;
        FoundDeprecated = true;
        //         if (D->GetType() != MDDetector::c_Strip2D &&
        //             D->GetType() != MDDetector::c_Strip3D &&
        //             D->GetType() != MDDetector::c_DriftChamber) {
        //           Typo("Option StripLength only supported for StripxD & DriftChamber");
        //           return false;
        //         }

        //         if (Tokenizer.GetNTokens() != 4) {
        //           Typo("Line must contain one string and 2 doubles,"
        //                " e.g. \"Wafer.StripLength 6.016 6.016\"");
        //           return false;
        //         }

        //         dynamic_cast<MDStrip2D*>(D)->SetStripLength(Tokenizer.GetTokenAtAsDouble(2), Tokenizer.GetTokenAtAsDouble(3));
      } else if (Tokenizer.IsTokenAt(1, "Orientation") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"The \"Orientation\" keyword is no longer supported, since it contained redundant information"<<endl;
        mout<<endl;
        FoundDeprecated = true;
        //         if (D->GetType() != MDDetector::c_Strip2D &&
        //             D->GetType() != MDDetector::c_Strip3D &&
        //             D->GetType() != MDDetector::c_DriftChamber) {
        //           Typo("Option StripLength only supported for StripxD & DriftChamber");
        //           return false;
        //         }

        //         if (Tokenizer.GetNTokens() != 3) {
        //           Typo("Line must contain one string and one integer,"
        //                " e.g. \"Wafer.Orientation 2\"");
        //           return false;
        //         }

        //         dynamic_cast<MDStrip2D*>(D)->SetOrientation(Tokenizer.GetTokenAtAsInt(2));
      }
      // Check for guard ring threshold
      else if (Tokenizer.IsTokenAt(1, "GuardRingTriggerThreshold") == true ||
               Tokenizer.IsTokenAt(1, "GuardringTriggerThreshold") == true) {
        if (D->GetType() != MDDetector::c_Strip2D &&
            D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Voxel3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option GuardRingTriggerThreshold only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() < 3 || Tokenizer.GetNTokens() > 4) {
          Typo("Line must contain two strings and 2 double,"
               " e.g. \"Wafer.GuardRingTriggerThreshold 30 10\"");
          return false;
        }
        if (D->GetType() == MDDetector::c_Voxel3D) {
          dynamic_cast<MDVoxel3D*>(D)->HasGuardRing(true);
          D->GetGuardRing()->SetActive(true);
          D->GetGuardRing()->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
          D->GetGuardRing()->SetNoiseThresholdEqualsTriggerThreshold(true);
          D->GetGuardRing()->SetTriggerThreshold(Tokenizer.GetTokenAtAsDouble(2));
          if (Tokenizer.GetNTokens() == 4) {
            D->GetGuardRing()->SetTriggerThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
          }
        } else {
          dynamic_cast<MDStrip2D*>(D)->HasGuardRing(true);
          D->GetGuardRing()->SetActive(true);
          D->GetGuardRing()->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
          D->GetGuardRing()->SetNoiseThresholdEqualsTriggerThreshold(true);
          D->GetGuardRing()->SetTriggerThreshold(Tokenizer.GetTokenAtAsDouble(2));
          if (Tokenizer.GetNTokens() == 4) {
            D->GetGuardRing()->SetTriggerThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
          }
        }
      }
      // Check for guard ring energy resolution
      else if (Tokenizer.IsTokenAt(1, "GuardRingEnergyResolution") == true ||
               Tokenizer.IsTokenAt(1, "GuardRingEnergyResolutionAt") == true ||
               Tokenizer.IsTokenAt(1, "GuardringEnergyResolution") == true ||
               Tokenizer.IsTokenAt(1, "GuardringEnergyResolutionAt") == true) {
        if (D->GetType() != MDDetector::c_Strip2D &&
            D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Voxel3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option GuardRingEnergyResolution only supported for StripxD, DriftChamber, Voxel3D");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.GuardRingEnergyResolution 30 10\"");
          return false;
        }
        if (D->GetType() == MDDetector::c_Voxel3D) {
          if (D->HasGuardRing() == false) {
            dynamic_cast<MDVoxel3D*>(D)->HasGuardRing(true);
          }
          D->GetGuardRing()->SetActive(true);
          D->GetGuardRing()->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
          D->GetGuardRing()->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                                 Tokenizer.GetTokenAtAsDouble(2),
                                                 Tokenizer.GetTokenAtAsDouble(3));
        } else {
          if (D->HasGuardRing() == false) {
            dynamic_cast<MDStrip2D*>(D)->HasGuardRing(true);
          }
          D->GetGuardRing()->SetActive(true);
          D->GetGuardRing()->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
          D->GetGuardRing()->SetEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                                 Tokenizer.GetTokenAtAsDouble(2),
                                                 Tokenizer.GetTokenAtAsDouble(3));
        }
      }
      // Check for Calorimeter specific tokens:
      else if (Tokenizer.IsTokenAt(1, "NoiseAxis") == true) {
        mout<<" *** Unsupported *** "<<endl;
        mout<<"The \"NoiseAxis\" keyword is no longer supported!"<<endl;
        mout<<"For all detectors, the z-axis is by default the depth-noised axis"<<endl;
        mout<<"For the depth resolution, use the \"DepthResolution\" keyword"<<endl;
        mout<<endl;
        FoundDeprecated = true;
      }
      else if (Tokenizer.IsTokenAt(1, "DepthResolution") == true ||
               Tokenizer.IsTokenAt(1, "DepthResolutionAt") == true) {
        bool Return = true;
        if (D->GetType() == MDDetector::c_Calorimeter) {
          if (Tokenizer.GetNTokens() == 4) {
            Return = dynamic_cast<MDCalorimeter*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                                                           Tokenizer.GetTokenAtAsDouble(3),
                                                                           0);
          } else if (Tokenizer.GetNTokens() == 5) {
            Return = dynamic_cast<MDCalorimeter*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                                                           Tokenizer.GetTokenAtAsDouble(3),
                                                                           Tokenizer.GetTokenAtAsDouble(4));
          }
        } else if (D->GetType() == MDDetector::c_Strip3D ||
                   D->GetType() == MDDetector::c_Strip3DDirectional ||
                   D->GetType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() == 4) {
            Return = dynamic_cast<MDStrip3D*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                                                       Tokenizer.GetTokenAtAsDouble(3),
                                                                       0);
          } else if (Tokenizer.GetNTokens() == 5) {
            Return = dynamic_cast<MDStrip3D*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                                                       Tokenizer.GetTokenAtAsDouble(3),
                                                                       Tokenizer.GetTokenAtAsDouble(4));
          }
        } else {
          Typo("Option DepthResolution only supported for Calorimeter, Strip3D, Strip3DDirectional, DriftChamber");
          return false;
        }
        if (Return == false) {
          Typo("Incorrect input");
          return false;
        }
      }
      else if (Tokenizer.IsTokenAt(1, "DepthResolutionThreshold") == true) {
        if (D->GetType() == MDDetector::c_Strip3D ||
            D->GetType() == MDDetector::c_Strip3DDirectional ||
            D->GetType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 doubles,"
                 " e.g. \"Wafer.DepthResolutionThreshold 25.0\"");
            return false;
          }
          dynamic_cast<MDStrip3D*>(D)->SetDepthResolutionThreshold(Tokenizer.GetTokenAtAsDouble(2));
        } else {
          Typo("Option DepthResolutionThreshold only supported for Strip3D, Strip3DDirectional, DriftChamber");
          return false;
        }
      }
      // Check for Scintillator specific tokens:
      else if (Tokenizer.IsTokenAt(1, "HitPosition") == true) {
        mout<<" *** Obsolete *** "<<endl;
        mout<<"The \"HitPosition\" keyword is no longer necessary in the current version and thus not used - please delete it!"<<endl;
      }
      // Check for Strip3D and higher specific tokens:
      else if (Tokenizer.IsTokenAt(1, "EnergyResolutionDepthCorrection") == true ||
               Tokenizer.IsTokenAt(1, "EnergyResolutionDepthCorrectionAt") == true) {
        if (D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option EnergyResolutionDepthCorrectionAt only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() < 3) {
          Typo("EnergyResolution keyword not correct.");
          return false;
        }
        MString Type = Tokenizer.GetTokenAt(2);
        char* Tester;
        double d = strtod(Type.Data(), &Tester); if (d != 0) d = 0; // Just to prevent the compiler from complaining
        if (Tester != Type.Data()) {
          // We have a number - do it the old way
          if (Tokenizer.GetNTokens() != 4) {
            Typo("Line must contain one string and 2,"
                 " e.g. \"Wafer.EnergyResolutionDeothCorrection 0.0 1.0\"");
            return false;
          }
          if (dynamic_cast<MDStrip3D*>(D)->SetEnergyResolutionDepthCorrection(Tokenizer.GetTokenAtAsDouble(2),
                                                                              1.0,
                                                                              Tokenizer.GetTokenAtAsDouble(3)) == false) {
            Typo("Incorrect input");
            return false;
          }
        } else {
          // New way:
          Type.ToLower();
          if (Type == "gauss" || Type == "gaus") {
            if (Tokenizer.GetNTokens() != 6 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolutionDepthCorrection Gauss 1.0 122 2.0\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeGauss &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGauss);
            dynamic_cast<MDStrip3D*>(D)->SetEnergyResolutionDepthCorrection(Tokenizer.GetTokenAtAsDouble(3),
                                                                            Tokenizer.GetTokenAtAsDouble(4),
                                                                            Tokenizer.GetTokenAtAsDouble(5));

          } else if (Type == "lorentz" || Type == "lorenz") {
            if (Tokenizer.GetNTokens() != 6 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolution Lorentz 122 122 2.0\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeLorentz &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeLorentz);
            dynamic_cast<MDStrip3D*>(D)->SetEnergyResolutionDepthCorrection(Tokenizer.GetTokenAtAsDouble(3),
                                                                            Tokenizer.GetTokenAtAsDouble(4),
                                                                            Tokenizer.GetTokenAtAsDouble(5));

          } else if (Type == "gauslandau" || Type == "gausslandau" || Type == "gauss-landau") {
            if (Tokenizer.GetNTokens() != 9 ) {
              Typo("EnergyResolution keyword not correct. Example:"
                   "\"Wafer.EnergyResolution GaussLandau 122 122 2.0 122 3.0 0.2\"");
              return false;
            }
            if (D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeGaussLandau &&
                D->GetEnergyResolutionType() != MDDetector::c_EnergyResolutionTypeUnknown) {
              Typo("The energy resolution type cannot change!");
              return false;
            }
            D->SetEnergyResolutionType(MDDetector::c_EnergyResolutionTypeGaussLandau);
            dynamic_cast<MDStrip3D*>(D)->SetEnergyResolutionDepthCorrection(Tokenizer.GetTokenAtAsDouble(3),
                                                                            Tokenizer.GetTokenAtAsDouble(4),
                                                                            Tokenizer.GetTokenAtAsDouble(5),
                                                                            Tokenizer.GetTokenAtAsDouble(6),
                                                                            Tokenizer.GetTokenAtAsDouble(7),
                                                                            Tokenizer.GetTokenAtAsDouble(8));

          } else {
            Typo("Unknown EnergyResolutionDepthCorrection parameters.");
            return false;
          }
        }
      }
      else if (Tokenizer.IsTokenAt(1, "TriggerThresholdDepthCorrection") == true ||
               Tokenizer.IsTokenAt(1, "TriggerThresholdDepthCorrectionAt") == true) {
        if (D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option TriggerThresholdDepthCorrectionAt only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.TriggerThresholdDepthCorrection 30 10\"");
          return false;
        }
        if (dynamic_cast<MDStrip3D*>(D)->SetTriggerThresholdDepthCorrection(Tokenizer.GetTokenAtAsDouble(2),
                                                                            Tokenizer.GetTokenAtAsDouble(3)) == false) {
          Typo("Incorrect input");
          return false;
        }
      }
      else if (Tokenizer.IsTokenAt(1, "NoiseThresholdDepthCorrection") == true ||
               Tokenizer.IsTokenAt(1, "NoiseThresholdDepthCorrectionAt") == true) {
        if (D->GetType() != MDDetector::c_Strip3D &&
            D->GetType() != MDDetector::c_Strip3DDirectional &&
            D->GetType() != MDDetector::c_DriftChamber) {
          Typo("Option NoiseThresholdDepthCorrection only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.NoiseThresholdDepthCorrection 30 10\"");
          return false;
        }
        if (dynamic_cast<MDStrip3D*>(D)->SetNoiseThresholdDepthCorrection(Tokenizer.GetTokenAtAsDouble(2),
                                                                          Tokenizer.GetTokenAtAsDouble(3)) == false) {
          Typo("Incorrect input");
          return false;
        }
      }
      // Check for Strip3DDirectional specific tokens:
      else if (Tokenizer.IsTokenAt(1, "DirectionalResolution") == true ||
               Tokenizer.IsTokenAt(1, "DirectionalResolutionAt") == true) {
        if (D->GetType() == MDDetector::c_Strip3DDirectional) {
          if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
            Typo("Line must contain two strings and 2 doubles,"
                 " e.g. \"Wafer.DirectionResolutionAt 662 39\"");
            return false;
          }
          if (Tokenizer.GetNTokens() == 4) {
            dynamic_cast<MDStrip3DDirectional*>(D)
              ->SetDirectionalResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                           Tokenizer.GetTokenAtAsDouble(3)*c_Rad,
                                           1E-6);
          } else if (Tokenizer.GetNTokens() == 5) {
            dynamic_cast<MDStrip3DDirectional*>(D)
              ->SetDirectionalResolutionAt(Tokenizer.GetTokenAtAsDouble(2),
                                           Tokenizer.GetTokenAtAsDouble(3)*c_Rad,
                                           Tokenizer.GetTokenAtAsDouble(4));
          }
        } else {
          Typo("Option DirectionResolution only supported for Strip3DDirectional");
          return false;
        }
      }
      // Check for Anger camera specific tokens
      else if (Tokenizer.IsTokenAt(1, "PositionResolution") == true ||
               Tokenizer.IsTokenAt(1, "PositionResolutionAt") == true) {
        if (D->GetType() != MDDetector::c_AngerCamera) {
          Typo("Option PositionResolution only supported for AngerCamera");
          return false;
        }
        if (Tokenizer.GetNTokens() == 4) {
          dynamic_cast<MDAngerCamera*>(D)->SetPositionResolution(Tokenizer.GetTokenAtAsDouble(2),
                                                                 Tokenizer.GetTokenAtAsDouble(3));
        } else if (Tokenizer.GetNTokens() == 6) {
          dynamic_cast<MDAngerCamera*>(D)->SetPositionResolutionXYZ(Tokenizer.GetTokenAtAsDouble(2),
                                                                    Tokenizer.GetTokenAtAsDouble(3),
                                                                    Tokenizer.GetTokenAtAsDouble(4),
                                                                    Tokenizer.GetTokenAtAsDouble(5));
        } else {
          Typo("Line must contain either two strings and 2 doubles (for XY, XYZ positioning),"
               " e.g. \"Anger.PositionResolutionAt 30 10\""
               " or two strings and 4 doubles (for XYZ independent Gaussians positioning),"
               " e.g. \"Anger.PositionResolutionAt 30 10 10 20\"");
          return false;
        }
      } else if (Tokenizer.IsTokenAt(1, "Positioning") == true) {
        if (D->GetType() == MDDetector::c_AngerCamera) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 double:"
                 " e.g. \"Anger.Positioning XYZ\"");
            return false;
          }
          if (Tokenizer.GetTokenAtAsString(2) == "XYZ") {
            dynamic_cast<MDAngerCamera*>(D)->SetPositioning(MDGridPoint::c_XYZAnger);
          } else if  (Tokenizer.GetTokenAtAsString(2) == "XYZIndependent") {
            dynamic_cast<MDAngerCamera*>(D)->SetPositioning(MDGridPoint::c_XYZIndependentAnger);
          } else if  (Tokenizer.GetTokenAtAsString(2) == "XY") {
            dynamic_cast<MDAngerCamera*>(D)->SetPositioning(MDGridPoint::c_XYAnger);
          } else {
            Typo("Unknown positioning type");
            return false;
          }
        } else {
          Typo("Option Positioning only supported for AngerCamera");
          return false;
        }
        // Check for DriftChamber specific tokens:
      } else if (Tokenizer.IsTokenAt(1, "LightSpeed") == true) {
        if (D->GetType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two string and 3 doubles:,"
                 " e.g. \"Chamber.LightSpeed 18E+9\"");
            return false;
          }
          dynamic_cast<MDDriftChamber*>(D)->SetLightSpeed(Tokenizer.GetTokenAtAsDouble(2));
        } else {
          Typo("Option LightSpeed only supported for DriftChamber");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "LightDetectorPosition") == true) {
        if (D->GetType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 double:"
                 " e.g. \"Chamber.LightDetectorPosition 3\"");
            return false;
          }
          dynamic_cast<MDDriftChamber*>(D)->SetLightDetectorPosition(Tokenizer.GetTokenAtAsInt(2));
        } else {
          Typo("Option LightDetectorPosition only supported for DriftChamber");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "DriftConstant") == true) {
        if (D->GetType() == MDDetector::c_DriftChamber || D->GetType() == MDDetector::c_Strip3D) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two string and 1 double:"
                 " e.g. \"Chamber.DriftConstant 3\"");
            return false;
          }
          dynamic_cast<MDStrip3D*>(D)->SetDriftConstant(Tokenizer.GetTokenAtAsDouble(2));
        } else {
          Typo("Option DriftConstant only supported for DriftChamber");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "EnergyPerElectron") == true) {
        if (D->GetType() == MDDetector::c_DriftChamber || D->GetType() == MDDetector::c_Strip3D) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 double:"
                 " e.g. \"Chamber.EnergyPerElectron 3\"");
            return false;
          }
          dynamic_cast<MDStrip3D*>(D)->SetEnergyPerElectron(Tokenizer.GetTokenAtAsDouble(2));
        } else {
          Typo("Option EnergyPerElectron only supported for DriftChamber");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "LightEnergyResolution") == true ||
                 Tokenizer.IsTokenAt(1, "LightEnergyResolutionAt") == true) {
        if (D->GetType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 4) {
            Typo("Line must contain two strings and 2 doubles,"
                 " e.g. \"Wafer.LightEnergyResolutionAt 662   39\"");
            return false;
          }
          dynamic_cast<MDDriftChamber*>(D)
            ->SetLightEnergyResolution(Tokenizer.GetTokenAtAsDouble(2),
                                       Tokenizer.GetTokenAtAsDouble(3));
        } else {
          Typo("Option LightEnergyResolution only supported for DriftChamber");
          return false;
        }

      } else if (Tokenizer.IsTokenAt(1, "Assign") == true) {
        // Handle this one after the volume tree is completed
      } else if (Tokenizer.IsTokenAt(1, "BlockTrigger") == true) {
        // Handle this one after validation of the detector
      } else if (Tokenizer.IsTokenAt(1, "NamedDetector") == true || Tokenizer.IsTokenAt(1, "Named")) {
        // Already handled
      } else {
        Typo("Unrecognized detector option");
        return false;
      }
    } // end detector



    // Now we have some unassigned token ...
    else{
      //Typo("Unrecognized option...");
      //return false;
    }
  } // end fourth loop


  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (analyzing all properties) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }

  if (m_WorldVolume == 0) {
    mout<<"   ***  Error  ***  No world volume"<<endl;
    mout<<"One volume needs to be the world volume!"<<endl;
    mout<<"It is charcterized by e.g. \"WorldVolume.Mother 0\" (0 = nil)"<<endl;
    mout<<"Stopping to scan geometry file!"<<endl;
    Reset();
    return false;
  }

  // The world volume is not allowed to have copies/clones
  if (m_WorldVolume->GetNClones() > 0) {
    mout<<"   ***  Error  ***  World volume"<<endl;
    mout<<"World volume is not allowed to have copies/clones!"<<endl;
    mout<<"Stopping to scan geometry file!"<<endl;
    Reset();
    return false;
  }

  if (m_WorldVolume->GetCloneTemplate() != 0) {
    mout<<"   ***  Error  ***  World volume"<<endl;
    mout<<"World volume is not allowed to be cloned/copied!"<<endl;
    mout<<"Stopping to scan geometry file!"<<endl;
    Reset();
    return false;
  }

  // Test if the surrounding sphere should be shown
  if (m_SurroundingSphereShow == true && m_LaunchedByGeomega == true) {
    MString MaterialName = "SurroundingSphereVolumeMaterial";
    MDMaterial* SuperDense = new MDMaterial(MaterialName);
    MDMaterialComponent* SuperDenseComponent = new MDMaterialComponent();
    SuperDenseComponent->SetByAtomicWeighting("Pb", 1);
    SuperDense->SetComponent(SuperDenseComponent);
    SuperDense->SetDensity(10000.0);

    MDShapeSPHE* Shape = new MDShapeSPHE("SurroundingSphereVolumeShape");
    Shape->Set(m_SurroundingSphereRadius-0.01, m_SurroundingSphereRadius+0.01);

    MDVolume* Sphere = new MDVolume("SurroundingSphereVolume");
    Sphere->SetMother(m_WorldVolume);
    Sphere->SetMaterial(SuperDense);
    Sphere->SetShape(Shape);
    Sphere->SetPosition(m_SurroundingSpherePosition);
    Sphere->SetVisibility(1);

    AddVolume(Sphere);
    AddMaterial(SuperDense);
    AddShape(Shape);
  }


  //
  if (ShowOnlySensitiveVolumes == true) {
    for (unsigned int i = 0; i < GetNVolumes(); i++) {
      if (GetVolumeAt(i)->IsSensitive() == true) {
        GetVolumeAt(i)->SetVisibility(1);
      } else {
        GetVolumeAt(i)->SetVisibility(0);
      }
    }
  }

  // Validate the orientations
  for (unsigned int s = 0; s < GetNOrientations(); ++s) {
    if (m_OrientationList[s]->Validate() == false) {
      Reset();
      return false;
    }
  }


  // Validate the shapes (attention: some shapes are valuated multiple times internally)...
  for (unsigned int s = 0; s < GetNShapes(); ++s) {
    if (m_ShapeList[s]->Validate() == false) {
      Reset();
      return false;
    }
  }


  // Set a possible default color for all volumes:
  if (m_DefaultColor >= 0) {
    for (unsigned int i = 0; i < GetNVolumes(); i++) {
      m_VolumeList[i]->SetColor(m_DefaultColor);
    }
  }

  // Fill the clones with life:
  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    if (m_VolumeList[i]->CopyDataToClones() == false) {
      return false;
    }
  }
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (m_MaterialList[i]->CopyDataToClones() == false) {
      return false;
    }
  }
  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (m_DetectorList[i]->CopyDataToNamedDetectors() == false) {
      return false;
    }
  }

  // Make sure the guard ring detectors show up in our list
  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (m_DetectorList[i]->HasGuardRing() == true && GetDetector(m_DetectorList[i]->GetGuardRing()->GetName()) == nullptr) {
      AddDetector(m_DetectorList[i]->GetGuardRing());
    }
  }


  if (m_ShowVolumes == false) {
    for (unsigned int i = 0; i < GetNVolumes(); i++) {
      if (m_VolumeList[i]->GetVisibility() <= 1) {
        m_VolumeList[i]->SetVisibility(0);
      }
    }
  }

  // Scale some volumes:
  map<MDVolume*, double>::iterator ScaleIter;
  for (ScaleIter = ScaledVolumes.begin();
       ScaleIter != ScaledVolumes.end(); ++ScaleIter) {
    if ((*ScaleIter).first->IsClone() == false) {
      (*ScaleIter).first->Scale((*ScaleIter).second);
    } else {
      mout<<"   ***  Error  ***  Scaling is not applicable to clones/copies"<<endl;
      Reset();
      return false;
    }
  }
  m_WorldVolume->ResetCloneTemplateFlags();

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (generating clones) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }

  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    if (m_VolumeList[i] != 0) {
      //cout<<(int) m_VolumeList[i]<<"!"<<m_VolumeList[i]->GetName()<<endl;
      //cout<<m_VolumeList[i]->ToString()<<endl;
    }
  }

  if (m_WorldVolume->Validate() == false) {
    return false;
  }

  // Virtualize non-detector volumes
  if (m_VirtualizeNonDetectorVolumes == true) {
    mout<<"   ***  Info  *** "<<endl;
    mout<<"Non-detector volumes are virtualized --- you cannot calculate absorptions!"<<endl;
    m_WorldVolume->VirtualizeNonDetectorVolumes();
  }

  vector<MDVolume*> NewVolumes;
  m_WorldVolume->RemoveVirtualVolumes(NewVolumes);
  for (MDVolume* V: NewVolumes) AddVolume(V);

  // mimp<<"Error if there are not positioned volumes --> otherwise GetRandomPosition() fails"<<endl;

  if (m_WorldVolume->Validate() == false) {
    return false;
  }

  if (VirtualizeNonDetectorVolumes == false) {
    if (m_WorldVolume->ValidateClonesHaveSameMotherVolume() == false) {
      return false;
    }
  }

  // A final loop over the data checks for the detector keyword "Assign"
  // We need a final volume tree, thus this is really the final loop
  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(false); // Do maths just when we do assign
    
    if (Tokenizer.GetNTokens() < 2) continue;

    // Check for detectors:
    if ((D = GetDetector(Tokenizer.GetTokenAt(0))) != 0) {
      // Check for global tokens

      // Check for simulation in voxels instead of a junk volume
      if (Tokenizer.IsTokenAt(1, "Assign") == true) {
        if (D->IsNamedDetector() == false) {
          Typo("The Assign keyword can only be used with named detectors");
          return false;
        }
        MVector Pos;
        if (Tokenizer.GetNTokens() == 3) {
          vector<MString> VolumeNames = Tokenizer.GetTokenAtAsString(2).Tokenize(".");
          if (VolumeNames.size() == 0) {
            Typo("The volume sequence is empty!");
            return false;
          }
          if (m_WorldVolume->GetName() != VolumeNames[0]) {
            Typo("The volume sequence must start with the world volume!");
            return false;
          }
          MDVolumeSequence Seq;
          MDVolume* Start = m_WorldVolume;
          Seq.AddVolume(Start);
          for (unsigned int i = 1; i < VolumeNames.size(); ++i) {
            bool Found = false;
            for (unsigned int v = 0; v < Start->GetNDaughters(); ++v) {
              //cout<<"Looking for "<<VolumeNames[i]<<" in "<<Start->GetDaughterAt(v)->GetName()<<endl;
              if (Start->GetDaughterAt(v)->GetName() == VolumeNames[i]) {
                Found = true;
                Start = Start->GetDaughterAt(v);
                Seq.AddVolume(Start);
                //cout<<"Assign: Found: "<<VolumeNames[i]<<endl;
                break;
              }
            }
            if (Found == false) {
              Typo("Cannot find all volumes in the volume sequence! Make sure you placed the right volumes!");
              return false;
            }
          }
          if (Start->GetDetector() == 0) {
            Typo("The volume sequence does not point to a detector!");
            return false;
          }
          if (Start->GetDetector() != D->GetNamedAfterDetector()) {
            Typo("The volume sequence does not point to the right detector!");
            return false;
          }
          if (Start->IsSensitive() == 0) {
            Typo("The volume sequence does not point to a sensitive volume!");
            return false;
          }
          Pos = Start->GetShape()->GetRandomPositionInside();
          Pos = Seq.GetPositionInFirstVolume(Pos, Start);
        }
        else if (Tokenizer.GetNTokens() == 5) {
          MTokenizer& TokenizerMaths = (*ContentIter).GetTokenizer(true); // Do maths just when we do reading
          Pos[0] = TokenizerMaths.GetTokenAtAsDouble(2);
          Pos[1] = TokenizerMaths.GetTokenAtAsDouble(3);
          Pos[2] = TokenizerMaths.GetTokenAtAsDouble(4);
        }
        else {
          Typo("Line must contain two strings and one volume sequence (\"NamedWafer.Assign WorldVolume.Tracker.Wafer1\")"
               " or two strings and three numbers as absolute position (\"NamedWafer.Assign 12.0 0.0 0.0\")");
          return false;
        }

        MDVolumeSequence* VS = new MDVolumeSequence();
        m_WorldVolume->GetVolumeSequence(Pos, VS);
        if (VS->GetDetector() == nullptr) {
          MString VSout = VS->ToString();
          Typo("The position is not within a detector");
          cout<<VSout<<endl;
          return false;
        }
        D->SetVolumeSequence(*VS);
        if (D->HasGuardRing() == true) {
          D->GetGuardRing()->SetVolumeSequence(*VS);
        }
        delete VS;
      }
    }
  }

  // Take care of the start volume:
  if (m_StartVolume != "") {
    MDVolume* SV = 0;
    mout<<"Trying to set start volume as world volume ... ";
    if ((SV = GetVolume(m_StartVolume)) != 0) {
      if (SV->IsVirtual() == true) {
        mout<<"impossible, it's a virtual volume..."<<endl;
        mgui<<"Start volume cannot be shown, because it's a virtual volume. Showing whole geometry."<<error;
      } else if (SV->GetMother() == 0 && SV != m_WorldVolume) {
        mout<<"impossible, it's not a regular positioned volume, but a clone template..."<<endl;
        mgui<<"Start volume cannot be shown, because it's not a regular positioned volume, but a clone template. Showing whole geometry."<<error;
      } else {
        // Determine the correct rotation and position of this volume, to keep all positions correct:
        MVector Position(0, 0, 0);
        MRotation Rotation;

        MDVolume* Volume = SV;
        while (Volume->GetMother() != 0) {
          Position = Volume->GetInvRotationMatrix()*Position;
          Position += Volume->GetPosition();
          Rotation *= Volume->GetInvRotationMatrix();
          Volume = Volume->GetMother();
        }
        if (Volume != m_WorldVolume) {
          mout<<"impossible, it doesn't have a regular volume tree..."<<endl;
          mgui<<"Start volume cannot be shown, because it doesn't have a regular volume tree. Showing whole geometry."<<error;
        } else {
          m_WorldVolume->RemoveAllDaughters();
          SV->SetMother(m_WorldVolume);
          SV->SetPosition(Position);
          SV->SetRotation(Rotation);
          mout<<"done"<<endl;
        }
      }
    } else {
      mout<<"failed!"<<endl;
      mgui<<"Start volume not found in volume tree."<<error;
    }
  }

  // Take care of preferred visible volumes
  if (m_PreferredVisibleVolumeNames.size() > 0) {

    // Take care of preferred visible volumes - make everything not visible
    if (m_PreferredVisibleVolumeNames.size() > 0) {
      for (unsigned int i = 0; i < GetNVolumes(); i++) {
        m_VolumeList[i]->SetVisibility(0);
        for (unsigned int c = 0; c < GetVolumeAt(i)->GetNClones(); ++c) {
          GetVolumeAt(i)->GetCloneAt(c)->SetVisibility(0);
        }
      }
    }

    bool FoundOne = false;
    for (auto N: m_PreferredVisibleVolumeNames) {
      for (unsigned int i = 0; i < GetNVolumes(); i++) {
        if (GetVolumeAt(i)->GetName() == N) {
          GetVolumeAt(i)->SetVisibility(1);
          FoundOne = true;
        }
        for (unsigned int c = 0; c < GetVolumeAt(i)->GetNClones(); ++c) {
          if (GetVolumeAt(i)->GetCloneAt(c)->GetName() == N) {
            GetVolumeAt(i)->GetCloneAt(c)->SetVisibility(1);
            FoundOne = true;
          }
        }
      }
    }
    if (FoundOne == false) {
      mout<<"ERROR: None of your preferred visible volumes has been found!"<<endl;
    }
  }

  //
  // Validation routines for the detectors:
  //

  // Determine the common volume for all sensitive volumes of all detectors
  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (m_DetectorList[i]->GetNSensitiveVolumes() > 1) {

      // Check that the sensitive volumes are no copies
      for (unsigned int l = 0; l < m_DetectorList[i]->GetNSensitiveVolumes(); ++l) {
        //cout<<"clone test for "<<m_DetectorList[i]->GetSensitiveVolume(l)->GetName()<<endl;
        if (m_DetectorList[i]->GetSensitiveVolume(l)->IsClone() == true || m_DetectorList[i]->GetSensitiveVolume(l)->IsCloneTemplate() == true ) {
          Typo("If your detector has multiple sensitive volumes, then those cannot by copies or a template for copies.");
          return false;
        }
      }

      vector<vector<MDVolume*> > Volumes;
      for (unsigned int l = 0; l < m_DetectorList[i]->GetNSensitiveVolumes(); ++l) {
        vector<MDVolume*> MotherVolumes;
        MotherVolumes.push_back(m_DetectorList[i]->GetSensitiveVolume(l));
        //cout<<"Tree "<<l<<": "<<MotherVolumes.back()->GetName()<<endl;
        while (MotherVolumes.back() != 0 && MotherVolumes.back()->GetMother()) {
          MotherVolumes.push_back(MotherVolumes.back()->GetMother());
          //cout<<"Tree "<<l<<": "<<MotherVolumes.back()->GetName()<<endl;
        }
        Volumes.push_back(MotherVolumes);
      }

      // Replace volumes by mother volumes until we have a common mother, or reached the end of the volume tree
      // Loop of all mothers of the first volume --- those are the test volumes
      for (unsigned int m = 0; m < Volumes[0].size(); ++m) {
        MDVolume* Test = Volumes[0][m];
        if (Test == 0) break;
        //cout<<"Testing: "<<Test->GetName()<<endl;
        bool FoundTest = true;

        for (unsigned int l = 1; l < Volumes.size(); ++l) {
          bool Found = false;
          for (unsigned int m = 0; m < Volumes[l].size(); ++m) {
            //cout<<"Comparing to: "<<Volumes[l][m]->GetName()<<" of tree "<<Volumes[l][0]->GetName()<<endl;
            if (Test == Volumes[l][m]) {
              //cout<<"Found sub"<<endl;
              Found = true;
              break;
            }
          }
          if (Found == false) {
            FoundTest = false;
            break;
          }
        }
        if (FoundTest == true) {
          m_DetectorList[i]->SetCommonVolume(Test);
          mout<<"Common mother volume for sensitive detectors of "<<m_DetectorList[i]->GetName()<<": "<<m_DetectorList[i]->GetCommonVolume()->GetName()<<endl;
          break;
        }
      }

      if (m_DetectorList[i]->GetCommonVolume() == nullptr) {
        mout<<"   ***  Error  ***  Multiple sensitive volumes per detector restriction for "<<m_DetectorList[i]->GetName()<<endl;
        mout<<"If your detector has multiple sensitive volumes, those must have a common volume and there are no copies allowed starting with the sensitive volume up to the common volume."<<endl;
        mout<<"Stopping to scan geometry file!"<<endl;
        Reset();
        return false;
      }
    }
    // The common volume is automatically set to the detector volume in MDetector::Validate(), if there is only one sensitive volume

    // Make sure there is always only one sensitive volume of a certain type in the common volume
    // Due to the above checks it is enough to simply check the number of sensitive volumes in the common volume
    if (m_DetectorList[i]->GetNSensitiveVolumes() > 1 && m_DetectorList[i]->GetNSensitiveVolumes() != m_DetectorList[i]->GetCommonVolume()->GetNSensitiveVolumes()) {
      mout<<"   ***  Error  ***  Multiple sensitive volumes per detector restriction for "<<m_DetectorList[i]->GetName()<<endl;
      mout<<"If your detector has multiple sensitive volumes, those must have a common volume, in which exactly one of those volumes is positioned, and in addition no other sensitive volume. The latter is not the case."<<endl;
      mout<<"Stopping to scan geometry file!"<<endl;
      Reset();
      return false;
    }
  }

  bool IsValid = true;
  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (m_DetectorList[i]->Validate() == false) {
      IsValid = false;
    }
    m_NDetectorTypes[m_DetectorList[i]->GetType()]++;
  }

  // Special detector loop for blocked channels:

  for (auto ContentIter = FileContent.begin(); ContentIter != FileContent.end(); ++ContentIter) {
    m_DebugInfo = (*ContentIter);
    MTokenizer& Tokenizer = (*ContentIter).GetTokenizer(true);
    
    if (Tokenizer.GetNTokens() < 2) continue;

    // Check for detectors:
    if ((D = GetDetector(Tokenizer.GetTokenAt(0))) != 0) {
      // Check for simulation in voxels instead of a junk volume
      if (Tokenizer.IsTokenAt(1, "BlockTrigger") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 integerd,"
               " e.g. \"Wafer.BlockTrigger 0 0 \"");
          return false;
        }
        D->BlockTriggerChannel(Tokenizer.GetTokenAtAsInt(2),
                               Tokenizer.GetTokenAtAsInt(3));
      }
    }
  }


  // Trigger sanity checks:
  for (unsigned int i = 0; i < GetNTriggers(); i++) {
    if (m_TriggerList[i]->Validate() == false) {
      IsValid = false;
    }
  }
  if (m_TriggerUnit->Validate() == false) {
    IsValid = false;
  }


  // Material sanity checks
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    m_MaterialList[i]->SetDefaultCrossSectionFileDirectory(m_DefaultCrossSectionFileDirectory);
    m_MaterialList[i]->SetCrossSectionFileDirectory(m_CrossSectionFileDirectory);
    if (m_MaterialList[i]->Validate() == false) {
      IsValid = false;
    }
  }


  // Check if we can apply the keyword komplex ER
  // Does not cover all possibilities (e.g. rotated detector etc.)
  if (m_ComplexER == false) {
    int NTrackers = 0;
    for (unsigned int i = 0; i < GetNDetectors(); i++) {
      if (m_DetectorList[i]->GetType() == MDDetector::c_Strip2D) {
        if (dynamic_cast<MDStrip2D*>(m_DetectorList[i])->GetOrientation() != 2) {
          mout<<"   ***  Error  ***  ComplexER"<<endl;
          mout<<"This keyword can only be applied for tracker which are oriented in z-axis!"<<endl;
          IsValid = false;
        } else {
          NTrackers++;
        }
      }
    }
    if (NTrackers > 1) {
      mout<<"   ***  Error  ***  ComplexER"<<endl;
      mout<<"This keyword can only be applied if only one or none tracker is available!"<<endl;
      Reset();
      return false;
    }
  }


  // We need a trigger criteria
  if (GetNTriggers() == 0) {
    mout<<"   ***  Warning  ***  "<<endl;
    mout<<"You have not defined any trigger criteria!!"<<endl;
  } else {
    // Check if each detector has a trigger criterion:
    for (unsigned int i = 0; i < GetNDetectors(); ++i) {
      // Guard ring trigger criteria are optional...
      if (m_DetectorList[i]->GetType() == MDDetector::c_GuardRing) continue;

      bool Found = false;
      for (unsigned int t = 0; t < GetNTriggers(); ++t) {
        if (m_TriggerList[t]->Applies(m_DetectorList[i]) == true) {
          Found = true;
          break;
        }
        // If we have a named detectors, in case the "named after detector" has a trigger criteria, we are fine
        if (m_DetectorList[i]->IsNamedDetector() == true) {
          if (m_TriggerList[t]->Applies(m_DetectorList[i]->GetNamedAfterDetector()) == true) {
            Found = true;
            break;
          }
        }
        // If the detector has named detectors which have a trigger criteria, we are fine to
        if (m_DetectorList[i]->HasNamedDetectors() == true) {
          for (unsigned int n = 0; n < m_DetectorList[i]->GetNNamedDetectors(); ++n) {
            for (unsigned int t2 = 0; t2 < GetNTriggers(); ++t2) {
              if (m_TriggerList[t2]->Applies(GetDetector(m_DetectorList[i]->GetNamedDetectorName(n))) == true) {
                Found = true;
                break;
              }
            }
          }
        }
      }
      if (Found == false) {
        mout<<"   ***  Warning  ***  "<<endl;
        mout<<"You have not defined any trigger criterion for detector: "<<m_DetectorList[i]->GetName()<<endl;
      }
    }
  }

  if (IsValid == false) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"There were errors while scanning this file. Correct them first!!"<<endl;
    Reset();
    return false;
  }
  
  // Check if all cross sections are present if not try to create them
  bool CrossSectionsPresent = true;
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (m_MaterialList[i]->AreCrossSectionsPresent() == false) {
      CrossSectionsPresent = false;
      break;
    }
  }
  if (CrossSectionsPresent == false && AllowCrossSectionCreation == true) {
    if (CreateCrossSectionFiles() == false) {
      mout<<"   ***  Warning  ***  "<<endl;
      mout<<"Not all cross section files are present!"<<endl;
    }
  }
  
  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (validation & post-processing) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }

  // Geant4 requires that the world volume is the first volume in the list
  // Thus resort the list
  m_VolumeList.erase(find(m_VolumeList.begin(), m_VolumeList.end(), m_WorldVolume));
  m_VolumeList.insert(m_VolumeList.begin(), m_WorldVolume);

  // The last stage is to optimize the geometry for hit searches:
  m_WorldVolume->OptimizeVolumeTree();

  m_GeometryScanned = true;

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (volume tree optimization) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
 }

  if (g_Verbosity >= c_Info) {
    mout<<"Geometry "<<m_FileName<<" successfully scanned within "<<Timer.ElapsedTime()<<"s"<<endl;
    mout<<"It contains "<<GetNVolumes()<<" volumes"<<endl;
  }

  if (FoundDeprecated == true) {
    mgui<<"Your geometry contains deprecated information (see console output for details)."<<endl;
    mgui<<"Please update it now to the latest conventions!"<<show;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::AddFile(MString FileName, list<MDDebugInfo>& FileContent)
{

  FileContent.clear();

  if (MFile::ExpandFileName(FileName) == false) {
    Typo("Unable to expand file name");
    return false;
  }

  // First edit the file name:
  if (gSystem->IsAbsoluteFileName(FileName) == false) {
    FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + FileName;
  }

  if (gSystem->AccessPathName(FileName) == 1) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"Included file \""<<FileName<<"\" does not exist."<<endl;
    return false;
  }

  if (IsIncluded(FileName) == true) {
    //mout<<"   ***  Warning  ***  "<<endl;
    //mout<<"The file has been included multiple times: "<<FileName<<endl;
    return true;
  }

  int LineCounter = 0;
  int LineLength = 10000;
  char* LineBuffer = new char[LineLength];

  ifstream FileStream;
  FileStream.open(FileName);

  if (FileStream.is_open() == 0) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"Can't open file "<<FileName<<endl;
    delete [] LineBuffer;
    return false;
  }

  int Comment = 0;
  MTokenizer Tokenizer;
  MDDebugInfo Info;
  while (FileStream.getline(LineBuffer, LineLength, '\n')) {
    Info = MDDebugInfo(LineBuffer, FileName, LineCounter++);
    Tokenizer.Analyse(Info.GetText(), false);
    if (Tokenizer.GetNTokens() >=1 && Tokenizer.GetTokenAt(0) == "Exit") {
      mout<<"Found \"Exit\" in file "<<FileName<<endl;
      break;
    }
    if (Tokenizer.GetNTokens() >= 1 && Tokenizer.GetTokenAt(0) == "EndComment") {
      //mout<<"Found \"EndComment\" in file "<<FileName<<endl;
      Comment--;
      if (Comment < 0) {
        mout<<"   ***  Error  ***  "<<endl;
        mout<<"Found \"EndComment\" without \"BeginComment\" in file "<<FileName<<endl;
        FileContent.clear();
        delete [] LineBuffer;
        return false;
      }
      continue;
    }
    if (Tokenizer.GetNTokens() >= 1 && Tokenizer.GetTokenAt(0) == "BeginComment") {
      //mout<<"Found \"BeginComment\" in file "<<FileName<<endl;
      Comment++;
      continue;
    }
    if (Comment == 0) {
      FileContent.push_back(Info);
    }
  }
  // Add an empty line, just in case the file didn't end with a new line
  FileContent.push_back(MDDebugInfo(" ", FileName, LineCounter++));

  AddInclude(FileName);

  delete [] LineBuffer;

  FileStream.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::ImportGDML(MString FileName, list<MDDebugInfo>& FileContent)
{
  
  FileContent.clear();
  
  if (MFile::ExpandFileName(FileName) == false) {
    Typo("Unable to expand file name");
    return false;
  }
  
  // First edit the file name:
  if (gSystem->IsAbsoluteFileName(FileName) == false) {
    FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + FileName;
  }
  
  if (gSystem->AccessPathName(FileName) == 1) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"Imported file \""<<FileName<<"\" does not exist."<<endl;
    return false;
  }
  
  if (IsIncluded(FileName) == true) {
    return true;
  }
  
  MDGDMLImport Importer;
  if (Importer.Parse(FileName) == true) {
    MString Text = Importer.GetAsGeomega();
    
  }
  
  
  /*
  int LineCounter = 0;
  int LineLength = 10000;
  char* LineBuffer = new char[LineLength];
  
  ifstream FileStream;
  FileStream.open(FileName);
  
  if (FileStream.is_open() == 0) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"Can't open file "<<FileName<<endl;
    delete [] LineBuffer;
    return false;
  }
  
  int Comment = 0;
  MTokenizer Tokenizer;
  MDDebugInfo Info;
  while (FileStream.getline(LineBuffer, LineLength, '\n')) {
    Info = MDDebugInfo(LineBuffer, FileName, LineCounter++);
    Tokenizer.Analyse(Info.GetText(), false);
    if (Tokenizer.GetNTokens() >=1 && Tokenizer.GetTokenAt(0) == "Exit") {
      mout<<"Found \"Exit\" in file "<<FileName<<endl;
      break;
    }
    if (Tokenizer.GetNTokens() >= 1 && Tokenizer.GetTokenAt(0) == "EndComment") {
      //mout<<"Found \"EndComment\" in file "<<FileName<<endl;
      Comment--;
      if (Comment < 0) {
        mout<<"   ***  Error  ***  "<<endl;
        mout<<"Found \"EndComment\" without \"BeginComment\" in file "<<FileName<<endl;
        FileContent.clear();
        delete [] LineBuffer;
        return false;
      }
      continue;
    }
    if (Tokenizer.GetNTokens() >= 1 && Tokenizer.GetTokenAt(0) == "BeginComment") {
      //mout<<"Found \"BeginComment\" in file "<<FileName<<endl;
      Comment++;
      continue;
    }
    if (Comment == 0) {
      FileContent.push_back(Info);
    }
  }
  // Add an empty line, just in case the file didn't end with a new line
  FileContent.push_back(MDDebugInfo(" ", FileName, LineCounter++));
  
  AddInclude(FileName);
  
  delete [] LineBuffer;
  
  FileStream.close();
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::Typo(MString Typo)
{
  // Print an error message

  mout<<"   ***  Error  ***  in setup file "<<m_DebugInfo.GetFileName()<<" at line "<<m_DebugInfo.GetLine()<<":"<<endl;
  mout<<"\""<<m_DebugInfo.GetText()<<"\""<<endl;
  mout<<Typo<<endl;
  mout<<"Stopping to scan geometry file!"<<endl;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::NameExists(MString Name)
{
  // Return true if the name Name already exists

  // Since all new names pass through this function (because we have to check
  // if it already exists), we can make sure that we are case insensitive!

  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    if (Name.AreIdentical(m_VolumeList[i]->GetName(), true)) {
      Typo("A volume of this name (case insensitive) already exists!");
      return true;
    }
  }

  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (Name.AreIdentical(m_MaterialList[i]->GetName(), true)) {
      Typo("A material of this name (case insensitive) already exists!");
      return true;
    }
  }

  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (Name.AreIdentical(m_DetectorList[i]->GetName(), true)) {
      Typo("A detector of this name (case insensitive) already exists!");
      return true;
    }
  }

  for (unsigned int i = 0; i < GetNTriggers(); i++) {
    if (Name.AreIdentical(m_TriggerList[i]->GetName(), true)) {
      Typo("A trigger of this name (case insensitive) already exists!");
      return true;
    }
  }

  for (unsigned int i = 0; i < GetNVectors(); i++) {
    if (Name.AreIdentical(m_VectorList[i]->GetName(), true)) {
      Typo("A vector of this name (case insensitive) already exists!");
      return true;
    }
  }

  for (unsigned int i = 0; i < m_ConstantList.size(); i++) {
    if (Name.AreIdentical(m_ConstantList[i], true)) {
      Typo("A constant of this name (case insensitive) already exists!");
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::DrawGeometry(TCanvas* Canvas, bool RestoreView, MString Mode)
{
  // The geometry must have been loaded previously
  // You cannot display 2 geometries at once!


  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned before it can be drawn!"<<endl;
    return false;
  }

  // Start by deleting the old windows:
  if (m_GeoView != nullptr) {
    if (gROOT->FindObject("MainCanvasGeomega") != nullptr) {
      delete m_GeoView;
    }
    m_GeoView = nullptr;
  } else {
    RestoreView = false;
  }

  //mdebug<<"NVolumes: "<<m_WorldVolume->GetNVisibleVolumes()<<endl;

  // Only draw the new windows if there are volumes to be drawn:
  if (m_WorldVolume->GetNVisibleVolumes() == 0) {
    mgui<<"There are no visible volumes in your geometry!"<<warn;
    return false;
  }

  MTimer Timer;
  double TimerLimit = 5;

  if (Canvas == nullptr) {
    if (RestoreView == true && m_ViewValid == true) {
      m_GeoView = new TCanvas("MainCanvasGeomega", "Geomega geometry display", m_ViewPositionX - m_ViewPositionShiftX, m_ViewPositionY - m_ViewPositionShiftY, m_ViewSizeX, m_ViewSizeY);
      //m_GeoView->SetWindowPosition(m_ViewPositionX, m_ViewPositionY);
      //m_GeoView->SetWindowSize(m_ViewSizeX, m_ViewSizeY);
    } else {
      int RefPos = 100;
      m_GeoView = new TCanvas("MainCanvasGeomega", "Geomega geometry display", RefPos, RefPos, 700, 700);
      m_ViewPositionShiftX = m_GeoView->GetWindowTopX() - RefPos;
      m_ViewPositionShiftY = m_GeoView->GetWindowTopY() - RefPos;
    }
  } else {
    Canvas->cd();
    if (RestoreView == true && m_ViewValid == true) {
      Canvas->SetWindowPosition(m_ViewPositionX, m_ViewPositionY);
      Canvas->SetWindowSize(m_ViewSizeX, m_ViewSizeY);
    }
  }


  m_WorldVolume->CreateRootGeometry(m_Geometry, 0);
  //m_Geometry->CloseGeometry(); // we do not close the geometry,
  //m_Geometry->SetMultiThread(true); // This crashes it...
  m_Geometry->SetVisLevel(1000);
  m_Geometry->SetNsegments(40);
  m_Geometry->SetVisDensity(-1.0);
  //m_Geometry->Voxelize("ALL");

  // Make sure we use the correct geometry for interactions
  gGeoManager = m_Geometry;
  if (Mode.ToLower() == "raytrace") {
    if (m_Geometry->GetTopVolume() != 0) m_Geometry->GetTopVolume()->Raytrace();
  } else {
    if (m_Geometry->GetTopVolume() != 0) m_Geometry->GetTopVolume()->Draw(Mode);
  }

  if (m_Geometry->GetListOfNavigators() == nullptr) {
    m_Geometry->AddNavigator();
  }
  m_Geometry->SetCurrentNavigator(0); // current is the first navigator?

  if (RestoreView == true && m_ViewValid == true) {
    TView* View = m_GeoView->GetView();
    View->SetRange(&m_ViewRangeMin[0], &m_ViewRangeMax[0]);
    
    int Reply = 0;
    View->SetView(m_ViewRotationTheta, m_ViewRotationPhi, m_ViewRotationPsi, Reply);
    
    View->SetDview(m_ViewDistanceCOPtoCOV);
    View->SetDproj(m_ViewDistanceCOPtoPL);
    
    if (m_ViewPerspective == true) {
      View->SetPerspective();
    } else {
      View->SetParallel();
    }
    View->AdjustScales();
  }

  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimerLimit) {
    mout<<"Geometry drawn within "<<Timer.ElapsedTime()<<" seconds."<<endl;
  }

  return true;
}
 

////////////////////////////////////////////////////////////////////////////////


void MDGeometry::StoreViewParameters()
{ 
  //! Store the view parameters (zoom, rotation, etc.)
 
  // Assume we do not have valid parameters if we cannot store everything
  m_ViewValid = false;
 
  if (m_GeoView == nullptr) {
    return;
  }
 
  TView* View = m_GeoView->GetView();
  if (View == nullptr) {
    return;
  }  
  
  m_GeoView->GetCanvasPar(m_ViewPositionX, m_ViewPositionY, m_ViewSizeX, m_ViewSizeY);
  
  View->GetRange(&m_ViewRangeMin[0], &m_ViewRangeMax[0]);
  
  m_ViewRotationPhi = View->GetLatitude();
  m_ViewRotationTheta = View->GetLongitude();
  m_ViewRotationPsi = View->GetPsi();
  
  m_ViewDistanceCOPtoCOV = View->GetDview();
  m_ViewDistanceCOPtoPL = View->GetDproj();
  
  m_ViewPerspective = View->IsPerspective();
  m_ViewValid = true;
}
 

////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::AreCrossSectionsPresent()
{
  // Check if all absorption files are present:

  for (unsigned int i = 0; i < m_MaterialList.size(); ++i) {
    if (m_MaterialList[i]->AreCrossSectionsPresent() == false) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::CheckOverlaps()
{
  // Check for overlaps using the ROOT overlap checker

  ostringstream Diagnostics;
  bool Return = CheckOverlaps(Diagnostics);
  mout<<Diagnostics.str();

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::CheckOverlaps(ostringstream& Diagnostics)
{
    // Check for overlaps using the ROOT overlap checker

  if (IsScanned() == false) {
    Error("bool MDGeometry::TestIntersections()",
          "You have to scan the geometry file first!");
    return false;
  }

  m_WorldVolume->CreateRootGeometry(m_Geometry, 0);
  m_Geometry->CloseGeometry();
  m_Geometry->CheckOverlaps(0.000001);

  TObjArray* Overlaps = m_Geometry->GetListOfOverlaps();
  if (Overlaps->GetEntries() > 0) {
    Diagnostics<<"List of extrusions and overlaps: "<<endl;
    for (int i = 0; i < Overlaps->GetEntries(); ++i) {
      TGeoOverlap* O = (TGeoOverlap*) (Overlaps->At(i));
      if (O->IsOverlap() == true) {
        Diagnostics<<"Overlap: "<<O->GetFirstVolume()->GetName()<<" with "<<O->GetSecondVolume()->GetName()<<" by "<<O->GetOverlap()<<" cm"<<endl;
      }
      if (O->IsExtrusion() == true) {
        Diagnostics<<"Extrusion: "<<O->GetSecondVolume()->GetName()<<" extrudes "<<O->GetFirstVolume()->GetName()<<" by "<<O->GetOverlap()<<" cm"<<endl;
      }
    }
  } else {
    Diagnostics<<"No extrusions or overlaps detected with ROOT (ROOT claims to be able to detect 95% of them)"<<endl;
  }

  return Overlaps->GetEntries() > 0 ? false : true;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::DumpInformation()
{
  // Dump the geometry information:

  if (IsScanned() == false) {
    Error("bool MDGeometry::DumpInformation()",
          "You have to scan the geometry file first!");
    return;
  }

  cout<<ToString()<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::CalculateMasses()
{
  // Calculate the masses of the geometry

  if (IsScanned() == false) {
    Error("bool MDGeometry::DumpInformation()",
          "You have to scan the geometry file first!");
    return;
  }

  double Total = 0;
  map<MDMaterial*, double> Masses;
  map<MDMaterial*, double>::iterator MassesIter;

  m_WorldVolume->GetMasses(Masses);

  size_t NameWidth = 0;
  for (MassesIter = (Masses.begin());
       MassesIter != Masses.end(); MassesIter++) {
    if ((*MassesIter).first->GetName().Length() > NameWidth) {
      NameWidth = (*MassesIter).first->GetName().Length();
    }
  }

  map<MString, double> MassByName;
  for (MassesIter = Masses.begin(); MassesIter != Masses.end(); ++MassesIter) {
    MassByName[(*MassesIter).first->GetName()] = (*MassesIter).second;
  }

  ostringstream out;
  out.setf(ios_base::fixed, ios_base::floatfield);
  out.precision(3);
  out<<endl;
  out<<"Mass summary by material: "<<endl;
  out<<endl;
  //for (MassesIter = (Masses.begin());
  //     MassesIter != Masses.end(); MassesIter++) {
  //  out<<setw(NameWidth+2)<<(*MassesIter).first->GetName()<<" :  "<<setw(12)<<(*MassesIter).second<<" g"<<endl;
  //  Total += (*MassesIter).second;
  //}
  for (auto I = MassByName.begin(); I != MassByName.end(); ++I) {
    out<<setw(NameWidth+2)<<(*I).first<<" :  "<<setw(12)<<(*I).second<<" g"<<endl;
    Total += (*I).second;
  }
  out<<endl;
  out<<setw(NameWidth+2)<<"Total"<<" :  "<<setw(12)<<Total<<" g"<<endl;
  out<<endl;
  out<<"No warranty for this information!"<<endl;
  out<<"This information is only valid, if "<<endl;
  out<<"(a) No volume intersects any volume which is not either its mother or daughter."<<endl;
  out<<"(b) All daughters lie completely inside their mothers"<<endl;
  out<<"(c) The material information is correct"<<endl;
  out<<"(d) tbd."<<endl;

  mout<<out.str()<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MString MDGeometry::WFS(MString Text)
{
  // Real name: WrapFortranStrings
  // A line in a FORTRAN77 file is not allowed to be larger than 72 characters
  // This functions cuts the lines appropriately:

  size_t CutLength = 72;

  if (Text.Length() <= CutLength) return Text;

  MString Cut;
  MString PreCut;
  MString Beyond;
  MString Formated;

  while (Text.Length() > 0) {
    int NextRet = Text.First('\n');
    if (NextRet == -1) {
      NextRet = Text.Length();
    } else {
      NextRet += 1;
    }
    Cut = Text.GetSubString(0, NextRet);
    Text.Remove(0, NextRet);
    if (Cut.Length() <= CutLength || Cut.BeginsWith("*") == true) {
      Formated += Cut;
    } else {
      Beyond = Cut.GetSubString(CutLength+1 , Cut.Length());
      bool BeyondHasText = false;
      for (size_t c = 0; c < Beyond.Length(); ++c) {
        char t = Beyond[c];
        if (t != ' ' || t != '\n') {
          BeyondHasText = true;
          break;
        }
      }
      if (BeyondHasText == true) {
        // Check if we can wrap the line at a comma...
        PreCut = Cut.GetSubString(0, CutLength-1);
        size_t NiceLength = PreCut.Last(',')+1;
        if (NiceLength < CutLength/2) NiceLength = CutLength;

        Formated += Cut.GetSubString(0, NiceLength) + "\n";
        Cut.Remove(0, NiceLength);

        while (Cut.Length() > 0) {
          // Check if we can wrap the line at a comma...
          PreCut = Cut.GetSubString(0, CutLength-7);
          size_t NiceLength2 = PreCut.Last(',')+1;
          if (NiceLength2 < CutLength/2) NiceLength2 = CutLength-6;

          Formated += "     &" + Cut.GetSubString(0, NiceLength2);
          Cut.Remove(0, NiceLength2);
          if (Cut.Length() > 0) Formated += MString("\n");
        }
      } else {
        Formated += Cut;
      }
    }
  }

  return Formated + Text;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::ValidName(MString Name)
{
  // Return true if name is a valid name, i.e. only contains alphanumeric
  // characters as well as "_"

  for (size_t i = 0; i < Name.Length(); ++i) {
    if (isalnum(Name[i]) == 0 &&
        Name[i] != '_') {
      mout<<"   *** Error *** in Name \""<<Name<<"\""<<endl;
      mout<<"Names are only allowed to contain alphanumeric characters as well as \"_\""<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MDGeometry::MakeValidName(MString Name)
{
  // Makes a valid name out of "Name"
  // If the returned string is empty this task was impossible

  MString ValidName;

  for (size_t i = 0; i < Name.Length(); ++i) {
    if (isalnum(Name[i]) != 0 ||
        Name[i] == '_' ||
        Name[i] == '-') {
      ValidName += Name[i];
    }
  }

  return ValidName;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddVolume(MDVolume* Volume)
{
  // Add a volume to the list

  m_VolumeList.push_back(Volume);
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDGeometry::GetWorldVolume()
{
  // return the world volume

  return m_WorldVolume;
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDGeometry::GetVolumeAt(const unsigned int i) const
{
  // return the volume at position i in the list. Counting starts with zero!

  if (i < m_VolumeList.size()) {
    return m_VolumeList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNVolumes()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDGeometry::GetVolume(const MString& Name)
{
  // Return the volume with name Name or 0 if it does not exist

  // This function is not reentrant!!!!

  // ---> begin extremely time critical
  const unsigned int Size = 20;

  list<MDVolume*>::iterator I;
  for (I = m_LastVolumes.begin(); I != m_LastVolumes.end(); ++I) {
    if ((*I)->GetName().AreIdentical(Name)) {
      return (*I);
    }
  }

  unsigned int i, i_max = m_VolumeList.size();
  for (i = m_LastVolumePosition; i < i_max; ++i) {
    if (m_VolumeList[i]->GetName().AreIdentical(Name)) {
      m_LastVolumes.push_front(m_VolumeList[i]);
      if (m_LastVolumes.size() > Size) m_LastVolumes.pop_back();
      m_LastVolumePosition = i;
      return m_VolumeList[i];
    }
  }
  for (i = 0; i < m_LastVolumePosition; ++i) {
    if (m_VolumeList[i]->GetName().AreIdentical(Name)) {
      m_LastVolumes.push_front(m_VolumeList[i]);
      if (m_LastVolumes.size() > Size) m_LastVolumes.pop_back();
      m_LastVolumePosition = i;
      return m_VolumeList[i];
    }
  }

  // Not optimized:
  //unsigned int i, i_max = m_VolumeList.size();
  //for (i = 0; i < i_max; ++i) {
  //  if (Name == m_VolumeList[i]->GetName()) {
  //    return m_VolumeList[i];
  //  }
  //}

  // Infos: CompareTo is faster than ==

  // <--- end extremely time critical

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetVolumeIndex(const MString& Name)
{
  // Return the index of volume with name Name or g_UnsignedIntNotDefined if it does not exist

  unsigned int i, i_max = m_VolumeList.size();
  for (i = 0; i < i_max; ++i) {
    if (Name == m_VolumeList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNVolumes() const
{
  // Return the number of volumes in the list

  return m_VolumeList.size();
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddDetector(MDDetector* Detector)
{
  // Add a volume to the list

  m_DetectorList.push_back(Detector);
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDGeometry::GetDetectorAt(unsigned int i)
{
  // return the volume at position i in the list. Counting starts with zero!

  if (i < GetNDetectors()) {
    return m_DetectorList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNDetectors()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDGeometry::GetDetector(const MString& Name)
{
  // Return the detector with name Name or 0 if it does not exist

  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (Name == m_DetectorList[i]->GetName()) {
      return m_DetectorList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDGeometry::GetDetector(MVector Position)
{
  // Return the detector which corresponds to position Pos

  MDVolumeSequence S = GetVolumeSequence(Position);
  return S.GetDetector();
}


////////////////////////////////////////////////////////////////////////////////


int MDGeometry::GetDetectorType(const MVector& Position)
{
  MDDetector* D = GetDetector(Position);
  
  if (D == nullptr) return 0;
  return D->GetType();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetDetectorIndex(const MString& Name)
{
  // Return the index of material with name Name or g_UnsignedIntNotDefined if it does not exist

  unsigned int i, i_max = GetNDetectors();
  for (i = 0; i < i_max; i++) {
    if (Name == m_DetectorList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNDetectors()
{
  // Return the number of volumes in the list

  return m_DetectorList.size();
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::AddShape(const MString& Type, const MString& Name)
{
  // Add a shape to the list

  MString S = Type;
  S.ToLowerInPlace();

  if (S == "box" || S == "brik") {
    AddShape(new MDShapeBRIK(Name));
  } else if (S == "sphe" || S == "sphere") {
    AddShape(new MDShapeSPHE(Name));
  } else if (S == "cone") {
    AddShape(new MDShapeCONE(Name));
  } else if (S == "cons") {
    AddShape(new MDShapeCONS(Name));
  } else if (S == "pcon") {
    AddShape(new MDShapePCON(Name));
  } else if (S == "pgon") {
    AddShape(new MDShapePGON(Name));
  } else if (S == "tubs" || S == "tube") {
    AddShape(new MDShapeTUBS(Name));
  } else if (S == "trap") {
    AddShape(new MDShapeTRAP(Name));
  } else if (S == "trd1") {
    AddShape(new MDShapeTRD1(Name));
  } else if (S == "trd2") {
    AddShape(new MDShapeTRD2(Name));
  } else if (S == "gtra") {
    AddShape(new MDShapeGTRA(Name));
  } else if (S == "subtraction") {
    AddShape(new MDShapeSubtraction(Name));
  } else if (S == "union") {
    AddShape(new MDShapeUnion(Name));
  } else if (S == "intersection") {
    AddShape(new MDShapeIntersection(Name));
  } else {
    Typo("Line does not contain a known shape type!");
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddShape(MDShape* Shape)
{
  // Add a shape to the list

  m_ShapeList.push_back(Shape);
}


////////////////////////////////////////////////////////////////////////////////


MDShape* MDGeometry::GetShapeAt(unsigned int i)
{
  // return the shape at position i in the list. Counting starts with zero!

  if (i < GetNShapes()) {
    return m_ShapeList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNShapes()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDShape* MDGeometry::GetShape(const MString& Name)
{
  // Return the shape with name Name or 0 if it does not exist

  for (unsigned int i = 0; i < GetNShapes(); i++) {
    if (Name == m_ShapeList[i]->GetName()) {
      return m_ShapeList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetShapeIndex(const MString& Name)
{
  // Return the index of the shape with name Name or g_UnsignedIntNotDefined if it does not exist

  unsigned int i, i_max = GetNShapes();
  for (i = 0; i < i_max; i++) {
    if (Name == m_ShapeList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNShapes()
{
  // Return the number of shapes in the list

  return m_ShapeList.size();
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddOrientation(MDOrientation* Orientation)
{
  // Add an orientation to the list

  m_OrientationList.push_back(Orientation);
}


////////////////////////////////////////////////////////////////////////////////


MDOrientation* MDGeometry::GetOrientationAt(unsigned int i)
{
  // return the orientation at position i in the list. Counting starts with zero!

  if (i < GetNOrientations()) {
    return m_OrientationList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNOrientations()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDOrientation* MDGeometry::GetOrientation(const MString& Name)
{
  // Return the orientation with name Name or 0 if it does not exist

  for (unsigned int i = 0; i < GetNOrientations(); i++) {
    if (Name == m_OrientationList[i]->GetName()) {
      return m_OrientationList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetOrientationIndex(const MString& Name)
{
  // Return the index of the orientation with name Name or g_UnsignedIntNotDefined if it does not exist

  unsigned int i, i_max = GetNOrientations();
  for (i = 0; i < i_max; i++) {
    if (Name == m_OrientationList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNOrientations()
{
  // Return the number of orientations in the list

  return m_OrientationList.size();
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddMaterial(MDMaterial* Material)
{
  // Add a material to the list

  m_MaterialList.push_back(Material);
}


////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDGeometry::GetMaterialAt(unsigned int i)
{
  // return the material at position i in the list. Counting starts with zero!

  if (i < GetNMaterials()) {
    return m_MaterialList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNMaterials()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDMaterial* MDGeometry::GetMaterial(const MString& Name)
{
  // Return the material with name Name or 0 if it does not exist

  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (Name == m_MaterialList[i]->GetName()) {
      return m_MaterialList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetMaterialIndex(const MString& Name)
{
  // Return the material with name Name or 0 if it does not exist

  unsigned int i, i_max = GetNMaterials();
  for (i = 0; i < i_max; i++) {
    if (Name == m_MaterialList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNMaterials()
{
  // Return the number of materials in the list

  return m_MaterialList.size();
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddTrigger(MDTrigger* Trigger)
{
  // Add a material to the list

  m_TriggerList.push_back(Trigger);
}


////////////////////////////////////////////////////////////////////////////////


MDTrigger* MDGeometry::GetTriggerAt(unsigned int i)
{
  // return the material at position i in the list. Counting starts with zero!

  if (i < GetNTriggers()) {
    return m_TriggerList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNTriggers()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDTrigger* MDGeometry::GetTrigger(const MString& Name)
{
  // Return the material with name Name or 0 if it does not exist

  unsigned int i;
  for (i = 0; i < GetNTriggers(); i++) {
    if (Name == m_TriggerList[i]->GetName()) {
      return m_TriggerList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetTriggerIndex(const MString& Name)
{
  // Return the material with name Name or 0 if it does not exist

  unsigned int i, i_max = GetNTriggers();
  for (i = 0; i < i_max; i++) {
    if (Name == m_TriggerList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNTriggers()
{
  // Return the number of materials in the list

  return m_TriggerList.size();
}


////////////////////////////////////////////////////////////////////////////////


MDSystem* MDGeometry::GetSystem(const MString& Name)
{
  // Return the system with name Name or 0 if it does not exist

  if (m_System->GetName() == Name) {
    return m_System;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddVector(MDVector* Vector)
{
  // Add a vector to the list

  m_VectorList.push_back(Vector);
}


////////////////////////////////////////////////////////////////////////////////


MDVector* MDGeometry::GetVectorAt(unsigned int i)
{
  // return the vector at position i in the list. Counting starts with zero!

  if (i < GetNVectors()) {
    return m_VectorList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNVectors()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MDVector* MDGeometry::GetVector(const MString& Name)
{
  // Return the vector with name Name or 0 if it does not exist

  unsigned int i;
  for (i = 0; i < GetNVectors(); i++) {
    if (Name == m_VectorList[i]->GetName()) {
      return m_VectorList[i];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetVectorIndex(const MString& Name)
{
  // Return the vector with name Name or 0 if it does not exist

  unsigned int i, i_max = GetNVectors();
  for (i = 0; i < i_max; i++) {
    if (Name == m_VectorList[i]->GetName()) {
      return i;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGeometry::GetNVectors()
{
  // Return the number of vectors in the list

  return m_VectorList.size();
}



////////////////////////////////////////////////////////////////////////////////


void MDGeometry::AddInclude(MString FileName)
{
  // Add the name of an included file and its md5 hash

  if (IsIncluded(FileName) == false) {
    m_IncludeList.push_back(FileName);
    
    TMD5* MD5 = TMD5::FileChecksum(FileName);
    if (MD5 == nullptr) {
      cout<<" *** ERROR: Unable to calculate checksum for file "<<FileName<<endl;
      m_IncludeListHashes.push_back("");
    } else {
      m_IncludeListHashes.push_back(MD5->AsString());
    }
    delete MD5;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::IsIncluded(MString FileName)
{
  // Check if the file has already been included

  for (unsigned int i = 0; i < m_IncludeList.size(); ++i) {
    if (m_IncludeList[i] == FileName) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


int MDGeometry::GetNIncludes()
{
  // Get the number of included files:

  return m_IncludeList.size();
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::RequiresReload()
{
  // Return true if the geometry needs to be reloaded since files have changed
  
  if (m_IncludeList.size() != m_IncludeListHashes.size()) {
    cout<<" *** ERROR: The number of stored hashes differs from the number of included files"<<endl;
    return true;
  }
  
  for (unsigned int i = 0; i < m_IncludeList.size(); ++i) {
    TMD5* MD5 = TMD5::FileChecksum(m_IncludeList[i]);
    MString Hash = "";
    if (MD5 == nullptr) {
      cout<<" *** ERROR: Unable to calculate checksum for file "<<m_IncludeList[i]<<endl;
    } else {
      Hash = MD5->AsString();
    }
    delete MD5;

    if (Hash != m_IncludeListHashes[i]) {
      cout<<"Info: File "<<m_IncludeList[i]<<" changed. Reload required!"<<endl;
      return true;
    }
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////


MString MDGeometry::ToString()
{
  //

  unsigned int i;
  ostringstream out;

  out<<endl<<"Description of geometry: "<<m_Name<<", version: "<<m_Version<<endl;

  if (m_ShapeList.size() > 0) {
    out<<endl<<endl<<"Description of individually defined shapes:"<<endl;
    for (i = 0; i < m_ShapeList.size(); ++i) {
      out<<m_ShapeList[i]->GetName()<<endl;
      out<<m_ShapeList[i]->ToString()<<endl;
    }
  }

  if (m_OrientationList.size() > 0) {
    out<<endl<<endl<<"Description of individually defined orientations:"<<endl;
    for (i = 0; i < m_OrientationList.size(); ++i) {
      out<<m_OrientationList[i]->GetName()<<endl;
      out<<m_OrientationList[i]->ToString()<<endl;
    }
  }

  out<<endl<<endl<<"Description of volumes:"<<endl;
  for (i = 0; i < m_VolumeList.size(); ++i) {
    out<<m_VolumeList[i]->GetName()<<endl;
    out<<m_VolumeList[i]->ToString()<<endl;
  }

  out<<endl<<endl<<"Description of volume-tree:"<<endl;
  if (m_WorldVolume != 0) {
    out<<m_WorldVolume->ToStringVolumeTree(0)<<endl;
  }

  out<<endl<<endl<<"Description of materials:"<<endl<<endl;
  for (i = 0; i < m_MaterialList.size(); ++i) {
    out<<m_MaterialList[i]->ToString();
  }

  out<<endl<<endl<<"Description of detectors:"<<endl<<endl;
  for (i = 0; i < m_DetectorList.size(); ++i) {
    out<<m_DetectorList[i]->ToString()<<endl;
  }

  out<<endl<<endl<<"Description of triggers:"<<endl<<endl;
  for (i = 0; i < m_TriggerList.size(); ++i) {
    out<<m_TriggerList[i]->ToString()<<endl;
  }

  return out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MDGeometry::GetName()
{
  //

  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


MString MDGeometry::GetFileName()
{
  //

  return m_FileName;
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometry::GetStartSphereRadius() const
{
  return m_SurroundingSphereRadius;
}


////////////////////////////////////////////////////////////////////////////////


double MDGeometry::GetStartSphereDistance() const
{
  return m_SurroundingSphereDistance;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDGeometry::GetStartSpherePosition() const
{
  return m_SurroundingSpherePosition;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::HasComplexER()
{
  // Return true if the co0mplex geometry is used for event reconstrcution,
  // i.e. if volume sequences are necessary

  return m_ComplexER;
}


////////////////////////////////////////////////////////////////////////////////


vector<MDMaterial*> MDGeometry::GetListOfUnusedMaterials()
{
  // Return a list of unused materials

  // First create a list of used materials:
  vector<MDMaterial*> Used;
  for (unsigned int v = 0; v < m_VolumeList.size(); ++v) {
    MDMaterial* M = m_VolumeList[v]->GetMaterial();
    if (find(Used.begin(), Used.end(), M) == Used.end()) {
      Used.push_back(M);
    }
  }

  // Now create a list of not used materials:
  vector<MDMaterial*> Unused;
  for (unsigned int m = 0; m < m_MaterialList.size(); ++m) {
    if (find(Used.begin(), Used.end(), m_MaterialList[m]) == Used.end()) {
      Unused.push_back(m_MaterialList[m]);
    }
  }

  return Unused;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence MDGeometry::GetVolumeSequence(MVector Pos, bool ForceDetector, bool ForceSensitiveVolume)
{
  // Return the volume sequence for this position...

  MDVolumeSequence* VSpointer = GetVolumeSequencePointer(Pos, ForceDetector, ForceSensitiveVolume);
  MDVolumeSequence VS = *VSpointer;
  delete VSpointer;

  return VS;
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence* MDGeometry::GetVolumeSequencePointer(MVector Pos, bool ForceDetector, bool ForceSensitiveVolume)
{
  // Return the volume sequence for this position...

  MDVolumeSequence* VS = new MDVolumeSequence();

  m_WorldVolume->GetVolumeSequence(Pos, VS);

  if ((ForceDetector == true && VS->GetDetector() == 0) ||
      (ForceSensitiveVolume == true && VS->GetSensitiveVolume() == 0)) {
    MVector OrigPos = Pos;

    double Tolerance = m_DetectorSearchTolerance;

    ostringstream out;

    out<<endl;
    out<<"  Warning:"<<endl;
    if (VS->GetDetector() == 0) {
      out<<"     No detector volume could be found for the hit at position ";
    } else {
      out<<"     No sensitive volume could be found for the hit at position ";
    }
    out<<setprecision(20)<<OrigPos[0]<<", "<<OrigPos[1]<<", "<<OrigPos[2]<<setprecision(6)<<endl;
    if (VS->GetDeepestVolume() != 0) {
      out<<"     The deepest volume is: "<<VS->GetDeepestVolume()->GetName()<<endl;
    }
    out<<"     Possible reasons are: "<<endl;
    out<<"       * The hit is just (+-"<<Tolerance<<" cm) outside the border of the volume:" <<endl;
    out<<"           -> Make sure you have stored your simulation file with enough digits"<<endl;
    out<<"              (cosima keyword \"StoreScientific\") so that the volume borders can be separated"<<endl;
    out<<"           -> Make sure your search tolerance given in the geometry file (geomega keyword"<<endl;
    out<<"              \"DetectorSearchTolerance\") is not too small"<<endl;
    out<<"       * The current and the simulation geometry are not identical"<<endl;
    out<<"       * There are overlaps in your geometry:"<<endl;

    // Check for overlaps:
    vector<MDVolume*> OverlappingVolumes;
    m_WorldVolume->FindOverlaps(Pos, OverlappingVolumes);
    if (OverlappingVolumes.size() > 1) {
      out<<"         The following volumes overlap:"<<endl;
      for (unsigned int i = 0; i < OverlappingVolumes.size(); ++i) {
        out<<"           "<<OverlappingVolumes[i]->GetName()<<endl;
      }
    } else {
      out<<"           -> No simple overlaps found, but you might do a full overlap check anyway..."<<endl;
    }

    // Start the search within a tolerance limit
    if (VS->GetDetector() == 0) {
      out<<"     Searching for a detector within "<<Tolerance<<" cm around the given position..."<<endl;
    } else {
      out<<"     Searching for a sensitive volume within "<<Tolerance<<" cm around the given position..."<<endl;
    }

    Pos = OrigPos;
    Pos[0] += Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    Pos = OrigPos;
    Pos[0] -= Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    Pos = OrigPos;
    Pos[1] += Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    Pos = OrigPos;
    Pos[1] -= Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    Pos = OrigPos;
    Pos[2] += Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    Pos = OrigPos;
    Pos[2] -= Tolerance;
    VS->Reset();
    m_WorldVolume->GetVolumeSequence(Pos, VS);
    if (VS->GetDeepestVolume() != 0) {
      if (VS->GetSensitiveVolume() != 0 && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct sensitive volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      } else if (VS->GetSensitiveVolume() == 0 && ForceSensitiveVolume == false && VS->GetDetector() != 0) {
        out<<"     --> Successfully guessed the correct detector volume: "<<VS->GetDeepestVolume()->GetName()<<endl;
        return VS;
      }
    }

    out<<"     --> No suitable volume found!"<<endl;

    // Only print the warning if we did not find anything
    cout<<out.str()<<endl; // cout instead of mout...
  }

  return VS;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDGeometry::GetGlobalPosition(const MVector& PositionInDetector, const MString& NamedDetector)
{
  //! Use this function to convert a position within a NAMED detector
  //! (i.e. uniquely identifyable) into a position in the global coordinate system

  MVector Position = g_VectorNotDefined;

  // Find the detector (class) which contains the given named detector
  bool Found = false;
  for (unsigned d = 0; d < m_DetectorList.size(); ++d) {
    if (m_DetectorList[d]->HasNamedDetector(NamedDetector) == true) {
      Position = m_DetectorList[d]->GetGlobalPosition(PositionInDetector, NamedDetector);
      Found = true;
      break;
    }
  }

  if (Found == false) {
    mout<<"   ***  Error  ***  Named detector not found: "<<NamedDetector<<endl;
  }

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDGeometry::GetRandomPositionInVolume(const MString& Name)
{
  //! Return a random position in the given volume --- excluding daughter volumes!
  //! Any of the clone templates!!! (needed by Cosima)

  if (m_GeometryScanned == false) {
    merr<<"Geometry has to be scanned first!"<<endl;
    return g_VectorNotDefined;
  }

  MDVolume* Volume = nullptr;
  if (m_LastFoundVolume_GetRandomPositionInVolume != nullptr &&
      m_LastFoundVolume_GetRandomPositionInVolume->GetName() == Name) {
    Volume = m_LastFoundVolume_GetRandomPositionInVolume;
  } else {
    Volume = GetVolume(Name);
  }

  if (Volume == 0) {
    merr<<"No volume of this name exists: "<<Name<<endl;
    return g_VectorNotDefined;
  }

  if (Volume->IsClone()) {
    Volume = Volume->GetCloneTemplate();
  }

  //cout<<"Volume: "<<Name<<endl;

  // First find out how many placements we have:
  vector<int> Placements;
  int TreeDepth = -1;
  if (m_LastFoundVolume_GetRandomPositionInVolume != nullptr &&
      m_LastFoundVolume_GetRandomPositionInVolume->GetName() == Name) {
    Placements = m_LastFoundPlacements_GetRandomPositionInVolume;
  } else {
    m_WorldVolume->GetNPlacements(Volume, Placements, TreeDepth);
  }

  int Total = 1;
  //cout<<"Placements"<<endl;
  for (unsigned int i = 0; i < Placements.size(); ++i) {
    //cout<<Placements[i]<<endl;
    if (Placements[i] != 0) {
      Total *= Placements[i];
    }
  }
  //cout<<"Total: "<<Total<<endl;

  int Random = m_RandomPositionInVolumeRNG.Integer(Total);

  //cout<<"Random: "<<Random<<endl;

  // Update the placements to reflect the ID of the random volume
  vector<int> NewPlacements;
  for (unsigned int i = 0; i < Placements.size(); ++i) {
    if (Placements[i] != 0) {
      Total = Total/Placements[i];
      NewPlacements.push_back(Random / Total);
      Random = Random % Total;
    } else {
      //NewPlacements[i] = 0;
    }
  }

  //cout<<"New placements"<<endl;
  //for (unsigned int i = 0; i < NewPlacements.size(); ++i) {
  //  cout<<NewPlacements[i]<<endl;
  //}

  TreeDepth = -1;
  MVector Pos = m_WorldVolume->GetRandomPositionInVolume(Volume, NewPlacements, TreeDepth);

  //cout<<Pos<<endl;

  if (m_LastFoundVolume_GetRandomPositionInVolume != nullptr &&
      m_LastFoundVolume_GetRandomPositionInVolume->GetName() == Name) {
    // Nothing if identical
  } else {
    m_LastFoundVolume_GetRandomPositionInVolume = Volume;
    m_LastFoundPlacements_GetRandomPositionInVolume = Placements;
  }

  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::CreateCrossSectionFiles()
{
  // Create the x-section files if cosima is present

  mout<<endl;
  mout<<"Cross sections have changed or are missing. Starting calculation using cosima (Geant4)!"<<endl;
  mout<<endl;

  if (MFile::Exists(g_MEGAlibPath + "/bin/cosima") == false) {
    mout<<"   ***  Warning  ***"<<endl;
    mout<<"Cannot create cross section files since cosima is not present."<<endl;
    return false;
  }


  // (1) Create a mimium cosima file:
  MString FileName = gSystem->TempDirectory();
  FileName += "/MEGAlib_CrossSections_";
  FileName += gRandom->Integer(900000000)+100000000;
  FileName += gRandom->Integer(900000000)+100000000;
  FileName += ".source";
  
  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    mout<<"   ***  Error  ***"<<endl;
    mout<<"Unable to create cosima source file for cross section creation: "<<FileName<<endl;
    return false;
  }

  out<<"Version                     1"<<endl;
  out<<"Geometry                   "<<m_FileName<<endl;
  out<<"PhysicsListEM               Standard"<<endl;
  out<<"CreateCrossSectionFiles    "<<m_CrossSectionFileDirectory<<endl;

  out.close();


  // (2) Run cosima
  mout<<"-------- Cosima output start --------"<<endl;
  MString WorkingDirectory = gSystem->WorkingDirectory();
  gSystem->ChangeDirectory(gSystem->TempDirectory());
  gSystem->Exec(MString("bash -c \"source ${MEGALIB}/bin/source-megalib.sh; cosima ") + FileName + MString("\""));
  gSystem->Exec(MString("rm -f DelMe.*.sim ") + FileName);
  gSystem->ChangeDirectory(WorkingDirectory);
  mout<<"-------- Cosima output stop ---------"<<endl;


  // (3) Check if cross sections are loaded could be created
  bool Success = true;
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (m_MaterialList[i]->LoadCrossSections(true) == false) {
      Success = false;
    }
  }
  if (Success == false) {
    mout<<"   ***  Warning  ***"<<endl;
    mout<<"Cannot load created cross section files correctly."<<endl;
    mout<<"Please read the above error output for a possible fix..."<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDGeometry::ReplaceWholeWords(MString& Text, const MString& OldWord, const MString& NewWord)
{
  // In "Text" replace all occurances of OldWord with NewWord if the character
  // after "OldWord is not alphanumerical or "_"

  // This algorithm also appears MDDebugInfo::Replace
  if (Text.Length() > 0) {
    size_t Pos = 0;
    while ((Pos = Text.Index(OldWord, Pos)) != MString::npos) {
      //cout<<Text<<":"<<Pos<<endl;
      if (Text.Length() > Pos + OldWord.Length()) {
        //cout<<"i: "<<Text[Pos + OldWord.Length()]<<endl;
        if (isalnum(Text[Pos + OldWord.Length()]) != 0 ||
            Text[Pos + OldWord.Length()] == '_') {
          //cout<<"cont."<<endl;
          Pos += OldWord.Length();
          continue;
        }
      }
      if (Pos > 0) {
        if (isalnum(Text[Pos - 1]) != 0 ||
            Text[Pos - 1] == '_') {
          //cout<<"cont."<<endl;
          Pos += OldWord.Length();
          continue;
        }
      }
      Text.Replace(Pos, OldWord.Length(), NewWord);
      Pos += NewWord.Length();
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::ContainsReplacableConstant(const MString& Text, const MString& Constant)
{
  // Check if "Text" contains Constant as a real keyword

  if (Text.Contains(Constant) == false) return false;

  if (Text.Length() > 0) {
    size_t Pos = 0;
    while ((Pos = Text.Index(Constant, Pos)) != MString::npos) {
      // Found it!
      // The characters immediately before and after are not allowed to be alphanumerical or "_"
      if (Text.Length() > Pos + Constant.Length()) {
        //cout<<"i: "<<Text[Pos + OldWord.Length()]<<endl;
        if (isalnum(Text[Pos + Constant.Length()]) || Text[Pos + Constant.Length()] == '_') {
          return false;
        }
      }
      if (Pos > 0) {
        if (isalnum(Text[Pos - 1]) || Text[Pos - 1] == '_') {
          return false;
        }
      }
      Pos += Constant.Length();
    }
  }

  return true;
}


// MDGeometry.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
