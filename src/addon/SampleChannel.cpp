/*
 * SampleChannel.C
 *
 * Copyright (C) by Andreas Zoglauer, Robert Andritschke, Florian Schopper.
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
 * This program 
 *
 * Usage:

Standard gamma-mode:
launch SampleChannel --directory /data/WaveformsD1_c

launch SampleChannel --mode 2 --detector 1 --directory /data/Test --setup setupfiles/20030109_SampleChannelD1.setup --gui

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
#include <TGFrame.h>
#include <TGButton.h>

// Mega libraries:
#include "MStreams.h"
#include "MDaq.h"
#include "MRun.h"
#include "MSlowControl.h"
#include "MDetector.h"
#include "MChip.h"
#include "MCsIPixelDouble.h"
#include "MCsIPixelSingle.h"
#include "MTime.h"
#include "MTriggerControl.h"
#include "MCoincidenceControl.h"
#include "MCoincidenceControlCard.h"

/******************************************************************************/


/******************************************************************************/

class MSampleChannel
{
  // Public Interface:
 public:
  MSampleChannel();
  virtual ~MSampleChannel();

  Bool_t ParseCommandLine(int argc, char** argv);

	Bool_t OnNext();
	Bool_t OnLoop();
	Bool_t OnStop();
	Bool_t OnClose();

  // protected methods:
 protected:
	bool GammaMode();
	bool TestPulseMode();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  bool m_StopLoop;

  // Data section:
  MString m_SetupFileName;
	MString m_Directory; 
  vector<vector<MString> > m_Adcs;
  vector<MString> m_AdcNames;
	MString m_AdcControlName;

  int m_NRandoms;
  int m_NSamples;
  int m_MaxTime;
	unsigned int m_MaxNChipsPerAdc;

	bool m_UseGui;
	int m_Mode;
	int m_Detector;


  enum { e_Next = 100, e_Loop, e_Stop, e_Close };
};


/*******************************************************************************
 * Entry point to the programm:
 */
int main(int argc, char** argv)
{
	// Define the names of the calibfiles:
  
	MTime Time;
	Time.Now();
	char Text [100];
	sprintf(Text, "SampleChannel.%s.log", Time.GetShortString().Data());
	mlog.Connect(Text);
  mlog<<" This is chunk software - not intended to be used again!!!"<<endl;


	// SampleChannel the data:
  //TApplication SampleChannelApp("SampleChannel", 0, 0);
  MSampleChannel* PF = new MSampleChannel();
  if (PF->ParseCommandLine(argc, argv) == true) {
		PF->OnNext();
    //SampleChannelApp.Run();
  }

	return 0;
}


/******************************************************************************
 *
 */
MSampleChannel::MSampleChannel()
{
  // Construct an instance of MSampleChannel and bring it to the screen

	m_Mode = 1;
	m_Detector = 1; 

	// For sampling D2 pulses
  //m_SetupFileName = "setupfiles/20021126_SampleChannel.setup";

	// For sampling D1 pulses
  m_SetupFileName = "setupfiles/20030109_SampleChannelD1.setup";

	//m_Directory = "data/Waveforms_OneOutOf10Scan";
	//m_Directory = "data/WaveformsD1";
	m_Directory = "data/Test";



// 	m_AdcNames.push_back("track1_p");
// 	m_AdcNames.push_back("track1_n");
// 	m_AdcNames.push_back("track2_p");
// 	m_AdcNames.push_back("track2_n");
// 	m_AdcNames.push_back("track3_p");
// 	m_AdcNames.push_back("track3_n");

  m_NRandoms = 100;
	m_NSamples = 80;
	m_MaxTime = 900; //3000;

	m_UseGui = false;
}


/******************************************************************************
 *
 */
MSampleChannel::~MSampleChannel()
{
  // Delete an instance of MSampleChannel
}



/******************************************************************************
 *
 */
Bool_t MSampleChannel::OnNext()
{
	if (m_Mode == 1) {
		return GammaMode();
	} else {
		return TestPulseMode();
	}
}



/******************************************************************************
 *
 */
