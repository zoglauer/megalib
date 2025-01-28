/*
 * MCRun.cxx
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


// Cosima:
#include "MCRun.hh"
#include "MCSource.hh"
#include "MCEventAction.hh"
#include "MCRunManager.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"

// MEGAlib:
#include "MStreams.h"


/******************************************************************************/

const int MCRun::c_StopByTriggers = 0;
const int MCRun::c_StopByEvents = 1;
const int MCRun::c_StopByTime = 2;

// bool NextEmissionSort(const MCSource* S1, const MCSource* S2) {
//   return (S1->GetNEventsToSkip() + S1->GetNEventsSkipped() < S2->GetNEventsToSkip() + S2->GetNEventsSkipped());
// }

/******************************************************************************
 * Default constructor - does nothing
 */
MCRun::MCRun()
{
  m_Duration = numeric_limits<double>::max();
  m_Triggers = numeric_limits<long>::max();
  m_Events = numeric_limits<long>::max();
  m_StopCondition = c_StopByTriggers;

  m_SimulatedTime = 0.0;
  m_NSimulatedEvents = 0;
  m_NTriggeredEvents = 0;
  m_NGeneratedParticles = 0;

  m_NextEmissionScale = 0.01;

  m_NAddedIsotopes = 0;

  m_NSkippedEvents = 0;
  
  m_FileName = "";
  m_ParallelID = 0;  
  m_IncarnationID = 0;  
  m_IsIncarnationIDFixed = false;
  m_Zip = false;
  m_StoreBinary = false;
  
  m_TcpIpHostName = "";
  m_TcpIpPort = 9090;

  m_IsotopeStoreNameUpdated = false;
}


/******************************************************************************
 * Default destructor - does nothing
 */
MCRun::~MCRun()
{
  // Intentionally left blank
}


/******************************************************************************
 * Return true, if the duration could be set correctly
 */
bool MCRun::SetDuration(const double& Duration) 
{ 
  if (Duration > 0) {
    m_Duration = Duration;
    m_StopCondition = c_StopByTime;
    return true;
  }

  return false;
}


/******************************************************************************
 * Return true, if the triggers could be set correctly
 */
bool MCRun::SetTriggers(const long& Triggers) 
{ 
  if (Triggers > 0) {
    m_Triggers = Triggers;
    m_StopCondition = c_StopByTriggers;
    return true;
  }

  return false;
}


/******************************************************************************
 * Return true, if the events could be set correctly
 */
bool MCRun::SetEvents(const long& Events) 
{ 
  if (Events > 0) {
    m_Events = Events;
    m_StopCondition = c_StopByEvents;
    return true;
  }

  return false;
}


/******************************************************************************
 * Set the activation mode 
 */
bool MCRun::SetActivationMode(const int ActivationMode)
{
  m_ActivationMode = ActivationMode;  
  
  return true;
}


/******************************************************************************
 * Add a source to the list - by name and only if it does not exist already
 */
bool MCRun::AddSource(const MString& Name, bool Test) 
{ 
  if (Test == true) {
    for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
      if (Name == m_SourceList[i]->GetName()) return false;
    }
  }

  MCSource* Source = new MCSource(Name);
  m_SourceList.push_back(Source);

  return true;
}


/******************************************************************************
 * Add a source to the list - by name and only if it does not exist already
 */
bool MCRun::AddSource(MCSource* Source, bool Test) 
{ 
  if (Test == true) {
    for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
      if (Source->GetName() == m_SourceList[i]->GetName()) return false;
    }
  }

  // Sort them according to their flux - descending (for access speed):
  m_SourceList.push_back(Source);

  return true;
}


/******************************************************************************
 * Get a specific source
 */  
MCSource* MCRun::GetSource(unsigned int i)
{
  if (i >= m_SourceList.size()) {
    merr<<"Source index out of bounds: index="<<i<<" max="<<m_SourceList.size()<<endl;
    assert(i < m_SourceList.size());
  }

  return m_SourceList[i];
}


/******************************************************************************
 * Return true, if the geometry file could be set correctly
 */
bool MCRun::SetGeometryFileName(const MString& Name) 
{ 
  // TODO: Add test is file exists!

  m_GeometryFileName = Name; 

  return true;
}


/******************************************************************************
 *  Add an event to an event list (used for activation simualtion)
 */
