/* 
 * ResponseToXSPEC.cxx
 *
 *
 * Copyright (C) by Clio Sleator.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Clio Sleators.
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
#include "fitsio.h"
#include <stdio.h>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TTree.h>

// MEGAlib
#include "MGlobal.h"
#include "MResponseMatrixON.h"
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MFileEventsTra.h"
#include "MFileEventsSim.h"
#include "MSettingsMimrec.h"
#include "MEventSelector.h"

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class ResponseToXSPEC
{
public:
  //! Default constructor
  ResponseToXSPEC();
  //! Default destructor
  ~ResponseToXSPEC();
  
	//! Initialize command line arguments
	bool Initialize();
	//! Check that certain command line arguments were filled out
	bool ConfirmCmdLineArguments();
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
	//! collapse 4d matrix to 2d: using energy.mimrecselected matrix
	void CollapseMatrix(const MResponseMatrixON& RspON,MResponseMatrixO2& RspO2,MResponseMatrixO2& RspO2ForArf);
	//! collapse 4d matrix to 2d: using energy.armcut matrix
	void CollapseMatrixArmCut(const MResponseMatrixON& RspON,MResponseMatrixO2& RspO2,MResponseMatrixO2& RspO2ForArf);
	//! make rmf file
	bool MakeRMF(MResponseMatrixO2 Rsp);
	//! make arf file
	bool MakeARF(MResponseMatrixO2 Rsp);
	//! make pha file
	bool MakePHA(MResponseMatrixO2 Rsp);
	//! read spectrum from tra file
	vector<float> SpectrumFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read spectrum from sim file
	vector<float> SpectrumFromSim(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read spectrum from ttree
	vector<float> SpectrumFromTTree(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from tra file
	vector<float> BkgSpecFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from ttree
	vector<float> BkgSpecFromTTree(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from ttree using phi slices (for Comptel data space case)
	vector<float> BkgSpecFromTTreePhiSlices(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from ttree, separating inner and outer cones, using phi slices
	vector<float> BkgSpecFromTTreeInOutPhiSlices(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from ttree using lookup matrix for scaling (detector coordinates)
	vector<vector<float> > BkgSpecFromTTreeLookupMatrixDetCoords(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read background spectrum from ttree using lookup matrix for scaling
	vector<vector<float> > BkgSpecFromTTreeLookupMatrix(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! calculate errors on source spectrum
	vector<float> MakeSrcError(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! calculate errors on background spectrum
	vector<float> MakeBkgError(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);	
	//! write energy array to txt file (to then pass to Alex's column density program)
	bool GetEnergyArray(MResponseMatrixO2 Rsp);
	//! calculate volume ratio of particular phi slices (defined by h1 and h2) in Comptel data space
	vector<double> CalculateVolumes(double h1, double h2, double a = 7.74);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
	//! input filename
	MString m_FileName;
	//! mode in which the code will run
	MString m_Mode;
	//! file saying how much time is spent in each FISBEL bin
	MString m_TimePerBinFile;
	//! input tra or sim or ttree file
	MString m_DataFileName;
	//! background tra or ttree file
	MString m_BkgFileName;
	//! mimrec source config file
	MString m_CfgFileName;
	//! mimrec background config file
	MString m_BkgCfgFileName;
	//! geometry file name
	MString m_GeoFileName;
	//! output file name
	MString m_OutFileName;
	//! output file name without path
	MString m_OutFileNameNoPath;
	//! power law index for response
	float m_alpha;
	//! minimum energy of response
	float m_Emin;
	//! maximum energy of response
	float m_Emax;
	//! exposure time of pha spectrum
	float m_Exposure;
	//! exposure time of background spectrum
	float m_BkgExposure;
	//! backscal parameter
	float m_Backscal;
	//! phabs filename
	MString m_PhabsFileName;
	//! spectrum filename
	MString m_SpectrumFileName;
	//! if true, calculate error on spectrum using Mark's method
	bool m_CalculateErrorImgReg;
	//! lookup matrix method
	bool m_LookupMatrix;

};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ResponseToXSPEC::ResponseToXSPEC() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ResponseToXSPEC::~ResponseToXSPEC()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////

//! initialize command line arguments
bool ResponseToXSPEC::Initialize(){

	m_FileName = "None";
	m_Mode = "x";
	m_DataFileName = "None";
	m_BkgFileName = "None";
	m_SpectrumFileName = "None";
	m_CfgFileName = "None";
	m_BkgCfgFileName = "None";
	m_GeoFileName = "None";
	m_OutFileName = "out";
	m_OutFileNameNoPath = "out";
	m_alpha = 0;
	m_Emin = 10;
	m_Emax = 10000;
	m_Exposure = 1;
	m_BkgExposure = 1;
	m_Backscal = 1;
	m_PhabsFileName = "None";
	m_CalculateErrorImgReg = false;
	m_LookupMatrix = false;

	return true;

}


////////////////////////////////////////////////////////////////////////////////

bool ResponseToXSPEC::ConfirmCmdLineArguments(){

	if (m_FileName == "None"){
		cout << "Please include a response filename" << endl; return false;
	}
	if (m_Mode != "x" && m_Mode != "e"){
		cout << "Please use a valid mode: options are 'x' or 'e'" << endl; return false;
	}
	if (m_Mode == "x"){
		if (m_DataFileName == "None"){
			cout << "Please include a tra or sim filename" << endl; return false;
		}
		if (m_GeoFileName == "None"){
			cout << "Please include a geometry filename" << endl; return false;
		}
	}

	return true;

}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ResponseToXSPEC::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ResponseToXSPEC <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   rsp file name"<<endl;
	Usage<<"         -m:   mode: 'e' for list of energies, 'x' to make files for XSPEC (default)"<<endl;
	Usage<<"         -tf:	 time per FISBEL bin file name"<<endl;
	Usage<<"         -t:   source tra file OR source ttree root file"<<endl;
	Usage<<"         -bt:  background tra file OR background ttree root file"<<endl;
	Usage<<"         -s:   spectrum file"<<endl;;
	Usage<<"         -g:   geometry file"<<endl;
	Usage<<"         -c:   source mimrec config file OR file containing source selection string"<<endl;
	Usage<<"				 -b:   background mimrec config file OR file containing background selection string"<<endl;
	Usage<<"         -o:   output file name"<<endl;
	Usage<<"         -a:   power law index"<<endl;
	Usage<<"         -e:   minimum simulation energy"<<endl;
	Usage<<"         -E:   maximum simulation energy"<<endl;
	Usage<<"         -x:   source exposure time"<<endl;
	Usage<<"         -bx:  background exposure time"<<endl;
	Usage<<"         -bs:  backscal value"<<endl;
	Usage<<"         -p:   phabs file"<<endl;
	Usage<<"         -er:  no second argument needed; calling this flag means that errors will be calculated according to Mark's paper"<<endl;
	Usage<<"         -l:   no second argument needed; calling this flag means that errors will be calculated according to the Lookup matrix method"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;


	Initialize();

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
    if (Option == "-m") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-t") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-bt") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-s") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
		}
    if (Option == "-g") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
	  if (Option == "-c") {
     if (!((argc > i+1) && 
           (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
 	  if (Option == "-b") {
     if (!((argc > i+1) && 
           (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-o") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-a") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-e") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-E") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-x") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-bx") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-bs") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-p") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-tf") {
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
      cout<<"Accepting response matrix file name: "<<m_FileName<<endl;
    }
		else if (Option == "-m") {
      m_Mode = argv[++i];
      cout<<"Accepting mode: "<<m_Mode<<endl;
    }
    else if (Option == "-t") {
      m_DataFileName = argv[++i];
      cout<<"Accepting tra file name: "<<m_DataFileName<<endl;
    }
    else if (Option == "-bt") {
      m_BkgFileName = argv[++i];
      cout<<"Accepting background tra file name: "<<m_BkgFileName<<endl;
    }
    else if (Option == "-s") {
      m_SpectrumFileName = argv[++i];
      cout<<"Accepting spectrum file name: "<<m_SpectrumFileName<<endl;
    }
	  else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeoFileName<<endl;
    }
    else if (Option == "-c") {
      m_CfgFileName = argv[++i];
      cout<<"Accepting mimrec source configuration file name: "<<m_CfgFileName<<endl;
    }
    else if (Option == "-b") {
      m_BkgCfgFileName = argv[++i];
      cout<<"Accepting mimrec background configuration file name: "<<m_BkgCfgFileName<<endl;
    }
	  else if (Option == "-o") {
      m_OutFileName = argv[++i];
			vector<MString> path_split = m_OutFileName.Tokenize("/");
			m_OutFileNameNoPath = path_split[path_split.size()-1];
      cout<<"Accepting output file name: "<<m_OutFileName<<endl;
			cout<<"Accepting output file name without path: "<<m_OutFileNameNoPath<<endl;
    }
    else if (Option == "-a") {
      m_alpha = atof(argv[++i]);
      cout<<"Accepting power law index: "<<m_alpha<<endl;
    }
    else if (Option == "-e") {
      m_Emin = atof(argv[++i]);
      cout<<"Accepting minimum energy: "<<m_Emin<<endl;
    }
    else if (Option == "-E") {
      m_Emax = atof(argv[++i]);
      cout<<"Accepting maximum energy: "<<m_Emax<<endl;
    }
    else if (Option == "-x") {
      m_Exposure = atof(argv[++i]);
      cout<<"Accepting exposure time: "<<m_Exposure<<endl;
    }
		else if (Option == "-bx") {
			m_BkgExposure = atof(argv[++i]);
			cout<<"Accepting background exposure time: "<<m_BkgExposure<<endl;
		}
		else if (Option == "-bs") {
			m_Backscal = atof(argv[++i]);
			cout<<"Accepting background scaling: "<<m_Backscal<<endl;
		}
		else if (Option == "-p") {
			m_PhabsFileName = argv[++i];
			cout<<"Accepting phabs file name: "<<m_PhabsFileName<<endl;
		}
		else if (Option == "-tf") {
			m_TimePerBinFile = argv[++i];
			cout<<"Accepting time per FISBEL bin file name: "<<m_TimePerBinFile<<endl;
		}
		else if (Option == "-er") {
			m_CalculateErrorImgReg = true;
			cout<<"Calculating errors with Mark's method."<<endl;
		}
		else if (Option == "-l") {
			m_LookupMatrix = true;
			cout<<"Calculating errors with Lookup matrix."<<endl;
		}
		else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }

  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ResponseToXSPEC::Analyze()
{
  if (m_Interrupt == true) return false;

	if(!ConfirmCmdLineArguments()){ return false; }

	//load response matrix
	MResponseMatrixON RspON;
	if (!RspON.Read(m_FileName)){
		cout << "failed to read rsp matrix, aborting..." << endl;
		return false;
	}

	MResponseMatrixO2 RspO2;
	MResponseMatrixO2 RspO2ForArf;
	CollapseMatrix(RspON, RspO2, RspO2ForArf);
//	CollapseMatrixArmCut(RspON, RspO2, RspO2ForArf);

	if (m_Mode == "x"){

		MakeRMF(RspO2);
		MakeARF(RspO2ForArf);

		MakePHA(RspO2);
	}

	else {
		GetEnergyArray(RspO2);
	}

	return true;

}

void ResponseToXSPEC::CollapseMatrix(const MResponseMatrixON& RspON, MResponseMatrixO2& RspO2, MResponseMatrixO2& RspO2ForArf){

		//make 2D response matrix, transfer 4D Rsp to 2D one of just energies
//		MResponseMatrixO2 RspO2;

		RspO2.SetSimulatedEvents(RspON.GetSimulatedEvents());
		RspO2.SetFarFieldStartArea(RspON.GetFarFieldStartArea());

		//load file with time per FISBEL bin
		vector<double> TimePerBin;
		float totalTime = 0.;

		ifstream binTimeF(m_TimePerBinFile);
		double a,b;
		if (binTimeF.is_open()){
			while (binTimeF >> a >> b){
				TimePerBin.push_back(b);
				totalTime += b;
			}
		}
		cout << "total time: " << totalTime << endl;


		//get theta phi bin: this part will need to be changed when source is moving
//		int ThPhBin = RspON.GetAxis(1).GetAxisBin(40,0);
//		cout << ThPhBin << endl;

		MResponseMatrixAxis IdealAx = RspON.GetAxis(0);
		MResponseMatrixAxis MeasuredAx = RspON.GetAxis(2);

		if (!(IdealAx.Has1DBinEdges() && MeasuredAx.Has1DBinEdges())){
			cout << "no 1D bin edges...aborting" << endl;
			return;
		}

		vector<double> idealEdgesD = IdealAx.Get1DBinEdges();
		vector<float> idealEdges(idealEdgesD.begin(), idealEdgesD.end());
		vector<double> measuredEdgesD = MeasuredAx.Get1DBinEdges();
		vector<float> measuredEdges(measuredEdgesD.begin(), measuredEdgesD.end());

		RspO2.SetAxisNames(IdealAx.GetNames()[0], MeasuredAx.GetNames()[0]);
		RspO2.SetAxis(idealEdges,measuredEdges);

		RspO2ForArf = RspO2;

		int ThPhBin = RspON.GetAxis(1).GetAxisBin(0,0);
		cout << "ThPhBin: " << ThPhBin << endl;


		vector<unsigned long> v;
		for (unsigned long i=0; i<IdealAx.GetNumberOfBins(); i++){
			for (unsigned long j=0; j<MeasuredAx.GetNumberOfBins(); j++){
				float sum = 0;
				float weighted_avg = 0;
				float T = 0;

				for (unsigned long b=0; b<TimePerBin.size(); b++){

					v.clear();
					v.push_back(i);
					v.push_back(b);
					v.push_back(j);

					float binContent = RspON.Get(v);

//					if (binContent != 0 && TimePerBin.at(b) == 0){ cout << b << endl; }
					if (binContent != 0){
						sum += binContent;
						weighted_avg += binContent*TimePerBin.at(b);
						T += TimePerBin.at(b);
					}

				}
				if (T!=0){ weighted_avg = weighted_avg/T; }
				RspO2.SetBinContent(i,j,weighted_avg);
//				RspO2.SetBinContent(i,j,sum);
				RspO2ForArf.SetBinContent(i,j,sum);
			}
		}

		cout << "matrix collapsed" << endl;

//	RspO2.Write("Oldway.rsp");

	return;

}

void ResponseToXSPEC::CollapseMatrixArmCut(const MResponseMatrixON& RspON, MResponseMatrixO2& RspO2, MResponseMatrixO2& RspO2ForArf){

		//make 2D response matrix, transfer 4D Rsp to 2D one of just energies
//		MResponseMatrixO2 RspO2;

		RspO2.SetSimulatedEvents(RspON.GetSimulatedEvents());
		RspO2.SetFarFieldStartArea(RspON.GetFarFieldStartArea());

		RspO2ForArf.SetSimulatedEvents(RspON.GetSimulatedEvents());
		RspO2ForArf.SetFarFieldStartArea(RspON.GetFarFieldStartArea());

		//load file with time per FISBEL bin
		vector<float> TimePerBin;
		float totalTime = 0.;

		ifstream binTimeF(m_TimePerBinFile);
		float a,b;
		if (binTimeF.is_open()){
			while (binTimeF >> a >> b){
				TimePerBin.push_back(b);
				totalTime += b;
			}
		}
		cout << "total time: " << totalTime << endl;


		MResponseMatrixAxis IdealAx = RspON.GetAxis(0);
		MResponseMatrixAxis MeasuredAx = RspON.GetAxis(1);

		if (!(IdealAx.Has1DBinEdges() && MeasuredAx.Has1DBinEdges())){
			cout << "no 1D bin edges...aborting" << endl;
			return;
		}

		vector<double> idealEdgesD = IdealAx.Get1DBinEdges();
		vector<float> idealEdges(idealEdgesD.begin(), idealEdgesD.end());
		vector<double> measuredEdgesD = MeasuredAx.Get1DBinEdges();
		vector<float> measuredEdges(measuredEdgesD.begin(), measuredEdgesD.end());

		RspO2.SetAxisNames(IdealAx.GetNames()[0], MeasuredAx.GetNames()[0]);
		RspO2.SetAxis(idealEdges,measuredEdges);

		RspO2ForArf.SetAxisNames(IdealAx.GetNames()[0], MeasuredAx.GetNames()[0]);
		RspO2ForArf.SetAxis(idealEdges,measuredEdges);

//		unsigned int ThPhBin = RspON.GetAxis(2).GetAxisBin(39,5);
		unsigned int ThPhBin = RspON.GetAxis(2).GetAxisBin(0,0);
		cout << "ThPhBin: " << ThPhBin << endl;
		vector<double> BinCenters = RspON.GetAxis(2).GetBinCenters(ThPhBin);
		cout << BinCenters[0] << '\t' << BinCenters[1] << endl;
		cout << RspON.GetAxis(2).GetArea(ThPhBin) << endl;

/*		MResponseMatrixAxisSpheric ArmCut("theta","phi");
		ArmCut.SetFISBEL(1650);
		int bin = ArmCut.GetAxisBin(39,5);
		cout << ArmCut.GetArea(bin) << endl;
		vector<MVector> ArmBinCenters = ArmCut.GetAllBinCenters();
		for (int i=0; i<ArmBinCenters.size(); i++){
			cout << ArmBinCenters[i].Phi()*c_Deg << '\t' << ArmBinCenters[i].Theta()*c_Deg << endl;
		}
*/

		vector<unsigned long> v;
		for (unsigned long i=0; i<IdealAx.GetNumberOfBins(); i++){
			for (unsigned long j=0; j<MeasuredAx.GetNumberOfBins(); j++){
				//float sum = 0;
				float weighted_avg = 0;
				float T = 0;

				for (unsigned long b=0; b<TimePerBin.size(); b++){
					if (b!=ThPhBin){ continue; } //DELETE THIS LINE
					TimePerBin.at(b) = 1; //DELETE THIS LINE TOO

					v.clear();
					v.push_back(i);
					v.push_back(j);
					v.push_back(b);

					float binContent = RspON.Get(v);
					if (binContent != 0){
						//sum += binContent;
						weighted_avg += binContent*TimePerBin.at(b);
						T += TimePerBin.at(b);
					}

				}
				if (T!=0){ weighted_avg = weighted_avg/T; }
					RspO2.SetBinContent(i,j,weighted_avg);
					RspO2ForArf.SetBinContent(i,j,weighted_avg);
          // RspO2.SetBinContent(i,j,sum);
          // RspO2ForArf.SetBinContent(i,j,sum);
			}
		}

  // RspO2.Write("Armcut.rsp");

	return;

}

