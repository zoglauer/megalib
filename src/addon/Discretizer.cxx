/* 
 * Discretizer.cxx
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
#include <sstream>
#include <csignal>
#include <vector>
using namespace std;

// ROOT
#include <TApplication.h>
#include <MVector.h>

// MEGAlib
#include "MAssert.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MFileEventsSim.h"
#include "MDGeometryQuest.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MDACS.h"
#include "MDStrip2D.h"
#include "MDStrip3D.h"
#include "MDCalorimeter.h"
#include "MDVolume.h"


/******************************************************************************/

class Discretizer
{
public:
  /// Default constructor
  Discretizer();
  /// Default destructor
  ~Discretizer();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// Name of the simulation file
  MString m_FileName;
  /// Name of the output file
  MString m_OutputFileName;
  /// The geometry
  MDGeometryQuest m_Geometry;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
Discretizer::Discretizer() : 
  m_Interrupt(false)
{
  // Intentionally left blanck
}


/******************************************************************************
 * Default destructor
 */
Discretizer::~Discretizer()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool Discretizer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Discretizer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -o:   output file name"<<endl;
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
    if (Option == "-f" || Option == "-g" || Option == "-o") {
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
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
			cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-g") {
      MString FileName = argv[++i];
      if (m_Geometry.ScanSetupFile(FileName) == true) {
        cout<<"Geometry "<<m_Geometry.GetName()<<" loaded!"<<endl;
      } else {
        cout<<"Loading of geometry "<<m_Geometry.GetName()<<" failed!!"<<endl;
        return false;
      }
      m_Geometry.ActivateNoising(false);
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage.str()<<endl;
			return false;
		}
  }

  if (m_FileName == "") {
    cout<<"Error: Please give a file name"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }
  if (m_OutputFileName == "") {
    m_OutputFileName = m_FileName;
    m_OutputFileName.Replace(m_FileName.Length()-4, 4, ".discretized.sim");
    cout<<"Accepting output file name: "<<m_OutputFileName<<endl;    
  }
  if (m_Geometry.GetName() == "unknown") {
    cout<<"Error: Please give a geometry file name"<<endl;
    cout<<Usage.str()<<endl;
    return false;    
  }


  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool Discretizer::Analyze()
{
  if (m_Interrupt == true) return false;

  // Open the simulation file:
  MFileEventsSim* EventFile = 
    new MFileEventsSim(m_FileName, &m_Geometry);
  if (EventFile->Open() == false) {
    cout<<"Unable to open simulation file!"<<endl;
    return false;
  }
  EventFile->ShowProgress();

  ofstream out(m_OutputFileName); 
  if (out.is_open() == false) {
    cout<<"Unable to open output file!"<<endl;
    return false;
  }

  // Write header:
  out<<"# Converted MGGPOD output file "<<endl;
  out<<endl;
  out<<"Type     SIM"<<endl;
  out<<"Version  2"<<endl;
//   out<<"# Warning: The format contains only the relevant information "<<endl;
//   out<<"# for the ResponseGenerator - it is not fully Geant4 compatible!!"<<endl;
  out<<"Geometry "<<m_Geometry.GetFileName()<<endl;
  out<<endl;

  MSimEvent* Event = 0;
  while ((Event = EventFile->GetNextEvent(false)) != 0) {

//     cout<<"Event: "<<Event->GetEventNumber()<<": IAs: "
//         <<Event->GetNIAs()<<"  HTs: "<<Event->GetNHTs()<<endl;

    // Upgrade detector information in IAs:
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MVector Pos = Event->GetIAAt(i)->GetPosition();
      MDVolumeSequence* S = m_Geometry.GetVolumeSequencePointer(Pos);
      if (S->GetDetector() != 0) {
        Event->GetIAAt(i)->SetDetector(S->GetDetector()->GetDetectorType());
      } else {
        Event->GetIAAt(i)->SetDetector(0);
      }
    }    

    // Upgrade the information about energy deposits in passive material:
    if (Event->GetNIAs() > 0) {
      double Energy = 0;
      for (unsigned int i = 0; i < Event->GetNHTs(); ++i) {
        Energy += Event->GetHTAt(i)->GetEnergy();
      }
      Energy = Event->GetIAAt(0)->GetEnergy()-Energy;
      if (Energy < 0.001) Energy = 0;
      Event->SetEnergyDepositNotSensitiveMaterial(Energy);
    }

//     // Upgrade mother energy information:
//     for (int i = 1; i < Event->GetNIAs(); ++i) {
//       if (Event->GetIAAt(i)->GetType() == "COMP") {
//         bool Found = false;
//         for (int h = 0; h < Event->GetNHTs(); ++h) {
//           massert(Event->GetHTAt(h)->GetNOrigins() == 1); // Critical!
//           if (Event->GetHTAt(h)->GetCoordinates() == 
//               Event->GetIAAt(i)->GetCoordinates()) {
//             double Energy;
//             MSimIA* Top = 0;
//             if (Event->GetIAAt(i-1)->GetOrigin() == Event->GetIAAt(i)->GetOrigin()) {
//               Top = Event->GetIAAt(i-1);
//             } else {
//               Top = Event->GetIAAt(Event->GetIAAt(i)->GetOrigin()-1);
//             }
//             if (Top->GetNumber() == 1) {
//               Energy = Top->GetEnergy() - 
//                 Event->GetIAAt(i)->GetEnergy();
//               cout<<Top->GetEnergy()<<"-"
//                   <<Event->GetIAAt(i)->GetEnergy()<<"+"
//                   <<Event->GetHTAt(h)->GetEnergy()<<endl;
//             } else {
//               Energy = Top->GetMotherEnergy() - 
//                 Event->GetIAAt(i)->GetEnergy();
//               cout<<Top->GetMotherEnergy()<<"-"
//                   <<Event->GetIAAt(i)->GetEnergy()<<"+"<<Event->GetHTAt(h)->GetEnergy()<<endl;            
//             }
//             Event->GetIAAt(i)->SetMotherEnergy(Energy);
//             Found = true;
//           }
//         }
//         if (Found == false) {
//           double Energy;
//           MSimIA* Top = 0;
//           if (Event->GetIAAt(i-1)->GetOrigin() == Event->GetIAAt(i)->GetOrigin()) {
//             Top = Event->GetIAAt(i-1);
//           } else {
//             Top = Event->GetIAAt(Event->GetIAAt(i)->GetOrigin()-1);
//           }
//           if (Top->GetNumber() == 1) {
//             Energy = Top->GetEnergy() - 
//               Event->GetIAAt(i)->GetEnergy();
//             cout<<Top->GetEnergy()<<"-"
//                 <<Event->GetIAAt(i)->GetEnergy()<<endl;
//           } else {
//             Energy = Top->GetMotherEnergy() - 
//               Event->GetIAAt(i)->GetEnergy();
//             cout<<Top->GetMotherEnergy()<<"-"
//                 <<Event->GetIAAt(i)->GetEnergy()<<endl;            
//           }
        
//           Event->GetIAAt(i)->SetMotherEnergy(Energy);
//         } 
//       }
//     }


    Event->Discretize(1);
    Event->Discretize(2);
    Event->Discretize(3);
    Event->Discretize(4);

    out<<Event->ToSimString(2);
    
    delete Event;
  }

  out.close();

  return true;
}


/******************************************************************************/

Discretizer* g_Prg = 0;

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

  cout<<endl<<endl;
  cout<<"Do no longer use this program!"<<endl;
  cout<<"Its functionality has been included in ConvertMGGPOD!"<<endl;
  cout<<endl<<endl;


//   TApplication DiscretizerApp("DiscretizerApp", 0, 0);

//   g_Prg = new Discretizer();

//   if (g_Prg->ParseCommandLine(argc, argv) == false) {
//     cerr<<"Error during parsing of command line!"<<endl;
//     return -1;
//   } 
//   if (g_Prg->Analyze() == false) {
//     cerr<<"Error during analysis!"<<endl;
//     return -2;
//   } 

//   //DiscretizerApp.Run();

//   cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
