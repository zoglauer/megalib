/* 
 * ConvertMGeant.cxx
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
#include <map>
#include <string>
#include <sstream>
#include <list>
#include <csignal>
using namespace std;

// ROOT
#include <TApplication.h>

// MEGAlib
#include "MAssert.h"
#include "MStreams.h"
#include "MParser.h"
#include "MTokenizer.h"

/******************************************************************************/

class ConvertMGeant
{
public:
  /// Default constructor
  ConvertMGeant();
  /// Default destructor
  ~ConvertMGeant();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Return the number of non comment tokens
  int GetNNonCommentTokens(MTokenizer* Tokenizer);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  ///
  MString m_MedFileName;
  ///
  MString m_MatFileName;
  ///
  MString m_GeoFileName;
  ///
  MString m_SetupFileName;
  /// True if comments should be kept:
  bool m_KeepComments;
  /// True if "MANY" should be kept, and not be replaced by "Virtual volumes"
  bool m_KeepMany;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
ConvertMGeant::ConvertMGeant() : 
  m_Interrupt(false), m_KeepComments(false), m_KeepMany(false)
{
  // Intentionally left blanck
}


/******************************************************************************
 * Default destructor
 */
ConvertMGeant::~ConvertMGeant()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool ConvertMGeant::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ConvertMGeant <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         --geo:            name of *.geo file"<<endl;
  Usage<<"         --med:            name of *.med file"<<endl;
  Usage<<"         --mat:            name of *.mat file"<<endl;
  Usage<<"         --setup:          name of the geomega *.geo.setup file"<<endl;
  Usage<<"         --keep-comments:  Write mgeant comments into geomega file"<<endl;
  Usage<<"         --keep-many:      Keep the \"MANY\" and risk errors in MEGAlib and Geant4"<<endl;
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
    if (Option == "--geo" || Option == "--mat" || Option == "--med") {
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
    if (Option == "-med" || Option == "--med") {
      m_MedFileName = argv[++i];
			cout<<"Accepting *.med file name: "<<m_MedFileName<<endl;
    } else if (Option == "-mat" || Option == "--mat") {
      m_MatFileName = argv[++i];
			cout<<"Accepting *.mat file name: "<<m_MatFileName<<endl;
    } else if (Option == "-geo" || Option == "--geo") {
      m_GeoFileName = argv[++i];
			cout<<"Accepting *.geo file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-setup" || Option == "--setup") {
      m_SetupFileName = argv[++i];
			cout<<"Accepting *.geo.setup file name: "<<m_SetupFileName<<endl;
    } else if (Option == "--keep-many") {
      m_KeepMany = true;
			cout<<"Accepting to keep \"MANY\" keyword --- and risk errors in MEGAlib and Geant4!"<<endl;
    } else if (Option == "--keep-comments") {
      m_KeepComments = true;
			cout<<"Accepting to keep mgeant comments"<<endl;
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage.str()<<endl;
			return false;
		}
  }

  if (m_MedFileName == "" || m_MatFileName == "" || 
      m_GeoFileName == "" || m_SetupFileName == "") {
    cout<<"Error: Not all file names are given!"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }

  return true;
}


/******************************************************************************
 * Return the number of non comment tokens
 */
