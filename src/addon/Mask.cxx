/* 
 * Mask.cxx
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
#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <vector>
#include <string>
#include <cstdlib>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TRandom.h>

// MEGAlib
#include "MString.h"

/******************************************************************************/

class Volume
{
public:
  Volume(int x, int y, int Level = 0, Volume* Mother = 0) : 
    m_X(x), m_Y(y), m_Level(Level), m_IsSolid(true), m_Mother(Mother) {
    if (m_Level > s_HighestLevel) s_HighestLevel = m_Level;
    m_Id = ++s_ID;
    if (m_Level == 0) {
      m_Name = "Mask";
    } else {
      ostringstream Name;
      Name<<"Level"<<m_Level<<"_Id"<<m_Id;
      m_Name = Name.str();
    }
  }
  ~Volume();

  bool Split();
  bool GenerateMap(vector<int>& Map, int x = 0, int y = 0);
  string Layout();


  string GetName() { return m_Name; }
  int GetLevel() { return m_Level; }
  int GetX() { return m_X; }
  int GetY() { return m_Y; }
  bool IsSolid() { return m_IsSolid; }

  static double s_PixelSize;
  static int s_ID;
  static int s_HighestLevel;
  static int s_NPixels;

private:
  int m_X;
  int m_Y;

  int m_Level;
  int m_Id;

  string m_Name;
  bool m_IsSolid;

  Volume* m_Mother;
  vector<Volume*> m_Daughters;

};


/******************************************************************************/


double Volume::s_PixelSize = 0;
int Volume::s_ID = 0;
int Volume::s_HighestLevel = 0;
int Volume::s_NPixels = 0;


/******************************************************************************
 * Layout all daughter volumes
 */
string Volume::Layout()
{
  string String;

  if (m_Mother == 0) {
    ostringstream S;
    S<<"Volume "<<m_Name<<endl;
    S<<m_Name<<".Visibility 0"<<endl;
    S<<m_Name<<".Mother WorldVolume"<<endl;
    S<<m_Name<<".Material Vacuum"<<endl;
    S<<m_Name<<".Position 0 0 0"<<endl;
    S<<m_Name<<".Shape BRIK "<<m_X*s_PixelSize/2<<" "
     <<m_Y*s_PixelSize/2<<" "<<s_PixelSize/2<<endl;
    S<<endl;
    String += S.str();
  }

  double PosX = -m_X*s_PixelSize/2;
  double PosY = -m_Y*s_PixelSize/2;

  for (unsigned int d = 0; d < m_Daughters.size(); ++d) {
    if (m_Daughters[d]->IsSolid() == false) {
      continue;
    } 
    ostringstream S;
    S<<"Volume "<<m_Daughters[d]->GetName()<<endl;
    S<<m_Daughters[d]->GetName()<<".Mother "<<m_Name<<endl;
    if (m_Daughters[d]->GetX()*m_Daughters[d]->GetY() == 1) {
      S<<m_Daughters[d]->GetName()<<".Material Densimet180"<<endl;
      S<<m_Daughters[d]->GetName()<<".Visibility 1"<<endl;
    } else {
      S<<m_Daughters[d]->GetName()<<".Material Vacuum"<<endl;
      S<<m_Daughters[d]->GetName()<<".Visibility 0"<<endl;
    }
    S<<m_Daughters[d]->GetName()<<".Shape BRIK "<<m_Daughters[d]->GetX()*s_PixelSize/2<<" "
     <<m_Daughters[d]->GetY()*s_PixelSize/2<<" "<<s_PixelSize/2<<endl;
    S<<m_Daughters[d]->GetName()<<".Position "<<PosX+0.5*(m_Daughters[d]->GetX()*s_PixelSize)
     <<" "<<PosY+0.5*(m_Daughters[d]->GetY()*s_PixelSize)<<" 0.0"<<endl;
    if (m_X == m_Daughters[d]->GetX()) {
      PosY += m_Daughters[d]->GetY()*s_PixelSize;
    } else {
      PosX += m_Daughters[d]->GetX()*s_PixelSize;
    }
    S<<endl;
    String += S.str();
  }

  for (unsigned int d = 0; d < m_Daughters.size(); ++d) {
    String += m_Daughters[d]->Layout();
  }

  return String;
}


/******************************************************************************
 * Generate a on of map
 */
bool Volume::GenerateMap(vector<int>& Map, int x, int y)
{
  if (m_IsSolid == true && m_X*m_Y == 1) {
    if (x + s_NPixels*y < s_NPixels*s_NPixels) {
      Map.at(x + s_NPixels*y) = 1;
    } else {
      cout<<"Index out of bounds: "<<x<<", "<<y<<", "<<x + s_NPixels*y<<endl;
    }
  }

  for (unsigned int d = 0; d < m_Daughters.size(); ++d) {
    if (m_X == m_Daughters[d]->GetX()) {
      int index = 0;
      for (unsigned int d2 = 1; d2 <= d; ++d2) {
        index += m_Daughters[d2-1]->GetY();
      }
      m_Daughters[d]->GenerateMap(Map, x, y + index);
    } else {
      int index = 0;
      for (unsigned int d2 = 1; d2 <= d; ++d2) {
        index += m_Daughters[d2-1]->GetX();
      }
      m_Daughters[d]->GenerateMap(Map, x + index, y);
    }
  }

  return true;
}


/******************************************************************************
 * Split a volume
 */