bool MCRun::SkipOneEvent(G4ParticleDefinition* ParticleType, 
                         MString VolumeName) 
{
  // --> Time critical
  // The source list gets sorted according to the number of skipped events in GeneratePrimaries
  bool Found = false;
  unsigned int i_max = m_SourceList.size();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SourceList[i]->GetParticleDefinition() == ParticleType) {
      if (m_SourceList[i]->GetVolume() == VolumeName) {
        m_SourceList[i]->NEventsToSkip(1);
        Found = true;
        break;
      }
    }
  }
  // <-- Time critical

  if (Found == false) {
    mout<<"Source to skip not found: "<<ParticleType->GetParticleName()<<" in "<<VolumeName<<endl;
    mout<<"This happens either if your initial activation run was too short to cover all events or your coincidence timings are different."<<endl;
  }

  return true;
}


/******************************************************************************
 *  Add an event to an event list (used for activation simualtion)
 *  Return true if new source has been added
 */
bool MCRun::AddToBuildUpEventList(double Energy, 
                                  G4ThreeVector Position, 
                                  G4ThreeVector Direction, 
                                  G4ThreeVector Polarization, 
                                  long double Time,
                                  G4ParticleDefinition* ParticleType, 
                                  MString VolumeName)
{
  // Time = 99.9*s - m_SimulatedTime;

  if (m_StopCondition == c_StopByTime) {
    if (Time+m_SimulatedTime > m_Duration) {
      //mout<<"Not storing event, since it happens beyond the duration of this run): "<<(Time+m_SimulatedTime)/s<<" sec"<<endl;
      return true;
    }
  }

  // if we already have a build-up event-list-source, add it (usually the event list is the last entry)
  // and redetermine the next particle emission from this source.
  for (unsigned int i = m_SourceList.size()-1; i < m_SourceList.size(); --i) {
    if (m_SourceList[i]->IsBuildUpEventList() == true) {
      m_SourceList[i]->AddToEventList(Energy, Position, Direction, Polarization, Time+m_SimulatedTime, ParticleType, VolumeName);
      //cout<<m_SourceList[i]->GetName()<<": Added: "<<ParticleType->GetParticleName()<<endl;
      m_NextEmission.erase(m_SourceList[i]); // force resorting
      m_SourceList[i]->CalculateNextEmission(m_SimulatedTime, m_NextEmissionScale);
      m_NextEmission.insert(m_SourceList[i]);
      return true;
    }
  }

  // ... otherwise create one
  MCSource* SourceEventList = new MCSource("DelayedDecaysList");
  m_SourceList.push_back(SourceEventList);
  m_SourceList.back()->AddToEventList(Energy, Position, Direction, Polarization, Time+m_SimulatedTime, ParticleType, VolumeName);
  m_SourceList.back()->SetFlux(1.0);
  m_SourceList.back()->SetBuildUpEventList();

  // Finally update the NextEmission-matrix of the source generator
  m_SourceList.back()->CalculateNextEmission(m_SimulatedTime, m_NextEmissionScale);
  m_NextEmission.insert(m_SourceList.back());
  //cout<<m_SourceList.back().GetName()<<": Added: "<<ParticleType->GetParticleName()<<endl;
  

  return false;
}


/******************************************************************************
 * Add an element to the radioactive particle store
 */
void MCRun::AddIsotope(G4Ions* Particle, G4TouchableHistory* Hist)
{
  G4LogicalVolume* V = Hist->GetVolume(0)->GetLogicalVolume();

//   for (int i = 0; i < Hist->GetHistoryDepth(); ++i) {
//     cout<<Hist->GetVolume(i)->GetLogicalVolume()->GetName()<<endl;
//   }

  // Remove the final "Log"
  MString Name = V->GetName();
  Name.Remove(Name.Length()-3, 3);

  m_IsotopeStore.Add(Name, 1000*G4int(Particle->GetPDGCharge()/eplus) + Particle->GetBaryonNumber(), Particle->GetExcitationEnergy());
  m_NAddedIsotopes++;
  
  // Save the store after each 10000 adds
  if (m_NAddedIsotopes % 10000 == 0) {
    SaveIsotopeStore();
  }
}


/******************************************************************************
 * Check if a file with the given name already exists, and give the new 
 * incarnation of the file:
 */