int ConvertMGeant::GetNNonCommentTokens(MTokenizer* T)
{
  int NTokens = 0;

  for (unsigned int i = 0; i < T->GetNTokens(); ++i) {
    if (T->GetTokenAt(i).BeginsWith("!") == false) {
      NTokens++;
    } else {
      break;
    }
  }

  return NTokens;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool ConvertMGeant::Analyze()
{
  if (m_Interrupt == true) return false;

  ostringstream Mat;
  ostringstream Geo;

  MParser MatFile(' ', false);
  if (MatFile.Open(m_MatFileName) == false) {
    mout<<"Unable to open file \""<<m_MatFileName<<"\"!"<<endl;
    return false;
  }

  MParser MedFile(' ', false);
  if (MedFile.Open(m_MedFileName) == false) {
    mout<<"Unable to open file \""<<m_MedFileName<<"\"!"<<endl;
    return false;
  }

  MParser GeoFile(' ', false);
  if (GeoFile.Open(m_GeoFileName) == false) {
    mout<<"Unable to open file \""<<m_GeoFileName<<"\"!"<<endl;
    return false;
  }

  map<MString, int> Z;
  Z["H"] = 1;
  Z["He"] = 2;
  Z["Li"] = 3;
  Z["Be"] = 4;
  Z["B"] = 5;
  Z["C"] = 6;
  Z["N"] = 7;
  Z["O"] = 8;
  Z["F"] = 9;
  Z["Ne"] = 10;
  Z["Na"] = 11;
  Z["Mg"] = 12;
  Z["Al"] = 13;
  Z["Si"] = 14;
  Z["P"] = 15;
  Z["S"] = 16;
  Z["Cl"] = 17;
  Z["Ar"] = 18;
  Z["K"] = 19;
  Z["Ca"] = 20;
  Z["Sc"] = 21;
  Z["Ti"] = 22;
  Z["V"] = 23;
  Z["Cr"] = 24;
  Z["Mn"] = 25;
  Z["Fe"] = 26;
  Z["Co"] = 27;
  Z["Ni"] = 28;
  Z["Cu"] = 29;
  Z["Zn"] = 30;
  Z["Ga"] = 31;
  Z["Ge"] = 32;
  Z["As"] = 33;
  Z["Se"] = 34;
  Z["Br"] = 35;
  Z["Kr"] = 36;
  Z["Rb"] = 37;
  Z["Sr"] = 38;
  Z["Y"] = 39;
  Z["Zr"] = 40;
  Z["Nb"] = 41;
  Z["Mo"] = 42;
  Z["Tc"] = 43;
  Z["Ru"] = 44;
  Z["Rh"] = 45;
  Z["Pd"] = 46;
  Z["Ag"] = 47;
  Z["Cd"] = 48;
  Z["In"] = 49;
  Z["Sn"] = 50;
  Z["Sb"] = 51;
  Z["Te"] = 52;
  Z["I"] = 53;
  Z["Xe"] = 54;
  Z["Cs"] = 55;
  Z["Ba"] = 56;
  Z["La"] = 57;
  Z["Ce"] = 58;
  Z["Pr"] = 59;
  Z["Nd"] = 60;
  Z["Pm"] = 61;
  Z["Sm"] = 62;
  Z["Eu"] = 63;
  Z["Gd"] = 64;
  Z["Tb"] = 65;
  Z["Dy"] = 66;
  Z["Ho"] = 67;
  Z["Er"] = 68;
  Z["Tm"] = 69;
  Z["Yb"] = 70;
  Z["Lu"] = 71;
  Z["Hf"] = 72;
  Z["Ta"] = 73;
  Z["W"] = 74;
  Z["Re"] = 75;
  Z["Os"] = 76;
  Z["Ir"] = 77;
  Z["Pt"] = 78;
  Z["Au"] = 79;
  Z["Hg"] = 70;
  Z["Tl"] = 81;
  Z["Pb"] = 82;
  Z["Bi"] = 83;
  Z["Po"] = 84;
  Z["At"] = 85;
  Z["Rn"] = 86;
  Z["Fr"] = 87;
  Z["Ra"] = 88;
  Z["Ac"] = 89;
  Z["Th"] = 90;
  Z["Pa"] = 91;
  Z["U"] = 92;
  Z["Np"] = 93;
  Z["Pu"] = 94;
  Z["Am"] = 95;
  Z["Cm"] = 96;
  Z["Bk"] = 97;
  Z["Cf"] = 98;
  Z["Es"] = 99;
  Z["Fm"] = 100;
  Z["Md"] = 101;
  Z["No"] = 102;
  Z["Lr"] = 103;

  map<MString, double> A;
  A["H"] = 1.0079;
  A["He"] = 4.0026;
  A["Li"] = 6.941;
  A["Be"] = 9.012182;
  A["B"] = 10.811;
  A["C"] = 12.0107;
  A["N"] = 14.0067;
  A["O"] = 15.994;
  A["F"] = 18.9984;
  A["Ne"] = 21.1697;
  A["Na"] = 22.98977;
  A["Mg"] = 24.3050;
  A["Al"] = 26.981538;
  A["Si"] = 28.0855;
  A["P"] = 30.973761;
  A["S"] = 32.065;
  A["Cl"] = 35.435;
  A["Ar"] = 39.948;
  A["K"] = 39.0983;
  A["Ca"] = 40.078;
  A["Sc"] = 44.955910;
  A["Ti"] = 47.867;
  A["V"] = 50.9415;
  A["Cr"] = 51.996;
  A["Mn"] = 54.9380;
  A["Fe"] = 55.845;
  A["Co"] = 58.9332;
  A["Ni"] = 58.69334;
  A["Cu"] = 63.546;
  A["Zn"] = 65.39;
  A["Ga"] = 69.723;
  A["Ge"] = 72.64;
  A["As"] = 74.921;
  A["Se"] = 78.96;
  A["Br"] = 79.904;
  A["Kr"] = 83.80;
  A["Rb"] = 85.4678;
  A["Sr"] = 87.62;
  A["Y"] = 88.90585;
  A["Zr"] = 91.224;
  A["Nb"] = 92.90638;
  A["Mo"] = 95.94;
  A["Tc"] = 98.9063;
  A["Ru"] = 101.07;
  A["Rh"] = 102.90550;
  A["Pd"] = 106.42;
  A["Ag"] = 107.86;
  A["Cd"] = 112.41;
  A["In"] = 114.8;
  A["Sn"] = 118.71;
  A["Sb"] = 121.76;
  A["Te"] = 127.60;
  A["I"] = 126.90;
  A["Xe"] = 131.293;
  A["Cs"] = 132.90;
  A["Ba"] = 137.237;
  A["La"] = 138.9055;
  A["Ce"] = 140.116;
  A["Pr"] = 140.907;
  A["Nd"] = 144.24;
  A["Pm"] = 146.9151;
  A["Sm"] = 150.36;
  A["Eu"] = 151.964;
  A["Gd"] = 157.25;
  A["Tb"] = 158.92;
  A["Dy"] = 162.50;
  A["Ho"] = 164.93;
  A["Er"] = 167.259;
  A["Tm"] = 168.93421;
  A["Yb"] = 173.04;
  A["Lu"] = 174.967;
  A["Hf"] = 178.49;
  A["Ta"] = 180.9479;
  A["W"] = 183.84;
  A["Re"] = 186.207;
  A["Os"] = 190.23;
  A["Ir"] = 192.217;
  A["Pt"] = 195.078;
  A["Au"] = 196.966;
  A["Hg"] = 200.59;
  A["Tl"] = 204.38;
  A["Pb"] = 207.2;
  A["Bi"] = 208.98;
  A["Po"] = 208.98;
  A["At"] = 209.98;
  A["Rn"] = 222.02;
  A["Fr"] = 223.01;
  A["Ra"] = 226.03;
  A["Ac"] = 227.03;
  A["Th"] = 232.04;
  A["Pa"] = 231.04;
  A["U"] = 238.02;
  A["Np"] = 237.05;
  A["Pu"] = 244.06;
  A["Am"] = 243.06;
  A["Cm"] = 247.07;
  A["Bk"] = 247.07;
  A["Cf"] = 251.08;
  A["Es"] = 252.09;
  A["Fm"] = 257.09;
  A["Md"] = 258.10;
  A["No"] = 259.10;
  A["Lr"] = 262.11;

  // Start Parsing the materials file:
  MTokenizer* T = 0;
  for (unsigned int l = 0; l < MatFile.GetNLines(); ++l) {
    T = MatFile.GetTokenizerAt(l);
    massert(T != 0);
    if (GetNNonCommentTokens(T) == 0) continue;
    if (m_KeepComments == true) {
      if (T->GetTokenAtAsString(0).BeginsWith("!") || 
          T->GetTokenAtAsString(0).BeginsWith("#") ||
          T->GetTokenAtAsString(0).BeginsWith("//")) {
        Mat<<T->GetText().ReplaceAll("\n", "")<<endl;
      }
    }
    if (T->IsTokenAt(0, "mate") == true) {
      if (GetNNonCommentTokens(T) != 9) {
        mout<<"mate: Line "<<l+1<<" must contain 9 objects instead of "<<GetNNonCommentTokens(T)<<": "<<T->GetText()<<endl;
        return false;
      }
      Mat<<"Material "<<T->GetTokenAtAsString(2)<<endl;
      Mat<<T->GetTokenAtAsString(2)<<".Density "<<T->GetTokenAtAsDouble(5)<<endl;
      Mat<<T->GetTokenAtAsString(2)<<".RadiationLength "<<T->GetTokenAtAsDouble(6)<<endl;
      Mat<<T->GetTokenAtAsString(2)<<".ComponentByAtoms "<<T->GetTokenAtAsDouble(3)<<" "<<T->GetTokenAtAsDouble(4)<<" 1"<<endl;
      Mat<<endl;
    } else if (T->IsTokenAt(0, "mixt") == true) {
      if (GetNNonCommentTokens(T) != 5) {
        mout<<"mixt: Line "<<l+1<<" must contain 5 objects instead of "<<GetNNonCommentTokens(T)<<": "<<T->GetText()<<endl;
        return false;
      }
      Mat<<"Material "<<T->GetTokenAtAsString(2)<<endl;
      Mat<<T->GetTokenAtAsString(2)<<".Density "<<T->GetTokenAtAsDouble(4)<<endl;
      MString MatName = T->GetTokenAtAsString(2);
      int NComponents = T->GetTokenAtAsInt(3);
      int i;
      for (i = 0; (i < abs(NComponents)) && (l < MatFile.GetNLines()); ++i) {
        T = MatFile.GetTokenizerAt(++l);
        massert(T != 0);
        if (GetNNonCommentTokens(T) != 3) {
          mout<<"components: Line "<<l+1<<" must contain 3 objects instead of "<<GetNNonCommentTokens(T)<<": "<<T->GetText()<<endl;
          return false;
        }        
        if (NComponents < 0) {
          Mat<<MatName<<".ComponentByAtoms "
             <<T->GetTokenAtAsDouble(0)<<" "
             <<T->GetTokenAtAsDouble(1)<<" "
             <<fabs(T->GetTokenAtAsDouble(2))<<endl;
        } else {
          Mat<<MatName<<".ComponentByMass "
             <<T->GetTokenAtAsDouble(0)<<" "
             <<T->GetTokenAtAsDouble(1)<<" "
             <<fabs(T->GetTokenAtAsDouble(2))<<endl;
        }
      }
      Mat<<endl;
    } else if (T->IsTokenAt(0, "umix") == true) {
      if (GetNNonCommentTokens(T) != 5) {
        mout<<"umix: Line "<<l+1<<" must contain 5 objects instead of "<<GetNNonCommentTokens(T)<<": "<<T->GetText()<<endl;
        return false;
      }
      Mat<<"Material "<<T->GetTokenAtAsString(2)<<endl;
      Mat<<T->GetTokenAtAsString(2)<<".Density "<<T->GetTokenAtAsDouble(4)<<endl;
      MString MatName = T->GetTokenAtAsString(2);
      int NComponents = T->GetTokenAtAsInt(3);
      int i;
      for (i = 0; (i < abs(NComponents)) && (l < MatFile.GetNLines()); ++i) {
        T = MatFile.GetTokenizerAt(++l);
        massert(T != 0);
        if (GetNNonCommentTokens(T) != 3) {
          mout<<"components: Line "<<l+1<<" must contain 3 objects instead of "<<GetNNonCommentTokens(T)<<": "<<T->GetText()<<endl;
          return false;
        }        
        if (NComponents < 0) {
          Mat<<MatName<<".ComponentByAtoms "
             <<A[T->GetTokenAtAsString(1)]<<" "
             <<Z[T->GetTokenAtAsString(1)]<<" "
             <<fabs(T->GetTokenAtAsDouble(2))<<endl;
        } else {
          Mat<<MatName<<".ComponentByMass "
             <<A[T->GetTokenAtAsString(1)]<<" "
             <<Z[T->GetTokenAtAsString(1)]<<" "
             <<fabs(T->GetTokenAtAsDouble(2))<<endl;
        }
      }
      Mat<<endl;
    }
  }
  
  
  for (unsigned int l = 0; l < MedFile.GetNLines(); ++l) {
    T = MedFile.GetTokenizerAt(l);
    massert(T != 0);
    if (GetNNonCommentTokens(T) == 0) continue;
    if (T->IsTokenAt(0, "tmed") == true) {
      if (GetNNonCommentTokens(T) != 13) {
        mout<<"tmed: Line "<<l+1<<" must contain 13 objects: "<<T->GetText()<<endl;
        return false;
      }
      int Sens = 0;
      if (T->GetTokenAtAsString(4) == "pass") Sens = 0;
      else if (T->GetTokenAtAsString(4) == "dete") Sens = 1;
      else if (T->GetTokenAtAsString(4) == "shld") Sens = 2;
      else if (T->GetTokenAtAsString(4) == "cryo") Sens = 3;
      else if (T->GetTokenAtAsString(4) == "mask") Sens = 4;
      else {
        mout<<"Line "<<l+1
            <<" contains an unknown \"pass\\dete\\shld\\cryo\\mask\" flag: "
            <<T->GetTokenAtAsString(4)<<endl;
        return false;
      }
      Mat<<T->GetTokenAtAsString(3)<<".Sensitivity "<<Sens<<endl;      

      if (T->GetTokenAtAsString(2) != T->GetTokenAtAsString(3)) {
        Mat<<T->GetTokenAtAsString(3)<<".Copy "<<T->GetTokenAtAsString(2)<<endl;
      }
    }
  }

  // First read all rotations:
  vector<int> RotId;
  vector<double> Rot1;
  vector<double> Rot2;
  vector<double> Rot3;
  vector<double> Rot4;
  vector<double> Rot5;
  vector<double> Rot6;
  for (unsigned int l = 0; l < GeoFile.GetNLines(); ++l) {
    T = GeoFile.GetTokenizerAt(l);
    massert(T != 0);
    if (GetNNonCommentTokens(T) == 0) continue;
    if (T->IsTokenAt(0, "rotm") == true) {
      if (GetNNonCommentTokens(T) != 8) {
        mout<<"rotm: Line "<<l+1<<" must contain 8 objects: "<<T->GetText()<<endl;
        return false;
      }
      if (Rot1.size() < (unsigned int) T->GetTokenAtAsInt(1)+2) {
        Rot1.resize(T->GetTokenAtAsInt(1)+2);
        Rot2.resize(T->GetTokenAtAsInt(1)+2);
        Rot3.resize(T->GetTokenAtAsInt(1)+2);
        Rot4.resize(T->GetTokenAtAsInt(1)+2);
        Rot5.resize(T->GetTokenAtAsInt(1)+2);
        Rot6.resize(T->GetTokenAtAsInt(1)+2);
      }

      Rot1[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(2);
      Rot2[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(3);
      Rot3[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(4);
      Rot4[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(5);
      Rot5[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(6);
      Rot6[T->GetTokenAtAsInt(1)] = T->GetTokenAtAsDouble(7);
    } else if (T->IsTokenAt(0, "tree") == true) {
      mout<<"tree: Line "<<l+1<<": Keyword tree is not handled right now!"<<endl;      
    }
  }


  // search the mother volume: it has no "posi"
  vector<MString> Rejected;
  map<MString, int> VolumePositioned;
  map<MString, int>::iterator VolumePositionedIter;
  for (unsigned int l = 0; l < GeoFile.GetNLines(); ++l) {
    T = GeoFile.GetTokenizerAt(l);
    massert(T != 0);
    if (GetNNonCommentTokens(T) == 0) continue;
    if (m_KeepComments == true) {
      if (T->GetTokenAtAsString(0).BeginsWith("!") || 
          T->GetTokenAtAsString(0).BeginsWith("#") ||
          T->GetTokenAtAsString(0).BeginsWith("//")) {
        Geo<<T->GetText().ReplaceAll("\n", "")<<endl;
      }
    }
    if (T->IsTokenAt(0, "volu") == true) {
      if (GetNNonCommentTokens(T) != 5) {
        mout<<"volu: Line "<<l+1<<" must contain 5 objects: "<<T->GetText()<<endl;
        return false;
      }
      unsigned int NParam = 0;
      vector<double> Param;

      MString Volume = T->GetTokenAtAsString(1);
      MString Shape = T->GetTokenAtAsString(2);
      if (Shape == "TUBE" || Shape == "TUBS" || Shape == "SPHE" ||
          Shape == "TRAP" || Shape == "GTRA" || Shape == "PCON" || 
          Shape == "TRD1" || Shape == "TRD2" || Shape == "BOX" || 
          Shape == "CONE" || Shape == "CONS" || 
          Shape == "PGON" ) {
        Geo<<"Volume "<<T->GetTokenAtAsString(1)<<endl;
        VolumePositioned[T->GetTokenAtAsString(1)] = 0;
        Geo<<T->GetTokenAtAsString(1)<<".Visibility 1"<<endl;
        Geo<<T->GetTokenAtAsString(1)<<".Material "<<T->GetTokenAtAsString(3)<<endl;
        NParam = T->GetTokenAtAsInt(4);
        while (Param.size() < NParam) {
          cout<<"Param shape : "<<Shape<<": "<<NParam<<"!"<<Param.size()<<endl;
          T = GeoFile.GetTokenizerAt(++l);
          massert(T != 0);
          for (int t = 0; t < GetNNonCommentTokens(T); ++t) {
            Param.push_back(T->GetTokenAtAsDouble(t));
          }
        }
        massert(Param.size() > 0);
        Geo<<Volume<<".Shape ";
      }
      if (Shape == "TUBE") {
        Geo<<"TUBS "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" 0 360 "<<endl;
      } else if (Shape == "TUBS") {
        Geo<<"TUBS "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<endl;
      } else if (Shape == "SPHE") {
        Geo<<"SPHE "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<" "
           <<Param.at(5)<<endl;
      } else if (Shape == "TRAP") {
        Geo<<"TRAP "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<" "
           <<Param.at(5)<<" "
           <<Param.at(6)<<" "
           <<Param.at(7)<<" "
           <<Param.at(8)<<" "
           <<Param.at(9)<<" "
           <<Param.at(10)<<endl;
      } else if (Shape == "GTRA") {
        Geo<<"GTRA "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<" "
           <<Param.at(5)<<" "
           <<Param.at(6)<<" "
           <<Param.at(7)<<" "
           <<Param.at(8)<<" "
           <<Param.at(9)<<" "
           <<Param.at(10)<<" "
           <<Param.at(11)<<endl;
      } else if (Shape == "PCON") {
        Geo<<"PCON ";
        for (unsigned int i = 0; i < Param.size(); ++i) {
          Geo<<Param.at(i)<<" ";
        }
        Geo<<endl;
      } else if (Shape == "PGON") {
        Geo<<"PGON ";
        for (unsigned int i = 0; i < Param.size(); ++i) {
          Geo<<Param.at(i)<<" ";
        }
        Geo<<endl;
      } else if (Shape == "TRD1") {
        Geo<<"TRD1 "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<endl;
      } else if (Shape == "TRD2") {
        Geo<<"TRD2 "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<endl;
      } else if (Shape == "PARA") {
        /*
          Geo<<"PARA "
          <<Param.at(0)<<" "
          <<Param.at(1)<<" "
          <<Param.at(2)<<" "
          <<Param.at(3)<<" "
          <<Param.at(4)<<" "
          <<Param.at(5)<<endl;
        */
        cout<<"Rejected volume "<<Volume<<" of type "<<Shape<<endl;
        Rejected.push_back(Volume);
      } else if (Shape == "CONE") {
        Geo<<"CONE "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<endl;
      } else if (Shape == "CONS") {
        Geo<<"CONS "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<" "
           <<Param.at(3)<<" "
           <<Param.at(4)<<" "
           <<Param.at(5)<<" "
           <<Param.at(6)<<endl;
      } else if (Shape == "BOX") {
        Geo<<"BRIK "
           <<Param.at(0)<<" "
           <<Param.at(1)<<" "
           <<Param.at(2)<<endl;
      } else {
        mout<<"Line "<<l<<": The shape "<<Shape<<" is not defined in geomega!"<<endl;
        return false;         
      }
      Geo<<endl;
    }
  }

  

  for (unsigned int l = 0; l < GeoFile.GetNLines(); ++l) {
    T = GeoFile.GetTokenizerAt(l);
    massert(T != 0);
    if (GetNNonCommentTokens(T) == 0) continue;

    if (T->IsTokenAt(0, "satt") == true) {
      if (GetNNonCommentTokens(T) != 4) {
        mout<<"satt: Line "<<l+1<<" must contain 4 objects: "<<T->GetText()<<endl;
        return false;
      }
      if (T->GetTokenAtAsString(2) == "SEEN") {
        Geo<<T->GetTokenAtAsString(1)<<".Visibility "<<T->GetTokenAtAsInt(3)<<endl;
      } else {
        mout<<"satt: Line "<<l+1<<": Keyword satt with option "
            <<T->GetTokenAtAsString(2)<<" is not handled right now!"<<endl;      
      }
    } else if (T->IsTokenAt(0, "posi") == true) {
      if (GetNNonCommentTokens(T) != 9) {
        mout<<"posi: Line "<<l+1<<" must contain 9 objects: "<<T->GetText()<<endl;
        return false;
      }
      // Do not position the volume if 
      // (a) it is in the list of rejected volumes
      // (b) its mother is in this list
      bool RejectedVol = false;
      for (unsigned int r = 0; r < Rejected.size(); ++r) {
        if (Rejected[r] == T->GetTokenAtAsString(3)) {
          RejectedVol = true;
          break;
        }
      }
      for (unsigned int r = 0; r < Rejected.size(); ++r) {
        if (Rejected[r] == T->GetTokenAtAsString(1)) {
          RejectedVol = true;
          Rejected.push_back(T->GetTokenAtAsString(1));
          break;
        }
      }

      if (RejectedVol == false) {
        MString Name = T->GetTokenAtAsString(1) + "_Copy" + T->GetTokenAtAsString(2);
        Geo<<endl;
        Geo<<T->GetTokenAtAsString(1)<<".Copy "<<Name<<endl;
        Geo<<Name<<".Position "
           <<T->GetTokenAtAsDouble(4)<<" "
           <<T->GetTokenAtAsDouble(5)<<" "
           <<T->GetTokenAtAsDouble(6)<<endl;
        if (T->GetTokenAtAsInt(7) != 0) {
          Geo<<Name<<".Rotation "
             <<Rot1[T->GetTokenAtAsInt(7)]<<" "
             <<Rot2[T->GetTokenAtAsInt(7)]<<" "
             <<Rot3[T->GetTokenAtAsInt(7)]<<" "
             <<Rot4[T->GetTokenAtAsInt(7)]<<" "
             <<Rot5[T->GetTokenAtAsInt(7)]<<" "
             <<Rot6[T->GetTokenAtAsInt(7)]<<endl;
        }
        Geo<<Name<<".Mother "<<T->GetTokenAtAsString(3)<<endl;
        if (T->GetTokenAtAsString(8) == "MANY") {
          if (m_KeepMany == true) {
            Geo<<"# Warning: Many is only supported in Geant3/MGGPOD, NOT in Root, MEGAlib, Geant4!!"<<endl;
            Geo<<T->GetTokenAtAsString(1)<<".Many true"<<endl;
          } else {
            mout<<"many: Line "<<l+1<<": MANY is only supported for vacuum volumes."<<endl;
            Geo<<T->GetTokenAtAsString(1)<<".Virtual true"<<endl;
          }
        }
      } else {
        mout<<"Rejected to position volume "<<T->GetTokenAtAsString(1)<<endl;
      }
      VolumePositioned[T->GetTokenAtAsString(1)] = 1;
    } else if (T->IsTokenAt(0, "posp") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    } else if (T->IsTokenAt(0, "divn") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    } else if (T->IsTokenAt(0, "dvn2") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    } else if (T->IsTokenAt(0, "divt") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    } else if (T->IsTokenAt(0, "dvt2") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    } else if (T->IsTokenAt(0, "divx") == true) {
      mout<<"Line "<<l<<": Keyword "<<T->GetTokenAtAsString(0)
          <<" is not handeled!"<<endl;      
    }
  }

  // take care of the world volume:
  unsigned int NWorldVolumes = 0;
  for (VolumePositionedIter = VolumePositioned.begin();
       VolumePositionedIter != VolumePositioned.end();
       VolumePositionedIter++) {
    if ((*VolumePositionedIter).second != 1) {
      Geo<<endl;
      Geo<<(*VolumePositionedIter).first<<".Mother 0"<<endl;
      NWorldVolumes++;
    }
  }

  if (NWorldVolumes == 0) {
    mout<<"There was no world volume found!!!!!"<<endl;
    return false;
  } else if (NWorldVolumes > 0) {
    mout<<"There was more than one unpositioned volume in your file!"<<endl;
    mout<<"They are currently all declared as world volumes, since I cannot distinguish which one the right one is!"<<endl;
    mout<<"You have to delete the \"...Mother 0\" for all but the real worldvolume from your file!"<<endl;
  }

  ofstream fout;

  fout.open(m_SetupFileName);
  if (fout.is_open() == false) {
    mout<<"Unable to open file \""<<m_SetupFileName<<"\"!"<<endl;
    return false;
  }

  fout<<"#"<<endl;
  fout<<"# Geomega geometry description"<<endl;
  fout<<"# This file has been automatically generated by ConvertMGeant from the"<<endl;
  fout<<"# following MGeant-files:"<<endl;
  fout<<"# mat: "<<m_MatFileName<<endl;
  fout<<"# med: "<<m_MedFileName<<endl;
  fout<<"# geo: "<<m_GeoFileName<<endl;
  fout<<"#"<<endl;
  fout<<"#"<<endl;
  fout<<"# This file is probably incomplete, because:"<<endl;
  fout<<"# - All volumes of the original file, which Geomega does not know, are missing"<<endl;
  fout<<"# - All volumes with the flag MANY are converted into virtual volumes,"<<endl;
  fout<<"#   which are removed during the construction of the volume tree"<<endl;
  fout<<"# - Divisions of volumes are not done"<<endl;
  fout<<"# - You have to define a correct surrounding sphere"<<endl;
  fout<<"# - You have to add trigger criteria"<<endl;
  fout<<"# - You have to define detectors"<<endl;
  fout<<"#"<<endl;
  fout<<"#"<<endl;
  fout<<"# If you do not consider all the above points, "<<endl;
  fout<<"# you will get wrong results from Geomega!"<<endl;
  fout<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"Name "<<m_MatFileName.Remove(m_MatFileName.Length()-4, m_MatFileName.Length())<<endl;
  fout<<"Version 0.1"<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"# You have to correct this surounding sphere:"<<endl;
  fout<<"SurroundingSphere 110  0.0  0.0  0.0  110.0"<<endl;
  fout<<endl;
  fout<<"Include $(MEGALIB)/resource/geometries/materials/MGeantDefaultMaterials.geo"<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"# Trigger conditions"<<endl;
  fout<<endl;
  fout<<"# Add your trigger conditions here!"<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"# Material definitions: "<<endl;
  fout<<Mat.str()<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"# Volume definitions:"<<endl;
  fout<<Geo.str()<<endl;
  fout<<endl;
  fout<<endl;
  fout<<"# Detector definitions:"<<endl;
  fout<<endl;
  fout<<"# Add here your detector definitions!!"<<endl;
  fout.close();



  /// Closing remarks...
  cerr<<endl;
  cerr<<endl;
  cerr<<"Closing remarks"<<endl;
  cerr<<"==============="<<endl;
  cerr<<endl;
  cerr<<"Geant3 contains a lot of features which are not implemented in"<<endl;
  cerr<<"Geomega, because Geomega has to be compatible with Geant3, MGeant"<<endl;
  cerr<<"Geant4 and ROOT, and -- more likeliy -- nobody had found the time to"<<endl;
  cerr<<"implement them."<<endl;
  cerr<<endl;
  cerr<<"Some of these missing features are: "<<endl;
  cerr<<"(*) Geomega does not know the feature \"MANY\" of Geant3:"<<endl;
  cerr<<"    All appearances of volumes which are declared as MANY"<<endl;
  cerr<<"    are converted to Geomegas' so called \"virtual volumes\":"<<endl;
  cerr<<"    Those are place-holder volumes, which are later REMOVED !! "<<endl;
  cerr<<"    from the volume tree!"<<endl;
  cerr<<"(*) Geomega does only know a limited number of shapes."<<endl;
  cerr<<"    All volumes, which it does not know, are not converted."<<endl;
  cerr<<"    Thus all volumes which are daughters of volumes with missing"<<endl;
  cerr<<"    shapes are also not converted. "<<endl;
  cerr<<"(*) Divisions of volumes are ignored"<<endl;
  cerr<<"(*) All the features of Geant3/MGeant I do not know of..."<<endl;
  cerr<<endl;
  cerr<<"Additionally MGeant also misses some features, which a working"<<endl;
  cerr<<"Geomega geometry DEFINITELY needs:"<<endl;
  cerr<<"(*) Surrounding Sphere"<<endl;
  cerr<<"(*) Trigger criteria"<<endl;
  cerr<<"(*) Detector description"<<endl;
  cerr<<endl;
  cerr<<"As consequence, you have add this features by hand or"<<endl;
  cerr<<"all results gained by Geomega are guaranteed to be wrong!!"<<endl;

  return true;
}

/******************************************************************************/

ConvertMGeant* g_Prg = 0;

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

  TApplication ConvertMGeantApp("ConvertMGeantApp", 0, 0);

  g_Prg = new ConvertMGeant();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //ConvertMGeantApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
