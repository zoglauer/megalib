/* 
 * AtmosphereAbsorptionsForGRBs.cxx
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
#include <cstdlib>
#include <fstream>
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

// MEGAlib
#include "MGlobal.h"
#include "MInterface.h"
#include "MResponseMatrixO1.h"
#include "MDGeometryQuest.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class AtmosphereAbsorptionsForGRBs
{
public:
  //! Default constructor
  AtmosphereAbsorptionsForGRBs();
  //! Default destructor
  ~AtmosphereAbsorptionsForGRBs();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The geometry file name
  MString m_GeometryFileName;
  //! The test position where to calculate the atmosphere absorptions
  MVector m_TestPoint;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
AtmosphereAbsorptionsForGRBs::AtmosphereAbsorptionsForGRBs() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  
  m_TestPoint.SetXYZ(0, 0, 3500000);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
AtmosphereAbsorptionsForGRBs::~AtmosphereAbsorptionsForGRBs()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool AtmosphereAbsorptionsForGRBs::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: AtmosphereAbsorptionsForGRBs <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
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
    if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool AtmosphereAbsorptionsForGRBs::Analyze()
{
  if (m_Interrupt == true) return false;

  gStyle->SetPalette(kBird);
  
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  
 
  //vector<double> Alpha = { -1.0, +0.0, -2.0, -1.0, -1.0, -1.0, -1.0, -2.0000 };
  //vector<double> Beta  = { -2.5, -2.5, -2.5, -1.5, -3.5, -2.5, -2.5, -2.0001 };
  //vector<double> E0    = {  150,  150,  150,  150,  150,   80,  800,  100000 };

	/*
  vector<double> Alpha = { -0.5 };
  vector<double> Beta  = { -2.3 };
  vector<double> E0    = {  480 };

  vector<int> Angle = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
 	*/
 
 	// Sample it
 	unsigned int NSamples = 50;
 	vector<int> Angle = { 0 };
 	
  vector<double> Alpha;
  vector<double> Beta;
  vector<double> E0;
 	for (unsigned int i = 0; i < NSamples; ++i) {
 		Alpha.push_back(gRandom->Gaus(-0.86, 0.39));
 		E0.push_back(pow(10, gRandom->Gaus(2.27, 0.4)));
		Beta.push_back(gRandom->Gaus(-2.3, 0.1)); 	
 	}
 	
 	
  MString Prefix("GRBSampled_");
  
  for (unsigned int i = 0; i < Alpha.size(); ++i) {
    for (unsigned int j = 0; j < Angle.size(); ++j) {
    
  		double Ebreak = (Alpha[i] - Beta[i])*E0[i];
  		double Eref  = 100.0;
  
  		double Time = 10.0;
  		double Flux = 1e-4; // erg/cm2 in Time
  
  		MVector Dir;
      Dir.SetMagThetaPhi(20000000, Angle[j]*c_Rad, 0.0);
  
  		double Emin = Eref;
  		double Emax = 100000.0;

  		double FluxPhotons = 0.0;
  		double FluxEnergy = 0.0;

  		double FluxPhotonsAfterAtmosphere = 0.0;
  		double FluxEnergyAfterAtmosphere = 0.0;

  		ofstream dout;
  		dout.open(Prefix + i + "_Angle_" + Angle[j] + ".dat");
  		if (dout.is_open() == false) return false;
  		dout.setf(ios_base::scientific);
  		dout<<"# Alpha: "<<Alpha[i]<<"  Beta: "<<Beta[i]<<"  E0: "<<E0[i]<<endl;
  		dout<<"IP LOGLOG"<<endl;


		  vector<double> Grid;
		  for (double E = Emin; E <= Emax; E += 1.0) {
		    double Out = 0.0;
		    if (E < Ebreak) {
		      Out = pow(E/Eref, Alpha[i]) * exp(-E/E0[i]);
		    } else {
		      Out = pow(E/Eref, Beta[i]) * pow(Ebreak/Eref, Alpha[i] - Beta[i]) * exp(Beta[i] - Alpha[i]);
		    }
		    //gout<<"DP "<<E<<" "<<Out<<endl;
		    FluxPhotons += Out;
		    FluxEnergy += Out*E;
    
		    // Add atmosphere
		    Out *= 1-Geometry->GetAbsorptionProbability(m_TestPoint, m_TestPoint + Dir, E);
		    
		    FluxPhotonsAfterAtmosphere += Out;
		    FluxEnergyAfterAtmosphere += Out*E;
		    
		    dout<<"DP "<<E<<" "<<Out<<endl;
		  }
		  
		  dout.close();
	
	    // First do the flux scaling for the befor atmosphere flux
	    
	    // Scale to erg/cm2	  
			FluxEnergy *= 1.60217646E-9; // now in erg
			
			// Now we can scale! 
  		double ScalerFluxEnergy = Flux/FluxEnergy;
  		
  		FluxEnergy *= ScalerFluxEnergy;
  
  		// Scale flux photons to get into ph/s/cm2
  		FluxPhotons *= ScalerFluxEnergy/Time;
  		// --> This is what cosima would need without atmosphere
 
 			
 			// Now do the same with atmosphere
 			FluxEnergyAfterAtmosphere *= ScalerFluxEnergy * 1.60217646E-9;
  		FluxPhotonsAfterAtmosphere *= ScalerFluxEnergy/Time;
 
 
 			cout<<endl;
 			cout<<"GRB #"<<i<<", angle "<<Angle[j]<<endl;
 
 			// The flux in the 
		  cout<<"Flux photons: "<<FluxPhotons<<" ph/cm2/s"<<endl; 
		  cout<<"Flux energy: "<<FluxEnergy<<" erg/cm2"<<endl;
		  
 			// The flux in the 
		  cout<<"Flux photons after atmosphere: "<<FluxPhotonsAfterAtmosphere<<" ph/cm2/s"<<endl; 
		  cout<<"Flux energy after atmosphere: "<<FluxEnergyAfterAtmosphere<<" erg/cm2"<<endl;
		  
		  // Now write the data files:

  		ofstream sout;
  		sout.open(Prefix + i + "_Angle_" + Angle[j] + "_pol.source");
  		if (sout.is_open() == false) return false;
  		sout.setf(ios_base::scientific);
  		sout<<"Include Base.source"<<endl;
  		sout<<"MyRun.FileName "<<Prefix<<i<<"_Angle_"<<Angle[j]<<"_pol"<<endl;
  		sout<<"GRB.Beam         FarFieldPointSource 0 "<<Angle[j]<<" "<<endl;
			sout<<"GRB.Spectrum     File "<<Prefix<<i<<"_Angle_"<<Angle[j]<<".dat"<<endl;
			sout<<"GRB.Flux         "<<FluxPhotonsAfterAtmosphere<<endl;
			sout<<"GRB.Polarization RelativeX 1.0 30.0"<<endl;
			sout.close();

  		ofstream snout;
  		snout.open(Prefix + i + "_Angle_" + Angle[j] + "_nopol.source");
  		if (snout.is_open() == false) return false;
  		snout.setf(ios_base::scientific);
  		snout<<"Include Base.source"<<endl;
  		snout<<"MyRun.FileName "<<Prefix<<i<<"_Angle_"<<Angle[j]<<"_nopol"<<endl;
  		snout<<"GRB.Beam         FarFieldPointSource 0 "<<Angle[j]<<" "<<endl;
			snout<<"GRB.Spectrum     File "<<Prefix<<i<<"_Angle_"<<Angle[j]<<".dat"<<endl;
			snout<<"GRB.Flux         "<<FluxPhotonsAfterAtmosphere<<endl;
			snout.close();
		  
 		}
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


AtmosphereAbsorptionsForGRBs* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication AtmosphereAbsorptionsForGRBsApp("AtmosphereAbsorptionsForGRBsApp", 0, 0);

  g_Prg = new AtmosphereAbsorptionsForGRBs();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //AtmosphereAbsorptionsForGRBsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