bool MSampleChannel::GammaMode()
{
  // Loop over all chips:
  for (unsigned int c = 6; c < m_MaxNChipsPerAdc; ++c) {
		//for (unsigned int c = 3; c < m_MaxNChipsPerAdc; ++c) {
    
    mlog<<"Switching to chips: ";
		for (unsigned int chip = 0; chip < m_Adcs.size(); ++chip) {
			if (m_Adcs[chip].size() <= c) continue;
			mlog<<m_Adcs[chip][c]<<"  ";
		}
		mlog<<endl;

    // Loop over all voxels, i.e. all (connected) channels:
		//for (unsigned int v = 2; v < 128; v+=11) {
		for (unsigned int v = 42; v < 128; v+=255) {

			MDaq Daq;
			Daq.Setup(m_SetupFileName);
      Daq.SetInputMode(MDaq::e_InputHardware);
      Daq.SetAnalysisMode(91);
      Daq.SetReadMode(MDaq::e_ReadFull);
    
      mlog<<endl;
      mlog<<"Section A: Initialization"<<endl;
      mlog<<endl;

      // Initialize 
      Daq.SetTriggerMode(MDaq::e_TriggerCoincidenceOnly, kFALSE, kFALSE);
      //Daq.SetTriggerMode(MDaq::e_TriggerRandom, kFALSE, kFALSE);
      Daq.SetStopConditionCollectedEvents(1);
      Daq.SetStopConditionEllapsedTime(15);
      Daq.StartDaq();

      // Open the file...
			// Attention the file name must contain the structure:
			// _%s_%s_%i : ChiopName1, ChipName2, ChannelNumber
			MTime Time;
			Time.Now();
      MRun* Run = new MRun();
			MStr S;
			S<<m_Directory.Data()<<"/WF_";
			for (unsigned int i = 0; i < m_Adcs.size(); ++i) {
				S<<m_Adcs[i][c].Data()<<"_";
			}
			S<<v<<"."<<Time.GetShortString()<<".";

//       sprintf(Text, "%s/WF_%s_%s_%i.%s.", 
// 							m_Directory.Data(), m_Adcs[0][c].Data(), m_Adcs[1][c].Data(), 
// 							v, Time.GetShortString().Data());
			

			MString FileName = S.GetString();
      Run->SetFileName(FileName + "rawa");
      Daq.WriteRawData(FileName + "rawa", 2);
      delete Run;
    
			// Store housekeeping:
			Daq.SetTaskStoreHousekeepingSingleShot();
			if (Daq.StoreHousekeeping(FileName + "house") == false) {
				cout<<"Unable to store house-keeping!"<<endl;
			}

      mlog<<endl;
      mlog<<"Section B: Blocking & Randoms"<<endl;
      mlog<<endl;

      // Block all channels of all chips, if they are not part of D1
      Int_t* TriggerSettings = new Int_t[128];
      for (int i = 0; i < 128; ++i) TriggerSettings[i] = 1; // == Blocked 

			for (int i = 0; i < Daq.GetNChips(); ++i) {
				MChip* Chip = Daq.GetChipAt(i);
				mlog<<"Blocking chip: "<<Chip->GetName()<<endl;
				for (int ii = 0; ii < Chip->GetNChannels(); ++ii) {
					Chip->SetTriggerSettings(TriggerSettings);
				}
			}

      // Now open the selected channels...
      int NOpenedChannels = 0;
      Daq.SampleChannelSelect(v+1+c*128);
      mlog<<"Sampling channel: "<<v+c*128<<endl;
      TriggerSettings[v] = 0;
      for (unsigned int chip = 0; chip < m_Adcs.size(); ++chip) {
				if (m_Adcs[chip].size() <= c) continue;
        // If the channel is connected and good
        MChip* Chip = Daq.GetChip(m_Adcs[chip][c]);
        if (Chip == 0) {
          mlog<<"Chip "<<m_Adcs[chip][c]<<" not found in Daq!"<<endl;
          mlog<<"Actually this is a fatal error...."<<endl;
          continue;
        }
        //  
        if (Chip->GetGoodChannel()[v] & 0x80) {

          // if this is a double sided D2 we have to open the channels on the other side too...
          MDetector* Detector = Daq.GetDetector(Chip);
          if (Detector == 0) {
            mlog<<"Chip "<<Chip->GetName()<<" does not belong to any detector!"<<endl;
            mlog<<"Actually this is a fatal error...."<<endl;
            continue;
          }
          if (Detector->GetNChips() == 2) {
						mlog<<"We have CsIDouble, so open second channel..."<<endl;
            MCsIPixelDouble* Double = (MCsIPixelDouble*) Detector;
            Int_t x = -1;
            MChip* SecondChip = 0;
            if (Double->GetChipAt(0)->GetName() == m_Adcs[chip][c]) {
              SecondChip = Double->GetChipAt(1);
              if (Double->GetOutsideMappingX(v) >= 0 &&  Double->GetOutsideMappingY(v) >= 0) {
                 x = Double->GetInsideInverseMapping(Double->GetOutsideMappingX(v), Double->GetOutsideMappingY(v));
              } else {
								mlog<<"Bad mapping....!!!"<<endl;
							}
            } else {
              SecondChip = Double->GetChipAt(0);
              if (Double->GetInsideMappingX(v) >= 0 &&  Double->GetInsideMappingY(v) >= 0) {
                x = Double->GetOutsideInverseMapping(Double->GetInsideMappingX(v), Double->GetInsideMappingY(v));
              } else {
								mlog<<"Bad mapping....!!!"<<endl;
							}
            }
            if (x >= 0) {
							NOpenedChannels++;
							Chip->SetTriggerSettings(TriggerSettings);
							if (SecondChip->GetGoodChannel()[x] & 0x80) {
								Int_t* NewTriggerSettings = new Int_t[128];
								for (int iiii = 0; iiii < 128; ++iiii) NewTriggerSettings[iiii] = 1; // == Blocked
								NewTriggerSettings[x] = 0;             
								SecondChip->SetTriggerSettings(NewTriggerSettings);
								delete [] NewTriggerSettings;
								mlog<<"Opening additional channel of D2: "<<Detector->GetName()
										<<" chip:"<<SecondChip->GetName()<<": channel: "<<x<<endl;
							} else {
								mlog<<"Second channel of D2: "<<Detector->GetName()
										<<" chip:"<<SecondChip->GetName()<<": channel: "<<x<<" is NOT good!"<<endl;
							}
            } else {
							mlog<<"Can not open second channel due to mapping problems!!"<<endl;
						}
          } else if (Detector->GetNChips() == 1) {
						// Again check the mapping:
            MCsIPixelSingle* Single = (MCsIPixelSingle*) Detector;
						if (Single->GetXMap()->At(v) >= 0 && Single->GetYMap()->At(v) >= 0) {
							NOpenedChannels++;
							Chip->SetTriggerSettings(TriggerSettings);
						} else {
							mlog<<"Can not open channel of "<<Single->GetName()<<" due to mapping problems!!"<<endl;
						}
          } else if (Detector->GetNChips() == 6) {
						// Again check the mapping:
						// No mapping problems can occurr in SiStrip detectors
						NOpenedChannels++;
						Chip->SetTriggerSettings(TriggerSettings);
					}
        }
      }

      delete [] TriggerSettings;

      if (NOpenedChannels == 0) {
				cout<<endl;
				cout<<"ERROR: no channels are open!!"<<endl;
				cout<<endl;
        continue;
      }

      // Download the trigger settings to the slowcontrol...
      ((MSlowControl*) (Daq.GetSlowControlList())->At(0))->LoadAllTbr();

      // Collect 100 Randoms...
      Daq.SetTriggerMode(MDaq::e_TriggerRandomOnly, kFALSE, kFALSE);
			if (m_Detector == 1) {
				Daq.SetReadMode(MDaq::e_ReadSampleChannelD1);
			} else {
				Daq.SetReadMode(MDaq::e_ReadSampleChannelD2);
			}
      for (int i = 0; i < m_NRandoms; ++i) {
        Daq.SingleDaq();
      }
      
      mlog<<endl;
      mlog<<"Section C: Sample channels"<<endl;
      mlog<<endl;
      
      // Collect 500 events in sample channel mode..
      Daq.SetTriggerMode(MDaq::e_TriggerCoincidenceOnly, kFALSE, kFALSE);
			if (m_Detector == 1) {
				Daq.SetReadMode(MDaq::e_ReadSampleChannelD1);
			} else {
				Daq.SetReadMode(MDaq::e_ReadSampleChannelD2);
			}
      Daq.SetStopConditionCollectedEvents(m_NSamples);
      Daq.SetStopConditionEllapsedTime(m_MaxTime);
      Daq.StartDaq();
			Daq.StopDaq();
    }    
  }

	return true;
}



