/* 
 * SimRandomCoincidence.cxx
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


/**

The time/trigger/coincidence model on which SimRandomCoincidence is based:


The total simulation time is known for each simulation file.
For each event in each file a random time stamp is determined (in accordance with the total simulation time for this file). 

The triggers are determined by moving along the time bar in steps which are 1/100 the highest (inverse) rate. 

All hits of one simulated event happen in parallel - and instantly reach the peak-hold in the front-end chips.
If the event raises a veto, the system waits the reset time (= dead time) and is then ready for the next event. 
The veto reset time should be large enough that the event can pass through the complete spacecraft.
If the event raises a (high-level) trigger (= trigger threshold in geomega) the system waits the coincidence time - if not the event is ignored (because it is not measured).
If during the wait another event raises a veto, the system waits the reset time and resets - it does not wait until the coincidence time has passed!

Depending upon the exact trigger conditions set in the geomega setup file, after the coincidence time has passed, 
it is checked, if the trigger conditions are fillfilled, i.e. if we have the correct and enough high-level triggers (energy deposits above trigger threshold).
If this is the case, then all channels above the low-level trigger (= noise threshold in geomega) are read out.
 
The read-out time might be as small as the reset time, because the system can again wait for the next trigger immediately after the last one is detected, or it can be large because the whole system is dead during read-out (like the MEGA prototype).   



*/


// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <map>
#include <limits>
#include <algorithm>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <MString.h>
#include <TRandom.h>

// MEGAlib
#include "MAssert.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

class SimRandomCoincidence
{
public:
  /// Default constructor
  SimRandomCoincidence();
  /// Default destructor
  ~SimRandomCoincidence();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Analyze the event, return true if it has to be writen to file
  bool AnalyzeEvent(MSimEvent& Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

protected:
  bool WriteEvents(vector<MSimEvent*>& Events, int ID);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  vector<MString> m_FileNames;
  /// Simulation lengths
  vector<MTime> m_Times;
  /// Simulation lengths
  vector<int> m_Events;

  /// Output file name
  MString m_OutputFileName;
  /// Geometry file name
  MString m_GeoFileName;

  /// 
  MDGeometryQuest m_Geometry;

  ///
  MTime m_TimeCoincidence;
  ///
  MTime m_TimeReadout;
  ///
  MTime m_TimeReset;

  ///
  bool m_LooseReadouts;

  ///
  bool m_Recycle;

  ///
  bool m_Merge;

  ///
  bool m_CoincidencesOnly;

  ///
  int m_Seed;

  ///
  MTime m_StopTime;

  ///
  MFileEventsSim m_SiWriter;

  ///
  double m_TimeStretch;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
SimRandomCoincidence::SimRandomCoincidence() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);

  m_TimeStretch = 1000;

  m_TimeCoincidence.Set(0, 0);
  m_TimeReadout.Set(0, 0);
  m_TimeReset.Set(0, 0);

  m_StopTime.Set(numeric_limits<long>::max(), static_cast<long>(0));

  m_CoincidencesOnly = false;
  m_LooseReadouts = false;
  m_Recycle = false;
  m_Merge = false;

  m_Seed = 0;

  cout<<"Restrictions/Limitations: "<<endl;
  cout<<"  a) Trigger and read-out times are the same for all detectors"<<endl;
  cout<<"  b) During the merging of hits an initial trigger has to be verified and genberated. However, this trigger is AGAIN generated during revan. Since the thresholds are varying with th energy resolution, the results between here and revan might not be the same."<<endl;
  cout<<endl;
}


/******************************************************************************
 * Default destructor
 */