bool ResponseToXSPEC::MakeRMF(MResponseMatrixO2 Rsp){

	//(1) Normalize rsp: each row normalized to 1 detected photon
	int nBinsMeasured = Rsp.GetAxisBins(2);
	int nBinsIdeal = Rsp.GetAxisBins(1);

	float sum;
	for (int x=0; x<nBinsIdeal; x++){
		sum = 0;
		for (int y=0; y<nBinsMeasured; y++){
			sum += Rsp.GetBinContent(x,y);
		}
		for (int y=0; y<nBinsMeasured; y++){
			float content = Rsp.GetBinContent(x,y);
			if (content != 0){
				content = content/sum;
				Rsp.SetBinContent(x,y,content);
			}
		}
	}

//	Rsp.Write("Armcut.rsp");
	
	//(2) Make arrays for EBOUNDS and MATRIX
	//EBOUNDS defines channels
	//Just use measured energy bins as defined in Rsp
	vector<int> channels;
	vector<float> eMin; vector<float> eMax;

	for (int y=0; y<nBinsMeasured; y++){
		channels.push_back(y);
		eMin.push_back(Rsp.GetAxisLowEdge(y,2));
		eMax.push_back(Rsp.GetAxisHighEdge(y,2));
//		cout << Rsp.GetAxisLowEdge(y,2) << '\t' << Rsp.GetAxisHighEdge(y,2) << ":" << '\t';
//		cout << Rsp.GetAxisHighEdge(y,2) - Rsp.GetAxisLowEdge(y,2) << endl;
	}


	//MATRIX is a bit more complicated...
	vector<float> eLow; vector<float> eHigh;
	vector<int> Ngrp;
	vector<vector<int> > Fchan; vector<vector<int> > Nchan;
	vector<vector<float> > Mat;

	for (int x=0; x<nBinsIdeal; x++){
		bool inSubset = false; //keeps track if we are in the middle of a channel subset
		int Ngrp_temp = 0; //counts number of channel subsets
		vector<int> Fchan_temp; vector<int> Nchan_temp;
		vector<float> Mat_temp;

		for (int y=0; y<nBinsMeasured; y++){
			float value = Rsp.GetBinContent(x,y);
			//I think following two lines make things worse..
//			float bin = Rsp.GetAxisHighEdge(x,1)-Rsp.GetAxisLowEdge(x,1);
//			value = value / bin;
			if (inSubset && value != 0){
				Mat_temp.push_back(value);
			}
			else if (inSubset && value == 0){
				Nchan_temp.push_back(y-Fchan_temp.at(Fchan_temp.size()-1));
				inSubset = false;
			}
			else if (!inSubset && value != 0){
				Ngrp_temp++;
				Fchan_temp.push_back(y);
				Mat_temp.push_back(value);
				inSubset = true;
			}
			else {continue; }
		}

		eLow.push_back(Rsp.GetAxisLowEdge(x,1));
		eHigh.push_back(Rsp.GetAxisHighEdge(x,1));

		//so that there aren't empty values
		if (Ngrp_temp == 0){
			Fchan_temp.push_back(0);
			Nchan_temp.push_back(0);
			Mat_temp.push_back(0);
		}

		Ngrp.push_back(Ngrp_temp);
		Fchan.push_back(Fchan_temp);
		Nchan.push_back(Nchan_temp);
		Mat.push_back(Mat_temp);

/*		if (Ngrp_temp == 0){
			for (int y=0; y<nBinsMeasured; y++){
				if (Rsp.GetBinContent(x,y) != 0){
					cout << "PROBLEM" << endl;
				}
			}
		}

		if (!(Fchan_temp.size() == Nchan_temp.size() && Fchan_temp.size() == Ngrp_temp)){
			cout << "FALSE" << endl;
		}
		int s = 0;
		for (int i=0; i<Nchan_temp.size(); i++){
			s += Nchan_temp.at(i);
		}
		if (s != Mat_temp.size()){ cout << "false" << endl; }
*/

	}

/*	for (int i=0; i<Ngrp.size(); i++){
		if (Ngrp.at(i) != 0){ cout << i << endl; }
	}*/

	//(3) Make and fill FITS file
	fitsfile *rmf = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&rmf, (m_OutFileName+MString(".rmf")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	// (3a) create image as primary (for some reason all responses have this
	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(rmf, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	// (3b) make two binary extensions: one for matrix, one for channel definitions

	//make MATRIX binary extension: redistribution matrix
	status = 0;
	const char* m_column_names[] = {"ENERG_LO", "ENERG_HI", "N_GRP", "F_CHAN", "N_CHAN", "MATRIX"};
	const char* m_format[] = {"E", "E", "I", "PI", "PI", "PE"}; //not sure about last 3
	const char* m_units[] = {"keV", "keV", "", "", "", ""};
	int nrows = eLow.size();

	if (fits_create_tbl(rmf, BINARY_TBL, nrows, 6, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"MATRIX", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}

	//1D arrays are easier
	float* eLowArr = &eLow[0];
	float* eHighArr = &eHigh[0];
	int* NgrpArr = &Ngrp[0];

	if (fits_write_col(rmf, TFLOAT, 1, 1, 1, nrows, eLowArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 2, 1, 1, nrows, eHighArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TINT, 3, 1, 1, nrows, NgrpArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}


	//2d arrays: have to go one element at a time
	int size;
	int* FchanArr;
	int* NchanArr;
	float* MatArr;
	for (int i=0; i<nrows; i++){

		//Fchan
		FchanArr = &Fchan[i][0];
		size = Fchan[i].size();

		status = 0;
		if (fits_write_col(rmf, TINT, 4, i+1, 1, size, FchanArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

		//Nchan
		NchanArr = &Nchan[i][0];
		size = Fchan[i].size();

		status = 0;
		if (fits_write_col(rmf, TINT, 5, i+1, 1, size, NchanArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

		//Mat
		MatArr = &Mat[i][0];
		size = Mat[i].size();

		status = 0;
		if (fits_write_col(rmf, TFLOAT, 6, i+1, 1, size, MatArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

	}


	//MATRIX header keywords
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite name", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	int detchans = channels.size();
	if (fits_write_key(rmf, TINT, (char*)"DETCHANS", &detchans, "total number of detector channels", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS2", (char*)"RSP_MATRIX", "dataset is a spectral response matrix", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUVERS", (char*)"1.3.0", "version of format", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	int number = 0;
	if (fits_write_key(rmf, TINT, (char*)"TLMIN4", &number, "minimum value legally allowed in column 4", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}




	//make EBOUNDS binary extension: to define 'channels'
	//COSI doesn't have channels, so we use energy bins
	status = 0;
	nrows = channels.size();
	const char* e_column_names[] = {"CHANNEL","E_MIN","E_MAX"};
	const char* e_format[] = {"I","E","E"};
	const char* e_units[] = {"","keV","keV"};

	if (fits_create_tbl(rmf, BINARY_TBL, nrows, 3, (char**)e_column_names, (char**)e_format, (char**)e_units, (char*)"EBOUNDS", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}

	//add values to table
	int* channelArr = &channels[0];
	float* eMinArr = &eMin[0];
	float* eMaxArr = &eMax[0];

	status = 0;
	if (fits_write_col(rmf, TINT, 1, 1, 1, nrows, channelArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 2, 1, 1, nrows, eMinArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 3, 1, 1, nrows, eMaxArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}


	//EBOUNDS header keywords
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TINT, (char*)"DETCHANS", &detchans, "total number of detector channels", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS2", (char*)"EBOUNDS", "dataset is a spectral response matrix", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUVERS", (char*)"1.2.0", "version of format", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}



	status = 0;
	if (fits_close_file(rmf, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "rmf file closed" << endl;

  return true;
}


bool ResponseToXSPEC::MakeARF(MResponseMatrixO2 Rsp){

	int nBinsMeasured = Rsp.GetAxisBins(2);
	int nBinsIdeal = Rsp.GetAxisBins(1);


	//(1) Define channels; figure out how many Nstart counts are in each channel
	vector<float> eLow; vector<float> eHigh;

	for (int y=0; y<nBinsMeasured; y++){
		eLow.push_back(Rsp.GetAxisLowEdge(y,2));
		eHigh.push_back(Rsp.GetAxisHighEdge(y,2));
	}

	//will need to rewrite this part based on what Andreas puts in RSP
	int nChan = eLow.size();
	double NStart = Rsp.GetSimulatedEvents();
	double AStart = Rsp.GetFarFieldStartArea();

//	NStart = 9299955.+5.*1030137741.+53*103020498.+3*1030155669.;
//	NStart = 53197100*24.; //for GRB response matrices
//	NStart = 1030102739.*48; 
//	NStart = 1030102983.;
	AStart = 11309.7;

	//figuring out NStartPerChan is the tricky part of the effective area
	//can either do it based on a known functional form (i.e. powerlaw)
	// or a spectrum file, as given to cosima
	vector<float> NStartPerChan;

	bool plaw = true;
	if (m_SpectrumFileName != "None"){ plaw = false; }
	//powerlaw case
	if (plaw){
		cout << "Accepting powerlaw" << endl;
		cout << "NStart: " << NStart << endl;
		cout << "AStart: " << AStart << endl;
		cout << "Alpha: " << m_alpha << endl;
		cout << "Emin: " << m_Emin << endl;
		cout << "Emax: " << m_Emax << endl;

		//assume f(E)dE = K(E^-a)dE
		//need to find K
		float K = NStart*(1-m_alpha)/(pow(m_Emax,1-m_alpha)-pow(m_Emin,1-m_alpha));

		for (int c=0; c<nChan; c++){
			float deltaE = eHigh.at(c)-eLow.at(c);
			//set energy to middle of bin?
			float E = eHigh.at(c)-deltaE/2.;
			float N = K*pow(E,-m_alpha)*deltaE;
			NStartPerChan.push_back(N);
		}
	}
	//spectrum file case: arbitrary spectral shape
	else {
		//read spectral file
		ifstream specFile(m_SpectrumFileName);
		string s;
		double e, c;

		vector<float> energyFromFile;
		vector<float> countsFromFile;

		if (specFile.is_open()){
			//read first line: don't actually need this information
			getline(specFile, s);
			while (specFile >> s >> e >> c){
				energyFromFile.push_back(e);
				countsFromFile.push_back(c);
			}
		}

		//interpolate: can use TGraph
		int nGrPoints = energyFromFile.size();
		float* grX = &energyFromFile[0];
		float* grY = &countsFromFile[0];
		TGraph gr(nGrPoints, grX, grY);

//		TFile f("graph.root","RECREATE");
//		gr.Write();
//		f.Close();

		vector<float> SpectralShape;
		float totalCounts = 0;

		for (int c=0; c<nChan; c++){
			float deltaE = eHigh.at(c)-eLow.at(c);
			float E = eHigh.at(c)-deltaE/2.;

			float N = gr.Eval(E)*deltaE;
//			cout << E << '\t' << N << endl;

			SpectralShape.push_back(N);
			//only want to count counts in the range that was simulated
			if (E >= m_Emin && E <= m_Emax){ totalCounts += N; }
		}

		cout << "NStart: " << NStart << endl;
		cout << "totalCounts: " << totalCounts << endl;
		cout << "nChan: " << nChan << endl;

		//find multiplicative factor so that flux is right
		float factor = NStart/totalCounts;
		cout << "factor: " << factor << endl;
		for (int c=0; c<nChan; c++){
			NStartPerChan.push_back(factor*SpectralShape.at(c));
		}
		
	}


	//include phabs in effective area
	bool usePhabs = false;
	vector<float> phabs;
	if (m_PhabsFileName != "None"){ usePhabs = true; }
	if (usePhabs){
		string line;
		ifstream phabsF(m_PhabsFileName);
		if (phabsF.is_open()){
			while( getline (phabsF,line) ){
				phabs.push_back(atof(line.c_str()));
			}
			phabsF.close();
		}
	}


	ofstream ofm("nMeasured.txt",ofstream::out);
	//(2) Make effective area array
	vector<float> Aeff;
	vector<float> AeffPhabs;
	for (int x=0; x<nBinsIdeal; x++){
		float nMeasuredCounts = 0;
		for (int y=0; y<nBinsMeasured; y++){
			nMeasuredCounts += Rsp.GetBinContent(x,y);
		}
		Aeff.push_back(AStart*nMeasuredCounts/NStartPerChan.at(x));
//		Aeff.push_back(nMeasuredCounts/NStartPerChan.at(x));
		ofm << eHigh.at(x) << '\t' << nMeasuredCounts << endl;
		if (usePhabs){
			AeffPhabs.push_back(AStart*nMeasuredCounts*phabs.at(x)/NStartPerChan.at(x));
		}
	}
	ofm.close();
	cout << nBinsMeasured << '\t' << nChan << '\t' << nBinsIdeal << endl;


	ofstream ofs("arf.txt",ofstream::out);
	for (int c=0; c<nChan; c++){
		ofs << eHigh.at(c) << '\t' << Aeff.at(c) << endl;
	}
	ofs.close();

	if (usePhabs){
		ofstream ofs2("arf_abs.txt",ofstream::out);
		for (int c=0; c<nChan; c++){
			ofs2 << eHigh.at(c) << '\t' << AeffPhabs.at(c) << endl;
		}
		ofs2.close();
	}

	//(3) Make and fill FITS file
	fitsfile *arf = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&arf, (m_OutFileName+MString(".arf")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	// (3a) create image as primary (for some reason all responses have this
	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(arf, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	// (3b) make binary table
	const char* m_column_names[] = {"ENERG_LO", "ENERG_HI", "SPECRESP"};
	const char* m_format[] = {"E", "E", "E"};
	const char* m_units[] = {"keV", "keV", "cm**2"};
	int nrows = eLow.size();

	status = 0;
	if (fits_create_tbl(arf, BINARY_TBL, nrows, 3, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECRESP", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}


	// (3c) fill table
	float* eLowArr = &eLow[0];
	float* eHighArr = &eHigh[0];
	float* AeffArr = &Aeff[0];

	status = 0;
	if (fits_write_col(arf, TFLOAT, 1, 1, 1, nrows, eLowArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(arf, TFLOAT, 2, 1, 1, nrows, eHighArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(arf, TFLOAT, 3, 1, 1, nrows, AeffArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	//table header
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "detector/instrument name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"EXTNAME", (char*)"SPECRESP", "name of this binary extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUCLAS2", (char*)"SPECRESP", "extension contains an ARF", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUVERS", (char*)"1.1.0", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}



	status = 0;
	if (fits_close_file(arf, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "arf file closed" << endl;



	//(4) Make and fill absorbed effective area FITS file
	if (usePhabs){
		fitsfile *arf_abs = NULL;

		//almost all cfitsio routines have status parameter as argument
		//if it's not set to 0, the routine will fail
		int status = 0;
		if (fits_create_file(&arf_abs, (m_OutFileName+MString("_abs.arf")).Data(), &status)){
			cout << "error in fits_create_file" << endl;
			fits_report_error(stdout, status);
		}
	
		// (4a) create image as primary (for some reason all responses have this
		status = 0;
		long ax[2];
		ax[0] = 0; ax[1] = 0;
		if (fits_create_img(arf_abs, FLOAT_IMG, 0, ax, &status)){
			cout << "error in fits_create_img" << endl;
			fits_report_error(stdout, status);
		}
	
		//add keywords to header
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
	
		// (4b) make binary table
		const char* m_column_names[] = {"ENERG_LO", "ENERG_HI", "SPECRESP"};
		const char* m_format[] = {"E", "E", "E"};
		const char* m_units[] = {"keV", "keV", "cm**2"};
		int nrows = eLow.size();
	
		status = 0;
		if (fits_create_tbl(arf_abs, BINARY_TBL, nrows, 3, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECRESP", &status)){
			cout << "error in fits_create_tbl" << endl;
			fits_report_error(stdout, status);
		}
	
	
		// (4c) fill table
		float* eLowArr = &eLow[0];
		float* eHighArr = &eHigh[0];
		float* AeffArr = &AeffPhabs[0];
	
		status = 0;
		if (fits_write_col(arf_abs, TFLOAT, 1, 1, 1, nrows, eLowArr, &status)){
			cout << "error in fits_write_col: SPECRESP table" << endl;
			fits_report_error(stdout, status);
		}
	
		status = 0;
		if (fits_write_col(arf_abs, TFLOAT, 2, 1, 1, nrows, eHighArr, &status)){
			cout << "error in fits_write_col: SPECRESP table" << endl;
			fits_report_error(stdout, status);
		}
	
		status = 0;
		if (fits_write_col(arf_abs, TFLOAT, 3, 1, 1, nrows, AeffArr, &status)){
			cout << "error in fits_write_col: SPECRESP table" << endl;
			fits_report_error(stdout, status);
		}
	
		//table header
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite name", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"INSTRUME", (char*)"COSI", "detector/instrument name", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"EXTNAME", (char*)"SPECRESP", "name of this binary extension", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"HDUCLAS2", (char*)"SPECRESP", "extension contains an ARF", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
		status = 0;
		if (fits_write_key(arf_abs, TSTRING, (char*)"HDUVERS", (char*)"1.1.0", "version of format", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
	
	
		status = 0;
		if (fits_close_file(arf_abs, &status)){
			cout << "error in fits_close_file" << endl;
			fits_report_error(stdout, status);
		}
		cout << "absorbed arf file closed" << endl;
	}


	return true;

}

vector<float> ResponseToXSPEC::SpectrumFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//return this vector if something is wrong with one of the files
	vector<float> returnIfBad;

	//initialize mimrec stuff
	//load geometry
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeoFileName) == true){
		cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
	}
	else{
		cout << "Unable to load geometry " << Geometry->GetName() << " - Aborting!" << endl;
		return returnIfBad;
	}

	//mimrec settings
	MSettingsMimrec MimrecSettings;
	if (MimrecSettings.Read(m_CfgFileName) == false){
		cout << "Unable to open mimrec configuration file " << m_CfgFileName;
		cout << " - Aborting!" << endl;
		return returnIfBad;
	}
	MEventSelector MimrecEventSelector;
	MimrecEventSelector.SetSettings(&MimrecSettings);

	MFileEventsTra EventFile;
	if (EventFile.Open(m_DataFileName) == false){
		cout << "Unable to open file " << m_DataFileName<<" - Aborting!" << endl;
		return returnIfBad;
	}


	//make energy-vs-channel array
	int nChans = eLow.size();
	vector<float> energy(nChans,0.);

	EventFile.ShowProgress();

	MPhysicalEvent* Event;

	while ((Event = EventFile.GetNextEvent()) != 0){
		if (Event->GetType() == c_ComptonEvent || Event->GetType() == c_PhotoEvent){
			if (MimrecEventSelector.IsQualifiedEvent(Event) == true){
				double E = Event->GetEnergy();

				for (int c=0; c<nChans; c++){
					if (E >= eLow.at(c) && E < eHigh.at(c)){
						energy[c] += 1;
						break;
					}
				}
			}
		}
	}

	ofstream ofs("spec.txt",ofstream::out);
	for (int c=0; c<nChans; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

vector<float> ResponseToXSPEC::BkgSpecFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//return this vector if something is wrong with one of the files
	vector<float> returnIfBad;

	//initialize mimrec stuff
	//load geometry
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeoFileName) == true){
		cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
	}
	else{
		cout << "Unable to load geometry " << Geometry->GetName() << " - Aborting!" << endl;
		return returnIfBad;
	}

	//mimrec settings
	MSettingsMimrec MimrecSettings;
	//figure out if there is a separate cfg file or just use src one
	MString cfgName;
	if (m_BkgCfgFileName != "None"){
		cfgName = m_BkgCfgFileName;
	} else { cfgName = m_CfgFileName; }

	if (MimrecSettings.Read(cfgName) == false){
		cout << "Unable to open mimrec configuration file " << m_CfgFileName;
		cout << " - Aborting!" << endl;
		return returnIfBad;
	}
	MEventSelector MimrecEventSelector;
	MimrecEventSelector.SetSettings(&MimrecSettings);

	MFileEventsTra EventFile;

	//figure out if there's a separate bkg tra file
	MString fName;
	if (m_BkgFileName != "None"){
		fName = m_BkgFileName;
	} else { fName = m_DataFileName; }

	if (EventFile.Open(fName) == false){
		cout << "Unable to open file " << m_DataFileName<<" - Aborting!" << endl;
		return returnIfBad;
	}


	//make energy-vs-channel array
	int nChans = eLow.size();
	vector<float> energy(nChans,0.);

	EventFile.ShowProgress();

	MPhysicalEvent* Event;

	while ((Event = EventFile.GetNextEvent()) != 0){
		if (Event->GetType() == c_ComptonEvent || Event->GetType() == c_PhotoEvent){
			if (MimrecEventSelector.IsQualifiedEvent(Event) == true){
				double E = Event->GetEnergy();

				for (int c=0; c<nChans; c++){
					if (E >= eLow.at(c) && E < eHigh.at(c)){
						energy[c] += 1;
						break;
					}
				}
			}
		}
	}

	ofstream ofs("bkgspec.txt",ofstream::out);
	for (int c=0; c<nChans; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

vector<float> ResponseToXSPEC::SpectrumFromTTree(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	cout << m_DataFileName << endl;
	TFile f(m_DataFileName);
	TTree* SrcTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D spec("spec","",nChan, &bins[0]);

	//get selection string
	string selection;
	if (m_CfgFileName == "None"){ selection = ""; }
	else{
		ifstream selectionF(m_CfgFileName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//fill histogram with correct selection from TTree
	double ncounts = SrcTree->Draw("Energy>>spec",MString(selection),"goff");
	cout << "n source counts  " << ncounts << endl;

	//try absorption-correcting the spectrum
/*	vector<float> phabs;
	string line;
	ifstream phabsF(m_PhabsFileName);
	if (phabsF.is_open()){
		while( getline (phabsF,line) ){
			phabs.push_back(atof(line.c_str()));
		}
		phabsF.close();
	}
*/
	double total = 0;
	vector<float> energy;
	//start iterating at 1 because 0 is underflow bin in root histograms
	for (int i=1; i<nChan+1; i++){
//		energy.push_back(spec.GetBinContent(i)/phabs[i]);
		energy.push_back(spec.GetBinContent(i));
		total += spec.GetBinContent(i);
	}

	cout << "N source counts: " << total << endl;

	ofstream ofs("specFromTTree.txt",ofstream::out);
	for (int c=0; c<nChan; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

vector<float> ResponseToXSPEC::BkgSpecFromTTree(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//see if there's a separate background file 
	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D bkspec("bkspec","",nChan, &bins[0]);

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//fill histogram with correct selection from TTree
	double ncounts = BkgTree->Draw("Energy>>bkspec",MString(selection),"goff");
	cout << "n background counts  " << ncounts << endl;

	//try absorption-correcting the spectrum
/*	vector<float> phabs;
	string line;
	ifstream phabsF(m_PhabsFileName);
	if (phabsF.is_open()){
		while( getline (phabsF,line) ){
			phabs.push_back(atof(line.c_str()));
		}
		phabsF.close();
	}
*/
	vector<float> energy;
	//start iterating at 1 because 0 is underflow bin in root histograms
	for (int i=1; i<nChan+1; i++){
//		energy.push_back(bkspec.GetBinContent(i)/phabs[i]);
		energy.push_back(bkspec.GetBinContent(i));
	}

	return energy;

}

vector<float> ResponseToXSPEC::BkgSpecFromTTreePhiSlices(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//see if there's a separate background file 
	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D bkspec("bkspec","",nChan, &bins[0]);

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//fill histogram with correct selection from TTree for each phi slice
	double slice_size = 3.0;
	double max_ComptonAngle = 65.0;
	double min_ComptonAngle = 0.0;

	double current_low = min_ComptonAngle;
	double current_high = min_ComptonAngle + slice_size;

	while (current_low < max_ComptonAngle){

		if (current_high > max_ComptonAngle){
			current_high = max_ComptonAngle;
		}

		//edit selection string to only use current phi slice
		string selection_slice;
		if (selection.empty()){
			selection_slice = "ComptonAngle>="+MString(current_low)+"&&ComptonAngle<"+MString(current_high);
		}
		else {
			selection_slice = selection.append("&&ComptonAngle>="+MString(current_low)+"&&ComptonAngle<"+MString(current_high));
		}

		//get ttree info into histogram
		TH1D bkspec_one_slice("bkspec"+MString(current_high),"",nChan,&bins[0]);
		BkgTree->Draw("Energy>>bkspec"+MString(current_high),MString(selection_slice),"goff");

		vector<double> volumes = CalculateVolumes(current_low,current_high);
		double V = volumes[0]/(volumes[1]+volumes[2]);
		bkspec_one_slice.Scale(V);

		bkspec.Add(&bkspec_one_slice);

		//update current slice
		current_low = current_high;
		current_high = current_low + slice_size;

	}



	vector<float> energy;
	//start iterating at 1 because 0 is underflow bin in root histograms
	for (int i=1; i<nChan+1; i++){
//		energy.push_back(bkspec.GetBinContent(i)/phabs[i]);
		energy.push_back(bkspec.GetBinContent(i));
	}

	return energy;

}

vector<float> ResponseToXSPEC::BkgSpecFromTTreeInOutPhiSlices(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//see if there's a separate background file 
/*	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");
*/

	//hard code FOR NOW
	MString oName = "/data1/users/clio/ComptelDS/SpectralAnalysis/Tratrees/BkgPlusPlawSrcComptAng_Outer.tratree.root";
	MString iName = "/data1/users/clio/ComptelDS/SpectralAnalysis/Tratrees/BkgPlusPlawSrcComptAng_Inner.tratree.root";

	TFile fo(oName);
	TFile fi(iName);
	TTree* OuterTree = (TTree*) fo.Get("cosi_tra_tree");
	TTree* InnerTree = (TTree*) fi.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D bkspec("bkspec","",nChan, &bins[0]);

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//fill histogram with correct selection from TTree for each phi slice
	double slice_size = 3.0;
	double max_ComptonAngle = 65.0;
	double min_ComptonAngle = 0.0;

	double current_low = min_ComptonAngle;
	double current_high = min_ComptonAngle + slice_size;

	while (current_low < max_ComptonAngle){

		if (current_high > max_ComptonAngle){
			current_high = max_ComptonAngle;
		}

		//edit selection string to only use current phi slice
		string selection_slice;
		if (selection.empty()){
			selection_slice = "ComptonAngle>="+MString(current_low)+"&&ComptonAngle<"+MString(current_high);
		}
		else {
			selection_slice = selection.append("&&ComptonAngle>="+MString(current_low)+"&&ComptonAngle<"+MString(current_high));
		}

		//get ttree info into histogram
		TH1D outer_one_slice("outer"+MString(current_high),"",nChan,&bins[0]);
		OuterTree->Draw("Energy>>outer"+MString(current_high),MString(selection_slice),"goff");
		TH1D inner_one_slice("inner"+MString(current_high),"",nChan,&bins[0]);
		InnerTree->Draw("Energy>>inner"+MString(current_high),MString(selection_slice),"goff");

		vector<double> volumes = CalculateVolumes(current_low,current_high);
		double VO = volumes[3]/volumes[1];
		double VI = volumes[4]/volumes[2];	

		if (volumes[2] ==  0){
			outer_one_slice.Scale(VO);
			bkspec.Add(&outer_one_slice);
		}
		else {
			outer_one_slice.Scale(VO);
			inner_one_slice.Scale(VI);
//			outer_one_slice.Scale(0.5);
//			inner_one_slice.Scale(0.5);
			bkspec.Add(&outer_one_slice);
			bkspec.Add(&inner_one_slice);
		}

		//update current slice
		current_low = current_high;
		current_high = current_low + slice_size;

	}



	vector<float> energy;
	//start iterating at 1 because 0 is underflow bin in root histograms
	for (int i=1; i<nChan+1; i++){
//		energy.push_back(bkspec.GetBinContent(i)/phabs[i]);
		energy.push_back(bkspec.GetBinContent(i));
	}

	return energy;

}





vector<double> ResponseToXSPEC::CalculateVolumes(double h1, double h2, double a){

	double Vouter = 0.0;
	double Vinner = 0.0;
	double Vsource = 0.0;
	double VSinner = 0.0;
	double VSouter = 0.0;

	double pi = atan(1)*4;

	//outer cone
	double R = h2 + 2*a;
	double S = h2 + a;
	double r = h1 + 2*a;
	double s = h1 + a;
	Vouter = pi/3.*((h2-h1)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)));

	//outer source cone
	R = h2 + a;
	S = h2;
	r = h1 + a;
	s = h1;
	VSouter = pi/3.*((h2-h1)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)));

	//inner source cone
	if (h1 >= a) {
		R = h2;
		S = h2 - a;
		r = h1;
		s = h1 - a;
		VSinner = pi/3.*((h2-h1)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)));
	}
	else {
		R = h2;
		S = h2 - a;
		r = a;
		s = 0;
		VSinner += pi/3.*((h2-a)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)) + pow(a,3)-pow(h1,3));
	}

	Vsource = VSouter + VSinner;

	//inner cone
	if (h2 <= a){
		Vinner = 0;
	}
	else if (h1 >= 2*a){
		R = h2 - a;
		S = h2 - 2*a;
		r = h1 - a;
		s = h1 - 2*a;
		Vinner = pi/3.*((h2-h1)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)));
	}
	else if (h2 >= 2*a && h1 <= a){
		R = h2 - a;
		S = h2 - 2*a;
		r = a;
		s = 0;
		Vinner = pi/3.*((h2-2*a)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)) + pow(a,3));
	}
	else if (h2 >= 2*a && h1 > a){
		R = h2 - a;
		S = h2 - 2*a;
		r = a;
		s = 0;
		Vinner = pi/3.*((h2-2*a)*(pow(R,2)+R*r+pow(r,2) - pow(S,2)-S*s-pow(s,2)) + pow(a,3) - pow(h1-a,3));
	}
	else if (h1 > a){
		Vinner = pi/3.*(pow(h2-a,3)+pow(h1-a,3));
	}
	else {
		Vinner = pi/3.*(pow(h2-a,3));
	}

	vector<double> volumes;
	volumes.push_back(Vsource);
	volumes.push_back(Vouter);
	volumes.push_back(Vinner);
	volumes.push_back(VSouter);
	volumes.push_back(VSinner);

	return volumes;

}

vector<vector<float> > ResponseToXSPEC::BkgSpecFromTTreeLookupMatrixDetCoords(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//see if there's a separate background file 
	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D bkspec("bkspec","",nChan, &bins[0]);

	for (int i=0; i<bkspec.GetNbinsX()+1; i++){
		bkspec.SetBinContent(i,0);
	}

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//load lookup matrix
//	MString LookupMatName = "/data1/users/clio/ComptelDS/Responses/Isotropic.Large3.10EBins.65PBins/Isotropic.Large3.10EBins.65PBins.p1.compteldataspace.rsp.gz";
	MString LookupMatName = "/volumes/cronus/users/clio/GRBSpec/ComptelDS_Test/LookupMatrix/GRBSimCDSResponseARM32.4.io.compteldataspace.rsp.gz";
//	MString LookupMatName = "/volumes/cronus/users/clio/ComptelDS/Responses/MatrixWithFlightData/GRBBackground573s.io.compteldataspace.rsp.gz";

	MResponseMatrixON LookupMat;
	if (!LookupMat.Read(LookupMatName)){
		cout << "failed to read rsp matrix, aborting..." << endl;
		vector<vector<float> > emptyEnergies;
		return emptyEnergies;
	}

	MResponseMatrixAxis PhiAx = LookupMat.GetAxis(3);
	vector<double> phiBinEdges = PhiAx.Get1DBinEdges();
	MResponseMatrixAxis EnergyAx = LookupMat.GetAxis(2);
	vector<double> energyBinEdges = EnergyAx.Get1DBinEdges();


	//TODO: different off-axis angles!
	unsigned long offAxisAng = 21;
	unsigned long azimuth = 29;

	//fill histogram with correct selection from TTree
	for (unsigned long C=0; C<LookupMat.GetAxis(3).GetNumberOfBins(); C++){

		int minPhi = phiBinEdges[C];
		int maxPhi = phiBinEdges[C+1];
		//TODO: Deal with this when we aren't using ALL Compton scatter angles
		selection = "ComptonAngle>="+MString(minPhi)+"&&ComptonAngle<"+MString(maxPhi);

		TH1D bkgspec_one_slice("bkgspec"+MString(minPhi),"",nChan,&bins[0]);
		bkgspec_one_slice.Sumw2();
		BkgTree->Draw("Energy>>bkgspec"+MString(minPhi),MString(selection),"goff");

		for (int i=1; i<nChan+1; i++){

			float e_low = eLow[i];
			float e_high = eHigh[i];

			double srcCounts = 0;
			double bkgCounts = 0;
			double srcError = 0;
			double bkgError = 0;

			for (unsigned long E=0; E<LookupMat.GetAxis(2).GetNumberOfBins(); E++){

				//spectral bin fully contained in lookup matrix bin
				if ( e_low > energyBinEdges[E] && e_high < energyBinEdges[E+1] ){

					srcCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,1});
					bkgCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,0});
					srcError = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,3});
					bkgError = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,2});
					break;
				}
				//spectral bin split between two lookup matrix bins
				//is averaging the right thing to do here?
				else if ( e_low > energyBinEdges[E] && e_low < energyBinEdges[E+1] && e_high > energyBinEdges[E+1] ){

					//TODO: is this the right way to handle edge cases?
					//i.e. bins like 2990 to 3030 (or anything that partly goes above 3000)
					if ( E+1 == LookupMat.GetAxis(2).GetNumberOfBins() ) {

						srcCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,1});
						bkgCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,0});
						srcError = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,3});
						bkgError = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,2});

					}
					else{

						srcCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,1})
											+ LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E+1,C,1});
						bkgCounts = LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,0})
											+ LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E+1,C,0});
						srcError = sqrt(pow(LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,3}),2)
											+ pow(LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E+1,C,3}),2));
						bkgError = sqrt(pow(LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E,C,2}),2)
											+ pow(LookupMat.Get(vector<unsigned long> {offAxisAng,azimuth,E+1,C,2}),2));

					}
					break;

				}
			}

			double scaleFactor;
			double scaleFactorError;
			if (bkgCounts >= 1){
				scaleFactor = srcCounts/bkgCounts;
				scaleFactorError = scaleFactor*sqrt(pow(srcError/srcCounts,2)+pow(bkgError/bkgCounts,2));
			}
			//TODO: something about this!!
			else { scaleFactor = 0; scaleFactorError = 0; }

			double binContent = bkgspec_one_slice.GetBinContent(i);
			bkgspec_one_slice.SetBinContent(i, binContent*scaleFactor);

			double error = 0.;
			if (binContent != 0 && scaleFactor != 0){
				error = binContent*scaleFactor;
				error *= sqrt(pow(scaleFactorError/scaleFactor,2)+pow(sqrt(binContent)/binContent,2));
			}
			bkgspec_one_slice.SetBinError(i, error);

		}

		bkspec.Add(&bkgspec_one_slice);
	}

	float total = 0.;
	vector<float> energy;
	vector<float> error;
	//start iterating at 1 because 0 is underflow bin in root histograms
	for (int i=1; i<nChan+1; i++){
		energy.push_back(bkspec.GetBinContent(i));
		total += bkspec.GetBinContent(i);
		error.push_back(bkspec.GetBinError(i));
	}

	cout << "N bkg counts: " << total << endl;

	vector<vector<float> > energy_error;
	energy_error.push_back(energy);
	energy_error.push_back(error);

	return energy_error;


}