void MCRun::CheckIncarnationID()
{
  if (m_FileName == "") return;
  
  mdebug<<"Checking incarnation ID"<<endl;
  if (m_IsIncarnationIDFixed == false) {
  
    m_IncarnationID = 0;
    int Id = 1;
    const int MaxCheckedId = 100;
    bool NoFilesFound = false;
    bool FileExists = true;
    
    mdebug<<"Incarnation check:"<<endl;
    do {
      // Incarnation check:
      Id = 1;
      do {
        m_IncarnationID++;
        ostringstream FileName;
        if (m_ParallelID == 0) {
          FileName<<m_FileName<<".inc"<<m_IncarnationID<<".id"<<Id<<".";
        } else {
          FileName<<m_FileName<<".p"<<m_ParallelID<<".inc"<<m_IncarnationID<<".id"<<Id<<".";
        }
        if (m_StoreBinary == true) {
          FileName<<".bin"; 
        }
        mdebug<<"Checking (loop 1): "<<FileName.str().c_str()<<"[sim,sim.gz,sim.zip] ...";
        if (MFile::FileExists((FileName.str() + "sim").c_str()) ||
            MFile::FileExists((FileName.str() + "sim.gz").c_str()) ||
            MFile::FileExists((FileName.str() + "sim.zip").c_str())) {
          FileExists = true;
          mdebug<<"file exists!"<<endl;
        } else {
          FileExists = false;
          mdebug<<"file does not exist!"<<endl;
        }    
      } while (FileExists == true);
      NoFilesFound = true;

      // ID check:
      for (int i = 1; i < MaxCheckedId; ++i) {
        Id++;
        ostringstream FileName;
        if (m_ParallelID == 0) {
          FileName<<m_FileName<<".inc"<<m_IncarnationID<<".id"<<Id<<".";
        } else {
          FileName<<m_FileName<<".p"<<m_ParallelID<<".inc"<<m_IncarnationID<<".id"<<Id<<".";
        }
        mdebug<<"Checking (loop 2): "<<FileName.str().c_str()<<"(...) ...";
        if (MFile::FileExists((FileName.str() + "sim").c_str()) ||
            MFile::FileExists((FileName.str() + "sim.gz").c_str()) ||
            MFile::FileExists((FileName.str() + "sim.zip").c_str())) {
          mdebug<<"file exists!"<<endl;
          NoFilesFound = false;
          break;
        } else {
          mdebug<<"file does not exist!"<<endl;
        }
      } 
    } while (NoFilesFound == false);
  }

  // Immediately create a dummy file, since it can take a long time until we create the real one:
  ostringstream Name;
  if (m_ParallelID == 0) {
    Name<<m_FileName<<".inc"<<m_IncarnationID<<".id1";
  } else {
    Name<<m_FileName<<".p"<<m_ParallelID<<".inc"<<m_IncarnationID<<".id1";
  }
  if (m_StoreBinary == true) {
    cout<<"Create binary sim file"<<endl;
    Name<<".bin"; 
  }
  Name<<".sim";
  if (m_Zip == true) {
    Name<<".gz"; 
  }
  ofstream out;
  out.open(Name.str().c_str());
  out<<"# You can delete me."<<endl;
  out.close();
  mdebug<<"Incarnation found: "<<Name.str()<<endl;
}


/******************************************************************************
 * Save the isotope store, if it has a name
 */
void MCRun::SaveIsotopeStore()
{
  m_IsotopeStore.SetTime(m_SimulatedTime);

  if (m_IsotopeStoreName != "") {
    if (m_IsotopeStoreNameUpdated == false) {
      ostringstream FileName;
      if (m_ParallelID == 0) {
        FileName<<m_IsotopeStoreName<<".inc"<<m_IncarnationID<<".dat";
      } else {
        FileName<<m_IsotopeStoreName<<".p"<<m_ParallelID<<".inc"<<m_IncarnationID<<".dat";
      }
      m_IsotopeStoreName = FileName.str().c_str();

      m_IsotopeStoreNameUpdated = true;
    }
    m_IsotopeStore.Save(m_IsotopeStoreName);
  }
}


/******************************************************************************
 * Check stop conditions
 */
