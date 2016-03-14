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
#include "MDSystem.h"
#include "MTimer.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
  m_IgnoreShortNames = true;
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
  
  // Make sure we ignore the default ROOT geometry...
  // BUG: In case we are multi-threaded and some one interact with the geometry
  //      before the gGeoManager is reset during new, we will get a seg-fault!
  gGeoManager = 0;
  // ... before 
  m_Geometry = new TGeoManager("Geomega geometry", "Geomega");
  
  m_LaunchedByGeomega = false;
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
  
  if (m_GeoView != 0) {
    if (gROOT->FindObject("MainCanvasGeomega") != 0) {
      delete m_GeoView;
    }
    m_GeoView = 0;
  }

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

  m_IgnoreShortNames = true; // This should NOT be reset...
  m_DoSanityChecks = true;
  m_ComplexER = true;
  m_VirtualizeNonDetectorVolumes = false;

  m_LastVolumes.clear();
  m_LastVolumePosition = 0;

  m_DetectorSearchTolerance = 0.000001;

  m_CrossSectionFileDirectory = g_MEGAlibPath + "/resource/geometries/materials";

  MDVolume::ResetIDs();
  MDDetector::ResetIDs();
  MDMaterial::ResetIDs();

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

  int Stage = 0;
  MTimer Timer;
  double TimeLimit = 10;
  bool FoundDepreciated = false;

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
  MFile::ExpandFileName(m_FileName);

  if (gSystem->IsAbsoluteFileName(m_FileName) == false) {
    m_FileName = gSystem->WorkingDirectory() + MString("/") + m_FileName;
  }

  if (gSystem->AccessPathName(m_FileName) == 1) {
    mgui<<"Geometry file \""<<m_FileName<<"\" does not exist. Aborting."<<error;
    return false;
  }

  m_CrossSectionFileDirectory = MFile::GetDirectoryName(m_FileName);
  m_CrossSectionFileDirectory += "/auxiliary";

  
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
  
  vector<MDDebugInfo> FileContent;
  if (AddFile(m_FileName, FileContent) == false) {
    mout<<"   *** Error reading included files. Aborting!"<<endl;
    return false;
  }

  // Now scan the data and search for "Include" files and add them 
  // to the original stored file content
  MTokenizer Tokenizer;

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
      Typo("Tokenizer error");
      return false;      
    }
                    
    if (Tokenizer.GetNTokens() == 0) continue;

    if (Tokenizer.IsTokenAt(0, "Include") == true) {

      // Test for old material path
      if (Tokenizer.GetTokenAt(1).EndsWith("resource/geometries/materials/Materials.geo") == true) {
        mout<<" *** Deprectiated *** "<<endl;
        mout<<"You are using the old MEGAlib material path:"<<endl;
        mout<<m_DebugInfo.GetText()<<endl;
        mout<<"Please update to the new path now!"<<endl;
        mout<<"Change: resource/geometries To: resource/examples/geomega "<<endl;
        mout<<endl;
        FoundDepreciated = true;
      }

      MString FileName = Tokenizer.GetTokenAt(1);
      MFile::ExpandFileName(FileName, m_FileName);

      if (MFile::Exists(FileName) == false) {
        mout<<"   *** Error finding file "<<FileName<<endl;
        Typo("File IO error");
        return false;
      }

      vector<MDDebugInfo> AddFileContent;
      if (AddFile(FileName, AddFileContent) == false) {
        mout<<"   *** Error reading file "<<FileName<<endl;
        Typo("File IO error");
        return false;
      }

      for (unsigned int j = 0; j < AddFileContent.size(); ++j) {
        //FileContent.push_back(AddFileContent[j]);
        FileContent.insert(FileContent.begin() + (i+1) + j, AddFileContent[j]);
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



  // Find lines which are continued in a second line by the "\\" keyword

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
      Typo("Tokenizer error");
      return false;      
    }

    if (Tokenizer.GetNTokens() == 0) continue;

    // Of course the real token is "\\"
    if (Tokenizer.IsTokenAt(Tokenizer.GetNTokens()-1, "\\\\") == true) {
      //cout<<"Found \\\\: "<<Tokenizer.ToString()<<endl;
      // Prepend this text to the next line
      if (FileContent.size() > i+1) {
        //cout<<"Next: "<<FileContent[i+1].GetText()<<endl;
        MString Prepend = "";
        for (unsigned int t = 0; t < Tokenizer.GetNTokens()-1; ++t) {
          Prepend += Tokenizer.GetTokenAt(t);
          Prepend += " ";
        }
        FileContent[i+1].Prepend(Prepend);
        FileContent[i].SetText("");
        //cout<<"Prepended: "<< FileContent[i+1].GetText()<<endl;
      }
    }
  }



  // Find constants
  // 

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
      Typo("Tokenizer error");
      return false;      
    }

    if (Tokenizer.GetNTokens() == 0) continue;

    // Constants
    if (Tokenizer.IsTokenAt(0, "Constant") == true) {
      if (Tokenizer.GetNTokens() != 3) {
        Typo("Line must contain three entries, e.g. \"Constant Distance 10.5\"");
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
          MTokenizer::EvaluateMaths(Constant);
          (*Iter1).second = Constant;
        }
      }
    }
    
    // Step 2: Replace constants in constants
    bool ConstantChangableWithMath = false;
    ConstantChanged = false;
    for (map<MString, MString>::iterator Iter1 = m_ConstantMap.begin();
         Iter1 != m_ConstantMap.end();
         ++Iter1) {
      //cout<<(*Iter1).first<<" - "<<(*Iter1).second<<" Pos: "<<i++<<" of "<<m_ConstantMap.size()<<endl;
      for (map<MString, MString>::iterator Iter2 = m_ConstantMap.begin();
           Iter2 != m_ConstantMap.end();
           ++Iter2) {
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
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) { // No maths since we are not yet ready for it...
      Typo("Tokenizer error");
      return false;
    }
    if (Tokenizer.GetNTokens() == 0) continue;
    MString Init = Tokenizer.GetTokenAt(0);
    if (Init == "Volume" ||
      Init == "Material" ||
      Init == "Trigger" ||
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
      FileContent[i].Replace((*Iter).first, (*Iter).second, true);
    }
  }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating constants) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }


  // Check for Vectors FIRST since those are used in ForVector loops...
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) { // No maths since we are not yet ready for it...
      Typo("Tokenizer error");
      return false;      
    }

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
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) { // No maths since we are not yet ready for it...
      Typo("Tokenizer error");
      return false;      
    }

    if (Tokenizer.GetNTokens() == 0) continue;
    
    if ((Vector = GetVector(Tokenizer.GetTokenAt(0))) != 0) {
      Tokenizer.Analyse(m_DebugInfo.GetText()); // let's do some maths...

      if (Tokenizer.IsTokenAt(1, "Matrix") == true) {
        // We need at least 9 keywords:
        if (Tokenizer.GetNTokens() < 9) {
          Typo("Vector.Matrix must contain at least nine keywords,"
               " e.g. \"MaskMatrix.Matrix  3 1.0  3 1.0  1 0.0  1 0 1 0 1 0 1 0 1\"");
          return false;
        }
        unsigned int x_max = Tokenizer.GetTokenAtAsUnsignedInt(2);
        unsigned int y_max = Tokenizer.GetTokenAtAsUnsignedInt(4);
        unsigned int z_max = Tokenizer.GetTokenAtAsUnsignedInt(6);
        double dx = Tokenizer.GetTokenAtAsDouble(3);
        double dy = Tokenizer.GetTokenAtAsDouble(5);
        double dz = Tokenizer.GetTokenAtAsDouble(7);

        // Now we know the real number of keywords:
        if (Tokenizer.GetNTokens() != 8+x_max*y_max*z_max) {
          Typo("This version of Vector.Matrix does not contain the right amount of numbers\"");
          return false;
        }
        
        for (unsigned int z = 0; z < z_max; ++z) {
          for (unsigned int y = 0; y < y_max; ++y) {
            for (unsigned int x = 0; x < x_max; ++x) {
              Vector->Add(MVector(x*dx, y*dy, z*dz), Tokenizer.GetTokenAtAsDouble(8 + x + y*x_max + z*x_max*y_max));
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


  // Check for "For"-loops as well as the special "ForVector"-loop
  int ForDepth = 0;
  int CurrentDepth = 0;
  vector<MDDebugInfo>::iterator Iter;
  for (Iter = FileContent.begin(); 
       Iter != FileContent.end(); 
       /* ++Iter erase */) {
    m_DebugInfo = (*Iter);
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
      Typo("Tokenizer error");
      return false;      
    }
    
    if (Tokenizer.GetNTokens() == 0) {
      ++Iter;
      continue;
    }

    if (Tokenizer.IsTokenAt(0, "For") == true) {

      Tokenizer.Analyse(m_DebugInfo.GetText(), true); // redo for math's evaluation just here

      CurrentDepth = ForDepth;
      ForDepth++;
      if (Tokenizer.GetNTokens() != 5) {
        Typo("Line must contain five entries, e.g. \"For I 3 -11.0 11.0\"");
        return false;
      }

      MString Index = Tokenizer.GetTokenAt(1);
      if (Tokenizer.GetTokenAtAsDouble(2) <= 0 || std::isnan(Tokenizer.GetTokenAtAsDouble(2))) { // std:: is required
        mout<<"Loop number: "<<Tokenizer.GetTokenAtAsDouble(2)<<endl;
        Typo("Loop number in for loop must be a positive integer");
        return false;       
      }
      unsigned int Loops = Tokenizer.GetTokenAtAsUnsignedInt(2);
      double Start = Tokenizer.GetTokenAtAsDouble(3);
      double Step = Tokenizer.GetTokenAtAsDouble(4);
      
      // Remove for line
      Iter = FileContent.erase(Iter++);

      // Store content of for loop: 
      vector<MDDebugInfo> ForLoopContent;
      for (; Iter != FileContent.end(); /* ++Iter erase */) {
        m_DebugInfo = (*Iter);
        if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
          Typo("Tokenizer error");
          return false;      
        }
        if (Tokenizer.GetNTokens() == 0) {
          Iter++;
          continue;
        }
        if (Tokenizer.IsTokenAt(0, "For") == true) {
          ForDepth++;
        }
        if (Tokenizer.IsTokenAt(0, "Done") == true) {
          ForDepth--;
          if (ForDepth == CurrentDepth) {
            Iter = FileContent.erase(Iter++);
            break;
          }
        }

        ForLoopContent.push_back(m_DebugInfo);
        Iter = FileContent.erase(Iter++);
      }

      // Add new content at the same place:
      vector<MDDebugInfo>::iterator LastIter = Iter;
      int Position = 0;
      for (unsigned int l = 1; l <= Loops; ++l) {
        MString LoopString;
        LoopString += l;
        MString ValueString;
        ValueString += (Start + (l-1)*Step);


        vector<MDDebugInfo>::iterator ForIter;
        for (ForIter = ForLoopContent.begin(); 
             ForIter != ForLoopContent.end(); 
             ++ForIter) {
          m_DebugInfo = (*ForIter);
          m_DebugInfo.Replace(MString("%") + Index, LoopString);
          m_DebugInfo.Replace(MString("$") + Index, ValueString);

          LastIter = FileContent.insert(LastIter, m_DebugInfo);
          LastIter++;
          Position++;
        }
      }
      Iter = LastIter - Position;
      continue;
    }

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
      
      // Remove for line
      Iter = FileContent.erase(Iter++);

      // Store content of ForVector loop: 
      vector<MDDebugInfo> ForLoopContent;
      for (; Iter != FileContent.end(); /* ++Iter erase */) {
        m_DebugInfo = (*Iter);
        if (Tokenizer.Analyse(m_DebugInfo.GetText(), false) == false) {
          Typo("Tokenizer error");
          return false;      
        }
        if (Tokenizer.GetNTokens() == 0) {
          Iter++;
          continue;
        }
        if (Tokenizer.IsTokenAt(0, "ForVector") == true) {
          ForDepth++;
        }
        if (Tokenizer.IsTokenAt(0, "DoneVector") == true) {
          ForDepth--;
          if (ForDepth == CurrentDepth) {
            Iter = FileContent.erase(Iter++);
            break;
          }
        }

        ForLoopContent.push_back(m_DebugInfo);
        Iter = FileContent.erase(Iter++);
      }

      // Add new content at the same place:
      vector<MDDebugInfo>::iterator LastIter = Iter;
      int Position = 0;
      for (unsigned int l = 1; l <= Vector->GetSize(); ++l) {
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

        vector<MDDebugInfo>::iterator ForIter;
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

          LastIter = FileContent.insert(LastIter, m_DebugInfo);
          LastIter++;
          Position++;
        }
      }
      Iter = LastIter - Position;
      continue;
    }
    ++Iter;
  }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating for loops) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }


  // Find random numbers
  TRandom3 R;
  R.SetSeed(11031879); // Do never modify!!!!
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    while (FileContent[i].Contains("RandomDouble") == true) {
      //cout<<"Before: "<<FileContent[i].GetText()<<endl;
      FileContent[i].ReplaceFirst("RandomDouble", R.Rndm());
      //cout<<"after: "<<FileContent[i].GetText()<<endl;
    }
  }