vector<vector<float> > ResponseToXSPEC::BkgSpecFromTTreeLookupMatrix(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//see if there's a separate background file 
	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();
	vector<float> bins = eLow;
	bins.push_back(eHigh.at(nChan-1));
	TH1D bkspec("bkspec","",nChan, &bins[0]);

	for (int i=0; i<bkspec.GetNbinsX()+1; i++){
		bkspec.SetBinContent(i,0);
	}

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	//load lookup matrix
//	MString LookupMatName = "/volumes/cronus/users/clio/ComptelDS/Responses/LingModel_ARM11.7_ARMFix/Ling.ARMFix.ARM11.7.30Zbins.io.compteldataspace.rsp.gz";
//	MString LookupMatName = "/volumes/cronus/users/clio/ComptelDS/Responses/LingModel_GalCoords_ARM11.7/Ling.GalCoords.ARM11.7.io.compteldataspace.rsp.gz";
	MString LookupMatName = "/volumes/cronus/users/clio/Crab/CrabPol/CDSResponseMatrices/CDSResponse.CSA0-65_1D1_AD0_no2site_ARM17.6.io.compteldataspace.rsp.gz";
//	MString LookupMatName = "/volumes/cronus/users/clio/Crab/CrabSpec/ComptelDS/LookupMatrix/CDSResponse_BkgSim40degPntCut_ARM17.6_OpenSelections.io.compteldataspace.rsp.gz";

	MResponseMatrixON LookupMat;
	if (!LookupMat.Read(LookupMatName)){
		cout << "failed to read rsp matrix, aborting..." << endl;
		vector<vector<float> > emptyEnergies;
		return emptyEnergies;
	}

	MResponseMatrixAxis ZenithAx = LookupMat.GetAxis(0);
	vector<double> zenithBinEdges = ZenithAx.Get1DBinEdges();
	MResponseMatrixAxis AzimuthAx = LookupMat.GetAxis(1);
	vector<double> azimuthBinEdges = AzimuthAx.Get1DBinEdges();
	MResponseMatrixAxis PhiAx = LookupMat.GetAxis(3);
	vector<double> phiBinEdges = PhiAx.Get1DBinEdges();
	MResponseMatrixAxis EnergyAx = LookupMat.GetAxis(2);
	vector<double> energyBinEdges = EnergyAx.Get1DBinEdges();


	//fill histogram with correct selection from TTree
	for (unsigned long Z=0; Z<LookupMat.GetAxis(0).GetNumberOfBins(); Z++){
		double minZenith = zenithBinEdges[Z];
		double maxZenith = zenithBinEdges[Z+1];
		cout << Z << endl;

		for (unsigned long A=0; A<LookupMat.GetAxis(1).GetNumberOfBins(); A++){
			double minAzimuth = azimuthBinEdges[A];
			double maxAzimuth = azimuthBinEdges[A+1];

			for (unsigned long C=0; C<LookupMat.GetAxis(3).GetNumberOfBins(); C++){

				int minPhi = phiBinEdges[C];
				int maxPhi = phiBinEdges[C+1];
				//TODO: Deal with this when we aren't using ALL Compton scatter angles
				MString newselection = "ComptonAngle>="+MString(minPhi)+"&&ComptonAngle<"+MString(maxPhi);
				newselection += "&&ZenithAngle>="+MString(minZenith)+"&&ZenithAngle<"+MString(maxZenith);
				newselection += "&&AzimuthAngle>="+MString(minAzimuth)+"&&AzimuthAngle<"+MString(maxAzimuth);
	
//				cout << newselection << endl;
	
				TH1D bkgspec_one_slice("bkgspec"+MString(minPhi)+"_"+MString(minZenith)+"_"+MString(minAzimuth),"",nChan,&bins[0]);
				bkgspec_one_slice.Sumw2();
				BkgTree->Draw("Energy>>bkgspec"+MString(minPhi)+"_"+MString(minZenith)+"_"+MString(minAzimuth),MString(newselection),"goff");
		
				for (int i=1; i<nChan+1; i++){
		
					float e_low = eLow[i];
					float e_high = eHigh[i];
		
					double srcCounts = 0;
					double bkgCounts = 0;
					double srcError = 0;
					double bkgError = 0;
		
					for (unsigned long E=0; E<LookupMat.GetAxis(2).GetNumberOfBins(); E++){
		
						//spectral bin fully contained in lookup matrix bin
						if ( e_low > energyBinEdges[E] && e_high < energyBinEdges[E+1] ){
		
							srcCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,1});
							bkgCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,0});
							srcError = LookupMat.Get(vector<unsigned long> {Z,A,E,C,3});
							bkgError = LookupMat.Get(vector<unsigned long> {Z,A,E,C,2});
							break;
						}
						//spectral bin split between two lookup matrix bins
						//is averaging the right thing to do here?
						else if ( e_low > energyBinEdges[E] && e_low < energyBinEdges[E+1] && e_high > energyBinEdges[E+1] ){
		
							//TODO: is this the right way to handle edge cases?
							//i.e. bins like 2990 to 3030 (or anything that partly goes above 3000)
							if ( E+1 == LookupMat.GetAxis(2).GetNumberOfBins() ) {
		
								srcCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,1});
								bkgCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,0});
								srcError = LookupMat.Get(vector<unsigned long> {Z,A,E,C,3});
								bkgError = LookupMat.Get(vector<unsigned long> {Z,A,E,C,2});
		
							}
							else{
		
								srcCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,1})
													+ LookupMat.Get(vector<unsigned long> {Z,A,E+1,C,1});
								bkgCounts = LookupMat.Get(vector<unsigned long> {Z,A,E,C,0})
													+ LookupMat.Get(vector<unsigned long> {Z,A,E+1,C,0});
								srcError = sqrt(pow(LookupMat.Get(vector<unsigned long> {Z,A,E,C,3}),2)
													+ pow(LookupMat.Get(vector<unsigned long> {Z,A,E+1,C,3}),2));
								bkgError = sqrt(pow(LookupMat.Get(vector<unsigned long> {Z,A,E,C,2}),2)
													+ pow(LookupMat.Get(vector<unsigned long> {Z,A,E+1,C,2}),2));
		
							}
							break;
		
						}
					}
		
					double scaleFactor;
					double scaleFactorError;


					if (bkgCounts > 0){
						scaleFactor = srcCounts/bkgCounts;
						scaleFactorError = scaleFactor*sqrt(pow(srcError/srcCounts,2)+pow(bkgError/bkgCounts,2));
					}
					//TODO: something about this!!
					else { scaleFactor = 0; scaleFactorError = 0; }
	
					double binContent = bkgspec_one_slice.GetBinContent(i);
					bkgspec_one_slice.SetBinContent(i, binContent*scaleFactor);
		
					double error = 0.;
					if (binContent != 0 && scaleFactor != 0){
						error = binContent*scaleFactor;
						error *= sqrt(pow(scaleFactorError/scaleFactor,2)+pow(sqrt(binContent)/binContent,2));
					}
					bkgspec_one_slice.SetBinError(i, error);
		
		/*			if (binContent != 0 && bkgCounts < 1){
						cout << minPhi << '\t' << e_low << '\t' << e_high << '\t';
						cout <<  bkgCounts << '\t' << binContent << '\t' << scaleFactor << endl;
					}
		*/
				}

				bkspec.Add(&bkgspec_one_slice);
			}

		}
	}

	vector<float> energy;
	vector<float> error;
	//start iterating at 1 because 0 is underflow bin in root histograms
	double total = 0.;
	for (int i=1; i<nChan+1; i++){
		energy.push_back(bkspec.GetBinContent(i));
		error.push_back(bkspec.GetBinError(i));
		total+=bkspec.GetBinContent(i);
	}

	cout << "scaled background: " << total << endl;

	vector<vector<float> > energy_error;
	energy_error.push_back(energy);
	energy_error.push_back(error);

	return energy_error;


}