/******************************************************************************
 *
 */
bool MSampleChannel::TestPulseMode()
{
	int Status;
	MTime EventTime;
	double EventTimeWalk, EventTimeWalk2;

	// Open and initialize the daq:
	MDaq Daq;
	Daq.Setup(m_SetupFileName);
	Daq.SetInputMode(MDaq::e_InputHardware);
	Daq.SetAnalysisMode(91);
// 	Daq.SetReadMode(MDaq::e_ReadFull);
     
// 	Daq.SetTriggerMode(MDaq::e_TriggerCoincidenceOnly, kFALSE, kFALSE);
// 	Daq.SetStopConditionCollectedEvents(1);
// 	Daq.SetStopConditionEllapsedTime(60);
// 	Daq.StartDaq();

	MAdcControl* AdcControl = Daq.GetAdcControl(m_AdcControlName);

	// Prepare for test pulses:
	AdcControl->SetNTestPulseSamples(256);
	Daq.SetReadMode(MDaq::e_ReadTest);
	AdcControl->SetTimer(-1, -1, 100, 200, -1, 190);


  // Loop over all Adcs:
	for (unsigned int adc = 2; adc < m_Adcs.size(); ++adc) {
		// and loop over each chip in this adc...

		for (unsigned int chip = 0; chip < m_Adcs[adc].size(); ++chip) {
			// and loop over each channel...

			for (unsigned int ch = 0; ch < 128; ++ch) {
				

				cout<<endl<<endl<<endl;
				cout<<"Switching to channel "<<ch<<" (zero-based) of chip: "<<m_Adcs[adc][chip]<<endl;
				cout<<endl;

				// Open a new file for output:
				// Attention the file name must contain the structure:
				// _%s_%s_%i : ChiopName1, ChipName2, ChannelNumber
				MTime Time;
				Time.Now();
				MRun* Run = new MRun();
				MStr S;
				S<<m_Directory.Data()<<"/TP_";
				S<<m_Adcs[adc][chip].Data()<<"_";
				S<<ch<<"."<<Time.GetShortString()<<".";
				MString FileName = S.GetString();
				Run->SetFileName(FileName + "rawa");
				Daq.WriteRawData(FileName + "rawa", 2);
				delete Run;

// 				// Block everything except this channel:
// 				for (int i = 0; i < Daq.GetNChips(); ++i) {
// 					Daq.GetChipAt(i)->CloseAllTrigger();
// 				}
// 				Daq.GetChip(m_Adcs[adc][chip])->CloseAllTriggerBut(ch);

				AdcControl->SelectChannel(m_Adcs[adc][chip], ch);

				// loop over different test amplitude heights:
				for (int amp = 2500; amp < 3000; amp += 10) { 
				//for (int amp = 3048; amp < 3500; amp += 100) { 

					cout<<"Setting amplitude: "<<amp<<endl;

					// Set the test amplitude:
					for (int s = 0; s <= Daq.GetSlowControlList()->GetLast(); ++s) {
						if (((MSlowControl *) Daq.GetSlowControlList()->At(s))->SetTestAmplitude(amp, m_Adcs[adc][chip]) == true) {
							break;
						}
					}

					// Send the test pulse:
					AdcControl->SendTestPulse();
					Status = AdcControl->WaitForData();
					
					// Store the data:
					if (Status != 0) {
						AdcControl->LookForData();

						// Read Trigger
						for (Int_t i = Daq.GetNTriggerControls()-1; i >= 0; i--) {
							//for (Int_t i = 0; i < GetNTriggerControls(); i++) {
							Daq.GetTriggerControlAt(i)->ReadResetTrigger();
						}
						

						// Read TimeWalk in case of coincident hardware readout
						EventTimeWalk = Daq.GetCoincidenceControl()->GetTimeWalk();
						EventTimeWalk2 = Daq.GetCoincidenceControl()->GetTimeWalk2();
						
			
						// Get the time:
						EventTime = Daq.GetTimeControl()->GetEventTime();
						
						// Read One Event from Hardware, a rawdata File, or random input
						Bool_t AllReadOK = kTRUE;
						if (AdcControl->ReadEvent() != 0) {	      

							cout<<"Read of event went wrong!"<<endl;
							AllReadOK = kFALSE;
						} else {
		 
							// test of mode bits in Coicidence electronics
							if (Daq.GetCoincidenceControl()->CoincHardwareAvailable()) {
								((MCoincidenceControlCard*) Daq.GetCoincidenceControl())->GetCoincidenceCard()->GetMode();
							}

							Daq.DoFileOutput(AllReadOK, EventTime, (int) EventTimeWalk, (int) EventTimeWalk2); 
						}
					} // end good event...
				} // end test amplitude loop
			} // end channel loop 
		} // end chip loop
	} // end adc loop

	return true;
}


