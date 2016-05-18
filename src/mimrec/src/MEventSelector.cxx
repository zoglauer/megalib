/*
 * MEventSelector.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MEventSelector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEventSelector.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MUnidentifiableEvent.h"
#include "MStreams.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"
#include "MProjection.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MEventSelector)
#endif


////////////////////////////////////////////////////////////////////////////////


MEventSelector::MEventSelector()
{
  // Construct an instance of MEventSelector

  m_Geometry = 0;

  m_FirstTotalEnergyMin = 0;
  m_FirstTotalEnergyMax = numeric_limits<double>::max();
  m_SecondTotalEnergyMin = 0;
  m_SecondTotalEnergyMax = numeric_limits<double>::max();
  m_ThirdTotalEnergyMin = 0;
  m_ThirdTotalEnergyMax = numeric_limits<double>::max();
  m_FourthTotalEnergyMin = 0;
  m_FourthTotalEnergyMax = numeric_limits<double>::max();
  
  m_TimeUseFile = false;
  m_TimeMin.Set(0);
  m_TimeMax.Set(2000000000);
  m_TimeFile = "";
  
  m_TimeWalkMin = -numeric_limits<double>::max();
  m_TimeWalkMax = numeric_limits<double>::max();

  m_ElectronEnergyMin = 0;
  m_ElectronEnergyMax = numeric_limits<double>::max();
  m_GammaEnergyMin = 0;
  m_GammaEnergyMax = numeric_limits<double>::max();
  m_ComptonAngleMin = 0;
  m_ComptonAngleMax = 180;

  m_LeverArmMin = 0; //cm
  m_LeverArmMax = numeric_limits<double>::max();

  m_FirstLeverArmMin = 0; //cm
  m_FirstLeverArmMax = numeric_limits<double>::max();

  m_SequenceLengthMin = 2; 
  m_SequenceLengthMax = numeric_limits<int>::max();

  m_TrackLengthMin = 1; 
  m_TrackLengthMax = numeric_limits<int>::max();

  m_ClusteringQualityFactorMin = 0; 
  m_ClusteringQualityFactorMax = numeric_limits<double>::max();

  m_ComptonQualityFactorMin = 0; 
  m_ComptonQualityFactorMax = numeric_limits<double>::max();

  m_TrackQualityFactorMin = 0; 
  m_TrackQualityFactorMax = numeric_limits<double>::max();

  m_CoincidenceWindowMin = 0; 
  m_CoincidenceWindowMax = numeric_limits<double>::max();

  m_ThetaDeviationMax = 180;

  m_EventIdMin = 0; 
  m_EventIdMax = numeric_limits<int>::max();

  m_UseSource = false;
  m_SourcePosition = MVector(0, 0, c_FarAway);
  m_ARMMin = 0.0;
  m_ARMMax = 180.0;
  m_SPDMin = 0.0;
  m_SPDMax = 180.0;

  m_UseBeam = false;
  m_BeamStart = MVector(0, 0, c_FarAway);
  m_BeamFocalSpot = MVector(0, 0, 0.0);
  m_BeamRadius = 100000.0;
  m_BeamDepth = 100000.0;

  m_UsePhotos = true;
  m_UsePairs = true;
  m_UseComptons = true;
  m_UseTrackedComptons = true;
  m_UseNotTrackedComptons = true;
  m_UseUnidentifiables = true;

  m_UseDecays = true;
  m_UseFlaggedAsBad = false;

  m_OpeningAnglePairMin = 0; // deg
  m_OpeningAnglePairMax = 180;
  m_InitialEnergyDepositPairMin = 0; // keV
  m_InitialEnergyDepositPairMax = numeric_limits<double>::max();

  m_PairQualityFactorMin = 0; 
  m_PairQualityFactorMax = numeric_limits<double>::max();

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MEventSelector::MEventSelector(const MEventSelector& EventSelector)
{
  // Default copy constructor
 
  (*this) = EventSelector;
}

////////////////////////////////////////////////////////////////////////////////


MEventSelector::~MEventSelector()
{
  // Delete this instance of MEventSelector
}


////////////////////////////////////////////////////////////////////////////////


const MEventSelector& MEventSelector::operator=(const MEventSelector& EventSelector)
{
  // Default assignment constructor

  m_Geometry = EventSelector.m_Geometry;
  m_ExcludedDetectors = EventSelector.m_ExcludedDetectors;

  m_FirstTotalEnergyMin = EventSelector.m_FirstTotalEnergyMin;               
  m_FirstTotalEnergyMax = EventSelector.m_FirstTotalEnergyMax;               
  m_SecondTotalEnergyMin = EventSelector.m_SecondTotalEnergyMin;              
  m_SecondTotalEnergyMax = EventSelector.m_SecondTotalEnergyMax;              
  m_ThirdTotalEnergyMin = EventSelector.m_ThirdTotalEnergyMin;              
  m_ThirdTotalEnergyMax = EventSelector.m_ThirdTotalEnergyMax;              
  m_FourthTotalEnergyMin = EventSelector.m_FourthTotalEnergyMin;              
  m_FourthTotalEnergyMax = EventSelector.m_FourthTotalEnergyMax;              
  
  m_TimeUseFile = EventSelector.m_TimeUseFile;
  m_TimeMin = EventSelector.m_TimeMin;                           
  m_TimeMax = EventSelector.m_TimeMax;
  m_TimeFile = EventSelector.m_TimeFile;
  m_TimeGTI = EventSelector.m_TimeGTI;
  
  m_TimeWalkMin = EventSelector.m_TimeWalkMin;                       
  m_TimeWalkMax = EventSelector.m_TimeWalkMax;                       
                                                                               
  m_ElectronEnergyMin = EventSelector.m_ElectronEnergyMin;                 
  m_ElectronEnergyMax = EventSelector.m_ElectronEnergyMax;                 
  m_GammaEnergyMin = EventSelector.m_GammaEnergyMin;                    
  m_GammaEnergyMax = EventSelector.m_GammaEnergyMax;                    
  m_ComptonAngleMin = EventSelector.m_ComptonAngleMin;                   
  m_ComptonAngleMax = EventSelector.m_ComptonAngleMax;                   
  m_LeverArmMin = EventSelector.m_LeverArmMin;                       
  m_LeverArmMax = EventSelector.m_LeverArmMax;                       
  m_FirstLeverArmMin = EventSelector.m_FirstLeverArmMin;                  
  m_FirstLeverArmMax = EventSelector.m_FirstLeverArmMax;                  
  m_SequenceLengthMin = EventSelector.m_SequenceLengthMin;                 
  m_SequenceLengthMax = EventSelector.m_SequenceLengthMax;                 
  m_TrackLengthMin = EventSelector.m_TrackLengthMin;                    
  m_TrackLengthMax = EventSelector.m_TrackLengthMax;                    
  m_ClusteringQualityFactorMin = EventSelector.m_ClusteringQualityFactorMin;              
  m_ClusteringQualityFactorMax = EventSelector.m_ClusteringQualityFactorMax;              
  m_ComptonQualityFactorMin = EventSelector.m_ComptonQualityFactorMin;              
  m_ComptonQualityFactorMax = EventSelector.m_ComptonQualityFactorMax;              
  m_TrackQualityFactorMin = EventSelector.m_TrackQualityFactorMin;              
  m_TrackQualityFactorMax = EventSelector.m_TrackQualityFactorMax;              
  m_CoincidenceWindowMin = EventSelector.m_CoincidenceWindowMin;              
  m_CoincidenceWindowMax = EventSelector.m_CoincidenceWindowMax;              
  m_EventIdMin = EventSelector.m_EventIdMin;                        
  m_EventIdMax = EventSelector.m_EventIdMax;                        
  m_ThetaDeviationMax = EventSelector.m_ThetaDeviationMax;              
  m_EarthHorizon = EventSelector.m_EarthHorizon;
                                                                               
  m_InitialEnergyDepositPairMin = EventSelector.m_InitialEnergyDepositPairMin;              
  m_InitialEnergyDepositPairMax = EventSelector.m_InitialEnergyDepositPairMax;              
  m_OpeningAnglePairMin = EventSelector.m_OpeningAnglePairMin;               
  m_OpeningAnglePairMax = EventSelector.m_OpeningAnglePairMax;               
  m_PairQualityFactorMin = EventSelector.m_PairQualityFactorMin;              
  m_PairQualityFactorMax = EventSelector.m_PairQualityFactorMax;              
                 
  m_UseSource = EventSelector.m_UseSource;
  m_SourcePosition = EventSelector.m_SourcePosition;
  m_ARMMin = EventSelector.m_ARMMin;
  m_ARMMax = EventSelector.m_ARMMax;
  m_SPDMin = EventSelector.m_SPDMin;
  m_SPDMax = EventSelector.m_SPDMax;

  m_UseBeam  = EventSelector.m_UseBeam;
  m_BeamStart = EventSelector.m_BeamStart;
  m_BeamFocalSpot = EventSelector.m_BeamFocalSpot;
  m_BeamRadius = EventSelector.m_BeamRadius;
  m_BeamDepth = EventSelector.m_BeamDepth;
                    
  m_UsePhotos = EventSelector.m_UsePhotos;                         
  m_UsePairs = EventSelector.m_UsePairs;                          
  m_UseComptons = EventSelector.m_UseComptons;                       
  m_UseTrackedComptons = EventSelector.m_UseTrackedComptons;                
  m_UseNotTrackedComptons = EventSelector.m_UseNotTrackedComptons;              
  m_UseUnidentifiables = EventSelector.m_UseUnidentifiables;              
                                                                               
  m_UseDecays = EventSelector.m_UseDecays;                         
  m_UseFlaggedAsBad = EventSelector.m_UseFlaggedAsBad;                         

  m_NAccepted = EventSelector.m_NAccepted;                   
  m_NRejectedIsGood = EventSelector.m_NRejectedIsGood;                   
  m_NRejectedStartDetector = EventSelector.m_NRejectedStartDetector;     
  m_NRejectedTotalEnergy = EventSelector.m_NRejectedTotalEnergy;              
  m_NRejectedTime = EventSelector.m_NRejectedTime;                     
  m_NRejectedTimeWalk = EventSelector.m_NRejectedTimeWalk;                 
  m_NRejectedElectronEnergy = EventSelector.m_NRejectedElectronEnergy;              
  m_NRejectedGammaEnergy = EventSelector.m_NRejectedGammaEnergy;              
  m_NRejectedComptonAngle = EventSelector.m_NRejectedComptonAngle;              
  m_NRejectedFirstLeverArm = EventSelector.m_NRejectedFirstLeverArm;              
  m_NRejectedLeverArm = EventSelector.m_NRejectedLeverArm;                 
  m_NRejectedEventId = EventSelector.m_NRejectedEventId;                  
  m_NRejectedTrackLength = EventSelector.m_NRejectedTrackLength;              
  m_NRejectedSequenceLength = EventSelector.m_NRejectedSequenceLength;              
  m_NRejectedClusteringQualityFactor = EventSelector.m_NRejectedClusteringQualityFactor;              
  m_NRejectedComptonQualityFactor = EventSelector.m_NRejectedComptonQualityFactor;              
  m_NRejectedTrackQualityFactor = EventSelector.m_NRejectedTrackQualityFactor;              
  m_NRejectedCoincidenceWindow = EventSelector.m_NRejectedCoincidenceWindow;              
  m_NRejectedEarthHorizonCut = EventSelector.m_NRejectedEarthHorizonCut;              
  m_NRejectedThetaDeviationMax = EventSelector.m_NRejectedThetaDeviationMax;              
  m_NRejectedUsePhotos = EventSelector.m_NRejectedUsePhotos;                
  m_NRejectedUsePairs = EventSelector.m_NRejectedUsePairs;                 
  m_NRejectedUseComptons = EventSelector.m_NRejectedUseComptons;               
  m_NRejectedUseMuons = EventSelector.m_NRejectedUseMuons;                 
  m_NRejectedUseUnidentifiables = EventSelector.m_NRejectedUseUnidentifiables;              
  m_NRejectedUseDecays = EventSelector.m_NRejectedUseDecays;                
  m_NRejectedUseFlaggedAsBad = EventSelector.m_NRejectedUseFlaggedAsBad;                
  m_NRejectedUseTrackedComptons = EventSelector.m_NRejectedUseTrackedComptons;              
  m_NRejectedUseNotTrackedComptons = EventSelector.m_NRejectedUseNotTrackedComptons;              
  m_NRejectedOpeningAnglePair = EventSelector.m_NRejectedOpeningAnglePair;              
  m_NRejectedInitialEnergyDepositPair = EventSelector.m_NRejectedInitialEnergyDepositPair;              
  m_NRejectedPairQualityFactor = EventSelector.m_NRejectedPairQualityFactor;              
  m_NRejectedARM = EventSelector.m_NRejectedARM;
  m_NRejectedSPD = EventSelector.m_NRejectedSPD;
  m_NRejectedBeam = EventSelector.m_NRejectedBeam;              

  return (*this);
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::Reset()
{
  m_NAccepted = 0;
  m_NRejectedIsGood = 0;
  m_NRejectedStartDetector = 0;
  m_NRejectedTotalEnergy = 0;
  m_NRejectedTime = 0;
  m_NRejectedTimeWalk = 0;
  m_NRejectedElectronEnergy = 0;
  m_NRejectedGammaEnergy = 0;
  m_NRejectedComptonAngle = 0;
  m_NRejectedFirstLeverArm = 0;
  m_NRejectedLeverArm = 0;
  m_NRejectedEventId = 0;
  m_NRejectedTrackLength = 0;
  m_NRejectedSequenceLength = 0;
  m_NRejectedClusteringQualityFactor = 0;
  m_NRejectedComptonQualityFactor = 0;
  m_NRejectedTrackQualityFactor = 0;
  m_NRejectedCoincidenceWindow = 0;
  m_NRejectedEarthHorizonCut = 0;
  m_NRejectedThetaDeviationMax = 0;
  m_NRejectedUsePhotos = 0;
  m_NRejectedUsePairs = 0;
  m_NRejectedUseComptons = 0;
  m_NRejectedUseMuons = 0;
  m_NRejectedUseUnidentifiables = 0;
  m_NRejectedUseTrackedComptons = 0;
  m_NRejectedUseNotTrackedComptons = 0;
  m_NRejectedUseDecays = 0;
  m_NRejectedUseFlaggedAsBad = 0;
  m_NRejectedOpeningAnglePair = 0;
  m_NRejectedInitialEnergyDepositPair = 0;
  m_NRejectedPairQualityFactor = 0;
  m_NRejectedARM = 0;
  m_NRejectedSPD = 0;
  m_NRejectedBeam = 0;
}

 
////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSettings(MSettingsEventSelections* S)
{
  // Update the event selector with data from this storage facility

  SetFirstTotalEnergy(S->GetFirstEnergyRangeMin(), S->GetFirstEnergyRangeMax());
  SetSecondTotalEnergy(S->GetSecondEnergyRangeMin(), S->GetSecondEnergyRangeMax());
  SetThirdTotalEnergy(S->GetThirdEnergyRangeMin(), S->GetThirdEnergyRangeMax());
  SetFourthTotalEnergy(S->GetFourthEnergyRangeMin(), S->GetFourthEnergyRangeMax());
  
  SetTimeUseFile(S->GetTimeUseFile());
  SetTime(S->GetTimeRangeMin(), S->GetTimeRangeMax());
  SetTimeFile(S->GetTimeFile());
  
  SetTimeWalk(S->GetTimeWalkRangeMin(), S->GetTimeWalkRangeMax());

  UseUnidentifiables(S->GetEventTypeUnidentifiable());
  UsePhotos(S->GetEventTypePhoto());
  UseDecays(S->GetEventTypeDecay());
  UsePairs(S->GetEventTypePair());
  UseComptons(S->GetEventTypeCompton());
  UseTrackedComptons(S->GetEventTypeComptonTracked());
  UseNotTrackedComptons(S->GetEventTypeComptonNotTracked());

  UseFlaggedAsBad(S->GetFlaggedAsBad());

  SetGammaEnergy(S->GetEnergyRangeGammaMin(), S->GetEnergyRangeGammaMax());
  SetElectronEnergy(S->GetEnergyRangeElectronMin(), S->GetEnergyRangeElectronMax());
  SetComptonAngle(S->GetComptonAngleRangeMin(), S->GetComptonAngleRangeMax());

  SetExcludedDetectors(S->GetExcludedDetectors());

  // Create an earth-horizon object...
  MEarthHorizon EH;
  EH.SetEarthHorizon(S->GetEHCEarthPosition(), S->GetEHCAngle()*c_Rad);
  if (S->GetEHCType() == MEarthHorizon::c_NoTest) {
    EH.SetNoTest();
  } else if (S->GetEHCType() == MEarthHorizon::c_IntersectionTest) {
    EH.SetIntersectionTest();
  } else if (S->GetEHCType() == MEarthHorizon::c_ProbabilityTest) {
    EH.SetProbabilityTest(S->GetEHCProbability(), S->GetEHCComptonProbabilityFileName(), S->GetEHCPairProbabilityFileName());
  } else {
    mgui<<"Unknown EHC Type!"<<endl;
  }
  SetEarthHorizonCut(EH);

  // Set the source:
  if (S->GetSourceUsePointSource() == true) {
    MVector Pos;
    if (S->GetSourceCoordinates() == MProjection::c_Galactic) {
      Pos.SetMagThetaPhi(c_FarAway, (S->GetSourceLatitude()+90)*c_Rad, S->GetSourceLongitude()*c_Rad);
    } else if (S->GetSourceCoordinates() == MProjection::c_Spheric) {
      Pos.SetMagThetaPhi(c_FarAway, S->GetSourceTheta()*c_Rad, S->GetSourcePhi()*c_Rad);
    } else {
      Pos.SetXYZ(S->GetSourceX(), S->GetSourceY(), S->GetSourceZ());
    }
    SetSourceWindow(true, Pos);
    SetSourceARM(S->GetSourceARMMin(), S->GetSourceARMMax());
    SetSourceSPD(S->GetSourceSPDMin(), S->GetSourceSPDMax());
  } else {
    SetSourceWindow(false);
  }

  SetBeam(S->GetBeamUse(), 
          MVector(S->GetBeamStartX(), S->GetBeamStartY(), S->GetBeamStartZ()),
          MVector(S->GetBeamFocalSpotX(), S->GetBeamFocalSpotY(), S->GetBeamFocalSpotZ()));
  SetBeamRadius(S->GetBeamRadius());
  SetBeamDepth(S->GetBeamDepth());
  
  SetThetaDeviationMax(S->GetThetaDeviationMax());
  SetInitialEnergyDepositPair(S->GetInitialEnergyDepositPairMin(), S->GetInitialEnergyDepositPairMax());
  SetOpeningAnglePair(S->GetOpeningAnglePairMin(), S->GetOpeningAnglePairMax());
  SetDistance(S->GetDistanceRangeMin(), S->GetDistanceRangeMax());
  SetFirstDistance(S->GetFirstDistanceRangeMin(), S->GetFirstDistanceRangeMax());
  SetSequenceLength(S->GetSequenceLengthRangeMin(), S->GetSequenceLengthRangeMax());
  SetTrackLength(S->GetTrackLengthRangeMin(), S->GetTrackLengthRangeMax());
  SetClusteringQualityFactor(S->GetClusteringQualityFactorRangeMin(), S->GetClusteringQualityFactorRangeMax());
  SetComptonQualityFactor(S->GetComptonQualityFactorRangeMin(), S->GetComptonQualityFactorRangeMax());
  SetTrackQualityFactor(S->GetTrackQualityFactorRangeMin(), S->GetTrackQualityFactorRangeMax());
  SetCoincidenceWindow(S->GetCoincidenceWindowRangeMin(), S->GetCoincidenceWindowRangeMax());
  SetEventId(S->GetEventIdRangeMin(), S->GetEventIdRangeMax());
}

////////////////////////////////////////////////////////////////////////////////


bool MEventSelector::IsQualifiedEvent(MPhysicalEvent* Event, bool DumpOutput)
{
  // Check if the event fullfills all event selection criteria
  // This is the detailed version which is output capable and stores 
  // which criteria trigger on the event

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  if (g_Verbosity > 1) DumpOutput = true;

	bool Return = true;

//   if (Event->AllHitsGood() == false) {
//     if (DumpOutput == true) {
//       cout<<"ID "<<Event->GetId()<<": Not all hits good!"<<endl;
//     }
//     Return = false;
  //  }


  if (Event->IsGoodEvent() == false && m_UseFlaggedAsBad == false) {
    m_NRejectedIsGood++;
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Event flagged \"not good\"!"<<endl;
    }
    Return = false;
  }

//   mimp<<"MEGA beam activated!"<<show;

//   vector<double> XPos;
//   XPos.push_back(-6.4);
//   XPos.push_back(0);
//   XPos.push_back(6.4);

//   vector<double> YPos;
//   YPos.push_back(-6.4);
//   YPos.push_back(0);
//   YPos.push_back(6.4);

//   double Radius = 1.27;
//   MVector Pos;
//   if (Event->GetType() == MPhysicalEvent::c_Compton ||
//       Event->GetType() == MPhysicalEvent::c_Pair) {
//     if (Event->GetType() == MPhysicalEvent::c_Compton) {
//       Pos = ((MComptonEvent *) Event)->C1();
//     } else {
//       Pos = ((MPairEvent *) Event)->GetPairCreationIA();
//     }

//     bool Inside = false;
//     for (unsigned int x = 0; x < XPos.size(); ++x) {
//       for (unsigned int y = 0; y < YPos.size(); ++y) {
//         if (sqrt((XPos[x]-Pos[0])*(XPos[x]-Pos[0])+(YPos[y]-Pos[1])*(YPos[y]-Pos[1])) < Radius &&
//             Pos[2] < 26 && Pos[2] > 15) {
//           Inside = true;
//         }
//       }
//     }
//     if (Inside == false) {
//       //cout<<"Outside beam: "<<Pos[0]<<"!"<<Pos[1]<<"!"<<Pos[2]<<endl;
//       Return = false;
//     }
//   } 

  /*
  cout<<Event->GetGalacticPointingZAxisLongitude()*c_Deg<<endl;
  if (fabs(Event->GetGalacticPointingZAxisLongitude()*c_Deg) > 60) {
    Return = false; 
  }
  */
  