SimRandomCoincidence::~SimRandomCoincidence()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool SimRandomCoincidence::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: SimRandomCoincidence <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -ft:  <simulation file name> <time in s> "<<endl;
  Usage<<"         -fte:  <simulation file name> <time in s> <number of events>"<<endl;
  Usage<<"         -c:   <coincidence time window in ns>"<<endl;
  Usage<<"         -r:   <read-out time in ns>"<<endl;
  Usage<<"         -x:   <reset time in ns>"<<endl;
  Usage<<"         -t:   <stop after this time in s>"<<endl;
  Usage<<"         -o:   <output simulation file name>"<<endl;
  Usage<<"         -g:   <geometry file name>"<<endl;
  Usage<<"         -s:   <integer for the seed of the random number generator, default=0=random>"<<endl;
  Usage<<"         -w:   Write only random choincidences to the file"<<endl;
  Usage<<"         -m:   Ignore all timeing (readout, reset, coincidence) and simply merge the files"<<endl;
  Usage<<"         -e:   Count the number of events in the file instead of reading the last ID line"<<endl;
  Usage<<"         -l:   Really loose events during read-out instead of counting them"<<endl;
  Usage<<"         -y:   Recycle events from used files until the last file is worked through"<<endl;
  Usage<<"         -d:   debug"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;


  bool CountEvents = false;
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
    if (Option == "-f" || Option == "-o" || Option == "-r" || Option == "-c" || Option == "-t" || Option == "-x") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    else if (Option == "-ft") {
      if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    // Multiple arguments_
    else if (Option == "-fte") {
      if (!((argc > i+3) && argv[i+1][0] != '-' && argv[i+2][0] != '-' && argv[i+3][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs three arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }

    // Then fulfill the options:
    if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-ft") {
      if (m_Events.size() != 0) {
        cout<<"Error: You are not allowed to mix -ft and -fte"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      m_FileNames.push_back(argv[++i]);
      m_Times.push_back(MTime(atof(argv[++i])));
      if (m_Times.back().GetAsDouble() <= 0) {
        cout<<"Error: Need a positive time"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      if (m_FileNames.back().EndsWith(".sim") == false) {
        cout<<"Error: Need a simulation file name, not a "<<m_FileNames.back()<<" file "<<endl;
        cout<<Usage.str()<<endl;
        return false;
      } else {
        // Get number of events
        cout<<"Accepting file name: "<<m_FileNames.back()<<" at time "<<m_Times.back()<<endl;
      }
    } else if (Option == "-fte") {
      if (m_Events.size() != m_Times.size()) {
        cout<<"Error: You are not allowed to mix -ft and -fte"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      m_FileNames.push_back(argv[++i]);
      m_Times.push_back(MTime(atof(argv[++i])));
      m_Events.push_back(atoi(argv[++i]));
      if (m_Times.back().GetAsDouble() <= 0) {
        cout<<"Error: Need a positive time"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      if (m_FileNames.back().EndsWith(".sim") == false) {
        cout<<"Error: Need a simulation file name, not a "<<m_FileNames.back()<<" file "<<endl;
        cout<<Usage.str()<<endl;
        return false;
      } else {
        // Get number of events
        cout<<"Accepting file name: "<<m_FileNames.back()<<" at time "<<m_Times.back()<<" with "<<m_Events.back()<<" events"<<endl;
      }
    } else if (Option == "-c") {
      m_TimeCoincidence.Set(0, atoi(argv[++i]));
      if (m_TimeCoincidence.GetSeconds() > 1 || m_TimeCoincidence.GetNanoSeconds() == 0) {
        cout<<"Error: Your coincidence window must be smaller than 1 s and larger equal 1 ns!"<<endl;
        cout<<Usage.str()<<endl;
        return false;        
      }
      cout<<"Accepting coincidence window: "<<m_TimeCoincidence<<endl;
    } else if (Option == "-t") {
      m_StopTime.Set(atof(argv[++i]), 0.0);
      if (m_StopTime.GetSeconds() <= 0 && m_StopTime.GetNanoSeconds() <= 0) {
        cout<<"Error: Your stop time must be larger than 0 s!"<<endl;
        cout<<Usage.str()<<endl;
        return false;        
      }
      cout<<"Accepting stop time: "<<m_StopTime<<endl;
    } else if (Option == "-r") {
      m_TimeReadout.Set(0, atoi(argv[++i]));
      if (m_TimeReadout.GetSeconds() > 1 || m_TimeReadout.GetNanoSeconds() == 0) {
        cout<<"Error: Your readout time must be smaller than 1 s and larger equal 1 ns!"<<endl;
        cout<<Usage.str()<<endl;
        return false;        
      }
      cout<<"Accepting read out time: "<<m_TimeReadout<<endl;
    } else if (Option == "-x") {
      m_TimeReset.Set(0, atoi(argv[++i]));
      if (m_TimeReset.GetSeconds() > 1 || m_TimeReset.GetNanoSeconds() == 0) {
        cout<<"Error: Your reset time must be smaller than 1 s and larger equal 1 ns!"<<endl;
        cout<<Usage.str()<<endl;
        return false;        
      }
      cout<<"Accepting read out time: "<<m_TimeReadout<<endl;
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-s") {
      m_Seed = atoi(argv[++i]);
      cout<<"Accepting geometry file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-e") {
      CountEvents = true;
      cout<<"Accepting count events"<<endl;
    } else if (Option == "-w") {
      m_CoincidencesOnly = true;
      cout<<"Writing only coincidences to the file"<<endl;
    } else if (Option == "-m") {
      m_Merge = true;
      cout<<"Only merging files"<<endl;
    } else if (Option == "-l") {
      m_LooseReadouts = true;
      cout<<"Accepting loose events happening during read out"<<endl;
    } else if (Option == "-y") {
      m_Recycle = true;
      cout<<"Accepting recycling events"<<endl;
    } else if (Option == "-d") {
      if (g_Verbosity < 2) g_Verbosity = 2;
      cout<<"Entering debug mode..."<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileNames.size() < 2) {
    cout<<"Error: Need at least two simulation file names!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_OutputFileName == "") {
    cout<<"Error: Need a name for the output sim file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  // Check times:
  if (m_TimeCoincidence == MTime(0.0)) {
    cout<<"Error: We need a coincidence time > 0 ns!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }    

  // Check times:
  if (m_TimeReadout == MTime(0.0)) {
    cout<<"Error: We need a read out time > 0 ns!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }    

  // Check times:
  if (m_TimeReset == MTime(0.0)) {
    cout<<"Error: We need a reset time > 0 ns!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }    


  // Load geometry:
  if (m_GeoFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  } else {
    if (m_Geometry.ScanSetupFile(m_GeoFileName) == true) {
      cout<<"Geometry "<<m_Geometry.GetName()<<" loaded!"<<endl;
      m_Geometry.ActivateNoising(false);
      m_Geometry.SetGlobalFailureRate(0.0);
    } else {
      cout<<"Loading of geometry "<<m_Geometry.GetName()<<" failed!!"<<endl;
      return false;
    }  
  }

  // Check number of events:
  if (m_Events.size() == 0) {
    for (unsigned int i = 0; i < m_FileNames.size(); ++i) {
      MFileEventsSim SiReader(&m_Geometry);
      if (SiReader.Open(m_FileNames[i]) == false) {
        cout<<"Error: Unable to open sim file "<<m_FileNames[i]<<endl; 
        return false;
      }
      int NEvents = SiReader.GetNEvents(CountEvents);
      if (NEvents > 0) {
        m_Events.push_back(NEvents);
        cout<<"Accepting file name: "<<m_FileNames[i]<<" with "<<NEvents<<" events and time "<<m_Times[i]<<endl;
      } else {
        cout<<"Error: Unable to read file "<<m_FileNames[i]<<" or file has no events "<<endl;
        return false;
      }
    }
  }

  // Stretch all times:
  m_TimeCoincidence *= m_TimeStretch;
  m_TimeReadout *= m_TimeStretch;
  m_TimeReset *= m_TimeStretch;
  m_StopTime *= m_TimeStretch;
  for (unsigned int i = 0; i <= m_Times.size(); ++i) {
    m_Times[i] *= m_TimeStretch;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool SimRandomCoincidence::Analyze()
{
  // The trigger unit:
  MDTriggerUnit* Trigger = m_Geometry.GetTriggerUnit();

  // The number of files:
  unsigned int size = m_FileNames.size();

  // The four modes of the coincidence electronics 
  const int WaitingForTrigger     = 0;
  const int WaitingForCoincidence = 1;
  const int WaitingForReset       = 2;
  const int WaitingForReadout     = 3;

  int Mode = WaitingForTrigger;


  // Determine time bar steps for the different modes:
  MTime MinTime(numeric_limits<long int>::max());
  MTime MaxTime(numeric_limits<long int>::max());
  for (unsigned int i = 0; i < size; ++i) {
    if (m_Times[i] < MaxTime) MaxTime = m_Times[i];
    if (m_Times[i]/m_Events[i] < MinTime) MinTime = m_Times[i]/m_Events[i];
  }


  MTime TimeStepTrigger(max(1e-9, 0.001*MinTime.GetAsDouble()));
  vector<double> EventsPerTimeStepTrigger;
  for (unsigned int i = 0; i < size; ++i) {
    EventsPerTimeStepTrigger.push_back(TimeStepTrigger.GetAsDouble()*m_Events[i]/m_Times[i].GetAsDouble());
    if (EventsPerTimeStepTrigger.back() > 1) {
      mout<<"Cannot handle rates which are larger than 1 cts/ns!!"<<endl;
      return false;
    }
  }
  mout<<"Time step size during wait for trigger: "<<TimeStepTrigger<<endl;

  
  MTime TimeStepCoincidence(max(1e-9, min(0.001*m_TimeCoincidence.GetAsDouble(), 0.01*MinTime.GetAsDouble())));
  vector<double> EventsPerTimeStepCoincidence;
  for (unsigned int i = 0; i < size; ++i) {
    EventsPerTimeStepCoincidence.push_back(TimeStepCoincidence.GetAsDouble()*m_Events[i]/m_Times[i].GetAsDouble());
    if (EventsPerTimeStepCoincidence.back() > 1) {
      mout<<"Cannot handle rates which are larger than 1 cts/ns!!"<<endl;
      return false;
    }
  }
  mout<<"Time step size in coincidence window: "<<TimeStepCoincidence<<endl;

  
  MTime TimeStepReset(max(1e-9, min(0.001*m_TimeReset.GetAsDouble(), 0.01*MinTime.GetAsDouble())));
  vector<double> EventsPerTimeStepReset;
  for (unsigned int i = 0; i < size; ++i) {
    EventsPerTimeStepReset.push_back(TimeStepReset.GetAsDouble()*m_Events[i]/m_Times[i].GetAsDouble());
    if (EventsPerTimeStepReset.back() > 1) {
      mout<<"Cannot handle rates which are larger than 1 cts/ns!!"<<endl;
      return false;
    }
  }
  mout<<"Time step size during reset: "<<TimeStepReset<<endl;

  
  MTime TimeStepReadout(max(1e-9, min(0.001*m_TimeReadout.GetAsDouble(), 0.01*MinTime.GetAsDouble())));
  vector<double> EventsPerTimeStepReadout;
  for (unsigned int i = 0; i < size; ++i) {
    EventsPerTimeStepReadout.push_back(TimeStepReadout.GetAsDouble()*m_Events[i]/m_Times[i].GetAsDouble());
    if (EventsPerTimeStepReadout.back() > 1) {
      mout<<"Cannot handle rates which are larger than 1 cts/ns!!"<<endl;
      return false;
    }
  }
  mout<<"Time step size during readout: "<<TimeStepReadout<<endl;


  // Open and prepare all files:

  // Open input files
  vector<MFileEventsSim*> SiReaders;
  for (unsigned int i = 0; i < size; ++i) {
    MFileEventsSim* SiReader = new MFileEventsSim(&m_Geometry);
    if (SiReader->Open(m_FileNames[i]) == false) {
      cout<<"Unable to open sim file \""<<m_FileNames[i]<<"\""<<endl; 
      return false;
    }
    SiReaders.push_back(SiReader);
  }

  // The data of these input files:
  MSimEvent* SiEvent = 0;
  vector<MSimEvent*> CurrentEventList;
  
  // Keep track if the end of the file has been reached:
  vector<bool> EndOfFiles;
  for (unsigned int i = 0; i < size; ++i) {
    EndOfFiles.push_back(false);
  }

  // Open output file:
  m_SiWriter.SetGeometry(&m_Geometry);
  if (m_SiWriter.Open(m_OutputFileName, MFile::c_Write) == false) {
    cout<<"Unable to open output file  \""<<m_OutputFileName<<"\""<<endl;
    return false;
  }

  ostringstream Header;
  int Version = SiReaders[0]->GetVersion();
  Header<<"Type SIM"<<endl;
  Header<<"Version "<<Version<<endl;
  Header<<"Geometry "<<m_GeoFileName<<endl;
  Header<<endl;
  m_SiWriter.SetVersion(Version);
  m_SiWriter.AddText(Header.str().c_str());



  // Initialize statistical information:
  int NWrittenEvents = 0;
  int NUsedEvents = 0;
  int NTriggersOriginal = 0;
  int NTriggersNotRaised = 0;
  int NTriggersRaised = 0;
  int NTriggersRaisedNoCoincidence = 0;
  int NTriggeredEvents = 0;
  int NVetoedEvents = 0;
  int NLost = 0;
  int NRandomCoincidences = 0;
  int NAboveThreshold = 0;
  TH2D* UpgradeMap = new TH2D("UpgradeMap", "UpgradeMap", 100, -0.5, 100.5, 100, -0.5, 100.5); 


  TRandom Random(m_Seed);

  // Keep track of all times...
  MTime TimeBar(0.0);

  MTime TimeEndOfCoincidence(0.0);
  MTime TimeEndOfReset(0.0);
  MTime TimeEndOfReadout(0.0);

  MTime DeadTime(0.0);

  // ... and significant changes
  bool StartReadout = false;
  bool TriggerRaised = false;
  bool VetoRaised = false;
  bool EventAdded = false;

  // Now start the loop...
  while (true) {

    // Some resets:
    StartReadout= false;
    TriggerRaised = false;
    VetoRaised = false;
    EventAdded = false;    

    // Advance time:
    if (Mode == WaitingForTrigger) {
      TimeBar += TimeStepTrigger;
    } else if (Mode == WaitingForCoincidence) {
      TimeBar += TimeStepCoincidence;
      DeadTime += TimeStepCoincidence;
    } else if (Mode == WaitingForReadout) {
      TimeBar += TimeStepReadout;
      DeadTime += TimeStepReadout;
    } else if (Mode == WaitingForReset) {
      TimeBar += TimeStepReset;
      DeadTime += TimeStepReset;
    }

    // Check for premature end:
    if (TimeBar >= m_StopTime) {
      mout<<"Stop time exceeded"<<endl;
      break;
    }



    // Loop over all files and add events...
    for (unsigned int i = 0; i < size; ++i) {

      double Rate = 0.0;
      if (Mode == WaitingForTrigger) {
        Rate = EventsPerTimeStepTrigger[i];
      } else if (Mode == WaitingForCoincidence) {
        Rate = EventsPerTimeStepCoincidence[i];
      } else if (Mode == WaitingForReadout) {
        Rate = EventsPerTimeStepReadout[i];
      } else if (Mode == WaitingForReset) {
        Rate = EventsPerTimeStepReset[i];
      }

      // In case we have to add a new event to this one:
      if (Random.Rndm() < Rate) {
        // If we are in read out, we loose this event:
        if (Mode == WaitingForReadout || Mode == WaitingForReset) {
          NLost++;
          // we only really loose if we want to, otherwise we use this event when we are outside the read-out window
          if (m_LooseReadouts == true) {
            SiEvent = SiReaders[i]->GetNextEvent(false);
            NUsedEvents++;
            if (SiEvent == 0) {
              EndOfFiles[i] = true;
              if (m_Recycle == true) {
                SiReaders[i]->Rewind();
                SiEvent = SiReaders[i]->GetNextEvent(false);
              }
            }
            delete SiEvent;
          }
        } 
        // Waiting for trigger or coincidence - we add the event to our current event list
        else {
          SiEvent = SiReaders[i]->GetNextEvent(false);
          if (SiEvent == 0) {
            EndOfFiles[i] = true;                        
            if (m_Recycle == true) {
              SiReaders[i]->Rewind();
              SiEvent = SiReaders[i]->GetNextEvent(false);
            }
          }

          if (CurrentEventList.size() != 0) {
            Trigger->Reset();
            for (unsigned int h = 0; h < SiEvent->GetNHTs(); ++h) {
              Trigger->AddHit(SiEvent->GetHTAt(h)->GetEnergy(), 
                              *(SiEvent->GetHTAt(h)->GetVolumeSequence()));
            }
            if (Trigger->HasTriggered() == true) {
              mdebug<<CurrentEventList[0]->GetID()<<": "<<SiEvent->GetID()<<" complete trigger - added!"<<endl;
            }
          }

          massert(SiEvent != 0);

          //mout<<"Adding: "<<SiEvent->GetID()<<" at "<<TimeBar<<endl;

          SiEvent->SetTime(TimeBar);
          CurrentEventList.push_back(SiEvent);
          if (m_Merge == true) {
            WriteEvents(CurrentEventList, ++NWrittenEvents);
          } else {
            EventAdded = true;
          }
          NUsedEvents++;
        }
      }
    }


    // We now have collected all new hits in CurrentEventList

    if (CurrentEventList.size() > 0) {

      // If we are waiting for a trigger, check if we have one: 

      if (Mode == WaitingForTrigger) {
        Trigger->Reset();
        
        NAboveThreshold = 0;
        for (unsigned int i = 0; i < CurrentEventList.size(); ++i) {
          for (unsigned int h = 0; h < CurrentEventList[i]->GetNHTs(); ++h) {
            if (Trigger->AddHit(CurrentEventList[i]->GetHTAt(h)->GetEnergy(), 
                                *(CurrentEventList[i]->GetHTAt(h)->GetVolumeSequence())) == true) {
              NAboveThreshold++;
            }
          }
        }

        if (Trigger->HasVetoed() == true) {
          VetoRaised = true;
          NVetoedEvents++;
          WriteEvents(CurrentEventList, ++NWrittenEvents);
        } else if (NAboveThreshold > 0) {
//           mout<<CurrentEventList[0]->GetID()<<" Trigger raised!"<<" at "<<TimeBar<<endl;
//           for (unsigned int i = 0; i < CurrentEventList.size(); ++i) {
//             mout<<CurrentEventList[i]->ToSimString()<<endl;
//           }
//           mout<<"Yes"<<endl;
          if (Trigger->HasTriggered() == true) {
            mdebug<<CurrentEventList[0]->GetID()<<" complete trigger initial!"<<endl;
            NTriggersOriginal++;
          }
          TriggerRaised = true;
          NTriggersRaised++;
        } else {
          // Nothing happened, e.g. only guard ring hit, etc.
          WriteEvents(CurrentEventList, ++NWrittenEvents);
          NTriggersNotRaised++;
        }
      } 

      // If we are waiting for a coincidence...
      else if (Mode == WaitingForCoincidence) {
        // ... if an event has beed added check if we have a veto:
        if (EventAdded == true) {
          Trigger->Reset();
          
          for (unsigned int i = 0; i < CurrentEventList.size(); ++i) {
            for (unsigned int h = 0; h < CurrentEventList[i]->GetNHTs(); ++h) {
              Trigger->AddHit(CurrentEventList[i]->GetHTAt(h)->GetEnergy(), 
                              *(CurrentEventList[i]->GetHTAt(h)->GetVolumeSequence()));
            }
          }
          if (Trigger->HasVetoed() == true) {
            mout<<CurrentEventList[0]->GetID()<<" Veto raised!"<<" at "<<TimeBar<<endl;
            VetoRaised = true;
            NVetoedEvents++;
            WriteEvents(CurrentEventList, ++NWrittenEvents);
          }
        }
      }
    }
    



    // Advance mode:
    if (Mode == WaitingForTrigger) {
      if (VetoRaised == true) {
        Mode = WaitingForReset;
        TimeEndOfReset = TimeBar + m_TimeReset;
      } else if (TriggerRaised == true) {
        Mode = WaitingForCoincidence;
        TimeEndOfCoincidence = TimeBar + m_TimeCoincidence;
      } else {
        // Interrupt only if we are waiting for a trigger
        if (m_Interrupt == true) break;
      }
    } else if (Mode == WaitingForCoincidence) {
      if (VetoRaised == true) {
        Mode = WaitingForReset;
        TimeEndOfReset = TimeBar + m_TimeReset;
      } else if (TimeBar >= TimeEndOfCoincidence) {
        Mode = WaitingForReadout;
        TimeEndOfCoincidence = 0;
        TimeEndOfReadout = TimeBar + m_TimeReadout;
        StartReadout = true;
      }
    } else if (Mode == WaitingForReadout) {
      if (TimeBar >= TimeEndOfReadout) {
        Mode = WaitingForTrigger;
        TimeEndOfReadout = 0;
      }
    } else if (Mode == WaitingForReset) {
      if (TimeBar >= TimeEndOfReset) {
        Mode = WaitingForTrigger;
        TimeEndOfReset = 0;
      }
    }



    // If we are the first time in readout mode (event list not yet empty)
    // then check for triggers and write the data;
    
    if (StartReadout == true) {
      // First check if we have a trigger:
      Trigger->Reset();
      
      int NFinalAboveThreshold = 0;
      for (unsigned int i = 0; i < CurrentEventList.size(); ++i) {
        for (unsigned int h = 0; h < CurrentEventList[i]->GetNHTs(); ++h) {
          if (Trigger->AddHit(CurrentEventList[i]->GetHTAt(h)->GetEnergy(), 
                              *(CurrentEventList[i]->GetHTAt(h)->GetVolumeSequence())) == true) {
            NFinalAboveThreshold++;
          }
        }
      }
      
      if (Trigger->HasTriggered() == true) {
        // We have a random coincidence if we have more than one event in the list:
        if (CurrentEventList.size() > 1) {
          mdebug<<CurrentEventList[0]->GetID()<<": Coincidence!"<<endl;
          NRandomCoincidences++;
        }
        if (CurrentEventList.size() >= 1) {
          NTriggeredEvents++;
          WriteEvents(CurrentEventList, ++NWrittenEvents);
        }
        //mout<<CurrentEventList[0]->GetID()<<" Final trigger raised!"<<" at "<<TimeBar<<endl;
        UpgradeMap->Fill(NAboveThreshold, NFinalAboveThreshold);
      } else {
        // We have not triggered in coincidence - same behaviour as during veto:
        VetoRaised = true;
        NTriggersRaisedNoCoincidence++;
        //mout<<CurrentEventList[0]->GetID()<<" No coincidence "<<" at "<<TimeBar<<endl;
        WriteEvents(CurrentEventList, ++NWrittenEvents);        
      }
    } 
    


    // Keep track of recycling and end of files:
    if (m_Recycle == false) {
      bool End = false;
      for (unsigned int i = 0; i < size; ++i) {
        if (EndOfFiles[i] == true) {
          End = true;
          break;
        }
      }    
      if (End == true) {
        break;
      }
    } else {
      bool End = true;
      for (unsigned int i = 0; i < size; ++i) {
        if (EndOfFiles[i] == false) {
          End = false;
          break;
        }
      }    
      if (End == true) {
        break;
      }
    }


  } // end of all time ;-)


  for (unsigned int i = 0; i < size; ++i) {
    SiReaders[i]->Close();
  }
  m_SiWriter.Close();

  cout<<endl;
  cout<<endl;
  cout<<"Some statistical information: "<<endl;
  cout<<endl;
  cout<<"This does not take into account noise/noise thresholds! For the correct answer use revan!"<<endl;
  cout<<endl;
  cout<<"Duration .................................. "<<TimeBar.GetAsDouble() / m_TimeStretch<<" s"<<endl;
  cout<<"Dead time ................................. "<<100 * DeadTime.GetAsDouble() / TimeBar.GetAsDouble()<<" %"<<endl;
  cout<<endl;
  cout<<"Used events ............................... "<<NUsedEvents<<endl;
  cout<<"No trigger raised ......................... "<<NTriggersNotRaised<<endl;
  cout<<"Raised triggers ........................... "<<NTriggersRaised<<endl;
  cout<<"Raised triggers (complete) ................ "<<NTriggersOriginal<<endl;
  cout<<"Raised triggers but not in coincidence .... "<<NTriggersRaisedNoCoincidence<<endl;
  cout<<"Triggered events .......................... "<<NTriggeredEvents<<endl;
  cout<<"Vetoed events ............................. "<<NVetoedEvents<<endl;
  cout<<"Random Coincidences ....................... "<<NRandomCoincidences<<endl;
  cout<<"Lost in readout/reset ..................... "<<NLost<<endl;
  cout<<endl;
  cout<<endl;

  TCanvas* UpgradeMapCanvas = new TCanvas();
  UpgradeMapCanvas->cd();
  UpgradeMap->Draw("colz");
  UpgradeMapCanvas->Update();


  return true;
}

/******************************************************************************/

bool SimRandomCoincidence::WriteEvents(vector<MSimEvent*>& CurrentEventList, int ID)
{
  if (m_CoincidencesOnly == true && CurrentEventList.size() == 1) {
    delete CurrentEventList[0];
  } else {
    MSimEvent* SiEvent = CurrentEventList[0];
    int StartedEventNumber = SiEvent->GetStartedEventNumber();
    //cout<<SiEvent->ToSimString()<<endl;
    for (unsigned int i = 1; i < CurrentEventList.size(); ++i) {
      //cout<<CurrentEventList[i]->ToSimString()<<endl;
      SiEvent->Add(*CurrentEventList[i]);
      StartedEventNumber += CurrentEventList[i]->GetStartedEventNumber();
      delete CurrentEventList[i];
    }
    
    SiEvent->SetID(ID);
    SiEvent->SetStartedEventNumber(StartedEventNumber);
    // We have to rediscretize in case two hits are in the same voxel:
    SiEvent->Discretize(1);
    SiEvent->Discretize(2);
    SiEvent->Discretize(3);
    SiEvent->Discretize(4);
    m_SiWriter.AddEvent(SiEvent);      
    delete SiEvent;
  }

  CurrentEventList.clear();

  return true;
}

/******************************************************************************/

SimRandomCoincidence* g_Prg = 0;
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
  signal(SIGINT, CatchSignal);

  TApplication SimRandomCoincidenceApp("SimRandomCoincidenceApp", 0, 0);

  g_Prg = new SimRandomCoincidence();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  SimRandomCoincidenceApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
