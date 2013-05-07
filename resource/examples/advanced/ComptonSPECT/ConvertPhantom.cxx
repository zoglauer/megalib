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
  /// Analyze what eveer needs to be analyzed...
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
		else if (Option == "-??") {
			if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
				cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
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
    for (int x = 0; x < Dims[0]; ++x) {
      for (int y = 0; y < Dims[1]; ++y) {
        for (int z = 0; z < Dims[2]; ++z) {
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
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Material Mat_"<<MaterialID<<endl;
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
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Position "<<(-(0.5*(Dims[0]-1.0)) + x + 0.5*(NIdentical-1.0))*Spacing[0]<<" 0.0 0.0"<<endl;
          geo<<"Phantom_Voxel_"<<z<<"_"<<y<<"_"<<x<<".Mother Phantom_Line_"<<z<<"_"<<y<<endl;
          geo<<endl;
          
          x += (NIdentical-1);
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
  for (unsigned int m = 0; m < MaterialIDs.size(); ++m) {
    geo<<"Material Mat_"<<MaterialIDs[m]<<endl;
    geo<<"Mat_"<<MaterialIDs[m]<<".Density 1.0    // Attention: Density fixed to 1.0"<<endl;
    geo<<"Mat_"<<MaterialIDs[m]<<".Component 16  8  2"<<endl;
    geo<<"Mat_"<<MaterialIDs[m]<<".Component 12  6  5"<<endl;
    geo<<"Mat_"<<MaterialIDs[m]<<".Component  1  1  8"<<endl;
    geo<<endl;
  }
  
  
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