vector<float> ResponseToXSPEC::SpectrumFromSim(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	vector<float> returnIfBad;

	//load geometry
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeoFileName) == true){
		cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
	}
	else{
		cout << "Unable to load geometry " << Geometry->GetName() << " - Aborting!" << endl;
		return returnIfBad;
	}

	MFileEventsSim EventFile(Geometry);
	if (EventFile.Open(m_DataFileName) == false){
		cout << "Unable to open file " << m_DataFileName<<" - Aborting!" << endl;
		return returnIfBad;
	}

	int nChans = eLow.size();
	vector<float> energy(nChans, 0);
	MSimEvent* Event;

	while ((Event = EventFile.GetNextEvent()) != 0){
		double E = Event->GetTotalEnergyDeposit();

		for (int c=0; c<nChans; c++){
			if (E >= eLow.at(c) && E < eHigh.at(c)){
				energy[c] += 1;
				break;
			}
		}
	}

	ofstream ofs("spec.txt",ofstream::out);
	for (int c=0; c<nChans; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

vector<float> ResponseToXSPEC::MakeSrcError(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	TFile f(m_DataFileName);
	TTree* SrcTree = (TTree*) f.Get("cosi_tra_tree");

	//make histogram with the bins defined by the response matrix
	int nChan = eLow.size();

	//get selection string
	string selection;
	if (m_CfgFileName == "None"){ selection = ""; }
	else{
		ifstream selectionF(m_CfgFileName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	vector<float> error;
	for (int c=0; c<nChan; c++){
		MString sel_ecut = MString(selection)+"&&Energy>="+eLow.at(c)+"&&Energy<"+eHigh.at(c);

		if (SrcTree->Draw("NOverlaps",sel_ecut,"goff") == 0){ error.push_back(0); continue; }

		vector<int> NC;
		for (int i=0; i<12; i++){
			int N = SrcTree->Draw("NOverlaps",sel_ecut+"&&NOverlaps=="+MString(i),"goff");
			NC.push_back(N);
		}

		float err = 0;
		for (int i=0; i<12; i++){
			err += NC[i]*pow((1-i/11.),2);
		}
		err = sqrt(err);

		error.push_back(err);
	}

	ofstream ofs("srcError.txt",ofstream::out);
	for (int c=0; c<nChan; c++){
		ofs << eLow.at(c) << '\t' << error[c] << endl;
	}
	ofs.close();

	return error;

}

vector<float> ResponseToXSPEC::MakeBkgError(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	MString fName;
	if (m_BkgFileName == "None"){ fName = m_DataFileName; }
	else { fName = m_BkgFileName; }

	TFile f(fName);
	TTree* BkgTree = (TTree*) f.Get("cosi_tra_tree");

	//get selection string
	string selection;
	if (m_CfgFileName == "None" && m_BkgCfgFileName == "None"){ selection = ""; }
	else{
		MString c_fName;
		if (m_BkgCfgFileName == "None"){ c_fName = m_CfgFileName; }
		else { c_fName = m_BkgCfgFileName; }

		ifstream selectionF(c_fName);
		if (selectionF.is_open()){
			getline(selectionF, selection);
		}
	}

	int nChan = eLow.size();

	vector<float> error;
	for (int c=0; c<nChan; c++){
		MString sel_ecut = MString(selection)+"&&Energy>="+eLow.at(c)+"&&Energy<"+eHigh.at(c);

		if (BkgTree->Draw("NOverlaps",sel_ecut,"goff") == 0){ error.push_back(0); continue; }

		vector<int> NC;
		for (int i=1; i<12; i++){
			int N = BkgTree->Draw("NOverlaps",sel_ecut+"&&NOverlaps=="+MString(i+11),"goff");
			NC.push_back(N/i);
		}

		float err = 0;
		for (int i=1; i<12; i++){
			int index = i-1;
//			err += NC[index]*pow((i/11.),2);
			err += NC[index]*pow(i,2);
		}
		err = sqrt(err);

		error.push_back(err);
	}

	ofstream ofs("bkgError.txt",ofstream::out);
	for (int c=0; c<nChan; c++){
		ofs << eLow.at(c) << '\t' << error[c] << endl;
	}
	ofs.close();

	return error;


}


bool ResponseToXSPEC::MakePHA(MResponseMatrixO2 Rsp){

	int nBinsMeasured = Rsp.GetAxisBins(2);

	//get channels from response matrix
	vector<int> channels;
	vector<float> eLow; vector<float> eHigh;

	for (int y=0; y<nBinsMeasured; y++){
		channels.push_back(y);
		eLow.push_back(Rsp.GetAxisLowEdge(y,2));
		eHigh.push_back(Rsp.GetAxisHighEdge(y,2));
	}

	//make energy-vs-channel array
	int nChans = channels.size();

	//figure out if file is tra or or sim
	vector<float> energy;
	vector<MString> SplitFName = m_DataFileName.Tokenize(".");
	bool isSimFile = false;
	bool isTraFile = false;
	bool isTTree = false;
	if (SplitFName.at(SplitFName.size()-1) == "sim" || SplitFName.at(SplitFName.size()-2) == "sim"){
		isSimFile = true;
	}
	if (SplitFName.at(SplitFName.size()-1) == "tra" || SplitFName.at(SplitFName.size()-2) == "tra"){
		isTraFile = true;
	}
	if (SplitFName.at(SplitFName.size()-1) == "root"){
		isTTree = true;
	}

	if (isSimFile){
		energy = SpectrumFromSim(Rsp, eLow, eHigh);
	}
	else if (isTraFile){
		energy = SpectrumFromTra(Rsp, eLow, eHigh);
	}
	else if (isTTree){
		energy = SpectrumFromTTree(Rsp, eLow, eHigh);
	}

	cout << "making background" << endl;

	//make background energy vs channel array IF tra file and bkg config file specified
	vector<float> bkg_energy;
	vector<vector<float> > bkg_energy_error; //for lookup matrix method: calculate error and energy together
	if (isTraFile && (m_BkgCfgFileName != "None" || m_BkgFileName != "None")){
		bkg_energy = BkgSpecFromTra(Rsp, eLow, eHigh);
	}
	else if (isTTree && (m_BkgCfgFileName != "None" || m_BkgFileName != "None")){
		bkg_energy = BkgSpecFromTTree(Rsp, eLow, eHigh);
//		bkg_energy = BkgSpecFromTTreePhiSlices(Rsp, eLow, eHigh);
//		bkg_energy = BkgSpecFromTTreeInOutPhiSlices(Rsp, eLow, eHigh);
//		bkg_energy_error = BkgSpecFromTTreeLookupMatrixDetCoords(Rsp, eLow, eHigh);
//		bkg_energy_error = BkgSpecFromTTreeLookupMatrix(Rsp, eLow, eHigh);
	}
	else{
		for (int c=0; c<nChans; c++){
			bkg_energy.push_back(0);
		}
	}

	//make channel vs error array in case errors are non-Poissonian: for Mark's method
	vector<float> src_error;
	vector<float> bkg_error;
	if (m_CalculateErrorImgReg && isTTree){
 		src_error = MakeSrcError(Rsp, eLow, eHigh);
		bkg_error = MakeBkgError(Rsp, eLow, eHigh);
	}

	//for lookup matrix method: calculate error and energy together
	if (m_LookupMatrix){
		bkg_energy = bkg_energy_error[0];
		bkg_error = bkg_energy_error[1];
	}

	//make ascii file of bad channels for grppha
/*	ofstream ofs((m_OutFileName+MString("_badchannels.dat")).Data(),ofstream::out);

	int startChan = 0;
	for (int c=0; c<nChans; c++){
		if (energy.at(c) != 0){
			startChan = c-1;
			break;
		}
	}
	int lastChan = nChans-1;
	for (int c=0; c<nChans; c++){
		if (eLow.at(c) >= m_Emax){
			lastChan = c;
			break;
		}
	}
	if (startChan >= 0){ ofs << 0 << '\t' << startChan << endl; }
	if (lastChan <= nChans-1){ ofs << lastChan << '\t' << nChans-1 << endl; }
	ofs.close();
*/

	/***************************************************/
	//make pha fits file
	fitsfile *pha = NULL;

	//used for keywords that aren't strings
	float number;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&pha, (m_OutFileName+MString(".pha")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(pha, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	//make binary table
	int nrows = nChans;
	if (m_CalculateErrorImgReg && isTTree){
		const char* m_column_names[] = {"CHANNEL", "COUNTS", "STAT_ERR"};
		const char* m_format[] = {"J", "J", "E"};
		const char* m_units[] = {"", "count", ""};
		status = 0;
		if (fits_create_tbl(pha, BINARY_TBL, nrows, 3, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
			cout << "error in fits_create_tbl" << endl;
			fits_report_error(stdout, status);
		}
	}
	else {
		const char* m_column_names[] = {"CHANNEL", "COUNTS"};
		const char* m_format[] = {"J", "J"};
		const char* m_units[] = {"", "count"};
		if (fits_create_tbl(pha, BINARY_TBL, nrows, 2, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
			cout << "error in fits_create_tbl" << endl;
			fits_report_error(stdout, status);
		}
	}



	//fill table
	int* chanArr = &channels[0];
	float* energyArr = &energy[0];

	status = 0;
	if (fits_write_col(pha, TINT, 1, 1, 1, nrows, chanArr, &status)){
		cout << "error in fits_write_col: pha SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(pha, TFLOAT, 2, 1, 1, nrows, energyArr, &status)){
		cout << "error in fits_write_col: pha SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	if (m_CalculateErrorImgReg && isTTree){
		float* srcErrArr = &src_error[0];
		status = 0;
		if (fits_write_col(pha, TFLOAT, 3, 1, 1, nrows, srcErrArr, &status)){
			cout << "error in fits_write_col: pha SPECTRUM table" << endl;
			fits_report_error(stdout,status);
		}
	}

	//keywords to binary table header
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"TELESCOP", (char*)"COSI", "telescope/mission name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"EXTNAME", (char*)"SPECTRUM", "name of extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"FILTER", (char*)"NONE", "filter type if any", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TFLOAT, (char*)"EXPOSURE", &m_Exposure, "integration time in seconds", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: background file name NOT hard coded
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"BACKFILE", (char*)(m_OutFileNameNoPath+MString("_bk.pha")).Data(), "background filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"BACKSCAL", &number, "background scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"CORRFILE", (char*)"NONE", "associated correction filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this!
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"CORRSCAL", &number, "correction file scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"RESPFILE", (char*)(m_OutFileNameNoPath+MString(".rmf")).Data(), "associated rmf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"ANCRFILE", (char*)(m_OutFileNameNoPath+MString(".arf")).Data(), "associated arf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"AREASCAL", &number, "area scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"STAT_ERR", &number, "no statistical error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"SYS_ERR", &number, "no systematic error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"GROUPING", &number, "no grouping of the data has been defined", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"QUALITY", &number, "no data quality information specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}


	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUCLAS1", (char*)"SPECTRUM", "PHA dataset", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUVERS", (char*)"1.2.1", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	if (!(m_CalculateErrorImgReg && isTTree)){
		status = 0;
		bool P = true;
		if (fits_write_key(pha, TLOGICAL, (char*)"POISSERR", &P, "Poissonian errors to be assumed", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TINT, (char*)"DETCHANS", &nChans, "total number of detector channels", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}




	status = 0;
	if (fits_close_file(pha, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "pha file closed" << endl;



	/***************************************************/
	//make background pha file
	fitsfile *bkg = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	status = 0;
	if (fits_create_file(&bkg, (m_OutFileName+MString("_bk.pha")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_create_img(bkg, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	//make binary table
	if ((m_CalculateErrorImgReg && isTTree) || m_LookupMatrix){
		const char* m_column_names[] = {"CHANNEL", "COUNTS", "STAT_ERR"};
		const char* m_format[] = {"J", "J", "E"};
		const char* m_units[] = {"", "count", ""};
		status = 0;
		if (fits_create_tbl(bkg, BINARY_TBL, nrows, 3, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
			cout << "error in fits_create_tbl" << endl;
			fits_report_error(stdout, status);
		}
	}
	else {
		const char* m_column_names[] = {"CHANNEL", "COUNTS"};
		const char* m_format[] = {"J", "J"};
		const char* m_units[] = {"", "count"};
		if (fits_create_tbl(bkg, BINARY_TBL, nrows, 2, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
			cout << "error in fits_create_tbl" << endl;
			fits_report_error(stdout, status);
		}
	}


	//fill table
	vector<float> bkg_energy_round;
	for (unsigned int i=0; i<bkg_energy.size(); i++){
		bkg_energy_round.push_back(round(bkg_energy[i]));
	}

	float* bkEnArr = &bkg_energy_round[0];

	status = 0;
	if (fits_write_col(bkg, TINT, 1, 1, 1, nrows, chanArr, &status)){
		cout << "error in fits_write_col: bkg SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(bkg, TFLOAT, 2, 1, 1, nrows, bkEnArr, &status)){
		cout << "error in fits_write_col: bkg SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	if ((m_CalculateErrorImgReg && isTTree) || m_LookupMatrix){
		float* bkgErrArr = &bkg_error[0];
		status = 0;
		if (fits_write_col(bkg, TFLOAT, 3, 1, 1, nrows, bkgErrArr, &status)){
			cout << "error in fits_write_col: bkg SPECTRUM table" << endl;
			fits_report_error(stdout,status);
		}
	}

	//keywords to binary table header
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"TELESCOP", (char*)"COSI", "telescope/mission name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"EXTNAME", (char*)"SPECTRUM", "name of extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"FILTER", (char*)"NONE", "filter type if any", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TFLOAT, (char*)"EXPOSURE", &m_BkgExposure, "integration time in seconds", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: background file name NOT hard coded
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"BACKFILE", (char*)"NONE", "background filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = m_Backscal;
	if (fits_write_key(bkg, TFLOAT, (char*)"BACKSCAL", &number, "background scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"CORRFILE", (char*)"NONE", "associated correction filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this!
	status = 0;
	number = 1.0;
	if (fits_write_key(bkg, TFLOAT, (char*)"CORRSCAL", &number, "correction file scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"RESPFILE", (char*)"NONE", "associated rmf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"ANCRFILE", (char*)"NONE", "associated arf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 1.0;
	if (fits_write_key(bkg, TFLOAT, (char*)"AREASCAL", &number, "area scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"STAT_ERR", &number, "no statistical error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"SYS_ERR", &number, "no systematic error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"GROUPING", &number, "no grouping of the data has been defined", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"QUALITY", &number, "no data quality information specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUCLAS1", (char*)"SPECTRUM", "PHA dataset", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUVERS", (char*)"1.2.1", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	if (!(m_CalculateErrorImgReg && isTTree) && !m_LookupMatrix){
		status = 0;
		bool P = true;
		if (fits_write_key(bkg, TLOGICAL, (char*)"POISSERR", &P, "Poissonian errors to be assumed", &status)){
			cout << "error in fits_write_key" << endl;
			fits_report_error(stdout, status);
		}
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TINT, (char*)"DETCHANS", &nChans, "total number of detector channels", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}




	status = 0;
	if (fits_close_file(bkg, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "bkg file closed" << endl;



	return true;

}

////////////////////////////////////////////////////////////////////////////////

bool ResponseToXSPEC::GetEnergyArray(MResponseMatrixO2 Rsp){

	int nBinsMeasured = Rsp.GetAxisBins(2);

	ofstream ofs("energy_array.txt",ofstream::out);

	float eMin, eMax, deltaE, E;

	for (int y=0; y<nBinsMeasured; y++){
		eMin = Rsp.GetAxisLowEdge(y,2);
		eMax = Rsp.GetAxisHighEdge(y,2);

		deltaE = eMax-eMin;
		E = eMin+deltaE/2.;
		ofs << E << endl;
	}

	ofs.close();

	return true;

}


////////////////////////////////////////////////////////////////////////////////


ResponseToXSPEC* g_Prg = 0;
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
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication ResponseToXSPECApp("ResponseToXSPECApp", 0, 0);

  g_Prg = new ResponseToXSPEC();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

//	if (gROOT->IsBatch() == false) {
//  	ResponseToXSPECApp.Run();
//	}

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