//   bool UseSideD2 = true;
//   bool UseBottomD2 = true;
//   // Second hit has to be in bottom calorimeters:
//   MVector BottomCenter(0, 0, 6.3);
//   MVector BottomDim(9, 8, 6); //4);
//   if (Event->GetType() == MPhysicalEvent::c_Compton) {
//     MVector Pos = ((MComptonEvent *) Event)->C2();
//     Pos -= BottomCenter;
//     if (fabs(Pos[0]) - BottomDim[0] > 0 || fabs(Pos[1]) - BottomDim[1] > 0 || fabs(Pos[2]) - BottomDim[2] > 0) {
//       if (UseSideD2 == false) {
//         Pos = ((MComptonEvent *) Event)->C2();
//         //cout<<"Deselecting side calor: "<<Pos[0]<<"!"<<Pos[1]<<"!"<<Pos[2]<<endl;
//         Return = false;
//       } 
//    } else {
//       if (UseBottomD2 == false) {
//         Pos = ((MComptonEvent *) Event)->C2();
//         //cout<<"Deselecting bottom calor: "<<Pos[0]<<"!"<<Pos[1]<<"!"<<Pos[2]<<endl;
//         Return = false;
//       }
// //       if (!(Pos[0] > 0 && Pos[1] > 0)) { //aphrodite
// //         Return = false;
// //       }
//     }
//   }

  //   if (Event->GetPosition().Z() < 0) {
  //     m_NRejectedStartDetector++;
  //     Return = false;
  //   }


  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  // Only start in certain detectors - needs an active geometry
  if (m_Geometry != 0) {
    if (m_ExcludedDetectors.size() > 0) {
      if (Event->GetType() == MPhysicalEvent::c_Compton ||
          Event->GetType() == MPhysicalEvent::c_Pair ||
          Event->GetType() == MPhysicalEvent::c_Photo) {
        MDVolumeSequence V = m_Geometry->GetVolumeSequence(Event->GetPosition(), true, false);
        if (V.GetDetector() == 0) {
          cout<<"ID "<<Event->GetId()<<": You have a hit without detector! --> You probably selected the wrong geometry: "<<Event->GetPosition()<<endl;
        } else {
          for (unsigned int e = 0; e < m_ExcludedDetectors.size(); ++e) {
            if (V.GetDetector()->GetName() == m_ExcludedDetectors[e]) {
              if (DumpOutput == true) {
                cout<<"ID "<<Event->GetId()<<": Wrong start detector: "
                    <<m_ExcludedDetectors[e]<<endl;
              }
              m_NRejectedStartDetector++;
              Return = false;
              break;
            }
          }
        }
      }
    }
  }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION


  // First test everything common to pairs and comptons:
  if (!(Event->Ei() >= m_FirstTotalEnergyMin && 
        Event->Ei() <= m_FirstTotalEnergyMax) &&
      !(Event->Ei() >= m_SecondTotalEnergyMin && 
        Event->Ei() <= m_SecondTotalEnergyMax) &&
      !(Event->Ei() >= m_ThirdTotalEnergyMin && 
        Event->Ei() <= m_ThirdTotalEnergyMax) &&
      !(Event->Ei() >= m_FourthTotalEnergyMin && 
        Event->Ei() <= m_FourthTotalEnergyMax)) {
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Total energy out of range: "
          <<m_FirstTotalEnergyMin<<"<"<<Event->Ei()<<"<"<<m_FirstTotalEnergyMax<<" && "
          <<m_SecondTotalEnergyMin<<"<"<<Event->Ei()<<"<"<<m_SecondTotalEnergyMax<<" && "
          <<m_ThirdTotalEnergyMin<<"<"<<Event->Ei()<<"<"<<m_ThirdTotalEnergyMax<<" && "
          <<m_FourthTotalEnergyMin<<"<"<<Event->Ei()<<"<"<<m_FourthTotalEnergyMax<<endl;
    }
    m_NRejectedTotalEnergy++;
    Return = false;
  }
  
  if (m_TimeUseFile == true) {
    if (m_TimeGTI.IsGood(Event->GetTime()) == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Time not in GTI: "<<Event->GetTime().GetString()<<endl;
      }
      m_NRejectedTime++;
      Return = false;
    }
  } else{
    if (Event->GetTime() < m_TimeMin || Event->GetTime() > m_TimeMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Time out of range: "<<m_TimeMin<<"<"<<Event->GetTime().GetString()<<"<"<<m_TimeMax<<endl;
      }
      m_NRejectedTime++;
      Return = false;
    }
  }
  if (Event->GetTimeWalk() < m_TimeWalkMin || 
      Event->GetTimeWalk() > m_TimeWalkMax) {
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Time-walk out of range: "
          <<m_TimeWalkMin<<"<"<<Event->GetTimeWalk()<<"<"<<m_TimeWalkMax<<endl;
    }
    m_NRejectedTimeWalk++;
    Return = false;
  }
  if (Event->GetId() < m_EventIdMin || 
      Event->GetId() > m_EventIdMax) {
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Not within event ID selection: "
          <<m_EventIdMin<<"<"<<Event->GetId()<<"<"<<m_EventIdMax<<"!"<<endl;
    }      
    m_NRejectedEventId++;
    Return = false;
  }
  if (Event->IsDecay() == true && m_UseDecays == false) {
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Decay!"<<endl;
    }      
    m_NRejectedUseDecays++;
    Return = false;
  }
  if (Event->IsBad() == true && m_UseFlaggedAsBad == false) {
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": Flagged as bad!"<<endl;
    }      
    m_NRejectedUseFlaggedAsBad++;
    Return = false;
  }
  if (m_UseBeam == true) {
    if (Event->GetType() == MPhysicalEvent::c_Photo ||
        Event->GetType() == MPhysicalEvent::c_Compton || 
        Event->GetType() == MPhysicalEvent::c_Pair) {
      MVector Position = Event->GetPosition();
      // The same equations appear in MResponseMultipleComptonLens
      double Radius = (Position - m_BeamFocalSpot).Cross(m_BeamStart - m_BeamFocalSpot).Mag()/(m_BeamStart - m_BeamFocalSpot).Mag();
      double Depth = (Position - m_BeamFocalSpot).Dot(m_BeamFocalSpot - m_BeamStart)/(m_BeamFocalSpot - m_BeamStart).Mag();
      
      if (Radius > m_BeamRadius || Depth > m_BeamDepth) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Not within beam selection: "
              <<0<<"<"<<Radius<<"<"<<m_BeamRadius<<" or "<<0<<"<"<<Depth<<"<"<<m_BeamDepth<<endl;
        }      
        m_NRejectedBeam++;
        Return = false;
      }
    }
  }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  // Compton events:
  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    MComptonEvent *C = (MComptonEvent *) Event;
    
    // Deselect bottom calorimeters in MPE prototype
    //MVector Pos = C->C2();
    //if (fabs(Pos.X()) < 9.5 && fabs(Pos.Y()) < 9.5) {
    //  Return = false;
    //}

    if (C->IsKinematicsOK() == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Flag \"Not good\" due to kinematics!"<<endl;
      }
      // no counting here!
    }

    if (m_UseComptons == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted Compton"<<endl;
      }
      m_NRejectedUseComptons++;
      Return = false;
    }
    if (C->HasTrack() == true && m_UseTrackedComptons == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted tracked Compton"<<endl;
      }
      m_NRejectedUseTrackedComptons++;
      Return = false;
    }
    if (C->HasTrack() == false && m_UseNotTrackedComptons == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted not tracked Compton"<<endl;
      }      
      m_NRejectedUseNotTrackedComptons++;
      Return = false;
    }
    if (C->Eg() < m_GammaEnergyMin || C->Eg() > m_GammaEnergyMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Gamma energy out of range: "
            <<m_GammaEnergyMin<<"<"<<C->Eg()<<"<"<<m_GammaEnergyMax<<endl;
      }
      m_NRejectedGammaEnergy++;
      Return = false;
    }
    if (C->Ee() < m_ElectronEnergyMin || C->Ee() > m_ElectronEnergyMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Electron energy out of range: "
            <<m_ElectronEnergyMin<<"<"<<C->Ee()<<"<"<<m_ElectronEnergyMax<<endl;
      }
      m_NRejectedElectronEnergy++;
      Return = false;
    }
    if (C->Phi()*c_Deg < m_ComptonAngleMin || 
        C->Phi()*c_Deg > m_ComptonAngleMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Compton angle out of range: "
            <<m_ComptonAngleMin<<"<"<<C->Phi()*c_Deg<<"<"
            <<m_ComptonAngleMax<<endl;
      }      
      m_NRejectedComptonAngle++;
      Return = false;
    }
    if (C->LeverArm() < m_LeverArmMin || 
        C->LeverArm() > m_LeverArmMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within lever arm selection: "
            <<C->LeverArm()<<" cm!"<<endl;
      }      
      m_NRejectedLeverArm++;
      Return = false;
    }
    if ((C->C1() - C->C2()).Mag() < m_FirstLeverArmMin || 
        (C->C1() - C->C2()).Mag() > m_FirstLeverArmMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within first lever arm selection: "
            <<(C->C1() - C->C2()).Mag()<<" cm!"<<endl;
      }      
      m_NRejectedFirstLeverArm++;
      Return = false;
    }
    if (C->SequenceLength() < m_SequenceLengthMin || 
        C->SequenceLength() > m_SequenceLengthMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within Compton sequence length selection: "
            <<m_SequenceLengthMin<<" !< "<<C->SequenceLength()<<" !< "<<m_SequenceLengthMax<<" Compton interactions!"<<endl;
      }      
      m_NRejectedSequenceLength++;
      Return = false;
    }
    if (C->TrackLength() < m_TrackLengthMin || 
        C->TrackLength() > m_TrackLengthMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within Track sequence length selection: "
            <<m_TrackLengthMin<<" !< "<<C->TrackLength()<<" !< "<<m_TrackLengthMax<<" Compton interactions!"<<endl;
      }      
      m_NRejectedTrackLength++;
      Return = false;
    }
    if (C->ClusteringQualityFactor() < m_ClusteringQualityFactorMin ||
        C->ClusteringQualityFactor() > m_ClusteringQualityFactorMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within Clustering quality factor selection: "
            <<m_ClusteringQualityFactorMin<<"<"<<C->ClusteringQualityFactor()<<"<"<<m_ClusteringQualityFactorMax<<endl;
      }      
      m_NRejectedClusteringQualityFactor++;
      Return = false;
    }
    if (C->ComptonQualityFactor1() < m_ComptonQualityFactorMin ||
        C->ComptonQualityFactor1() > m_ComptonQualityFactorMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within Compton quality factor selection: "
            <<m_ComptonQualityFactorMin<<"<"<<C->ComptonQualityFactor1()<<"<"<<m_ComptonQualityFactorMax<<endl;
      }      
      m_NRejectedComptonQualityFactor++;
      Return = false;
    }
    if (C->TrackQualityFactor1() < m_TrackQualityFactorMin || 
        C->TrackQualityFactor1() > m_TrackQualityFactorMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within track quality factor selection: "
            <<m_TrackQualityFactorMin<<"<"<<C->TrackQualityFactor1()<<"<"<<m_TrackQualityFactorMax<<endl;
      }      
      m_NRejectedTrackQualityFactor++;
      Return = false;
    }

    if (C->CoincidenceWindow() < m_CoincidenceWindowMin || 
        C->CoincidenceWindow() > m_CoincidenceWindowMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Not within coincidence window selection: "
            <<m_CoincidenceWindowMin<<"<"<<C->CoincidenceWindow()<<"<"<<m_CoincidenceWindowMax<<endl;
      }      
      m_NRejectedCoincidenceWindow++;
      Return = false;
    }

    if (C->TrackLength() > 1) {
      if (fabs(C->DeltaTheta())*c_Deg > m_ThetaDeviationMax) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Theta difference too large: "
              <<fabs(C->DeltaTheta())*c_Deg<<" > "<<m_ThetaDeviationMax<<endl;
        }
        m_NRejectedThetaDeviationMax++;
        Return = false;
      }
    }

    if (Event->IsGoodEvent() == true) {
      if (m_EarthHorizon.IsEventFromEarth(Event, DumpOutput) == true) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Not within earth horizon cut "<<endl;
        }      
        m_NRejectedEarthHorizonCut++;
        Return = false;
      }
    }

    if (m_UseSource == true) {
      //m_SourcePosition.SetMagThetaPhi(c_FarAway, (-5.78 + 90)*c_Rad, (184.56)*c_Rad);

      if (fabs(C->GetARMGamma(m_SourcePosition)*c_Deg) < m_ARMMin || 
          fabs(C->GetARMGamma(m_SourcePosition)*c_Deg) > m_ARMMax) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Not within ARM cut around "<<m_SourcePosition<<": "
              <<m_ARMMin<<" < "<<fabs(C->GetARMGamma(m_SourcePosition))*c_Deg<<" < "<<m_ARMMax<<endl;
        }      
        m_NRejectedARM++;
        Return = false;
      }
      if (C->TrackLength() > 1) {
        if (fabs(C->GetSPDElectron(m_SourcePosition))*c_Deg < m_SPDMin || 
            fabs(C->GetSPDElectron(m_SourcePosition))*c_Deg > m_SPDMax) {
          if (DumpOutput == true) {
            cout<<"ID "<<Event->GetId()<<": Not within SPD cut around "<<m_SourcePosition<<": "
                <<m_SPDMax<<" < "<<fabs(C->GetSPDElectron(m_SourcePosition))*c_Deg<<" < "<<m_SPDMax<<endl;
          }      
          m_NRejectedSPD++;
          Return = false;
        }
      }
    }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    MPairEvent* Pair = (MPairEvent*) Event;

    if (m_UsePairs == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted Pair!"<<endl;
      }
      m_NRejectedUsePairs++;
      Return = false;
    }
    if (Pair->GetOpeningAngle()*c_Deg < m_OpeningAnglePairMin || 
        Pair->GetOpeningAngle()*c_Deg > m_OpeningAnglePairMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()
            <<": Not within opening angle selection selection: "
            <<Pair->GetOpeningAngle()*c_Deg<<" deg!"<<endl;
      }
      m_NRejectedOpeningAnglePair++;
      Return = false;
    }
    if (Pair->GetInitialEnergyDeposit() < m_InitialEnergyDepositPairMin || 
        Pair->GetInitialEnergyDeposit() > m_InitialEnergyDepositPairMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()
            <<": Not within initial energy deposit selection: "
            <<Pair->GetInitialEnergyDeposit()<<" keV!"<<endl;
      }
      m_NRejectedInitialEnergyDepositPair++;
      Return = false;
    }
    if (Pair->GetTrackQualityFactor() < m_PairQualityFactorMin || 
        Pair->GetTrackQualityFactor() > m_PairQualityFactorMax) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()
            <<": Not within pair quality factor: "
            <<Pair->GetTrackQualityFactor()<<"!"<<endl;
      }
      m_NRejectedPairQualityFactor++;
      Return = false;
    }
    if (Event->IsGoodEvent() == true) {
      if (m_EarthHorizon.IsEventFromEarth(Event, DumpOutput) == true) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Not within earth horizon cut "<<endl;
        }      
        m_NRejectedEarthHorizonCut++;
        Return = false;
      }
    }
    if (m_UseSource == true) {
      if (fabs(Pair->GetARMGamma(m_SourcePosition)*c_Deg) < m_ARMMin || 
          fabs(Pair->GetARMGamma(m_SourcePosition)*c_Deg) > m_ARMMax) {
        if (DumpOutput == true) {
          cout<<"ID "<<Event->GetId()<<": Not within ARM cut around "<<m_SourcePosition<<": "
              <<m_ARMMin<<" < "<<fabs(Pair->GetARMGamma(m_SourcePosition))<<" < "<<m_ARMMax<<endl;
        }      
        m_NRejectedARM++;
        Return = false;
      }
    }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
    //MPhotoEvent* Photo = (MPhotoEvent*) Event;

    if (m_UsePhotos == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted photoeffect event!"<<endl;
      }
      m_NRejectedUsePhotos++;
      Return = false;
    }
  } else if (Event->GetType() == MPhysicalEvent::c_Muon) {
    m_NRejectedUseMuons++;
    Return = false;
  } else if (Event->GetType() == MPhysicalEvent::c_Decay) {
    m_NRejectedUseDecays++;
    Return = false;
  } else if (Event->GetType() == MPhysicalEvent::c_Unidentifiable) {
    if (m_UseUnidentifiables == false) {
      if (DumpOutput == true) {
        cout<<"ID "<<Event->GetId()<<": Unwanted unidentifiable event!"<<endl;
      }
      m_NRejectedUseUnidentifiables++;
      Return = false;
    }
  } else {
    cout<<"Unknown event type: "<<Event->GetType()<<endl;
    Return = false;
  }

  if (Return == true) {
    m_NAccepted++;
    if (DumpOutput == true) {
      cout<<"ID "<<Event->GetId()<<": ok              <-------------------"<<endl;
    }
  }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MEventSelector::IsQualifiedEventFast(MPhysicalEvent* Event)
{
  //! The same as IsQualifiedEvent, it only immdiately return false if one clause is wrong
  //! In addition the test sequence is more optimally arranged

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  if (Event->IsGoodEvent() == false) {
    return false;
  }


  // First test everything common to pairs and comptons:
  if (!(Event->Ei() >= m_FirstTotalEnergyMin && 
        Event->Ei() <= m_FirstTotalEnergyMax) &&
      !(Event->Ei() >= m_SecondTotalEnergyMin && 
        Event->Ei() <= m_SecondTotalEnergyMax) &&
      !(Event->Ei() >= m_ThirdTotalEnergyMin && 
        Event->Ei() <= m_ThirdTotalEnergyMax) &&
      !(Event->Ei() >= m_FourthTotalEnergyMin && 
        Event->Ei() <= m_FourthTotalEnergyMax)) {
    return false;
  }

  // Compton events:
  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    MComptonEvent *C = (MComptonEvent *) Event;
    

    if (C->IsKinematicsOK() == false) {
      // no counting here!
    }

    if (m_UseComptons == false) {
      return false;
    }
    if (C->HasTrack() == true && m_UseTrackedComptons == false) {
      return false;
    }
    if (C->HasTrack() == false && m_UseNotTrackedComptons == false) {
      return false;
    }
    if (C->Phi()*c_Deg < m_ComptonAngleMin || 
        C->Phi()*c_Deg > m_ComptonAngleMax) {
      return false;
    }
    if (C->Eg() < m_GammaEnergyMin || C->Eg() > m_GammaEnergyMax) {
      return false;
    }
    if (C->Ee() < m_ElectronEnergyMin || C->Ee() > m_ElectronEnergyMax) {
      return false;
    }
    if (C->LeverArm() < m_LeverArmMin || 
        C->LeverArm() > m_LeverArmMax) {
      return false;
    }
    if ((C->C1() - C->C2()).Mag() < m_FirstLeverArmMin || 
        (C->C1() - C->C2()).Mag() > m_FirstLeverArmMax) {
      return false;
    }
    if (C->SequenceLength() < m_SequenceLengthMin || 
        C->SequenceLength() > m_SequenceLengthMax) {
      return false;
    }
    if (C->TrackLength() < m_TrackLengthMin || 
        C->TrackLength() > m_TrackLengthMax) {
      return false;
    }
    if (C->ClusteringQualityFactor() < m_ClusteringQualityFactorMin ||
        C->ClusteringQualityFactor() > m_ClusteringQualityFactorMax) {
      return false;
    }
    if (C->ComptonQualityFactor1() < m_ComptonQualityFactorMin ||
        C->ComptonQualityFactor1() > m_ComptonQualityFactorMax) {
      return false;
    }
    if (C->TrackQualityFactor1() < m_TrackQualityFactorMin || 
        C->TrackQualityFactor1() > m_TrackQualityFactorMax) {
      return false;
    }

    if (C->CoincidenceWindow() < m_CoincidenceWindowMin || 
        C->CoincidenceWindow() > m_CoincidenceWindowMax) {
      return false;
    }

    if (C->TrackLength() > 1) {
      if (fabs(C->DeltaTheta())*c_Deg > m_ThetaDeviationMax) {
        return false;
      }
    }

    if (Event->IsGoodEvent() == true) {
      if (m_EarthHorizon.IsEventFromEarth(Event) == true) {
        return false;
      }
    }

    if (m_UseSource == true) {
      if (fabs(C->GetARMGamma(m_SourcePosition)*c_Deg) < m_ARMMin || 
          fabs(C->GetARMGamma(m_SourcePosition)*c_Deg) > m_ARMMax) {
        return false;
      }
      if (C->TrackLength() > 1) {
        if (fabs(C->GetSPDElectron(m_SourcePosition))*c_Deg < m_SPDMin || 
            fabs(C->GetSPDElectron(m_SourcePosition))*c_Deg > m_SPDMax) {
          return false;
        }
      }
    }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    MPairEvent* Pair = (MPairEvent*) Event;

    if (m_UsePairs == false) {
      return false;
    }
    if (Pair->GetOpeningAngle()*c_Deg < m_OpeningAnglePairMin || 
        Pair->GetOpeningAngle()*c_Deg > m_OpeningAnglePairMax) {
      return false;
    }
    if (Pair->GetInitialEnergyDeposit() < m_InitialEnergyDepositPairMin || 
        Pair->GetInitialEnergyDeposit() > m_InitialEnergyDepositPairMax) {
      return false;
    }
    if (Pair->GetTrackQualityFactor() < m_PairQualityFactorMin || 
        Pair->GetTrackQualityFactor() > m_PairQualityFactorMax) {
      return false;
    }
    if (Event->IsGoodEvent() == true) {
      if (m_EarthHorizon.IsEventFromEarth(Event) == true) {
        return false;
      }
    }
    if (m_UseSource == true) {
      if (fabs(Pair->GetARMGamma(m_SourcePosition)*c_Deg) < m_ARMMin || 
          fabs(Pair->GetARMGamma(m_SourcePosition)*c_Deg) > m_ARMMax) {
        return false;
      }
    }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  } else if (Event->GetType() == MPhysicalEvent::c_Photo) {
    //MPhotoEvent* Photo = (MPhotoEvent*) Event;

    if (m_UsePhotos == false) {
      return false;
    }
  } else if (Event->GetType() == MPhysicalEvent::c_Muon) {
    return false;
  } else if (Event->GetType() == MPhysicalEvent::c_Decay) {
    return false;
  } else if (Event->GetType() == MPhysicalEvent::c_Unidentifiable) {
    return false;
  } else {
    cout<<"Unknown event type: "<<Event->GetType()<<endl;
    return false;
  }

  if (m_TimeUseFile == true) {
    if (m_TimeGTI.IsGood(Event->GetTime()) == false) {
      return false; 
    }
  } else {
    if (Event->GetTime() < m_TimeMin || Event->GetTime() > m_TimeMax) {
      return false;
    }
  }
  if (Event->GetTimeWalk() < m_TimeWalkMin || 
      Event->GetTimeWalk() > m_TimeWalkMax) {
    return false;
  }
  if (Event->GetId() < m_EventIdMin || 
      Event->GetId() > m_EventIdMax) {
    return false;
  }
  if (Event->IsDecay() == true && m_UseDecays == false) {
    return false;
  }
  if (Event->IsBad() == true && m_UseFlaggedAsBad == false) {
    return false;
  }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  if (m_UseBeam == true) {
    if (Event->GetType() == MPhysicalEvent::c_Photo ||
        Event->GetType() == MPhysicalEvent::c_Compton || 
        Event->GetType() == MPhysicalEvent::c_Pair) {
      MVector Position = Event->GetPosition();
      // The same equations appear in MResponseMultipleComptonLens
      double Radius = (Position - m_BeamFocalSpot).Cross(m_BeamStart - m_BeamFocalSpot).Mag()/(m_BeamStart - m_BeamFocalSpot).Mag();
      double Depth = (Position - m_BeamFocalSpot).Dot(m_BeamFocalSpot - m_BeamStart)/(m_BeamFocalSpot - m_BeamStart).Mag();
      
      if (Radius > m_BeamRadius || Depth > m_BeamDepth) {
        return false;
      }
    }
  }

  // Only start in certain detectors - needs an active geometry
  if (m_Geometry != 0) {
    if (m_ExcludedDetectors.size() > 0) {
      if (Event->GetType() == MPhysicalEvent::c_Compton ||
          Event->GetType() == MPhysicalEvent::c_Pair ||
          Event->GetType() == MPhysicalEvent::c_Photo) {
        MDVolumeSequence V = m_Geometry->GetVolumeSequence(Event->GetPosition(), true, false);
        if (V.GetDetector() == 0) {
          cout<<"ID "<<Event->GetId()<<": You have a hit without detector! --> You probably selected the wrong geometry: "<<Event->GetPosition()<<endl;
        } else {
          for (unsigned int e = 0; e < m_ExcludedDetectors.size(); ++e) {
            if (V.GetDetector()->GetName() == m_ExcludedDetectors[e]) {
              return false;
            }
          }
        }
      }
    }
  }

  // ATTENTION: PUT ALL CHANGES HERE INTO BOTH (FAST & DETAILED) VERSION OF THIS FUNCTION

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MEventSelector::IsDirectionWithinARMWindow(MVector Direction)
{
  // Just do a little test if the direction is within a selected ARM window
  // This has nothing to do with the current event, just for testing of the 
  // event selection... (required by SensitivityOptimizer)

  if (m_UseSource == true) {
    double Angle = m_SourcePosition.Angle(Direction);
    if (Angle*c_Deg >= m_ARMMin && Angle*c_Deg <= m_ARMMax) {
      return true;
    }
  }

  return false;
}