//   // All constants and for loops are expanded, let's print some text ;-)
//   for (unsigned int i = 0; i < FileContent.size(); i++) {
//     cout<<FileContent[i].GetText()<<endl;
//   }

  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating random numbers) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }


  // Check for "If"-clauses
  int IfDepth = 0;
  int CurrentIfDepth = 0;
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(FileContent[i].GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }
    
    if (Tokenizer.GetNTokens() == 0) {
      continue;
    }

    if (Tokenizer.IsTokenAt(0, "If") == true) {
      
      // Take care of nesting
      CurrentIfDepth = IfDepth;
      IfDepth++;

      
      if (Tokenizer.GetNTokens() != 2) {
        Typo("The If-line must contain two entries, the last one must be math, e.g. \"If { 1 == 2 } or If { $Value > 0 } \"");
        return false;
      }
      
      // Retrieve data:
      bool Bool = Tokenizer.GetTokenAtAsBoolean(1);
      
      // Clear the if line
      FileContent[i].SetText("");

      // Forward its endif:
      for (unsigned int j = i+1; j < FileContent.size(); ++j) {
        if (Tokenizer.Analyse(FileContent[j].GetText(), false) == false) {
          Typo("Tokenizer error");
          return false;      
        }
        if (Tokenizer.GetNTokens() == 0) {
          continue;
        }
        if (Tokenizer.IsTokenAt(0, "If") == true) {
          IfDepth++;
        }
        if (Tokenizer.IsTokenAt(0, "EndIf") == true) {
          IfDepth--;
          if (IfDepth == CurrentIfDepth) {
            FileContent[j].SetText("");
            break;
          }
        }
        if (Bool == false) {
          FileContent[j].SetText("");          
        }
      }
    } // Is if
  } // global loop


  ++Stage;
  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimeLimit) {
    mout<<"Stage "<<Stage<<" (evaluating if clauses) finished after "<<Timer.ElapsedTime()<<" sec"<<endl;
  }


  // All constants and for loops are expanded, let's print some text ;-)
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText(), true) == false) {
      Typo("Tokenizer error");
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

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
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

      AddVolume(new MDVolume(Tokenizer.GetTokenAt(1), 
                             CreateShortName(Tokenizer.GetTokenAt(1))));
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
      
      continue;
    }

    // Show the surrounding sphere 
    else if (Tokenizer.IsTokenAt(0, "ShowSurroundingSphere") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: ShowSurroundingSphere true/false");
        return false;
      }

      m_ShowSurroundSphere = Tokenizer.GetTokenAtAsBoolean(1);
      
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
        Typo("Line must contain two values: ShowVolumes false");
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
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: IgnoreShortNames true");
        return false;
      }

      m_IgnoreShortNames = Tokenizer.GetTokenAtAsBoolean(1);

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
             Tokenizer.IsTokenAt(0, "CrossSectionFilesDirectory") == true) {
      if (Tokenizer.GetNTokens() != 2) {
        Typo("Line must contain two values: CrossSectionFilesDirectory auxiliary");
        return false;
      }

      m_CrossSectionFileDirectory = Tokenizer.GetTokenAtAsString(1);
      MFile::ExpandFileName(m_CrossSectionFileDirectory);

      if (gSystem->IsAbsoluteFileName(m_CrossSectionFileDirectory) == false) {
        m_CrossSectionFileDirectory = 
          MString(MFile::GetDirectoryName(m_FileName)) + "/" + m_CrossSectionFileDirectory;
      }
      
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
             
      AddMaterial(new MDMaterial(Tokenizer.GetTokenAt(1), 
                                 CreateShortName(Tokenizer.GetTokenAt(1)),
                                 CreateShortName(Tokenizer.GetTokenAt(1), 19, false, true)));
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
    else if (Tokenizer.IsTokenAt(0, "Trigger") == true) {
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

      AddTrigger(new MDTrigger(Tokenizer.GetTokenAt(1)));
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

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }

    if (Tokenizer.GetNTokens() < 3) continue;
 

    // Check for volumes with copies
    if (Tokenizer.IsTokenAt(1, "Copy") == true) {
      if ((V = GetVolume(Tokenizer.GetTokenAt(0))) != 0) {
        if (GetVolume(Tokenizer.GetTokenAt(2)) != 0) {
          Typo("A volume of this name already exists!");
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

        MCopy = new MDMaterial(Tokenizer.GetTokenAt(2), 
                               CreateShortName(Tokenizer.GetTokenAt(2)),
                               CreateShortName(Tokenizer.GetTokenAt(2), 19));

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

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }

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
               " e.g. \"Alu.ComponentByAtoms 27.0 13.0 1.0  --> but this is depreciated\""
               " or three string and one double\""
               " e.g. \"Alu.ComponentByAtoms Al 1.0\"");
          return false;
        }
        if (Tokenizer.GetNTokens() == 4) {
          if (M->SetComponent(Tokenizer.GetTokenAtAsString(2), 
                              Tokenizer.GetTokenAtAsDouble(3), 
                              MDMaterialComponent::c_ByAtoms) == false) {
            Typo("Element not found!");
            return false;
          }
        } else {
          M->SetComponent(Tokenizer.GetTokenAtAsDouble(2), 
                          Tokenizer.GetTokenAtAsDouble(3), 
                          Tokenizer.GetTokenAtAsDouble(4),
                          MDMaterialComponent::c_ByAtoms);
          mout<<"   ***  Info  ***  "<<endl;
          mout<<"Remember to use a component description which contains the element name, if you want natural isotope composition during Geant4 simulations"<<endl;
          mout<<"e.g. \"Alu.ComponentByAtoms Al 1.0\""<<endl;
        }
      } else if (Tokenizer.IsTokenAt(1, "ComponentByMass") == true) {
       if (Tokenizer.GetNTokens() < 4 || Tokenizer.GetNTokens() > 5) {
          Typo("Line must contain two strings and 3 doubles,"
               " e.g. \"Alu.ComponentByMass 27.0 13.0 1.0  --> but this is depreciated\""
               " or three string and one double\""
               " e.g. \"Alu.ComponentByMass Al 1.0\"");
          return false;
        }
        if (Tokenizer.GetNTokens() == 4) {
          if (M->SetComponent(Tokenizer.GetTokenAtAsString(2), 
                              Tokenizer.GetTokenAtAsDouble(3), 
                              MDMaterialComponent::c_ByMass) == false) {
            Typo("Element not found!");
            return false;
          }
        } else {
          M->SetComponent(Tokenizer.GetTokenAtAsDouble(2), 
                          Tokenizer.GetTokenAtAsDouble(3), 
                          Tokenizer.GetTokenAtAsDouble(4),
                          MDMaterialComponent::c_ByMass);
          mout<<"   ***  Info  ***  "<<endl;
          mout<<"Remember to use a component description which contains the element name, if you want natural isotope composition during Geant4 simulations"<<endl;
          mout<<"e.g. \"Alu.ComponentByMass Al 1.0\""<<endl;
        }
      } else if (Tokenizer.IsTokenAt(1, "Sensitivity") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one int,"
               " e.g. \"Alu.Sensitivity 1\"");
          return false;
        }
        M->SetSensitivity(Tokenizer.GetTokenAtAsInt(2));
        //      }
        //       // Check for total absorption coefficients:
        //       else if (Tokenizer.IsTokenAt(1, "TotalAbsorptionFile") == true) {
        //         if (Tokenizer.GetNTokens() < 3) {
        //           Typo("Line must contain three strings"
        //                " e.g. \"Wafer.TotalAbsorptionFile MyFile.abs");
        //           return false;
        //         }
        //         M->SetAbsorptionFileName(Tokenizer.GetTokenAfterAsString(2));
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
        FoundDepreciated = true;

        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 integer,"
               " e.g. \"D1D2Trigger.DetectorType 1 2\"");
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
        T->SetVeto(false);
        T->SetTriggerByChannel(true);
        T->SetDetectorType(Tokenizer.GetTokenAtAsInt(2), Tokenizer.GetTokenAtAsInt(3));
      } else if (Tokenizer.IsTokenAt(1, "TriggerByChannel") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean,"
               " e.g. \"D1D2Trigger.TriggerByChannel true\"");
          return false;
        }
        T->SetTriggerByChannel(Tokenizer.GetTokenAtAsBoolean(2));
      } else if (Tokenizer.IsTokenAt(1, "TriggerByDetector") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean,"
               " e.g. \"D1D2Trigger.TriggerByDetector true\"");
          return false;
        }
        T->SetTriggerByDetector(Tokenizer.GetTokenAtAsBoolean(2));
      } else if (Tokenizer.IsTokenAt(1, "Veto") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and one boolean,"
               " e.g. \"D1D2Trigger.Veto true\"");
          return false;
        }
        T->SetVeto(Tokenizer.GetTokenAtAsBoolean(2));
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
        T->SetDetectorType(MDDetector::GetDetectorType(Tokenizer.GetTokenAtAsString(2)), 
                           Tokenizer.GetTokenAtAsInt(3));
  
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
        T->SetDetector(TriggerDetector, Tokenizer.GetTokenAtAsInt(3));
      } else if (Tokenizer.IsTokenAt(1, "GuardringDetectorType") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
               " e.g. \"D1D2Trigger.GuardringDetectorType Strip3D 1\"");
          return false;
        }
        if (MDDetector::IsValidDetectorType(Tokenizer.GetTokenAtAsString(2)) == false) {
          Typo("Line must contain a valid detector type, e.g. Strip2D, DriftChamber, etc.");
          return false;          
        }
        T->SetGuardringDetectorType(MDDetector::GetDetectorType(Tokenizer.GetTokenAtAsString(2)), 
                                    Tokenizer.GetTokenAtAsInt(3));
      } else if (Tokenizer.IsTokenAt(1, "GuardringDetector") == true) {
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain three strings and one int,"
               " e.g. \"D1D2Trigger.GuardringDetector MyStrip2D 1\"");
          return false;
        }
        MDDetector* TriggerDetector;
        if ((TriggerDetector = GetDetector(Tokenizer.GetTokenAt(2))) == 0) {
          Typo("A detector of this name does not exist!");
          return false;
        }
        T->SetGuardringDetector(TriggerDetector, Tokenizer.GetTokenAtAsInt(3));
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
            Typo("A volume of this name does not exist!");
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
        FoundDepreciated = true;
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

          V->SetRotation(Tokenizer.GetTokenAtAsDouble(2), 
                         Tokenizer.GetTokenAtAsDouble(3), 
                         Tokenizer.GetTokenAtAsDouble(4), 
                         Tokenizer.GetTokenAtAsDouble(5), 
                         Tokenizer.GetTokenAtAsDouble(6), 
                         Tokenizer.GetTokenAtAsDouble(7));
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

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }

    if (Tokenizer.GetNTokens() < 2) continue;

    // Check for detectors:
    if ((D = GetDetector(Tokenizer.GetTokenAt(0))) != 0) {
      // Check for global tokens

      // Check for simulation in voxels instead of a junk volume
      if (Tokenizer.IsTokenAt(1, "VoxelSimulation") == true) {
        if (Tokenizer.GetNTokens() != 3) {
          Typo("Line must contain two strings and 1 boolean,"
               " e.g. \"Wafer.VoxelSimulation true\"");
          return false;
        }
        if (Tokenizer.GetTokenAtAsBoolean(2) == true) {
          D->UseDivisions(CreateShortName(MString("X" + D->GetName())),
                          CreateShortName(MString("Y" + D->GetName())),
                          CreateShortName(MString("Z" + D->GetName())));
        }
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
          Typo("A volume of this name does not exist!");
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
          Typo("A volume of this name does not exist!");
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
        MFile::ExpandFileName(FileName, m_FileName);
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
          //FoundDepreciated = true;
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
        MFile::ExpandFileName(FileName, m_FileName);
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
        FoundDepreciated = true;
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
        FoundDepreciated = true;
        //         if (D->GetDetectorType() != MDDetector::c_Strip2D && 
        //             D->GetDetectorType() != MDDetector::c_DriftChamber &&
        //             D->GetDetectorType() != MDDetector::c_Strip3D) {
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
        FoundDepreciated = true;
        //         if (D->GetDetectorType() != MDDetector::c_Strip2D && 
        //             D->GetDetectorType() != MDDetector::c_Strip3D &&
        //             D->GetDetectorType() != MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() != MDDetector::c_Strip2D && 
            D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_Voxel3D &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
          Typo("Option Offset only supported for StripxD & DriftChamber");
          return false;
        }
        
        if (D->GetDetectorType() == MDDetector::c_Voxel3D) {
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
        if (D->GetDetectorType() != MDDetector::c_Strip2D && 
            D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() != MDDetector::c_Voxel3D) {
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
        FoundDepreciated = true;
        //         if (D->GetDetectorType() != MDDetector::c_Strip2D && 
        //             D->GetDetectorType() != MDDetector::c_Strip3D &&
        //             D->GetDetectorType() != MDDetector::c_DriftChamber) {
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
        FoundDepreciated = true;
        //         if (D->GetDetectorType() != MDDetector::c_Strip2D && 
        //             D->GetDetectorType() != MDDetector::c_Strip3D &&
        //             D->GetDetectorType() != MDDetector::c_DriftChamber) {
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
      else if (Tokenizer.IsTokenAt(1, "GuardringTriggerThreshold") == true) {
        if (D->GetDetectorType() != MDDetector::c_Strip2D && 
            D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Voxel3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
          Typo("Option GuardRingTriggerThreshold only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() < 3 || Tokenizer.GetNTokens() > 4) {
          Typo("Line must contain two strings and 2 double,"
               " e.g. \"Wafer.GuardringTriggerThreshold 30 10\"");
          return false;
        }
        if (D->GetDetectorType() == MDDetector::c_Voxel3D) {
          dynamic_cast<MDVoxel3D*>(D)->SetGuardringTriggerThreshold(Tokenizer.GetTokenAtAsDouble(2));
          if (Tokenizer.GetNTokens() == 4) {
            dynamic_cast<MDVoxel3D*>(D)->SetGuardringTriggerThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
          }
        } else {
          dynamic_cast<MDStrip2D*>(D)->SetGuardringTriggerThreshold(Tokenizer.GetTokenAtAsDouble(2));
          if (Tokenizer.GetNTokens() == 4) {
            dynamic_cast<MDStrip2D*>(D)->SetGuardringTriggerThresholdSigma(Tokenizer.GetTokenAtAsDouble(3));
          }
        }
      }
      // Check for guard ring energy resolution
      else if (Tokenizer.IsTokenAt(1, "GuardringEnergyResolution") == true || 
               Tokenizer.IsTokenAt(1, "GuardringEnergyResolutionAt") == true) {
        if (D->GetDetectorType() != MDDetector::c_Strip2D && 
            D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Voxel3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
          Typo("Option GuardringEnergyResolutionAt only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.GuardringEnergyResolutionAt 30 10\"");
          return false;
        }
        if (D->GetDetectorType() == MDDetector::c_Voxel3D) {
          if (dynamic_cast<MDVoxel3D*>(D)->SetGuardringEnergyResolution(Tokenizer.GetTokenAtAsDouble(2), 
                                                                        Tokenizer.GetTokenAtAsDouble(3)) == false) {
            Typo("Incorrect input");
            return false;
          }
        } else {
          if (dynamic_cast<MDStrip2D*>(D)->SetGuardringEnergyResolution(Tokenizer.GetTokenAtAsDouble(2), 
                                                                        Tokenizer.GetTokenAtAsDouble(3)) == false) {
            Typo("Incorrect input");
            return false;
          }
        }
      }
      // Check for Calorimeter specific tokens:
      else if (Tokenizer.IsTokenAt(1, "NoiseAxis") == true) {
        mout<<" *** Unsupported *** "<<endl;
        mout<<"The \"NoiseAxis\" keyword is no longer supported!"<<endl;
        mout<<"For all detectors, the z-axis is by default the depth-noised axis"<<endl; 
        mout<<"For the depth resolution, use the \"DepthResolution\" keyword"<<endl; 
        mout<<endl;
        FoundDepreciated = true;
      }
      else if (Tokenizer.IsTokenAt(1, "DepthResolution") == true ||
               Tokenizer.IsTokenAt(1, "DepthResolutionAt") == true) {
        bool Return = true;
        if (D->GetDetectorType() == MDDetector::c_Calorimeter) {
          if (Tokenizer.GetNTokens() == 4) {
            Return = dynamic_cast<MDCalorimeter*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2), 
                                                                           Tokenizer.GetTokenAtAsDouble(3), 
                                                                           0);
          } else if (Tokenizer.GetNTokens() == 5) {
            Return = dynamic_cast<MDCalorimeter*>(D)->SetDepthResolutionAt(Tokenizer.GetTokenAtAsDouble(2), 
                                                                           Tokenizer.GetTokenAtAsDouble(3), 
                                                                           Tokenizer.GetTokenAtAsDouble(4));
          }
        } else if (D->GetDetectorType() == MDDetector::c_Strip3D ||
                   D->GetDetectorType() == MDDetector::c_Strip3DDirectional ||
                   D->GetDetectorType() == MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() == MDDetector::c_Strip3D ||
            D->GetDetectorType() == MDDetector::c_Strip3DDirectional ||
            D->GetDetectorType() == MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
          Typo("Option TriggerThresholdDepthCorrectionAt only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.TriggerThresholdDepthCorrectionGuardringTriggerThresholdAt 30 10\"");
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
        if (D->GetDetectorType() != MDDetector::c_Strip3D &&
            D->GetDetectorType() != MDDetector::c_Strip3DDirectional &&
            D->GetDetectorType() != MDDetector::c_DriftChamber) {
          Typo("Option NoiseThresholdDepthCorrectionAt only supported for StripxD & DriftChamber");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.NoiseThresholdDepthCorrectionGuardringNoiseThresholdAt 30 10\"");
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
        if (D->GetDetectorType() == MDDetector::c_Strip3DDirectional) {
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
        if (D->GetDetectorType() != MDDetector::c_AngerCamera) {
          Typo("Option PositionResolution only supported for AngerCamera");
          return false;
        }
        if (Tokenizer.GetNTokens() != 4) {
          Typo("Line must contain two strings and 2 doubles,"
               " e.g. \"Wafer.PositionResolutionAt 30 10\"");
          return false;
        }
        dynamic_cast<MDAngerCamera*>(D)->SetPositionResolution(Tokenizer.GetTokenAtAsDouble(2), 
                                                               Tokenizer.GetTokenAtAsDouble(3));
      } else if (Tokenizer.IsTokenAt(1, "Positioning") == true) {
        if (D->GetDetectorType() == MDDetector::c_AngerCamera) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 double:"
                 " e.g. \"Anger.Postioning XYZ\"");
            return false;
          }
          if (Tokenizer.GetTokenAtAsString(2) == "XYZ") {
            dynamic_cast<MDAngerCamera*>(D)->SetPositioning(MDGridPoint::c_XYZAnger);
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
        if (D->GetDetectorType() == MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() == MDDetector::c_DriftChamber) {
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
        if (D->GetDetectorType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two string and 1 double:"
                 " e.g. \"Chamber.DriftConstant 3\"");
            return false;
          }
          dynamic_cast<MDDriftChamber*>(D)->SetDriftConstant(Tokenizer.GetTokenAtAsDouble(2));
        } else {           
          Typo("Option DriftConstant only supported for DriftChamber");
          return false;
        }
       
      } else if (Tokenizer.IsTokenAt(1, "EnergyPerElectron") == true) {
        if (D->GetDetectorType() == MDDetector::c_DriftChamber) {
          if (Tokenizer.GetNTokens() != 3) {
            Typo("Line must contain two strings and 1 double:"
                 " e.g. \"Chamber.EnergyPerElectron 3\"");
            return false;
          }
          dynamic_cast<MDDriftChamber*>(D)->SetEnergyPerElectron(Tokenizer.GetTokenAtAsDouble(2));
        } else {           
          Typo("Option EnergyPerElectron only supported for DriftChamber");
          return false;
        }
       
      } else if (Tokenizer.IsTokenAt(1, "LightEnergyResolution") == true || 
                 Tokenizer.IsTokenAt(1, "LightEnergyResolutionAt") == true) {
        if (D->GetDetectorType() == MDDetector::c_DriftChamber) {
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
  if (m_ShowSurroundSphere == true && m_LaunchedByGeomega == true) {
    MString MaterialName = "SurroundingSphereVolumeMaterial";
    MDMaterial* SuperDense = new MDMaterial(MaterialName, 
                                 CreateShortName(MaterialName),
                                 CreateShortName(MaterialName, 19, false, true));
    MDMaterialComponent* SuperDenseComponent = 
      new MDMaterialComponent(MDMaterialComponent::c_NaturalComposition, 82, 1, MDMaterialComponent::c_ByAtoms);
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
  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }

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
                //cout<<"Found: "<<VolumeNames[i]<<endl;
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
          Pos[0] = Tokenizer.GetTokenAtAsDouble(2);
          Pos[1] = Tokenizer.GetTokenAtAsDouble(3);
          Pos[2] = Tokenizer.GetTokenAtAsDouble(4);
        }
        else {
          Typo("Line must contain two strings and one volume sequence (\"NamedWafer.Assign WorldVolume.Tracker.Wafer1\")"
               " or two strings and three numbers as absolute position (\"NamedWafer.Assign 12.0 0.0 0.0\")");
          return false;
        }

        MDVolumeSequence* VS = new MDVolumeSequence();
        m_WorldVolume->GetVolumeSequence(Pos, VS);
        D->SetVolumeSequence(*VS);
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
          mout<<"Common mother volume for sensitive detectors of "<<m_DetectorList[i]->GetName()<<": "<<Test->GetName()<<endl;
          break;
        }
      }
      
      if (m_DetectorList[i]->GetCommonVolume() == 0) {
        mout<<"   ***  Error  ***  Multiple sensitive volumes per detector restriction"<<endl;
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
      mout<<"   ***  Error  ***  Multiple sensitive volumes per detector restriction"<<endl;
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
    m_NDetectorTypes[m_DetectorList[i]->GetDetectorType()]++;
  }

  // Special detector loop for blocked channels:

  for (unsigned int i = 0; i < FileContent.size(); i++) {
    m_DebugInfo = FileContent[i];
    if (Tokenizer.Analyse(m_DebugInfo.GetText()) == false) {
      Typo("Tokenizer error");
      return false;      
    }

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
  // Make sure that all detectors which have only veto triggers have NoiseThresholdEqualsTriggerThreshold setf 
  for (unsigned int d = 0; d < GetNDetectors(); ++d) {
    int NVetoes = 0;
    int NTriggers = 0;
    for (unsigned int t = 0; t < GetNTriggers(); ++t) {
      if (GetTriggerAt(t)->Applies(GetDetectorAt(d)) == true) {
        if (GetTriggerAt(t)->IsVeto() == true) {
          NVetoes++;
        } else {
          NTriggers++; 
        }
      }
    }
    if (NVetoes > 0 && NTriggers == 0 && GetDetectorAt(d)->GetNoiseThresholdEqualsTriggerThreshold() == false) {
      mout<<"   ***  Error  ***  Triggers with vetoes"<<endl;
      mout<<"A detector (here: "<<GetDetectorAt(d)->GetName()<<"), which only has veto triggers, must have the flag \"NoiseThresholdEqualsTriggerThreshold true\"!"<<endl;
      Reset();
      return false;
    }
  }

  // Material sanity checks
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    m_MaterialList[i]->SetCrossSectionFileDirectory(m_CrossSectionFileDirectory);
    if (m_MaterialList[i]->Validate() == false) {
      IsValid = false;
    }
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


  // Check if we can apply the keyword komplex ER 
  // Does not cover all possibilities (e.g. rotated detector etc.)
  if (m_ComplexER == false) {
    int NTrackers = 0;
    for (unsigned int i = 0; i < GetNDetectors(); i++) {
      if (m_DetectorList[i]->GetDetectorType() == MDDetector::c_Strip2D) {
        if (dynamic_cast<MDStrip2D*>(m_DetectorList[i])->GetOrientation() != 2) {
          mout<<"   ***  Error  ***  ComplexER"<<endl;
          mout<<"This keyword can only be applied for tracker which are oriented in z-axis!"<<endl;
          Reset();
          return false;
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
    vector<MDDetector*> Detectors;
    for (unsigned int i = 0; i < GetNDetectors(); ++i) Detectors.push_back(m_DetectorList[i]);
    
    for (unsigned int t = 0; t < GetNTriggers(); ++t) {
      vector<MDDetector*> TriggerDetectors = m_TriggerList[t]->GetDetectors();
      for (unsigned int d1 = 0; d1 < Detectors.size(); ++d1) {
        for (unsigned int d2 = 0; d2 < TriggerDetectors.size(); ++d2) {
          if (Detectors[d1] == 0) continue;
          if (Detectors[d1] == TriggerDetectors[d2]) {
            Detectors[d1] = 0;
            break;
          }
          // If we have a named detectors, in case the "named after detector" has a trigger criteria, we are fine
          if (Detectors[d1]->IsNamedDetector() == true) {
            if (Detectors[d1]->GetNamedAfterDetector() == TriggerDetectors[d2]) {
              Detectors[d1] = 0;
              break;
            }
          }
        }
      }

      vector<int> TriggerDetectorTypes = m_TriggerList[t]->GetDetectorTypes();
      for (unsigned int d1 = 0; d1 < Detectors.size(); ++d1) {
        if (Detectors[d1] == 0) continue;
        for (unsigned int d2 = 0; d2 < TriggerDetectorTypes.size(); ++d2) {
          if (Detectors[d1]->GetDetectorType() == TriggerDetectorTypes[d2]) {
            Detectors[d1] = 0;
            break;
          }
        }
      } 
    }
    
    for (unsigned int i = 0; i < Detectors.size(); ++i) {
      if (Detectors[i] != 0) {
        mout<<"   ***  Warning  ***  "<<endl;
        mout<<"You have not defined any trigger criterion for detector: "<<Detectors[i]->GetName()<<endl;
      }
    }
  }

  if (IsValid == false) {
    mout<<"   ***  Error  ***  "<<endl;
    mout<<"There were errors while scanning this file. Correct them first!!"<<endl;
    Reset();
    return false;
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
  
  if (FoundDepreciated == true) {
    mgui<<"Your geometry contains depreciated information (see console output for details)."<<endl;
    mgui<<"Please update it now to the latest conventions!"<<show;
  }

  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::AddFile(MString FileName, vector<MDDebugInfo>& FileContent)
{

  FileContent.clear();

  MFile::ExpandFileName(FileName);

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


bool MDGeometry::DrawGeometry(TCanvas* Canvas, MString Mode)
{
  // The geometry must have been loaded previously
  // You cannot display 2 geometries at once!


  if (m_GeometryScanned == false) {
    mgui<<"Geometry has to be scanned before it can be drawn!"<<endl;
    return false;
  }

  // Start by deleting the old windows:
  if (m_GeoView != 0) {
    if (gROOT->FindObject("MainCanvasGeomega") != 0) {
      delete m_GeoView;
    }
    m_GeoView = 0;
  }

  //mdebug<<"NVolumes: "<<m_WorldVolume->GetNVisibleVolumes()<<endl;
  
  // Only draw the new windows if there are volumes to be drawn:
  if (m_WorldVolume->GetNVisibleVolumes() == 0) {
    mgui<<"There are no visible volumes in your geometry!"<<warn;
    return false;
  }

  MTimer Timer;
  double TimerLimit = 5;

  if (Canvas == 0) { 
    m_GeoView = new TCanvas("MainCanvasGeomega","MainCanvasGeomega",800,800);
  } else {
    Canvas->cd();
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

  if (g_Verbosity >= c_Info || Timer.ElapsedTime() > TimerLimit) {
    mout<<"Geometry drawn within "<<Timer.ElapsedTime()<<" seconds."<<endl;
  }

  return true;
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


bool MDGeometry::TestIntersections()
{
  // Test for intersections
  // Attention: Not all can be found!

  cout<<"Testing intersections!"<<endl;

  if (IsScanned() == false) {
    Error("bool MDGeometry::TestIntersections()",
          "You have to scan the geometry file first!");
    return false;
  }

  if (m_WorldVolume->ValidateIntersections() == false) {
    return false;
  }

  cout<<"Testing intersections finished!"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::CheckOverlaps()
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
    mout<<"List of extrusions and overlaps: "<<endl;
    for (int i = 0; i < Overlaps->GetEntries(); ++i) {
      TGeoOverlap* O = (TGeoOverlap*) (Overlaps->At(i));
      if (O->IsOverlap() == true) {
        mout<<"Overlap: "<<O->GetFirstVolume()->GetName()<<" with "<<O->GetSecondVolume()->GetName()<<" by "<<O->GetOverlap()<<" cm"<<endl;
      }
      if (O->IsExtrusion() == true) {
        mout<<"Extrusion: "<<O->GetSecondVolume()->GetName()<<" extrudes "<<O->GetFirstVolume()->GetName()<<" by "<<O->GetOverlap()<<" cm"<<endl;
      }
    }
  } else {
    mout<<endl;
    mout<<"No extrusions and overlaps detected with ROOT (ROOT claims to be able to detect 95% of them)"<<endl;     
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

  ostringstream out;
  out.setf(ios_base::fixed, ios_base::floatfield);
  out.precision(3);
  out<<endl;
  out<<"Mass summary by material: "<<endl;
  out<<endl;
  for (MassesIter = (Masses.begin()); 
       MassesIter != Masses.end(); MassesIter++) {
    out<<setw(NameWidth+2)<<(*MassesIter).first->GetName()<<" :  "<<setw(12)<<(*MassesIter).second<<" g"<<endl;
    Total += (*MassesIter).second;
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


bool MDGeometry::WriteGeant3Files()
{
  // Create GEANT3 files
  
  if (m_GeometryScanned == false) {
    Error("bool MDGeometry::WriteGeant3Files()",
          "Geometry has to be scanned first");
    return false;
  }

  // Some sanity checks:
  if (GetNMaterials() > 200) {
    mout<<"Error: GMega only supports 200 materials at the moment!"<<endl;
    return false;
  }

  // open the geometry-file:
  fstream FileStream; // = new fstream();
  // gcc 2.95.3: FileStream.open("ugeom.f", ios::out, 0664);
  FileStream.open("ugeom.f", ios_base::out);

  // Write header:
  ostringstream Text;
  Text<<
    "************************************************************************\n"
    "*\n"
    "*     Copyright (C) by the MEGA-team.\n"
    "*     All rights reserved.\n"
    "*\n"
    "*\n"
    "*     This code implementation is the intellectual property of the \n"
    "*     MEGA-team at MPE.\n"
    "*\n"
    "*     By copying, distributing or modifying the Program (or any work\n"
    "*     based on the Program) you indicate your acceptance of this \n"
    "*     statement, and all its terms.\n"
    "*\n"
    "************************************************************************\n"
    "\n"
    "\n"
    "      SUBROUTINE UGEOM\n"
    "\n"
    "************************************************************************\n"
    "*\n"
    "*     Initializes the geometry\n"
    "*\n"
    "*     Author: This file has been automatically generated by the \n"
    "*             geometry-program GeoMega "<<g_VersionString<<"\n"
    "*\n"
    "************************************************************************\n"
    "\n"
    "      IMPLICIT NONE\n"
    "\n"
    "\n"
    "      INCLUDE 'common.f'\n"
    "\n"
    "\n"
    "\n"
    "      INTEGER IVOL\n"
    "      DIMENSION IVOL("<<MDVolume::m_IDCounter<<")\n"
    "      REAL UBUF\n"
    "      DIMENSION UBUF(2)\n\n"<<endl;

  FileStream<<WFS(Text.str().c_str());
  Text.str("");

  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    FileStream<<WFS(m_MaterialList[i]->GetGeant3DIM());
  }

  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    FileStream<<WFS(m_VolumeList[i]->GetGeant3DIM());
  }

  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    FileStream<<WFS(m_MaterialList[i]->GetGeant3DATA());
  }

  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    FileStream<<WFS(m_VolumeList[i]->GetGeant3DATA());
  }

  Text<<endl<<
    "      ZNMAT = "<<GetNMaterials()<<endl;
  FileStream<<WFS(Text.str().c_str());
  Text.str("");
  
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    FileStream<<WFS(m_MaterialList[i]->GetGeant3());
  }

  Text<<endl<<
    "      CALL GPART"<<endl<<
    "      CALL GPHYSI"<<endl<<endl;
  FileStream<<WFS(Text.str().c_str());
  Text.str("");

  FileStream<<WFS(m_WorldVolume->GetGeant3())<<endl;
  
  //FileStream.setf(ios_base::fixed, ios_base::floatfield);
  FileStream.setf(ios::fixed, ios::floatfield);
  //FileStream.precision(3);

  // Finally position the volumes
  MString Name, MotherName, CopyName;

  // Scan through the tree...
  int IDCounter = 1;
  FileStream<<WFS(m_WorldVolume->GetGeant3Position(IDCounter))<<endl;


  Text<<endl;
  Text<<"      CALL GGCLOS"<<endl;
  Text<<"      GEONAM = \""<<m_FileName<<"\""<<endl;

  double MinDist;
  MVector RSize = m_WorldVolume->GetSize();
  MinDist = RSize.X();
  if (RSize.Y() < MinDist) MinDist = RSize.Y();
  if (RSize.Z() < MinDist) MinDist = RSize.Z();
  Text<<"      MDIST = "<<MinDist<<endl;
  Text<<endl;

  Text<<"      SPHR = "<<m_SurroundingSphereRadius<<endl;
  Text<<"      SPHX = "<<m_SurroundingSpherePosition.X()<<endl;
  Text<<"      SPHY = "<<m_SurroundingSpherePosition.Y()<<endl;
  Text<<"      SPHZ = "<<m_SurroundingSpherePosition.Z()<<endl;
  Text<<"      SPHD = "<<m_SurroundingSphereDistance<<endl;

  Text<<endl;
  Text<<"      RETURN"<<endl;
  Text<<"      END"<<endl;;
  FileStream<<WFS(Text.str().c_str());
  Text.str("");

  FileStream.close();

  
  // open the geometry-file:
  // gcc 2.95.3: FileStream.open("detinit.f", ios::out, 0664);
  FileStream.open("detinit.f", ios_base::out);

  
  Text<<
    "************************************************************************\n"
    "*\n"
    "*     Copyright (C) by the MEGA-team.\n"
    "*     All rights reserved.\n"
    "*\n"
    "*\n"
    "*     This code implementation is the intellectual property of the \n"
    "*     MEGA-team at MPE.\n"
    "*\n"
    "*     By copying, distributing or modifying the Program (or any work\n"
    "*     based on the Program) you indicate your acceptance of this \n"
    "*     statement, and all its terms.\n"
    "*\n"
    "************************************************************************\n"
    "\n"
    "\n"
    "      SUBROUTINE DETINIT\n"
    "\n"
    "************************************************************************\n"
    "*\n"
    "*     Initializes the detectors\n"
    "*\n"
    "*     Author: This file has been automatically generated by the \n"
    "*             geometry-program GeoMega "<<g_VersionString<<"\n"
    "*\n"
    "************************************************************************\n"
    "\n"
    "      IMPLICIT NONE\n"
    "\n"
    "      INCLUDE 'common.f'\n"
    "\n"<<endl;

  Text<<"      NDET = "<<GetNDetectors()<<endl<<endl;

  if (GetNDetectors() > 0) {
    Text<<"      NSENS = "<<GetDetectorAt(0)->GetGlobalNSensitiveVolumes()<<endl<<endl;
  }

  // Write detectors
  for(unsigned int i = 0; i < GetNDetectors(); i++) {
    Text<<m_DetectorList[i]->GetGeant3();
  }


  // Write trigger conditions
  Text<<"      TNTRIG = "<<GetNTriggers()<<endl<<endl;
  for(unsigned int i = 0; i < GetNTriggers(); i++) {
    Text<<m_TriggerList[i]->GetGeant3(i+1);
  }

  Text<<endl;
  Text<<"      EVINFO = 1"<<endl;
  Text<<"      SNAM = '"<<m_Name<<"_"<<m_Version<<"'"<<endl<<endl;
  Text<<"      RETURN"<<endl;
  Text<<"      END"<<endl;

  FileStream<<WFS(Text.str().c_str());
  Text.str("");
  
  FileStream.close();
  
  // Clean up...
  m_WorldVolume->ResetCloneTemplateFlags();

  return true;
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


bool MDGeometry::WriteMGeantFiles(MString FilePrefix, bool StoreIAs, bool StoreVetoes)
{
  // This routine generates an intermediate geometry-file, which can be decoded
  // by MGEANT or MGGPOD
  //
  // Guest author: RMK
  
  MString FileName; 
  fstream FileStream;

  if (m_GeometryScanned == false) {
    Error("bool MDGeometry::WriteMGeantFiles()",
          "Geometry has to be scanned first");
    return false;
  }

  // Extract file part of pathname
  MString theFile = m_FileName;
  if (theFile.Contains("/")) {
    theFile.Remove(0,theFile.Last('/')+1);
  }

  // ---------------------------------------------------------------------------
  // Create the materials file: MEGA_materials.mat
  // ---------------------------------------------------------------------------
  //

  if (FilePrefix == "") {
    FileName = "materials.mat";
  } else {
    FileName = FilePrefix + ".mat";
  }

  // gcc 2.95.3: FileStream.open("MEGA_materials.mat", ios::out, 0664);
  FileStream.open(FileName, ios_base::out);
 
  FileStream<<
    "! +============================================================================+"<<endl<<
    "! MEGA_materials.mat         MGEANT/MGGPOD materials list file                  "<<endl<<
    "!                                                                               "<<endl<<
    "! Based on setup file: "<< theFile << endl <<
    "!                                                                               "<<endl<<
    "! Copyright (C) by the MEGA-team.                                     "<<endl<<
    "! All rights reserved.                                                          "<<endl<<
    "!                                                                               "<<endl<<
    "! Author: This file has been automatically generated by the                     "<<endl<<
    "!         geometry-program GeoMega (Version: "<<g_VersionString<<")"<<endl<<
    "! +============================================================================+"<<endl<<
    "! Built-in Materials (numbers 1-16) -->                                         "<<endl<<
    "!      hydrogen, deuterium, helium, lithium, beryllium, carbon, nitrogen,       "<<endl<<
    "!      neon, aluminum, iron, copper, tungsten, lead, uranium, air, vacuum       "<<endl<<
    "!                                                                               "<<endl<<
    "! Format for User Materials -->                                                 "<<endl<<
    "!  mate  imate  chmat  A  Z  dens  radl  absl(=1.0)  nwbuf <CR>                 "<<endl<<
    "!        [ubuf]                                                                 "<<endl<<
    "!  mixt  imate  chmat  nlmat  dens <CR>                                         "<<endl<<
    "!        A(1)  Z(1)  wmat(1)                                                    "<<endl<<
    "!        ...                                                                    "<<endl<<
    "!        A(N)  Z(N)  wmat(N)                                                    "<<endl<<
    "!       (wmat = prop by number(nlmat<0) or weight(nlmat>0); N = abs(nlmat))     "<<endl<<
    "!                                                                               "<<endl<<
    "!  umix  imate  chmat  nlmat  dens <CR>                                         "<<endl<<
    "!        A(1)  elenam(1)  wmat(1)                                               "<<endl<<
    "!        ...                                                                    "<<endl<<
    "!        A(N)  elenam(N)  wmat(N)                                               "<<endl<<
    "!       (wmat = prop by number(nlmat<0) or weight(nlmat>0); N = abs(nlmat),     "<<endl<<
    "!        use A(i) == 0.0 to select natural isotopic abundance mixture)          "<<endl<<
    "!                                                                               "<<endl<<
    "! +============================================================================+"<<endl;
  FileStream<<endl<<endl;

  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    FileStream << m_MaterialList[i]->GetMGeant();
  }

  FileStream << endl << "end" << endl;

  FileStream.close();

  // ---------------------------------------------------------------------------
  // Create the tracking media file: MEGA_media.med
  // ---------------------------------------------------------------------------  

  if (FilePrefix == "") {
    FileName = "media.med";
  } else {
    FileName = FilePrefix + ".med";
  }

  // gcc 2.95.3: FileStream.open("MEGA_media.med", ios::out, 0664);
  FileStream.open(FileName, ios_base::out);

  FileStream<<
    "! +============================================================================+"<<endl<<
    "! MEGA_media.med             MGEANT/MGGPOD tracking media list file             "<<endl<<
    "!                                                                               "<<endl<<
    "! Based on setup file: " << theFile << endl <<
    "!                                                                               "<<endl<<
    "! Copyright (C) by the MEGA-team.                                     "<<endl<<
    "! All rights reserved.                                                          "<<endl<<
    "!                                                                               "<<endl<<
    "! Author: This file has been automatically generated by the                     "<<endl<<
    "!         geometry-program GeoMega (Version: "<<g_VersionString<<")"<<endl<<
    "! +============================================================================+"<<endl<<
    "!  Format for User Media -->                                                    "<<endl<<
    "!  tmed itmed chmed chmat pass/dete/shld/mask ->                                "<<endl<<
    "!       ->ifield fieldm *tmaxfd *stemax *deemax epsil *stmin nwbuf <CR>         "<<endl<<
    "!       [ubuf]                                                                  "<<endl<<
    "!       (* set negative for automatic calculation of tracking parameter)        "<<endl<<
    "!  tpar chmed chpar parval                                                      "<<endl<<
    "! +============================================================================+"<<endl;
  FileStream<<endl<<endl;
  FileStream<<"! Some additional comments:"<<endl;
  FileStream<<"! Adjust material sorting by hand to dete, pass, shld, or mask"<<endl;
  FileStream<<"! as required - see MGEANT manual!"<<endl;
  FileStream<<endl<<endl;


  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    // Check if a detector consists of this material:
    int Sensitivity = 0;
    for (unsigned int d = 0; d < GetNDetectors(); ++d) {
      for (unsigned int v = 0; v < GetDetectorAt(d)->GetNSensitiveVolumes(); ++v) {
        if (GetDetectorAt(d)->GetSensitiveVolume(v)->GetMaterial()->GetName() == GetMaterialAt(i)->GetName()) {
          if (GetDetectorAt(d)->GetDetectorType() == MDDetector::c_ACS) {
            Sensitivity = 2;
          } else {
            Sensitivity = 1;
          }
        }
      }
    }
    FileStream << m_MaterialList[i]->GetMGeantTmed(Sensitivity);
  }

  FileStream << endl << "end" << endl;

  FileStream.close();


  // ---------------------------------------------------------------------------
  // Create the geometry file: MEGA_setup.geo
  // ---------------------------------------------------------------------------

  if (FilePrefix == "") {
    FileName = "setup.geo";
  } else {
    FileName = FilePrefix + ".geo";
  }

  // gcc 2.95.3: FileStream.open("MEGA_setup.geo", ios::out, 0664);
  FileStream.open(FileName, ios_base::out);

  FileStream<<
    "! +============================================================================+"<<endl<<
    "! MEGA_setup.geo             MGEANT/MGGPOD geometry list file                   "<<endl<<
    "!                                                                               "<<endl<<
    "! Based on setup file: " << theFile << endl <<
    "!                                                                               "<<endl<<
    "! Copyright (C) by the MEGA-team.                                     "<<endl<<
    "! All rights reserved.                                                          "<<endl<<
    "!                                                                               "<<endl<<
    "! Author: This file has been automatically generated by the                     "<<endl<<
    "!         geometry-program GeoMega (Version: "<<g_VersionString<<")"<<endl<<
    "! +============================================================================+"<<endl<<
    "! Format for Shape and Position Parameters Input -->                            "<<endl<<
    "! rotm irot theta1 phi1 theta2 phi2 theta3 phi3                                 "<<endl<<
    "! volu chname chshap chmed npar <CR>                                            "<<endl<<
    "!      [parms]                                                                  "<<endl<<
    "! posi chname copy chmoth x y z irot chonly                                     "<<endl<<
    "! posp chname copy chmoth x y z irot chonly npar <CR>                           "<<endl<<
    "!      parms                                                                    "<<endl<<
    "! divn chname chmoth ndiv iaxis                                                 "<<endl<<
    "! dvn2 chname chmoth ndiv iaxis co chmed                                        "<<endl<<
    "! divt chname chmoth step iaxis chmed ndvmx                                     "<<endl<<
    "! dvt2 chname chmoth step iaxis co chmed ndvmx                                  "<<endl<<
    "! divx chname chmoth ndiv iaxis step co chmed ndvmx                             "<<endl<<
    "! satt chname chiatt ival                                                       "<<endl<<
    "! tree ndets firstdetnum detlvl shldlvl masklvl                                 "<<endl<<
    "! Euclid support =>                                                             "<<endl<<
    "! eucl filename                                                                 "<<endl<<
    "! ROTM irot theta1 phi1 theta2 phi2 theta3 phi3                                 "<<endl<<
    "! VOLU 'chname' 'chshap' numed npar <CR>                                        "<<endl<<
    "!      [parms]                                                                  "<<endl<<
    "! POSI 'chname' copy 'chmoth' x y z irot 'chonly'                               "<<endl<<
    "! POSP 'chname' copy 'chmoth' x y z irot 'chonly' npar <CR>                     "<<endl<<
    "!      parms                                                                    "<<endl<<
    "! DIVN 'chname' 'chmoth' ndiv iaxis                                             "<<endl<<
    "! DVN2 'chname' 'chmoth' ndiv iaxis co numed                                    "<<endl<<
    "! DIVT 'chname' 'chmoth' step iaxis numed ndvmx                                 "<<endl<<
    "! DVT2 'chname' 'chmoth' step iaxis co numed ndvmx                              "<<endl<<
    "! +============================================================================+"<<endl;
  FileStream<<endl<<endl;


  // Tree command - not needed for ACT / INIT - but do not delete
  // FileStream << "! Tree Structure (must be modified manually!)" << endl;
  // FileStream << "tree 1 1 1 1 1" << endl << endl;

  // Volume tree data
  FileStream << m_WorldVolume->GetMGeant() << endl;
  
  // Volume position data
  int IDCounter = 1;
  FileStream << m_WorldVolume->GetMGeantPosition(IDCounter) << endl;

  FileStream << endl << "end" << endl;

  FileStream.close();

  // ---------------------------------------------------------------------------
  // Create the detector initialization file: detector.det
  // ---------------------------------------------------------------------------

  if (FilePrefix == "") {
    FileName = "detector.det";
  } else {
    FileName = FilePrefix + "_detector.det";
  }
  
  // open the geometry-file:
  FileStream.open(FileName, ios_base::out);
  
  FileStream<<
    "! +============================================================================+"<<endl<<
    "! detector.det         MGGPOD-MEGALIB-extension detector & trigger description  "<<endl<<
    "!                                                                               "<<endl<<
    "! Based on setup file: " << theFile << endl <<
    "!                                                                               "<<endl<<
    "! Copyright (C) by the MEGA-team.                                     "<<endl<<
    "! All rights reserved.                                                          "<<endl<<
    "!                                                                               "<<endl<<
    "! Author: This file has been automatically generated by the                     "<<endl<<
    "!         geometry-program GeoMega (Version: "<<g_VersionString<<")"<<endl<<
    "! +============================================================================+"<<endl<<
    "\n"<<endl;
  
  FileStream<<"NDET "<<GetNDetectors()<<endl<<endl;

  if (GetNDetectors() > 0) {
    FileStream<<"NSEN "<<GetDetectorAt(0)->GetGlobalNSensitiveVolumes()<<endl<<endl;
  }

  // Write detectors
  for(unsigned int i = 0; i < GetNDetectors(); i++) {
    FileStream<<m_DetectorList[i]->GetMGeant();
  }

  // Write trigger conditions
  FileStream<<"NTRG "<<GetNTriggers()<<endl<<endl;
  for(unsigned int i = 0; i < GetNTriggers(); i++) {
    FileStream<<m_TriggerList[i]->GetMGeant(i+1);
  }

 FileStream<<endl;
  FileStream<<"END"<<endl;
  FileStream.close();

  if (FilePrefix == "") {
    FileName = "megalib.ini";
  } else {
    FileName = FilePrefix + "_megalib.ini";
  }
  
  // open the geometry-file:
  FileStream.open(FileName, ios_base::out);
  
  FileStream<<
    "! +============================================================================+"<<endl<<
    "! megalib.ini                       MGGPOD-MEGALIB-extension setup input file   "<<endl<<
    "!                                                                               "<<endl<<
    "! Based on setup file: " << theFile << endl <<
    "!                                                                               "<<endl<<
    "! Copyright (C) by the MEGA-team.                                     "<<endl<<
    "! All rights reserved.                                                          "<<endl<<
    "!                                                                               "<<endl<<
    "! Author: This file has been automatically generated by the                     "<<endl<<
    "!         geometry-program GeoMega (Version: "<<g_VersionString<<")"<<endl<<
    "! +============================================================================+"<<endl<<
    "\n"<<endl;

  FileStream<<endl;
  FileStream<<"GNAM "<<theFile<<endl;
  FileStream<<endl;
  FileStream<<"VERS 24"<<endl;
  FileStream<<endl;
  if (StoreIAs == true) {
    FileStream<<"EIFO 1"<<endl;
  } else {
    FileStream<<"EIFO 0"<<endl;
  }
  if (StoreVetoes == true) {
    FileStream<<"VIFO 1"<<endl;
  } else {
    FileStream<<"VIFO 0"<<endl;
  }
  FileStream<<endl;
  FileStream<<"END"<<endl;
  FileStream.close();
 

  // Clean up...
  m_WorldVolume->ResetCloneTemplateFlags();

  return true;
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


MString MDGeometry::CreateShortName(MString Name, unsigned int Length, bool Fill, bool KeepKeywords)
{
  // Create a Length character short name out of name, which is not in one of
  // the lists (detectors, materials, volumes)
  // e.g. if there exist two volumes, TrackerBox and TrackerStalk,
  // the first one is called TRAC, the second TRA0 

  if (m_IgnoreShortNames == true) {
    return "IGNO";
  }

  MString SN;
  Name.ToLower();

  bool FoundMicsetKeyword = false;
  MString MicsetKeyword = "_micset";
  if (KeepKeywords == true && Name.Contains(MicsetKeyword) == true) {
    //mout<<"   *** Info ***"<<endl;
    //mout<<"Special MGGPOD material keyword "<<MicsetKeyword<<" found!"<<endl;
    FoundMicsetKeyword = true;
    Name.ReplaceAll(MicsetKeyword, "");
    Length -= MicsetKeyword.Length();
  }
  bool FoundGeRecoilKeyword = false;
  MString GeRecoilKeyword = "_ge_recoil";
  if (KeepKeywords == true && Name.Contains(GeRecoilKeyword) == true) {
    //mout<<"   *** Info ***"<<endl;
    //mout<<"Special MGGPOD material keyword "<<GeRecoilKeyword<<" found!"<<endl;
    FoundGeRecoilKeyword = true;
    Name.ReplaceAll(GeRecoilKeyword, "");
    Length -= GeRecoilKeyword.Length();
  }
  bool FoundSiRecoilKeyword = false;
  MString SiRecoilKeyword = "_si_recoil";
  if (KeepKeywords == true && Name.Contains(SiRecoilKeyword) == true) {
    //mout<<"   *** Info ***"<<endl;
    //mout<<"Special MGGPOD material keyword "<<SiRecoilKeyword<<" found!"<<endl;
    FoundSiRecoilKeyword = true;
    Name.ReplaceAll(SiRecoilKeyword, "");
    Length -= SiRecoilKeyword.Length();
  }
  bool FoundCZTRecoilKeyword = false;
  MString CZTRecoilKeyword = "_czt_recoil";
  if (KeepKeywords == true && Name.Contains(CZTRecoilKeyword) == true) {
    //mout<<"   *** Info ***"<<endl;
    //mout<<"Special MGGPOD material keyword "<<CZTRecoilKeyword<<" found!"<<endl;
    FoundCZTRecoilKeyword = true;
    Name.ReplaceAll(CZTRecoilKeyword, "");
    Length -= CZTRecoilKeyword.Length();
  }
  bool FoundAddRecoilKeyword = false;
  MString AddRecoilKeyword = "_addrec";
  if (KeepKeywords == true && Name.Contains(AddRecoilKeyword) == true) {
    mout<<"   *** Info ***"<<endl;
    mout<<"Special MGGPOD material keyword "<<AddRecoilKeyword<<" found!"<<endl;
    FoundAddRecoilKeyword = true;
    Name.ReplaceAll(AddRecoilKeyword, "");
    Length -= AddRecoilKeyword.Length();
  }
  

  // Remove everything which is not alphanumerical from the name:
  for (size_t i = 0; i < Name.Length(); ++i) {
    if (isalnum(Name[i]) == false && 
        Name[i] != '_' && 
        Name[i] != '-') {
      Name.Remove(i, 1);
    }
  }

  // Keep only first "Length" charcters:callgrind.out.16396
  Name = Name.GetSubString(0, Length);

  if (Length < 4) Length = 4;

  // if we are smaller, we can try to expand the name:
  if (Name.Length() < Length) {
    if (ShortNameExists(Name) == true) {
      unsigned int MaxExpand = 0;
      if (pow(10.0, (int) (Length-Name.Length())) - 1 > numeric_limits<unsigned int>::max()) {
        MaxExpand = numeric_limits<unsigned int>::max();
      } else {
        MaxExpand = (unsigned int) (pow(10.0, (int) (Length-Name.Length())) - 1.0);
      }
      for (unsigned int i = 1; i < MaxExpand; ++i) {
        SN = Name;
        SN += i;     
        if (ShortNameExists(SN) == false) {
          Name = SN;
          break;
        }
      }
    }
  }
  
  // If we still haven't found a suited short name: 
  if (ShortNameExists(Name) == true) {
    
    // Step one: test the first "Length" letters
    SN = Name.Replace(Length, Name.Length() - Length, ""); 
    if (ShortNameExists(SN) == true) {
      // Step three: Replace the last character by a number ...
      for (int j = (int) '0'; j < (int) '9'; j++) {
        SN[Length-1] = (char) j;
        if (ShortNameExists(SN) == false) {
          break;
        }
      }
    }
    if (ShortNameExists(SN) == true) {      
      // Step four: Replace the last two characters by a numbers ...
      for (int i = (int) '0'; i < (int) '9'; i++) {
        for (int j = (int) '0'; j < (int) '9'; j++) {
          SN[Length-2] = (char) i;
          SN[Length-1] = (char) j;
          if (ShortNameExists(SN) == false) {
            break;
          }
        }
        if (ShortNameExists(SN) == false) {
          break;
        }
      }
    }
    if (ShortNameExists(SN) == true) {      
      // Step five: Replace the last three characters by a numbers ...
      for (int k = (int) '0'; k < (int) '9'; k++) {
        for (int i = (int) '0'; i < (int) '9'; i++) {
          for (int j = (int) '0'; j < (int) '9'; j++) {
            SN[Length-3] = (char) k;
            SN[Length-2] = (char) i;
            SN[Length-1] = (char) j;
            if (ShortNameExists(SN) == false) {
              break;
            }
          }
          if (ShortNameExists(SN) == false) {
            break;
          }
        }
        if (ShortNameExists(SN) == false) {
          break;
        }
      }
    }
    if (ShortNameExists(SN) == true) {
      // That's too much:
      merr<<"You have too many volumes starting with "<<Name<<endl;
      merr<<"Please add \"IgnoreShortNames true\" into your geometry file!"<<endl;
      merr<<"As a result you are not able to do Geant3/MGEANT/MGGPOS simulations"<<endl;
      m_IgnoreShortNames = true;
      return "IGNO";
    }                    
    Name = SN;
  }

  if (KeepKeywords == true && FoundMicsetKeyword == true) {
    Name += MicsetKeyword;
    Length += MicsetKeyword.Length();
  }
  
  if (KeepKeywords == true && FoundGeRecoilKeyword == true) {
    Name += GeRecoilKeyword;
    Length += GeRecoilKeyword.Length();
  }
  
  if (KeepKeywords == true && FoundSiRecoilKeyword == true) {
    Name += SiRecoilKeyword;
    Length += SiRecoilKeyword.Length();
  }
  
  if (KeepKeywords == true && FoundCZTRecoilKeyword == true) {
    Name += CZTRecoilKeyword;
    Length += CZTRecoilKeyword.Length();
  }
  
  if (KeepKeywords == true && FoundAddRecoilKeyword == true) {
    Name += AddRecoilKeyword;
    Length += AddRecoilKeyword.Length();
  }
  
  // We always need a name which has exactly Length characters
  while (Name.Length() < Length) {
    if (Fill == true) {
      Name += '_';
    } else {
      Name += ' ';
    }
  }

  return Name;
}


////////////////////////////////////////////////////////////////////////////////


bool MDGeometry::ShortNameExists(MString Name)
{
  // Check all lists if "Name" is already listed 

  for (unsigned int i = 0; i < GetNVolumes(); i++) {
    if (Name.AreIdentical(m_VolumeList[i]->GetName(), true) == true ||
        Name.AreIdentical(m_VolumeList[i]->GetShortName(), true) == true) {
      return true;
    }
  }
  
  // Test materials
  for (unsigned int i = 0; i < GetNMaterials(); i++) {
    if (Name.AreIdentical(m_MaterialList[i]->GetName(), true) == true || 
        Name.AreIdentical(m_MaterialList[i]->GetShortName(), true) == true || 
        Name.AreIdentical(m_MaterialList[i]->GetOriginalMGeantShortName(), true) == true) {
      return true;
    }
  }
  
  // Test detectors
  for (unsigned int i = 0; i < GetNDetectors(); i++) {
    if (Name.AreIdentical(m_DetectorList[i]->GetName(), true) == true || 
        Name.AreIdentical(m_DetectorList[i]->GetShortNameDivisionX(), true) == true || 
        Name.AreIdentical(m_DetectorList[i]->GetShortNameDivisionY(), true) == true || 
        Name.AreIdentical(m_DetectorList[i]->GetShortNameDivisionZ(), true) == true) {
      return true;
    }
  }
  

  // Test triggers
  for (unsigned int i = 0; i < GetNTriggers(); i++) {
   if (Name.AreIdentical(m_TriggerList[i]->GetName(), true) == true) {
      return true;
    }
  }

  return false;
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
  // Add the name of an included file

  if (IsIncluded(FileName) == false) {
    m_IncludeList.push_back(FileName);
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


MString MDGeometry::ToString()
{
  // 

  unsigned int i;
  ostringstream out;

  out<<endl<<"Description of geometry: "<<m_Name<<", version: "<<m_Version<<endl;
  
  out<<endl<<endl<<"Description of volumes:"<<endl;
  for (i = 0; i < m_VolumeList.size(); ++i) {
    out<<m_VolumeList[i]->GetName()<<endl;;
    out<<m_VolumeList[i]->ToString()<<endl;;
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
    mout<<out.str()<<endl;
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

  MDVolume* Volume = GetVolume(Name);

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
  m_WorldVolume->GetNPlacements(Volume, Placements, TreeDepth);

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
  FileName += "/DelMe.source";

  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    mout<<"   ***  Error  ***"<<endl;
    mout<<"Unable to create cosima source file for cross section creation"<<endl;
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
  gSystem->Exec(MString("cosima ") + FileName);
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
    mout<<"Cannot load create cross section files correctly."<<endl;
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
