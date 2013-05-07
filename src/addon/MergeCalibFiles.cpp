/*
 * MergeCalibFiles.cpp                              v0.1  30.07.2003
 *
 *
 * Copyright (C) by Robert Andritschke. All rights reserved.
 *
 * This code implementation is the intellectual property of Robert Andritschke, Florian Schopper, Andreas Zoglauer 
 * at MPE.
 *
 * By copying, distributing or modifying the Program (or any work based on the 
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */

/*******************************************************************************

Add calibration histogramms of two calib files

********************************************************************************/

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <iostream>

// root classes
#include <MString.h>
#include <TH1.h>
#include <TH2.h>

// Megalyze classes
#include "MDaq.h"
#include "MSiStrip.h"
#include "MCsIPixelSingle.h"
#include "MCsIPixelDouble.h"


using namespace std;

int main(int argc, char **argv)
{

	MString SetupFileName="";
	string DataFileName="";


	// should be parsed or autorecognized later on:
	//int DetectorType = 1; // 1 for SiStrip, 2 for CsI (definitions like MDetector)

	// first command line parsing...
  if (argc<4) {
		cout << "Not enough arguments!" << endl;
		cout << "Usage: " << argv[0] << " SetupFile CalibFile1 CalibFile2 [CalibFile3] ..." << endl;
		exit(1);
	}

	// opening Daq
	MDaq Daq;
	Daq.Setup(argv[1]);
	Daq.SetAnalysisMode(15);

	// opening calib files
	vector <TFile *> Files;
	for (Int_t i=0; i< argc-2; ++i) {
		Files.push_back(new TFile(argv[i+2], "READ"));
		if (!(Files.back()->IsOpen())) {
			cout << endl << "Cannot open file \"" << argv[i+2] << "\"!" << endl;
			delete Files.back();
			Files.pop_back();
		}
	}
	if (Files.size() > 2) {
		TObjArray* AllDetectorList = Daq.GetDetectorList();
		for (int i = 0; i< AllDetectorList->GetEntriesFast(); ++i) {
			MDetector * Detector = (MDetector *)AllDetectorList->At(i);
			vector <MSiStrip *> D1Dets;
			vector <MCsIPixelSingle *> D2SDets;
			vector <MCsIPixelDouble *> D2DDets;

			// loop over detectors
			switch (Detector->GetDetectorType()) {
			case MDetector::e_SubD1:
				D1Dets.resize(0);
				for (UInt_t i=0; i<Files.size(); ++i) {
					MSiStrip * Det = (MSiStrip*) Files[i]->Get(Detector->GetDetName().Data());
					if (Det != 0)	D1Dets.push_back(Det); 
					else
						cout << "No detector " << Detector->GetDetName() << " available in calib file!" << endl;
				}
				// Loop over channels
				for (UInt_t side=0; side < 2; ++side) {
					for (UInt_t strip=0; strip < 384; ++strip) {
						TH1S * DaqCalibHist = ((MSiStrip*)Detector)->GetCalibHist((Bool_t)side, strip+1);
						vector <TH1S *> FileCalibHists;
						for (UInt_t i=0; i<D1Dets.size(); ++i) {
							TH1S * hist = D1Dets[i]->GetCalibHist((Bool_t)side, strip+1);
							if (hist != 0)	FileCalibHists.push_back(hist); 
						}
						
						for (UInt_t i=0; i< FileCalibHists.size(); ++i) {
							for(Int_t bin=0; bin<DaqCalibHist->GetNbinsX(); ++bin) {
								DaqCalibHist->AddBinContent(bin, FileCalibHists[i]->GetBinContent(bin));
							}
						}
					}
				}
				break;
			case MDetector::e_D2Single:
				D2SDets.resize(0);
				for (UInt_t i=0; i<Files.size(); ++i) {
					MCsIPixelSingle * Det = (MCsIPixelSingle*) Files[i]->Get(Detector->GetDetName().Data());
					if (Det != 0)	D2SDets.push_back(Det); 
					else
						cout << "No detector " << Detector->GetDetName() << " available in calib file!" << endl;
				}
				// Loop over channels
				for (UInt_t x=0; x < 10; ++x) {
					for (UInt_t y=0; y < 12; ++y) {
						TH1S * DaqCalibHist = ((MCsIPixelSingle*)Detector)->GetCalibHist(x+1, y+1);
						vector <TH1S *> FileCalibHists;
						for (UInt_t i=0; i<D2SDets.size(); ++i) {
							TH1S * hist = D2SDets[i]->GetCalibHist(x+1, y+1);
							if (hist != 0)	FileCalibHists.push_back(hist); 
						}

						for (UInt_t i=0; i< FileCalibHists.size(); ++i) {
							DaqCalibHist->Add(FileCalibHists[i]);
						}
					}
				}
				break;
			case MDetector::e_D2Double:
				D2DDets.resize(0);
				for (UInt_t i=0; i<Files.size(); ++i) {
					MCsIPixelDouble * Det = (MCsIPixelDouble*) Files[i]->Get(Detector->GetDetName().Data());
					if (Det != 0)	D2DDets.push_back(Det); 
					else
						cout << "No detector " << Detector->GetDetName() << " available in calib file!" << endl;
				}
				// Loop over channels
				for (UInt_t x=0; x < 10; ++x) {
					for (UInt_t y=0; y < 12; ++y) {
						TH2S * DaqCalibHist = ((MCsIPixelDouble*)Detector)->GetCalibHist(x+1, y+1);
						vector <TH2S *> FileCalibHists;
						for (UInt_t i=0; i<D2DDets.size(); ++i) {
							TH2S * hist = D2DDets[i]->GetCalibHist(x+1, y+1);
							if (hist != 0)	FileCalibHists.push_back(hist); 
						}

						for (UInt_t i=0; i< FileCalibHists.size(); ++i) {
							DaqCalibHist->Add(FileCalibHists[i]);
						}
					}
				}
				break;
			case MDetector::e_SubDummy:
			case MDetector::e_SubUnknown:
				break;
			}
		}
	}

	// Save calibration and end
	Daq.SingleDaq();
	Daq.SaveCalibration("out.calib");
	cout << "Ende" << endl;

	for (UInt_t i=0; i<Files.size(); ++i) {
		Files[i]->Close();
		delete Files[i];
	}
}
