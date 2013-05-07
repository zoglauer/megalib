/*
 * Extract.C                                    v1.0  24/10/2002
 *
 * Copyright (C) by Andreas Zoglauer. 
 * All rights reserved.
 *
 * This code implementation is the intellectual property 
 * of Robert Andritschke, Florian Schopper, Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/******************************************************************************
 *
 * This program extract the overflow from several calibration files...
 *
 *
 */


/******************************************************************************/

// Standard libraries:
#include <vector>
#include <string>
#include <fstream>

// Root libraries:
#include <TSystem.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TApplication.h>

// Mega libraries:
#include "MFile.h"
#include "MCalibrationFileCsIPixelSingle.h"
#include "MCalibrationFileCsIPixelDouble.h"
#include "MCalibrationFileSiStrip.h"

/******************************************************************************/

const Int_t c_Unknown = 0;
const Int_t c_SiStrip = 1;
const Int_t c_CsISingle = 2;
const Int_t c_CsIDouble = 3;

bool Extract(const vector<string>& FileNames);

/*******************************************************************************
 * Entry point to the programm:
 */
int main(int argc, char *argv[])
{
	// Define the names of the calibfiles:
	vector<string> FileNames;
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_bachus_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_apoll_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_neptun_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_vulkan_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_poseidon_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_ikarus_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_merkur_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_odysseus_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_achilles_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021010_sisyphus_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_circe_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_ariadne_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_antigone_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_aetna_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_minerva_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_medusa_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_hydra_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_helena_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_fortuna_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_europa_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_diane_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_venus_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_thetis_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_persephone_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_penelope_calibration_v3.csv");
// 	FileNames.push_back("$(MEGALYZE2)/calibfiles/20021021_pallas_calibration_v3.csv");

  FileNames.push_back("$(MEGALYZE2)/calibfiles/t11_athene_calibrate.csv");
  FileNames.push_back("$(MEGALYZE2)/calibfiles/t11_aphrodite_calibrate.csv");
  FileNames.push_back("$(MEGALYZE2)/calibfiles/t11_xantippe_calibrate.csv");
  FileNames.push_back("$(MEGALYZE2)/calibfiles/t11_daphne_calibrate.csv");

	// Extract the data:
	Extract(FileNames);

  TApplication ExtractApp("Megalyze", 0, 0);
	ExtractApp.Run();

	return 0;
}

/*******************************************************************************
 * Extract the data from the calibfiles:
 */
bool Extract(const vector<string>& FileNames)
{
	// Make an Overflow-Bin Histogram:
	TH1D* Overflow = 
    new TH1D("OverflowBins [keV]", "OverflowBins [keV]", 50, 0, 50000);
	int NChannels = 0;
	int NFailedChannels = 0;

	for (unsigned int c = 0; c < FileNames.size(); ++c) {
	  MString Name = FileNames[c];
	  MFile::ExpandFileName(Name);
		cout<<"Extract: Analysing file \""<<Name<<"\""<<endl;
	
		// Read the first lines of the file and decide on its type:
		fstream Stream;
		Stream.open(Name, ios_base::in);

		if (Stream.is_open() == false) {
			cout<<"Extract: Unable to open file \""<<Name<<"\"!"<<endl;
			continue;
		}  
	
		// Search for the type:
		int Type = c_Unknown;
		const int LineLength = 10000;
		char* LineBuffer = new char[LineLength];
		while (Stream.getline(LineBuffer, LineLength, '\n')) {
			if (strstr(LineBuffer, "cvs_csisingle") != 0) {
				Type = c_CsISingle;
				cout<<"Extract: File type is cvs_csisingle"<<endl;
				break;
			}
			if (strstr(LineBuffer, "csv_csidouble") != 0) {
				Type = c_CsIDouble;
				cout<<"Extract: File type is cvs_csidouble"<<endl;
				break;
			}
			if (strstr(LineBuffer, "cvs_sistrip") != 0) {
				Type = c_SiStrip;
				cout<<"Extract: File type is cvs_sistrip"<<endl;
				break;
			}
		}
		Stream.close();

		if (Type == c_Unknown) {
			cout<<"Extract: Unable to identify type of file \""<<Name<<"\"!"<<endl;
			continue;
		}

		// Now start the parser, which extracts the data:
		if (Type == c_CsISingle) {
			MCalibrationFileCsIPixelSingle CalibFile(Name.c_str());
			CalibFile.Open();
			for (unsigned int o = 0; o < CalibFile.GetOverflow().size(); ++o) {
				NChannels++;
				if (CalibFile.GetOverflow()[o] == 1024) {
					NFailedChannels++;
				} else {
					Overflow->Fill(CalibFile.GetCalibration()[o].
                         GetEnergy(CalibFile.GetOverflow()[o]), 1);
				}
			}
		} else if (Type == c_CsIDouble) {
			MCalibrationFileCsIPixelDouble CalibFile(Name.c_str());
			CalibFile.Open();
			for (unsigned int o = 0; o < CalibFile.GetOverflowI().size(); ++o) {
				NChannels++;
        cout<<CalibFile.GetOverflowI()[o]
            <<"!"<<CalibFile.GetOverflowA()[o]<<endl;
        if (CalibFile.GetCalibration()[o].GetNEnergyFitPoints() <= 1) {
					NFailedChannels++;
        }
				if (!(CalibFile.GetOverflowI()[o] == 1024 || 
            CalibFile.GetOverflowA()[o] == 1024)) {
          double sum = 
            CalibFile.GetOverflowI()[o] + CalibFile.GetOverflowA()[o];
          double eta = 
            (CalibFile.GetOverflowI()[o] - CalibFile.GetOverflowA()[o])/sum;
					Overflow->Fill(CalibFile.GetCalibration()[o].GetEnergy(eta, sum), 1);
          cout<<"Energy: "<<CalibFile.GetCalibration()[o].GetEnergy(eta, sum)<<endl;
          cout<<"Z "<<CalibFile.GetCalibration()[o].GetZ(eta)<<endl;
				}
			}
		} else if (Type == c_SiStrip) {
			MCalibrationFileSiStrip CalibFile(Name.c_str());
			CalibFile.Open();
			for (unsigned int o = 0; o < CalibFile.GetOverflowNSide().size(); ++o) {
				NChannels++;
				if (CalibFile.GetOverflowNSide()[o] == 1024) {
					NFailedChannels++;
				} else {
					Overflow->Fill(CalibFile.GetCalibrationNSide()[o].
                         GetEnergy(CalibFile.GetOverflowNSide()[o]), 1);
				}
			}
 			for (unsigned int o = 0; o < CalibFile.GetOverflowPSide().size(); ++o) {
				NChannels++;
				if (CalibFile.GetOverflowPSide()[o] == 1024) {
					NFailedChannels++;
				} else {
					Overflow->Fill(CalibFile.GetCalibrationPSide()[o].
                         GetEnergy(CalibFile.GetOverflowPSide()[o]), 1);
				}
			}
		}
	}

	// Finally draw the overflow histogram:
	TCanvas* OverflowHist = new TCanvas();
	Overflow->Draw();
	OverflowHist->Update();

  if (NChannels > 0) {
    cout<<"Percentage of failed channels: "
        <<(double) NFailedChannels/NChannels*100.0<<"%"<<endl;
  }

	return true; 
}