bool Volume::Split()
{
  //if (m_Mother != 0) {
  //  cout<<"Splitting: "<<m_Name<<" ("<<m_Mother->GetName()<<"): "<<m_X*m_Y<<endl;
  //}

  if (m_X*m_Y  <= 1) {
    if (gRandom->Rndm() > 0.5) {
      m_IsSolid = false;
    } 
    return false;
  }

  Volume* New = 0;
  if (m_X >= m_Y) {
    New = new Volume(m_X/2, m_Y, m_Level+1, this);
    m_Daughters.push_back(New);
    New = new Volume(m_X - m_X/2, m_Y, m_Level+1, this);
    m_Daughters.push_back(New);
  } else {
    New = new Volume(m_X, m_Y/2, m_Level+1, this);
    m_Daughters.push_back(New);
    New = new Volume(m_X, m_Y - m_Y/2, m_Level+1, this);
    m_Daughters.push_back(New);
  }

  for (unsigned int d = 0; d < m_Daughters.size(); ++d) {
    m_Daughters[d]->Split();
  }

  return true;
}

/******************************************************************************/

class Mask
{
public:
  /// Default constructor
  Mask();
  /// Default destructor
  ~Mask();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  /// Calculate anti correlation function:
  void ACF(vector<int> Map);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  
  /// Name of the output file:
  MString m_FileName;

  /// Dimensions of the pixels
  double m_PixelSize;

  /// Number of pixels in the mask
  int m_NPixels;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
Mask::Mask() : m_Interrupt(false), m_FileName("NoName"), m_PixelSize(0.2), m_NPixels(4)
{
  // Intentionally left blanck
}


/******************************************************************************
 * Default destructor
 */
Mask::~Mask()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool Mask::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Mask <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
  Usage<<"         -p:   dimension (cm) of the (kubic) pixels"<<endl;
  Usage<<"         -n:   n^2 = number of pixels the mask)"<<endl;
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
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
			}
		} 
		// Multiple arguments_
		//else if (Option == "-??") {
		//	if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
		//		cout<<"Error: Option "<<argv[i][1]<<" needs two arguments argument!"<<endl;
		//		cout<<Usage.str()<<endl;
		//		return false;
		//	}
		//}

		// Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
			cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-p") {
      m_PixelSize = atof(argv[++i]);
			cout<<"Accepting pixel size: "<<m_PixelSize<<" cm"<<endl;
    } else if (Option == "-n") {
      m_NPixels = atoi(argv[++i]);
			cout<<"Accepting number of pixels: "<<m_NPixels<<endl;
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage.str()<<endl;
			return false;
		}
  }

  if (m_FileName == "NoName") {
    ostringstream o;
    o<<"Mask."<<m_NPixels<<".geo.setup";
    m_FileName = o.str().c_str();
  }

  Volume::s_PixelSize = m_PixelSize;
  Volume::s_NPixels = m_NPixels;

  gRandom->SetSeed(0);

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool Mask::Analyze()
{
  if (m_Interrupt == true) return false;

  Volume* World = new Volume(m_NPixels, m_NPixels);
  World->Split();
  vector<int> Map(m_NPixels*m_NPixels, 0);
  World->GenerateMap(Map);
  ACF(Map);

//   for (int x = 0; x < m_NPixels; ++x) {
//     for (int y = 0; y < m_NPixels; ++y) {
//       cout<<Map[x+m_NPixels*y]<<" ";
//     }
//     cout<<endl;
//   }

  ofstream out(m_FileName);
  out<<"Name Mask"<<endl;
  out<<"Version 1.0"<<endl;
  out<<endl;
  out<<"DoSanityChecks false"<<endl;
  out<<"IgnoreShortNames true"<<endl;
  //out<<"ShowVolumes false"<<endl;
  out<<endl;
  out<<"SurroundingSphere 1000  0.0  0.0  0.0  1000.0"<<endl;
  out<<endl;
  out<<"Material Densimet180           "<<endl;               
  out<<"Densimet180.Density                  18     "<<endl;    
  out<<"Densimet180.Component                183.8  74 950"<<endl;
  out<<"Densimet180.Component                 55.84 26  16"<<endl;
  out<<"Densimet180.Component                 58.69 28  44"<<endl;
  out<<"Densimet180.Sensitivity              1"<<endl;
  out<<endl;
  out<<"Material Vacuum           "<<endl;               
  out<<"Vacuum.Density                  1E-16     "<<endl;    
  out<<"Vacuum.RadiationLength          1E+16      "<<endl;    
  out<<"Vacuum.Component                1 1 1 "<<endl;
  out<<"Vacuum.Sensitivity              1"<<endl;
  out<<endl;
  out<<"Volume WorldVolume        "<<endl;     
  out<<"WorldVolume.Material Vacuum"<<endl;
  out<<"WorldVolume.Visibility 0   "<<endl;
  out<<"WorldVolume.Shape BRIK 5000. 5000. 5000."<<endl;
  out<<"WorldVolume.Mother 0  "<<endl;
  out<<endl;
  out<<World->Layout()<<endl;
  out.close();

  return true;
}



/******************************************************************************
 * Calculate the anti correlation function
 */
void Mask::ACF(vector<int> Map)
{
  vector<int> A(Map.size(), 0);

  int On = 0;
  for (unsigned int i = 0; i < Map.size(); ++i) {
    On += Map[i];
  }
  cout<<"Solid fraction: "<<double(On)/m_NPixels/m_NPixels<<endl;
}


/******************************************************************************/

Mask* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  //void (*handler)(int);
	//handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  TApplication MaskApp("MaskApp", 0, 0);

  g_Prg = new Mask();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //MaskApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