bool MCRun::CheckStopConditions()
{
  bool ActiveSources = false;
  for (unsigned int so = 0; so < m_SourceList.size(); ++so) {
    if (m_SourceList[so]->IsActive() == true) {
      ActiveSources = true;
      break;
    }
  }
  if (ActiveSources == false) {
    mdebug<<"No active sources left!"<<endl;
    return true;
  }

  if (m_NTriggeredEvents == numeric_limits<long>::max()) {
    mout<<"Maximum number of triggered events ("<<numeric_limits<long>::max()<<") reached"<<endl;
    return true;
  }

  if (m_NSimulatedEvents == numeric_limits<long>::max()) {
    mout<<"Maximum number of simulated events ("<<numeric_limits<long>::max()<<") reached"<<endl;
    return true;
  }

  if (m_SimulatedTime == numeric_limits<double>::max()) {
    mout<<"Maximum time ("<<numeric_limits<double>::max()<<") reached"<<endl;
    return true;
  }

  if (m_StopCondition == c_StopByTriggers) {
    if (m_NTriggeredEvents >= m_Triggers) {
      mdebug<<"Stop condition \"StopByTriggers\" fulfilled!"<<endl;
      return true;
    }
  }
  else if (m_StopCondition == c_StopByTime) {
    if (m_SimulatedTime >= m_Duration) {
      mdebug<<"Stop condition \"StopByTime\" fulfilled!"<<endl;
      return true;
    }
  }
  else if (m_StopCondition == c_StopByEvents) {
    if (m_NSimulatedEvents >= m_Events) {
      mdebug<<"Stop condition \"StopByEvents\" fulfilled!"<<endl;
      return true;
    }
  }

  return false;
}


/******************************************************************************
 * Stop this run
 */
void MCRun::Stop()
{
  G4RunManager::GetRunManager()->AbortRun(true);
}


/******************************************************************************
 * Dump a Run statistics
 */
void MCRun::DumpRunStatistics(double CPUTime)
{
  // Use cout to make sure it is also dumped in "-v 0" mode

  cout<<endl;
  cout<<endl;
  cout<<"Summary for run "<<m_Name<<endl;
  cout<<endl;
  cout<<"Total number of generated particles:     "<<m_NGeneratedParticles<<endl;
  for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
    int Length = m_SourceList[i]->GetName().Length();
    cout<<"  Source "<<m_SourceList[i]->GetName()<<": ";
    for (int l = Length+11; l < 41; ++l) cout<<" ";
    cout<<m_SourceList[i]->GetNGeneratedParticles()<<endl;
  }
  cout<<endl;
  cout<<"Total CPU time spent in run:             "<<CPUTime<<" sec"<<endl;
  if (m_NGeneratedParticles > 0) {
    cout<<"Time spent per event:                    "<<CPUTime/m_NGeneratedParticles<<" sec"<<endl;
  } else {
    cout<<"Time spent per event:                    "<<0<<" sec"<<endl;
  }
  cout<<endl;
  cout<<"Observation time:                        "<<m_SimulatedTime/s<<" sec"<<endl;
  cout<<endl;
}


/******************************************************************************
 * Some preparation before we can start the run:
 */
bool SourceListFluxSort(MCSource* S1, MCSource* S2) {
  return (S1->GetFlux() > S2->GetFlux());
}

bool MCRun::Initialize()
{
  // We have to generate the particle definition here, 
  // because they seem to depend on the physics lists!! 
  for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
    m_SourceList[i]->GenerateParticleDefinition();
  }

  sort(m_SourceList.begin(), m_SourceList.end(), SourceListFluxSort);
  
  // Advance the time if we have orientations with defined start and stop times
  if (m_SkyOrientation.IsOriented() && m_SkyOrientation.IsLooping() == false) {
    if (m_SkyOrientation.GetStartTime() > m_SimulatedTime) {
      m_SimulatedTime = m_SkyOrientation.GetStartTime();
    }
  }
  if (m_DetectorOrientation.IsOriented() && m_DetectorOrientation.IsLooping() == false) {
    if (m_DetectorOrientation.GetStartTime() > m_SimulatedTime) {
      m_SimulatedTime = m_DetectorOrientation.GetStartTime();
    }
  }
  
  // Determine the first emissions
  for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
    if (m_SourceList[i]->IsSuccessor() == false) {
      m_SourceList[i]->CalculateNextEmission(m_SimulatedTime, m_NextEmissionScale);
      m_NextEmission.insert(m_SourceList[i]);
    } 
    //else {
    //  m_NextEmission[m_SourceList[i]] = -1;
    //}
    //cout<<"Initial emission for "<<m_SourceList[i]->GetName()<<": "<<m_SourceList[i]->GetNextEmission()<<":"<< m_NextEmission.size()<<endl;
  }

  //cout<<"Time: "<<m_SimulatedTime<<endl;
  
  return true;
}


/******************************************************************************
 * Generate the start vertex(ices) (energy, position, momentum, polarisation)
 * for a new event by using the particle gun
 */