////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetGeometry(MDGeometryQuest* Geometry)
{
  // Set the range of

  m_Geometry = Geometry;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetExcludedDetectors(vector<MString> ExcludedDetectors)
{
  // Set the detectors in which the events are not allowed to start:
  
  m_ExcludedDetectors = ExcludedDetectors;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSourceWindow(bool Use, MVector SourcePosition)
{
  m_UseSource = Use;
  m_SourcePosition = SourcePosition;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSourceARM(double ARMMin, double ARMMax)
{
  m_ARMMin = ARMMin;
  m_ARMMax = ARMMax;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSourceSPD(double SPDMin, double SPDMax)
{
  m_SPDMin = SPDMin;
  m_SPDMax = SPDMax;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetBeam(bool Use, MVector BeamStart, MVector BeamFocalSpot)
{
  m_UseBeam = Use;
  m_BeamStart = BeamStart;
  m_BeamFocalSpot = BeamFocalSpot;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetBeamRadius(double Radius)
{
  m_BeamRadius = Radius;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetBeamDepth(double Depth)
{
  m_BeamDepth = Depth;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetFirstTotalEnergy(double Min, double Max)
{
  // Set the range of

  m_FirstTotalEnergyMin = Min;
  m_FirstTotalEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSecondTotalEnergy(double Min, double Max)
{
  // Set the range of

  m_SecondTotalEnergyMin = Min;
  m_SecondTotalEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetThirdTotalEnergy(double Min, double Max)
{
  // Set the range of

  m_ThirdTotalEnergyMin = Min;
  m_ThirdTotalEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetFourthTotalEnergy(double Min, double Max)
{
  // Set the range of

  m_FourthTotalEnergyMin = Min;
  m_FourthTotalEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTimeUseFile(bool TimeUseFile)
{
  m_TimeUseFile = TimeUseFile;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTimeFile(MString TimeFile)
{
  m_TimeFile = TimeFile;
  
  m_TimeGTI.Reset(true);
  
  if (MFile::Exists(m_TimeFile) == true) {
    if (m_TimeGTI.Load(m_TimeFile) == false) {
      cout<<"Error: Unable to load GTI file! Using all open one!"<<endl;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTime(const MTime& Min, const MTime& Max)
{
  // Set the range of time

  m_TimeMin = Min;
  m_TimeMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTimeWalk(double Min, double Max)
{
  // Set the range of time

  m_TimeWalkMin = Min;
  m_TimeWalkMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseComptons(bool Comptons)
{
  // Pass through/Neglect compton events

  m_UseComptons = Comptons;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseTrackedComptons(bool Comptons)
{
  // Pass through/Neglect tracked compton events

  m_UseTrackedComptons = Comptons;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseNotTrackedComptons(bool Comptons)
{
  // Pass through/Neglect not tracked compton events

  m_UseNotTrackedComptons = Comptons;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetElectronEnergy(double Min, double Max)
{
  // Set the range of electron energy

  m_ElectronEnergyMin = Min;
  m_ElectronEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetGammaEnergy(double Min, double Max)
{
  // Set the range of gamma energy

  m_GammaEnergyMin = Min;
  m_GammaEnergyMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetComptonAngle(double Min, double Max)
{
  // Set the range of

  m_ComptonAngleMin = Min;
  m_ComptonAngleMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetOpeningAnglePair(double Min, double Max)
{
  // Set the range of

  m_OpeningAnglePairMin = Min;
  m_OpeningAnglePairMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetEarthHorizonCut(const MEarthHorizon& EH)
{
  // Set the range of

  m_EarthHorizon = EH;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetEarthHorizonCutAngle(double Angle) 
{ 
  m_EarthHorizon.SetEarthHorizon(MVector(0, 0, -1E20), Angle*c_Rad); 
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetThetaDeviationMax(double Min)
{
  // Set the range of

  m_ThetaDeviationMax = Min;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetInitialEnergyDepositPair(double Min, double Max)
{
  // Set the range of

  m_InitialEnergyDepositPairMin = Min;
  m_InitialEnergyDepositPairMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetPairQualityFactor(double Min, double Max)
{
  // Set the range of

  m_PairQualityFactorMin = Min;
  m_PairQualityFactorMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetDistance(double Min, double Max)
{
  // Set the range of

  m_LeverArmMin = Min;
  m_LeverArmMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetFirstDistance(double Min, double Max)
{
  // Set the range of

  m_FirstLeverArmMin = Min;
  m_FirstLeverArmMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetSequenceLength(int Min, int Max)
{
  // Set the range of

  m_SequenceLengthMin = Min;
  m_SequenceLengthMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTrackLength(int Min, int Max)
{
  // Set the range of

  m_TrackLengthMin = Min;
  m_TrackLengthMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetClusteringQualityFactor(double Min, double Max)
{
  // Set the range of

  m_ClusteringQualityFactorMin = Min;
  m_ClusteringQualityFactorMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetComptonQualityFactor(double Min, double Max)
{
  // Set the range of

  m_ComptonQualityFactorMin = Min;
  m_ComptonQualityFactorMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetTrackQualityFactor(double Min, double Max)
{
  // Set the range of

  m_TrackQualityFactorMin = Min;
  m_TrackQualityFactorMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetCoincidenceWindow(double Min, double Max)
{
  // Set the range of

  m_CoincidenceWindowMin = Min;
  m_CoincidenceWindowMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::SetEventId(int Min, int Max)
{
  // Set the range of

  m_EventIdMin = Min;
  m_EventIdMax = Max;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UsePairs(bool Pairs)
{
  // Pass through/Neglect pairs

  m_UsePairs = Pairs;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UsePhotos(bool Photos)
{
  // Pass through/Neglect photos

  m_UsePhotos = Photos;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseUnidentifiables(bool Unidentifiables)
{
  // Pass through/Neglect photos

  m_UseUnidentifiables = Unidentifiables;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseDecays(bool Decays)
{
  // Pass through/Neglect decays

  m_UseDecays = Decays;
}


////////////////////////////////////////////////////////////////////////////////


void MEventSelector::UseFlaggedAsBad(bool FlaggedAsBad)
{
  // Pass through/Neglect events flagged as bad

  m_UseFlaggedAsBad = FlaggedAsBad;
}


////////////////////////////////////////////////////////////////////////////////


MString MEventSelector::ToString()
{
  ostringstream s;

  s<<"Rejection reasons:"<<endl;
  s<<endl;
  s<<"Not good  ......................  "
   <<m_NRejectedIsGood<<endl;
  s<<"Event Id .......................  "
   <<m_NRejectedEventId<<endl;
  s<<"Start detector .................  "
   <<m_NRejectedStartDetector<<endl;
  s<<"Beam  ..........................  "
   <<m_NRejectedBeam<<endl;
  s<<"Total energy  ..................  "
   <<m_NRejectedTotalEnergy<<endl;
  s<<"Time  ..........................  "
   <<m_NRejectedTime<<endl;
  s<<"Time walk  .....................  "
   <<m_NRejectedTimeWalk<<endl;
  s<<"Electron energy  ...............  "
   <<m_NRejectedElectronEnergy<<endl;
  s<<"Gamma energy  ..................  "
   <<m_NRejectedGammaEnergy<<endl;
  s<<"Compton angle  .................  "
   <<m_NRejectedComptonAngle<<endl;
  s<<"First Lever arm  ...............  "
   <<m_NRejectedFirstLeverArm<<endl;
  s<<"Any lever arm  .................  "
   <<m_NRejectedLeverArm<<endl;
  s<<"Length Compton sequence ........  "
   <<m_NRejectedSequenceLength<<endl;
  s<<"Clustering quality factor ......  "
   <<m_NRejectedClusteringQualityFactor<<endl;
  s<<"Compton quality factor .........  "
   <<m_NRejectedComptonQualityFactor<<endl;
  s<<"Track quality factor ...........  "
   <<m_NRejectedTrackQualityFactor<<endl;
  s<<"Coincidence window .............  "
   <<m_NRejectedCoincidenceWindow<<endl;
  s<<"Earth-Horizon cut ..............  "
   <<m_NRejectedEarthHorizonCut<<endl;
  s<<"Max. theta deviation ...........  "
   <<m_NRejectedThetaDeviationMax<<endl;
  s<<"Max. ARM .......................  "
   <<m_NRejectedARM<<endl;
  s<<"Max. SPD .......................  "
   <<m_NRejectedSPD<<endl;
  s<<"Length track ...................  "
   <<m_NRejectedTrackLength<<endl;
  s<<"Opening angle pair  ............  "
   <<m_NRejectedOpeningAnglePair<<endl;
  s<<"Initial energy deposit pair  ...  "
   <<m_NRejectedInitialEnergyDepositPair<<endl;
  s<<"Pair quality factor ............  "
   <<m_NRejectedPairQualityFactor<<endl;
  s<<"Use photos  ....................  "
   <<m_NRejectedUsePhotos<<endl;
  s<<"Use pairs  .....................  "
   <<m_NRejectedUsePairs<<endl;
  s<<"Use Compton  ...................  "
   <<m_NRejectedUseComptons<<endl;
  s<<"Use tracked Compton  ...........  "
   <<m_NRejectedUseTrackedComptons<<endl;
  s<<"Use not tracked Compton  .......  "
   <<m_NRejectedUseNotTrackedComptons<<endl;
  s<<"Use muons  .....................  "
   <<m_NRejectedUseMuons<<endl;
  s<<"Use unidentifiables  ...........  "
   <<m_NRejectedUseUnidentifiables<<endl;
  s<<"Use decays  ....................  "
   <<m_NRejectedUseDecays<<endl;
  s<<"Use flagged as bad  ............  "
   <<m_NRejectedUseFlaggedAsBad<<endl;
  s<<endl;
  s<<"ACCEPTED  ......................  "
   <<m_NAccepted<<endl;

  return s.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, MEventSelector& S)
{
  os<<"Use photo effect events:          "
    <<((S.m_UsePhotos == true) ? "true" : "false")<<endl;
  os<<"Use Compton scattering events:    "
    <<((S.m_UseComptons == true) ? "true" : "false")<<endl;
  os<<"Use tracked Compton:              "
    <<((S.m_UseTrackedComptons == true) ? "true" : "false")<<endl;
  os<<"Use not tracked Compton:          "
    <<((S.m_UseNotTrackedComptons == true) ? "true" : "false")<<endl;
  os<<"Use pair creation events:         "
    <<((S.m_UsePairs == true) ? "true" : "false")<<endl;
  os<<"Use decays:                       "
    <<((S.m_UseDecays == true) ? "true" : "false")<<endl;
  os<<"Use events flagged as bad:        "
    <<((S.m_UseFlaggedAsBad == true) ? "true" : "false")<<endl;

  if (S.m_Geometry != 0) {
    os<<"Not allowed start detectors:      ";
    for (unsigned int e = 0; e < S.m_ExcludedDetectors.size(); ++e) {
      os<<S.m_ExcludedDetectors[e]<<"  ";
    }
    os<<"from geometry "<<S.m_Geometry->GetName()<<endl;
    os<<endl;
  }

  os<<"Beam radius:                      "
    <<S.m_BeamRadius<<endl;
  os<<"Beam depth:                       "
    <<S.m_BeamDepth<<endl;
  os<<"First energy window:              "
    <<S.m_FirstTotalEnergyMin<<"-"<<S.m_FirstTotalEnergyMax<<endl;
  os<<"Second energy window:             "
    <<S.m_SecondTotalEnergyMin<<"-"<<S.m_SecondTotalEnergyMax<<endl;
  os<<"Time window:                      "
    <<S.m_TimeMin<<"-"<<S.m_TimeMax<<endl;
  os<<"Time walk window:                 "
    <<S.m_TimeWalkMin<<"-"<<S.m_TimeWalkMax<<endl;
  os<<"Recoil electron energy window:    "
    <<S.m_ElectronEnergyMin<<"-"<<S.m_ElectronEnergyMax<<endl;
  os<<"Scatter gamma-ray energy window:  "
    <<S.m_GammaEnergyMin<<"-"<<S.m_GammaEnergyMax<<endl;
  os<<"Compton scatter angle:            "
    <<S.m_ComptonAngleMin<<"-"<<S.m_ComptonAngleMax<<endl;
  os<<"Any lever arm:                    "
    <<S.m_LeverArmMin<<"-"<<S.m_LeverArmMax<<endl;
  os<<"First lever arm:                  "
    <<S.m_FirstLeverArmMin<<"-"<<S.m_FirstLeverArmMax<<endl;
  os<<"Compton squence length:           "
    <<S.m_SequenceLengthMin<<"-"<<S.m_SequenceLengthMax<<endl;
  os<<"First track's length:             "
    <<S.m_TrackLengthMin<<"-"<<S.m_TrackLengthMax<<endl;
  os<<"Clustering quality factor:        "
    <<S.m_ClusteringQualityFactorMin<<"-"<<S.m_ClusteringQualityFactorMax<<endl;
  os<<"Compton quality factor:           "
    <<S.m_ComptonQualityFactorMin<<"-"<<S.m_ComptonQualityFactorMax<<endl;
  os<<"Track quality factor:             "
    <<S.m_TrackQualityFactorMin<<"-"<<S.m_TrackQualityFactorMax<<endl;
  os<<"Coincidence window:               "
    <<S.m_CoincidenceWindowMin<<"-"<<S.m_CoincidenceWindowMax<<endl;
  os<<"Event ID:                         "
    <<S.m_EventIdMin<<"-"<<S.m_EventIdMax<<endl;
  os<<"Earth horizon cut:                "
    <<S.m_EarthHorizon<<endl;
  os<<"Max. theta deviation              "
    <<S.m_ThetaDeviationMax<<endl;
  os<<"Max. ARM                          "
    <<S.m_ARMMax<<endl;
  os<<"Max. SPD                          "
    <<S.m_SPDMax<<endl;
  os<<"Initial energy deposit pair:      "
    <<S.m_InitialEnergyDepositPairMin<<"-"<<S.m_InitialEnergyDepositPairMax<<endl;
  os<<"Opening angle pair:               "
    <<S.m_OpeningAnglePairMin<<"-"<<S.m_OpeningAnglePairMax<<endl;
  os<<"Pair quality factor:              "
    <<S.m_PairQualityFactorMin<<"-"<<S.m_PairQualityFactorMax<<endl;

  return os;
}

// MEventSelector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
