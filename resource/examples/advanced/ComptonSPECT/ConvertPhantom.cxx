/* 
 * ConvertPhantom.cxx
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

// Standard
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MParser.h"


/******************************************************************************/

class ConvertPhantom
{
public:
  /// Default constructor
  ConvertPhantom();
  /// Default destructor
  ~ConvertPhantom();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  
private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// 
  MString m_FileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
ConvertPhantom::ConvertPhantom() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
ConvertPhantom::~ConvertPhantom()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool ConvertPhantom::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ConvertPhantom <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name (*.mhd)"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;
  
  string Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    
    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
        (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
        }
    } 
    // Multiple arguments template
    /*
     *    else if (Option == "-??") {
     *      if (!((argc > i+2) && 
     *            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
     *            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
     *        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
     *        cout<<Usage.str()<<endl;
     *        return false;
  }
  }
  */
    
    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // Check if the files exist:
  if (MFile::Exists(m_FileName) == false) {
    cout<<"Error: Cannot find mhd file"<<endl;
    return false;
  }
  
  
  return true;
}



/******************************************************************************
 * Convert the phantom into MEGAlib/Geomega format
 */
bool ConvertPhantom::Analyze()
{
  if (m_Interrupt == true) return false;
  
  MParser Mhd;
  Mhd.Open(m_FileName);
  
  MTokenizer* T;
  
  T = Mhd.GetTokenizerAt(0);
  if (T->GetTokenAt(0) != "NDims") {
    cout<<"Error: NDims keyword not found at correct position"<<endl;
    return false;
  }
  int NDims = T->GetTokenAtAsInt(2);
  cout<<"NDims: "<<NDims<<endl;
  
  T = Mhd.GetTokenizerAt(1);
  vector<int> Dims;
  if (T->GetTokenAt(0) != "DimSize") {
    cout<<"Error: DimSize keyword not found at correct position"<<endl;
    return false;
  }
  unsigned int NBins = 1;
  for (int i = 0; i < NDims; ++i) {
    Dims.push_back(T->GetTokenAtAsInt(2 + i));
    NBins *= Dims.back();
    cout<<"Dim "<<i+1<<": "<<Dims.back()<<endl;
  }
  
  T = Mhd.GetTokenizerAt(3);
  vector<double> Spacing;
  if (T->GetTokenAt(0) != "ElementSpacing") {
    cout<<"Error: ElementSpacing keyword not found at correct position"<<endl;
    return false;
  }
  for (int i = 0; i < NDims; ++i) {
    Spacing.push_back(T->GetTokenAtAsDouble(2 + i)/10);
    cout<<"Spacing "<<i+1<<": "<<Spacing.back()<<endl;
  }
  
  T = Mhd.GetTokenizerAt(5);
  MString RawFile;
  if (T->GetTokenAt(0) != "ElementDataFile") {
    cout<<"Error: ElementDataFile keyword not found at correct position"<<endl;
    return false;
  }
  RawFile = T->GetTokenAt(2);
  cout<<"Raw file name: "<<RawFile<<endl;
  
  ifstream Raw;
  Raw.open(RawFile.Data(), ios::in | ios::binary);
  
  short Value;
  vector<short> Matrix;
  if (Raw.is_open() == true) {
    for (unsigned int i = 0; i < NBins; ++i) {
      Raw.read((char*) &Value, sizeof(Value));
      Value = (Value>>8) | (Value<<8);
      Matrix.push_back(Value);
    }
  } else {
    cout<<"Unable to open file: "<<RawFile<<endl;
  }
  Raw.close();
  
  if (Matrix.size() != NBins) {
    cout<<"Matrix size "<<Matrix.size()<<" != "<<NBins<<endl;
    return false;
  }
  if (NDims == 3) {
    TH3D* Hist = new TH3D("3D Matrix", "3D Matrix", Dims[0], 0, Spacing[0]*Dims[0], Dims[1], 0, Spacing[1]*Dims[1], Dims[2], 0, Spacing[2]*Dims[2]);
    for (int z = 0; z < Dims[2]; ++z) {
      for (int y = 0; y < Dims[1]; ++y) {
        for (int x = 0; x < Dims[0]; ++x) {
          Hist->SetBinContent(x, y, z, Matrix[x + y*Dims[0] + z*Dims[0]*Dims[1]]);
        }
      }   
    }
    Hist->Draw();
  }
  
  int VoxelCount = 0;
  
  // Create Geomega geometry
  ofstream geo;
  geo.open("Phantom.geo");
  
  geo<<"Volume Phantom"<<endl;
  geo<<"Phantom.Material Vacuum"<<endl;
  geo<<"Phantom.Visibility 0"<<endl;
  geo<<"Phantom.Shape BOX "<<0.5*Dims[0]*Spacing[0]<<" "<<0.5*Dims[1]*Spacing[1]<<" "<<0.5*Dims[2]*Spacing[2]<<endl;
  geo<<endl;
  
  vector<short> MaterialIDs;
  string Mat_name("");
  for (int z = 0; z < Dims[2]; ++z) {
    
    bool FoundLine = false;
    for (int y = 0; y < Dims[1]; ++y) {
      
      bool FoundVoxel = false;
      for (int x = 0; x < Dims[0]; ++x) {
        if (Matrix[x + y*Dims[0] + z*Dims[0]*Dims[1]] > 0) {
          FoundVoxel = true;
          VoxelCount++;
          short MaterialID = Matrix[x + y*Dims[0] + z*Dims[0]*Dims[1]];
          // Zoom forward and check how many have the same ID
          int NIdentical = 1;
          for (int xi = x+1; xi < Dims[0]; ++xi) {
            if (Matrix[xi + y*Dims[0] + z*Dims[0]*Dims[1]] != MaterialID) break;
            NIdentical++;
          }
          geo<<"Volume Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<endl;
          
          switch ( MaterialID)
          {
            case 2: Mat_name="SpineBone"; break;
            case 10: Mat_name="Muscle"; break;
            case 11: Mat_name="Muscle"; break;
            case 12: Mat_name="Muscle"; break;
            case 13: Mat_name="Air"; break;
            case 14: Mat_name="Muscle"; break;
            case 15: Mat_name="Muscle"; break;
            case 18: Mat_name="Muscle"; break;
            case 39: Mat_name="Lung"; break;
            default: Mat_name="Water"; break;
          }
          
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Material "<<Mat_name<<endl;
          bool MaterialFound = false;
          for (unsigned int m = 0; m < MaterialIDs.size(); ++m) {
            if (MaterialIDs[m] == MaterialID) {
              MaterialFound = true;
              break;
            }
          }
          if (MaterialFound == false) {
            MaterialIDs.push_back(MaterialID);
          }
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Visibility 1"<<endl;
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Shape BOX "<<0.5*NIdentical*Spacing[0]<<" "<<0.5*Spacing[1]<<" "<<0.5*Spacing[2]<<endl;
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Position "<<(-0.5*Dims[0]+ x + 0.5*NIdentical)*Spacing[0]<<" 0.0 0.0"<<endl;
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Mother Phantom_Line_"<<z<<"_"<<y<<endl;
          geo<<endl;
          
          x += (NIdentical-1);
          VoxelCount += NIdentical-1;
        }
      }
      
      if (FoundVoxel == true) {
        FoundLine = true;
        geo<<"Volume Phantom_Line_"<<z<<"_"<<y<<endl;
        geo<<"Phantom_Line_"<<z<<"_"<<y<<".Material Vacuum"<<endl;
        geo<<"Phantom_Line_"<<z<<"_"<<y<<".Visibility 0"<<endl;
        geo<<"Phantom_Line_"<<z<<"_"<<y<<".Shape BOX "<<0.5*Dims[0]*Spacing[0]<<" "<<0.5*Spacing[1]<<" "<<0.5*Spacing[2]<<endl;
        geo<<"Phantom_Line_"<<z<<"_"<<y<<".Position 0.0 "<<(-(0.5*(Dims[1]-1.0))+y)*Spacing[1]<<" 0.0"<<endl;
        geo<<"Phantom_Line_"<<z<<"_"<<y<<".Mother Phantom_Plane_"<<z<<endl;
        geo<<endl;
      }
    }
    
    if (FoundLine == true) {
      geo<<"Volume Phantom_Plane_"<<z<<endl;
      geo<<"Phantom_Plane_"<<z<<".Material Vacuum"<<endl;
      geo<<"Phantom_Plane_"<<z<<".Visibility 0"<<endl;
      geo<<"Phantom_Plane_"<<z<<".Shape BOX "<<0.5*Dims[0]*Spacing[0]<<" "<<0.5*Dims[1]*Spacing[1]<<" "<<0.5*Spacing[2]<<endl;
      geo<<"Phantom_Plane_"<<z<<".Position 0.0 0.0 "<<(-(0.5*(Dims[2]-1.0))+z)*Spacing[2]<<endl;
      geo<<"Phantom_Plane_"<<z<<".Mother Phantom"<<endl;
      geo<<endl;
    }
    
  }
  
  // Write materials
  geo<<"Material Water"<<endl;
  geo<<"Water.Density 1.0    // Attention: Density fixed to 1.0"<<endl;
  geo<<"Water.Component 16  8  1"<<endl;
  geo<<"Water.Component  1.01  1  2"<<endl<<endl;
  
  geo<<"Material RibBone"<<endl;
  geo<<"RibBone"<<".Density 1.92"<<endl;
  geo<<"RibBone.ComponentByMass 1.01  1  0.034  // H"<<endl;
  geo<<"RibBone.ComponentByMass 12.01  6.  0.155  // C"<<endl;
  geo<<"RibBone.ComponentByMass 14.01  7  0.042  // N"<<endl;
  geo<<"RibBone.ComponentByMass 16.  8.  0.435  // O"<<endl;
  geo<<"RibBone.ComponentByMass 22.99  11  0.001  // Na"<<endl;
  geo<<"RibBone.ComponentByMass 24.305  12  0.002  // Mg"<<endl;
  geo<<"RibBone.ComponentByMass 30.97  15  0.103  // P"<<endl;
  geo<<"RibBone.ComponentByMass 32.066  16  0.003  // S"<<endl;
  geo<<"RibBone.ComponentByMass 40.08  20  0.225  // Ca"<<endl<<endl;
  
  geo<<"Material SpineBone"<<endl;
  geo<<"SpineBone"<<".Density 1.42"<<endl;
  geo<<"SpineBone.ComponentByMass 1.01  1  0.063  // H"<<endl;
  geo<<"SpineBone.ComponentByMass 12.01  6.  0.261  // C"<<endl;
  geo<<"SpineBone.ComponentByMass 14.01  7  0.039  // N"<<endl;
  geo<<"SpineBone.ComponentByMass 16.  8.  0.436  // O"<<endl;
  geo<<"SpineBone.ComponentByMass 22.99  11  0.001  // Na"<<endl;
  geo<<"SpineBone.ComponentByMass 24.305  12  0.001  // Mg"<<endl;
  geo<<"SpineBone.ComponentByMass 30.97  15  0.061  // P"<<endl;
  geo<<"SpineBone.ComponentByMass 32.066  16  0.003  // S"<<endl;
  geo<<"SpineBone.ComponentByMass 35.45  17  0.001  // Cl"<<endl;
  geo<<"SpineBone.ComponentByMass 39.098  16  0.001  // K"<<endl;
  geo<<"SpineBone.ComponentByMass 40.08  20  0.133  // Ca"<<endl<<endl;
  
  geo<<"Material Muscle"<<endl;
  geo<<"Muscle"<<".Density 1.05"<<endl;
  geo<<"Muscle.ComponentByMass 1.01  1  0.102  // H"<<endl;
  geo<<"Muscle.ComponentByMass 12.01  6.  0.143  // C"<<endl;
  geo<<"Muscle.ComponentByMass 14.01  7  0.034  // N"<<endl;
  geo<<"Muscle.ComponentByMass 16.  8.  0.71  // O"<<endl;
  geo<<"Muscle.ComponentByMass 22.99  11  0.001  // Na"<<endl;
  geo<<"Muscle.ComponentByMass 30.97  15  0.002  // P"<<endl;
  geo<<"Muscle.ComponentByMass 32.066  16  0.003  // S"<<endl;
  geo<<"Muscle.ComponentByMass 35.45  17  0.001  // Cl"<<endl;
  geo<<"Muscle.ComponentByMass 39.098  16  0.004  // K"<<endl<<endl;
  
  geo<<"Material Lung"<<endl;
  geo<<"Lung"<<".Density 0.26"<<endl;
  geo<<"Lung.ComponentByMass 1.01  1  0.103  // H"<<endl;
  geo<<"Lung.ComponentByMass 12.01  6.  0.105  // C"<<endl;
  geo<<"Lung.ComponentByMass 14.01  7  0.031  // N"<<endl;
  geo<<"Lung.ComponentByMass 16.  8.  0.749  // O"<<endl;
  geo<<"Lung.ComponentByMass 22.99  11  0.002  // Na"<<endl;
  geo<<"Lung.ComponentByMass 30.97  15  0.002  // P"<<endl;
  geo<<"Lung.ComponentByMass 32.066  16  0.003  // S"<<endl;
  geo<<"Lung.ComponentByMass 35.45  17  0.003  // Cl"<<endl;
  geo<<"Lung.ComponentByMass 39.098  16  0.002  // K"<<endl<<endl;
  
  geo<<"Material Brain"<<endl;
  geo<<"Brain"<<".Density 1.04"<<endl;
  geo<<"Brain.ComponentByMass 1.01  1  0.107  // H"<<endl;
  geo<<"Brain.ComponentByMass 12.01  6.  0.145  // C"<<endl;
  geo<<"Brain.ComponentByMass 14.01  7  0.022  // N"<<endl;
  geo<<"Brain.ComponentByMass 16.  8.  0.712  // O"<<endl;
  geo<<"Brain.ComponentByMass 22.99  11  0.002  // Na"<<endl;
  geo<<"Brain.ComponentByMass 30.97  15  0.004  // P"<<endl;
  geo<<"Brain.ComponentByMass 32.066  16  0.002  // S"<<endl;
  geo<<"Brain.ComponentByMass 35.45  17  0.003  // Cl"<<endl;
  geo<<"Brain.ComponentByMass 39.098  16  0.003  // K"<<endl<<endl;
  
  geo<<"Material Skull"<<endl;
  geo<<"Skull"<<".Density 1.61"<<endl;
  geo<<"Skull.ComponentByMass 1.01  1  0.05  // H"<<endl;
  geo<<"Skull.ComponentByMass 12.01  6.  0.212  // C"<<endl;
  geo<<"Skull.ComponentByMass 14.01  7  0.04  // N"<<endl;
  geo<<"Skull.ComponentByMass 16.  8.  0.435  // O"<<endl;
  geo<<"Skull.ComponentByMass 22.99  11  0.001  // Na"<<endl;
  geo<<"Skull.ComponentByMass 24.305  12  0.002  // Mg"<<endl;
  geo<<"Skull.ComponentByMass 30.97  15  0.081  // P"<<endl;
  geo<<"Skull.ComponentByMass 32.066  16  0.003  // S"<<endl;
  geo<<"Skull.ComponentByMass 40.08  20  0.176  // Ca"<<endl<<endl;
  
  // geo<<endl;
  
  
  geo.close();
  
  cout<<"Voxel count: "<<VoxelCount<<endl;
  
  return true;
}


/******************************************************************************/

ConvertPhantom* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);
  
  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("ConvertPhantom", "a ConvertPhantom example program");
  
  TApplication ConvertPhantomApp("ConvertPhantomApp", 0, 0);
  
  g_Prg = new ConvertPhantom();
  
  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 
  
  ConvertPhantomApp.Run();
  
  cout<<"Program exited normally!"<<endl;
  
  return 0;
}


/*
 * ConvertPhantom: the end...
 ******************************************************************************/