bool SourceListSkippedSort(MCSource* S1, MCSource* S2) {
  if (S1->IsBuildUpEventList() == true) {
    return false;
  }
  if (S2->IsBuildUpEventList() == true) {
    return true;
  }
  return (S1->GetNEventsToSkip() + S1->GetNEventsSkipped() > S2->GetNEventsToSkip() + S2->GetNEventsSkipped());
}

void MCRun::GeneratePrimaries(G4Event* Event, G4GeneralParticleSource* ParticleGun)
{
  int NGeneratedParticles = 0;

  // This should only happen once at the beginning for the first event
//   if (m_SourceList.size() != m_NextEmission.size()) {
//     DetermineNextEmissions();
//   }

//   unsigned int MinID = 0;

  MCEventAction* EventAction = MCRunManager::GetMCRunManager()->GetEventAction();

  bool IsBuildUpSource = false;
  vector<int> InitialParticleTypes;
  bool GenerationSuccessful = true;
  do { // Until we have some events...

    // Step 1: Find the source which is next
    //         The info is stored in m_NextEmission
//     for (unsigned int i = 1; i < m_SourceList.size(); ++i) { // start with 1 ok since MinID already == 0
//       if (m_SourceList[i]->IsSuccessor() == false) {
//         if (m_NextEmission[i] < m_NextEmission[MinID]) MinID = i;
//       }
//     }
    
    MCSource* NextSource = *(m_NextEmission.begin());

//     set<MCSource*>::iterator Iter;
//     for (Iter = m_NextEmission.begin(); Iter != m_NextEmission.end(); ++Iter) {
//       cout<<"Next emission of: "<<(*Iter)->GetName()<<": "<<(*Iter)->GetNextEmission()<<endl;
//     }


    if (NextSource->IsBuildUpEventList() == true) {
      IsBuildUpSource = true;
    }

    //cout<<"Next emission: "<<NextSource->GetNextEmission()/s<<" sec --- "<<m_SimulatedTime/s<<"sec"<<endl;


    // Step 2: Check if one of the stop conditions is fulfilled
    //         Then we can stop now
    if (NextSource->GetNextEmission() == numeric_limits<long double>::max()) {
      // We ran out of events here, thus we are done. 
      // Thus just pick the event stop condition, and say it is fullfilled.
      m_StopCondition = c_StopByEvents;
      m_Events = m_NSimulatedEvents;
      return;
    }

    if (m_SkyOrientation.IsOriented() == true && m_SkyOrientation.IsLooping() == false) {
      if (NextSource->GetNextEmission() > m_SkyOrientation.GetStopTime()) {
    
        m_StopCondition = c_StopByEvents;
        m_Events = m_NSimulatedEvents;
        return;
      }
    }     
    
    m_SimulatedTime = NextSource->GetNextEmission();
    //cout<<"Time: "<<m_SimulatedTime/s<<"  Particle: "<<NextSource->GetName()<<endl;
    
    
    if (CheckStopConditions() == true) {
      // If we stop by time, we can safely set the duration as the current time...
      //cout<<"Stop: "<<NextSource->GetName()<<endl;
      if (GetStopCondition() == MCRun::c_StopByTime) {
        m_SimulatedTime = m_Duration;
      }
      return; // without new events...
    }


    // Step 3: Fill the event with primaries:
    //         Save the INIT information

    bool HasSuccessor = false;
    do {
//       if (m_SourceList[MinID].IsBuildUpEventList() == true) { 
//         break;
//       }

      // If the event is from the DelayedEventsList, then we have to skip the next event in the real activation list
      if (NextSource->IsBuildUpEventList() == true && m_ActivationMode == MCParameterFile::c_DecayModeActivationDelayedDecay) {
        
        unsigned int so_max = m_SourceList.size();
        G4ParticleDefinition* NextParticle = NextSource->GetEventListNextParticle();
        MString NextVolume = NextSource->GetEventListNextVolume();
        for (unsigned int so = 0; so < so_max; ++so) {
          if (m_SourceList[so]->IsEventList() == true) continue;
          
          if (NextParticle == m_SourceList[so]->GetParticleDefinition() &&
              NextVolume == m_SourceList[so]->GetVolume()) {
            m_SourceList[so]->NEventsToSkip(1);
            m_NSkippedEvents++;

            cout<<"Skips: "<<m_NSkippedEvents<<endl;
            // Some (older) compilers need double's
            if (m_NSkippedEvents >= 100 && m_NSkippedEvents%100 == 0 && fabs(log10(double(m_NSkippedEvents)) - int(log10(double(m_NSkippedEvents)))) < 1.0E-15) {
              cout<<"Resorting and thus optimizing source list..."<<m_NSkippedEvents<<":"<<fabs(log10(double(m_NSkippedEvents)) - int(log10(double(m_NSkippedEvents))))<<endl;
              sort(m_SourceList.begin(), m_SourceList.end(), SourceListSkippedSort);

              vector<MCSource*>::iterator Iter;
              for (Iter = m_SourceList.begin(); Iter != m_SourceList.end(); ++Iter) {
                cout<<"Skips of: "<<(*Iter)->GetName()<<": "<<(*Iter)->GetNEventsToSkip() + (*Iter)->GetNEventsSkipped()<<endl;
              }
            }

            break;
          }
        }
      }

      GenerationSuccessful = NextSource->GenerateParticles(ParticleGun);
      if (GenerationSuccessful == false) {
        HasSuccessor = false;
        break; // Break required to make sure we continue the time
      }


      ParticleGun->GeneratePrimaryVertex(Event);
      NGeneratedParticles++;
      
      InitialParticleTypes.push_back(NextSource->GetParticleType());

      //cout<<"Generated particle: "<<ParticleGun->GetParticleDefinition()->GetParticleName()<<endl;

      if (m_SkyOrientation.IsOriented() == true && m_SkyOrientation.GetCoordinateSystem() == MCOrientationCoordinateSystem::c_Galactic) {
        double XLat = 0.0, XLong = 0.0, ZLat = 0.0, ZLong = 0.0; 
        
        m_SkyOrientation.GetOrientation(m_SimulatedTime, XLat, XLong, ZLat, ZLong);
        EventAction->SetGalacticPointing(XLat, XLong, ZLat, ZLong);
      }
      
      
      EventAction->AddIA("INIT", 
                         NGeneratedParticles,
                         0,
                         0,
                         0.0,
                         ParticleGun->GetParticlePosition(),
                         0,
                         G4ThreeVector(0.0, 0.0, 0.0),
                         G4ThreeVector(0.0, 0.0, 0.0),
                         0.0,
                         NextSource->GetParticleType(),
                         ParticleGun->GetParticleMomentumDirection(),
                         ParticleGun->GetParticlePolarization(),
                         ParticleGun->GetParticleEnergy());
      
      if (NextSource->GetSuccessor() != "") {
        for (unsigned int so = 0; so < m_SourceList.size(); ++so) {
          if (m_SourceList[so]->GetName() == NextSource->GetSuccessor()) {
            NextSource = m_SourceList[so];
            HasSuccessor = true;
            if (NextSource->GetBeamType() == MCSource::c_NearFieldReverseDirectionToPredecessor) {
               NextSource->SetPosition(ParticleGun->GetParticlePosition().getX(), 
                                       ParticleGun->GetParticlePosition().getY(),
                                       ParticleGun->GetParticlePosition().getZ(),
                                       -ParticleGun->GetParticleMomentumDirection().getX(), 
                                       -ParticleGun->GetParticleMomentumDirection().getY(),
                                       -ParticleGun->GetParticleMomentumDirection().getZ());
            }
            
            break;
          }
        }
      } else {
        HasSuccessor = false;                            
      }
    } while (HasSuccessor == true);

//     // Advance the time stored in NextEmission, and...
//     for (unsigned int i = 0; i < m_SourceList.size(); ++i) {
//       if (i != MinID) {
//         m_NextEmission[i] -= m_NextEmission[MinID];
//       }
//     }
    // ... for the used source, we have to determine the next emission:
    NextSource = *(m_NextEmission.begin());
    m_NextEmission.erase(NextSource); // force resorting
    NextSource->CalculateNextEmission(m_SimulatedTime, m_NextEmissionScale);
    m_NextEmission.insert(NextSource);

  } while (NGeneratedParticles == 0 && GenerationSuccessful == false);
  

  MCRunManager::GetMCRunManager()->GetTrackingAction()->SetNGeneratedParticles(NGeneratedParticles);

  MCRunManager::GetMCRunManager()->GetSteppingAction()->PrepareForNextEvent();
  MCRunManager::GetMCRunManager()->GetSteppingAction()->SetInitialParticles(InitialParticleTypes);
  MCRunManager::GetMCRunManager()->GetSteppingAction()->SetParticleOriginIsBuildUpSource(IsBuildUpSource);
  
  // We do not need those functions any more....
  m_NSimulatedEvents++;
  m_NGeneratedParticles += NGeneratedParticles;
}  


/*
 * MCRun.cc: the end...
 ******************************************************************************/