/******************************************************************************
 *
 */
Bool_t MSampleChannel::OnLoop()
{
  m_StopLoop = false;

	return true;
}


/******************************************************************************
 *
 */
Bool_t MSampleChannel::OnStop()
{
  m_StopLoop = true;

	return true;
}



/******************************************************************************
 *
 */
Bool_t MSampleChannel::OnClose()
{
	// The Apply button has been pressed

	return kTRUE;
}


/******************************************************************************
 * Dito
 */
Bool_t MSampleChannel::ParseCommandLine(int argc, char** argv)
{
  MStr Usage;
  Usage<<endl;
  Usage<<"  Usage: SampleChannel <options>"<<endl;
	Usage<<"         --gui            Use a graphical user interface to start the run"<<endl;
	Usage<<"         --mode:          1 for real gamma-events or 2 for test pulses (default: 1)"<<endl;
	Usage<<"         --detector:      1 for D1 or 2 for D2 (default: D1)"<<endl;
	Usage<<"         --directory:     directory where to store all files (default: .)"<<endl;
	Usage<<"         --setup:         name of setup file"<<endl;
  Usage<<"         --help:          This text..."<<endl;
  Usage<<endl;

  MString Option;

  // Check for help
  for (Int_t i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (Int_t i = 1; i < argc; i++) {
		Option = argv[i];

		// Single argument
    if (Option == "--mode" ||
				Option == "--detector" ||
				Option == "--directory" ||
				Option == "--setup") {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage<<endl;
				return false;
			}
		} 

		if (Option == "--gui") {
			cout<<"Option: Using Gui"<<endl;
			m_UseGui = true;
		} else if (Option == "--mode") {
			m_Mode = atoi(argv[++i]);
			if (m_Mode == 1) {
				cout<<"Option: Using gamma mode!"<<endl;
			} else if ( m_Mode == 2) {
				cout<<"Option: Using test pulse mode!"<<endl;
			} else {
				cout<<"Error in option --mode: Invalid mode: "<<m_Mode<<endl;
				cout<<Usage<<endl;
				return false;
			}
		} else if (Option == "--detector") {
			m_Detector = atoi(argv[++i]);
			if (m_Detector == 1) {
				cout<<"Option: Using detector D1!"<<endl;
			} else if ( m_Detector == 2) {
				cout<<"Option: Using detector D2!"<<endl;
			} else {
				cout<<"Error in option --detector: Invalid detector: "<<m_Detector<<endl;
				cout<<Usage<<endl;
				return false;
			}
		} else if (Option == "--setup") {
			m_SetupFileName = argv[++i];
			cout<<"Option: Using setup file "<<m_SetupFileName<<endl;
		} else if (Option == "--directory") {
			m_Directory = argv[++i];
			cout<<"Option: Using directory "<<m_Directory<<endl;
		}
  }

	if (m_Detector == 1) {
		m_AdcControlName = "sequenzD1";
		m_MaxNChipsPerAdc = 12;
		m_Adcs.resize(6);

		m_Adcs[4].push_back("bachus:Chip#4");
		m_Adcs[4].push_back("bachus:Chip#5");
		m_Adcs[4].push_back("bachus:Chip#6");
		m_Adcs[4].push_back("apoll:Chip#4");
		m_Adcs[4].push_back("apoll:Chip#5");
		m_Adcs[4].push_back("apoll:Chip#6");
		m_Adcs[4].push_back("neptun:Chip#4");
		m_Adcs[4].push_back("neptun:Chip#5");
		m_Adcs[4].push_back("neptun:Chip#6");
		m_Adcs[4].push_back("vulkan:Chip#4");
		m_Adcs[4].push_back("vulkan:Chip#5");
		m_Adcs[4].push_back("vulkan:Chip#6");

		m_Adcs[5].push_back("bachus:Chip#1");
		m_Adcs[5].push_back("bachus:Chip#2");
		m_Adcs[5].push_back("bachus:Chip#3");
		m_Adcs[5].push_back("apoll:Chip#1");
		m_Adcs[5].push_back("apoll:Chip#2");
		m_Adcs[5].push_back("apoll:Chip#3");
		m_Adcs[5].push_back("neptun:Chip#1");
		m_Adcs[5].push_back("neptun:Chip#2");
		m_Adcs[5].push_back("neptun:Chip#3");
		m_Adcs[5].push_back("vulkan:Chip#1");
		m_Adcs[5].push_back("vulkan:Chip#2");
		m_Adcs[5].push_back("vulkan:Chip#3");

		m_Adcs[2].push_back("poseidon:Chip#4");
		m_Adcs[2].push_back("poseidon:Chip#5");
		m_Adcs[2].push_back("poseidon:Chip#6");
		m_Adcs[2].push_back("sisyphus:Chip#4");
		m_Adcs[2].push_back("sisyphus:Chip#5");
		m_Adcs[2].push_back("sisyphus:Chip#6");
		m_Adcs[2].push_back("achilles:Chip#4");
		m_Adcs[2].push_back("achilles:Chip#5");
		m_Adcs[2].push_back("achilles:Chip#6");
		m_Adcs[2].push_back("ikarus:Chip#4");
		m_Adcs[2].push_back("ikarus:Chip#5");
		m_Adcs[2].push_back("ikarus:Chip#6");

		m_Adcs[3].push_back("poseidon:Chip#1");
		m_Adcs[3].push_back("poseidon:Chip#2");
		m_Adcs[3].push_back("poseidon:Chip#3");
		m_Adcs[3].push_back("sisyphus:Chip#1");
		m_Adcs[3].push_back("sisyphus:Chip#2");
		m_Adcs[3].push_back("sisyphus:Chip#3");
		m_Adcs[3].push_back("achilles:Chip#1");
		m_Adcs[3].push_back("achilles:Chip#2");
		m_Adcs[3].push_back("achilles:Chip#3");
		m_Adcs[3].push_back("ikarus:Chip#1");
		m_Adcs[3].push_back("ikarus:Chip#2");
		m_Adcs[3].push_back("ikarus:Chip#3");

		m_Adcs[0].push_back("odysseus:Chip#4");
		m_Adcs[0].push_back("odysseus:Chip#5");
		m_Adcs[0].push_back("odysseus:Chip#6");
		m_Adcs[0].push_back("merkur:Chip#4");
		m_Adcs[0].push_back("merkur:Chip#5");
		m_Adcs[0].push_back("merkur:Chip#6");
		m_Adcs[0].push_back("hermes:Chip#4");
		m_Adcs[0].push_back("hermes:Chip#5");
		m_Adcs[0].push_back("hermes:Chip#6");
		
		m_Adcs[1].push_back("odysseus:Chip#1");
		m_Adcs[1].push_back("odysseus:Chip#2");
		m_Adcs[1].push_back("odysseus:Chip#3");
		m_Adcs[1].push_back("merkur:Chip#1");
		m_Adcs[1].push_back("merkur:Chip#2");
		m_Adcs[1].push_back("merkur:Chip#3");
		m_Adcs[1].push_back("hermes:Chip#1");
		m_Adcs[1].push_back("hermes:Chip#2");
		m_Adcs[1].push_back("hermes:Chip#3");
	} else {
		m_AdcControlName = "sequenzD2";
		m_MaxNChipsPerAdc = 12;
		m_Adcs.resize(2);

		// D2 chip list
		m_Adcs[0].push_back("xantippe:Chip#1");
		m_Adcs[0].push_back("xantippe:Chip#2");
		m_Adcs[0].push_back("ariadne:Chip#1");
		m_Adcs[0].push_back("daphne:Chip#1");
		m_Adcs[0].push_back("daphne:Chip#2");
		m_Adcs[0].push_back("helena:Chip#1");
		m_Adcs[0].push_back("fortuna:Chip#1");
		m_Adcs[0].push_back("diane:Chip#1");
		m_Adcs[0].push_back("persephone:Chip#1");
		m_Adcs[0].push_back("aetna:Chip#1");
		m_Adcs[0].push_back("thetis:Chip#1");
		m_Adcs[0].push_back("circe:Chip#1");

		m_Adcs[1].push_back("athene:Chip#1");  // 0
		m_Adcs[1].push_back("athene:Chip#2");  // 1
		m_Adcs[1].push_back("penelope:Chip#1"); // 2
		m_Adcs[1].push_back("aphrodite:Chip#1"); // 3
		m_Adcs[1].push_back("aphrodite:Chip#2"); // 4
		m_Adcs[1].push_back("venus:Chip#1"); // 5
		m_Adcs[1].push_back("hydra:Chip#1");  // 6
		m_Adcs[1].push_back("pallas:Chip#1"); // 7
		m_Adcs[1].push_back("minerva:Chip#1"); // 8
		m_Adcs[1].push_back("medusa:Chip#1"); // 9
		m_Adcs[1].push_back("antigone:Chip#1"); // 10
		m_Adcs[1].push_back("europa:Chip#1"); // 11
	}

  return true;
}


/*
 * MSampleChannel end...
 ******************************************************************************/
