/*
 * MERCSRChiSquare.cxx
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
// MERCSRChiSquare
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRChiSquare.h"

// Standard libs:
#include <cmath>
#include <limits>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCSRChiSquare)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MERCSRChiSquare::c_TSUnknown           = -1;
const int MERCSRChiSquare::c_TSSimple            = 0;
const int MERCSRChiSquare::c_TSSimpleWithErrors  = 1;
const int MERCSRChiSquare::c_TSChiSquare         = 2;

const int MERCSRChiSquare::c_TSFirst             = c_TSSimple;
const int MERCSRChiSquare::c_TSLast              = c_TSChiSquare;

const int MERCSRChiSquare::c_UndecidedIgnore                        = 0;
const int MERCSRChiSquare::c_UndecidedAssumestartD1                 = 1;
const int MERCSRChiSquare::c_UndecidedLargerKleinNishina            = 2;
const int MERCSRChiSquare::c_UndecidedLargerKleinNishinaTimesPhoto  = 3;
const int MERCSRChiSquare::c_UndecidedLargerEnergyDeposit           = 4;


////////////////////////////////////////////////////////////////////////////////


MERCSRChiSquare::MERCSRChiSquare() : MERCSR(), m_UseComptelTypeEvents(false), m_RejectOneDetectorTypeOnlyEvents(false), m_UndecidedHandling(0), m_AssumeD1First(false), m_TypeTestStatistics(c_TSSimple)
{
  // Construct an instance of MERCSRChiSquare
}


////////////////////////////////////////////////////////////////////////////////


MERCSRChiSquare::~MERCSRChiSquare()
{
  // Delete this instance of MERCSRChiSquare
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRChiSquare::SetParameters(MGeometryRevan* Geometry, 
                                    double QualityFactorMin, 
                                    double QualityFactorMax, 
                                    int MaxNInteractions,
                                    bool GuaranteeStartD1,
                                    bool CreateOnlyPermutations,
                                    bool UseComptelTypeEvents, 
                                    int UndecidedHandling,
                                    bool RejectOneDetectorTypeOnlyEvents,
                                    MGeometryRevan* OriginObjects)
{
  if (MERCSR::SetParameters(Geometry, QualityFactorMin, QualityFactorMax, MaxNInteractions, GuaranteeStartD1, CreateOnlyPermutations) == false) return false;
  
  m_UseComptelTypeEvents = UseComptelTypeEvents;
  m_UndecidedHandling = UndecidedHandling;
  m_RejectOneDetectorTypeOnlyEvents = RejectOneDetectorTypeOnlyEvents;
  m_OriginObjects = OriginObjects;
  
  if (Geometry->AreCrossSectionsPresent() == false) {
    if (m_UndecidedHandling == c_UndecidedLargerKleinNishinaTimesPhoto) {
      mgui<<"Analyzing undecided events via Klein-Nishina and photo effect"<<endl;
      mgui<<"with the classic event reconstruction requires absorption probabilities in Geomega."<<endl;
      mgui<<"However, Geomega indicates that those have not been proberly loaded."<<endl;
      mgui<<"Aborting analysis"<<error;
      return false;
    }
  }
  
  mdebug<<"UseComptelTypeEvents: "<<((m_UseComptelTypeEvents == true) ? "true" : "false")<<endl;
  mdebug<<"AssumeD1First: "<<((m_AssumeD1First == true) ? "true" : "false")<<endl;
  mdebug<<"RejectOneDetectorTypeOnlyEvents: "<<((m_RejectOneDetectorTypeOnlyEvents == true) ? "true" : "false")<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRChiSquare::FindComptonSequenceDualHitEvent(MRERawEvent* RE)
{
  // This method takes care of events with only two "hits": one in the Tracker,
  // one in the calorimeter und tries to find out, which hit was first...
  
  // Theta limits are handled im mimrec --- not here!
  const double LimitTheta = 180.0*c_Rad;
  
  double QualityFactor1 = -1.0;
  double QualityFactor2 = -1.0;
  
  if (RE->GetNRESEs() != 2) {
    merr<<"FindComptonSequenceDualHitEvent called with "<<RE->GetNRESEs()<<" hits instead of exactly 2!"<<endl;
    RE->SetEventType(MRERawEvent::c_UnknownEvent);
    RE->SetEventReconstructed(true);
    return;
  }
  
  mdebug<<"CSR-CS - Dual hit analysis"<<endl;
  
  if (m_RejectOneDetectorTypeOnlyEvents == true) {
    // Reject D1 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 1 && RE->GetRESEAt(1)->GetDetector() == 1) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD1Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D1 only event"<<endl;
      return;
    }
    // Reject D2 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 2 && RE->GetRESEAt(1)->GetDetector() == 2) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD2Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D2 only event"<<endl;
      return;
    }
    // Reject D3 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 3 && RE->GetRESEAt(1)->GetDetector() == 3) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD3Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D3 only event"<<endl;
      return;
    }
    // Reject D4 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 4 && RE->GetRESEAt(1)->GetDetector() == 4) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD4Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D4 only event"<<endl;
      return;
    }
    // Reject D5 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 5 && RE->GetRESEAt(1)->GetDetector() == 5) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD5Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D5 only event"<<endl;
      return;
    }
    // Reject D6 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 6 && RE->GetRESEAt(1)->GetDetector() == 6) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD6Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D6 only event"<<endl;
      return;
    }
    // Reject D7 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 7 && RE->GetRESEAt(1)->GetDetector() == 7) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD7Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D7 only event"<<endl;
      return;
    }
    // Reject D8 only events:
    if (RE->GetRESEAt(0)->GetDetector() == 8 && RE->GetRESEAt(1)->GetDetector() == 8) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionD8Only);
      mdebug<<"CSR-CS - Dual hit: Rejecting a D8 only event"<<endl;
      return;
    }
  }
  
  // Reject events which have only two tracks:
  if (RE->GetRESEAt(0)->GetType() == MRESE::c_Track && RE->GetRESEAt(1)->GetType() == MRESE::c_Track) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionTwoTracksOnly);
    mdebug<<"CSR-CS - Dual hit: Rejecting a event, which consists of nothing but two electron tracks"<<endl;
    return;  
  }
  
  
  // A single D1 hit or the track of multiple D1 hits is assumed to be the initial estimate for first hit
  // This will be revised later in some cases
  int EstimatedFirst = -1;
  if ((RE->GetRESEAt(0)->GetDetector() == 1 || RE->GetRESEAt(0)->GetDetector() == 5) && (RE->GetRESEAt(1)->GetDetector() != 1 && RE->GetRESEAt(1)->GetDetector() != 5)) {
    EstimatedFirst = 0;
  } else if ((RE->GetRESEAt(1)->GetDetector() == 1 || RE->GetRESEAt(1)->GetDetector() == 5) && (RE->GetRESEAt(0)->GetDetector() != 1 && RE->GetRESEAt(0)->GetDetector() != 5)) {
    EstimatedFirst = 1;
  } else if (RE->GetRESEAt(1)->GetType() == MRESE::c_Track && RE->GetRESEAt(0)->GetType() != MRESE::c_Track) {
    EstimatedFirst = 1;
  } else if (RE->GetRESEAt(0)->GetType() == MRESE::c_Track && RE->GetRESEAt(1)->GetType() != MRESE::c_Track) {
    EstimatedFirst = 0;
  }
  // There are many cases in which we do not have an estimated first! -- not sure we really need it any more - it was just a helper during the times of MEGA...
  
  //mdebug<<"Location of estimate first is D"<<RE->GetRESEAt(EstimatedFirst)->GetDetector()<<" with index "<<EstimatedFirst<<endl;
  
  bool IsGood = false;
  double Theta = 0;
  
  
  // Compute the compton data:
  int HitSequence1 = 0;
  int HitSequence2 = 0;
  double EnergySequence1 = 0;
  double EnergySequence2 = 0;
  MVector PositionSequence1;
  MVector PositionSequence2;
  MVector ElectronDirection = MVector(0.0, 0.0, 0.0);
  bool HasTrack = false;
  
  if (EstimatedFirst >= 0) {
    MRETrack* Track = nullptr;
    for (int i = 0; i < RE->GetNRESEs(); i++) {
      if (i == EstimatedFirst) {
        HitSequence1 = i;
        EnergySequence1 = RE->GetRESEAt(i)->GetEnergy();
        PositionSequence1 = RE->GetRESEAt(i)->GetPosition();
        if (RE->GetRESEAt(i)->GetType() == MRESE::c_Track) {
          Track = ((MRETrack *) RE->GetRESEAt(i));
          HasTrack = true;
        } else {
          HasTrack = false;
        } 
        //mdebug<<"Seq1 is in D"<<RE->GetRESEAt(i)->GetDetector()<<" with index "<<i<<endl;
      } else {
        HitSequence2 = i;
        EnergySequence2 = RE->GetRESEAt(i)->GetEnergy();
        PositionSequence2 = RE->GetRESEAt(i)->GetPosition();
        //mdebug<<"Seq2 is in D"<<RE->GetRESEAt(i)->GetDetector()<<" with index "<<i<<endl;
      }
    }
    if (HasTrack == true) {
      ElectronDirection = Track->GetDirection();
      Theta = ElectronDirection.Angle(PositionSequence2 - PositionSequence1);
    }
  } else {
    // We can not determine which hit was in D1,
    // therefore the first hit is the first sequence
    HitSequence1 = 0;
    EnergySequence1 = RE->GetRESEAt(0)->GetEnergy();
    PositionSequence1 = RE->GetRESEAt(0)->GetPosition();
    HitSequence2 = 1;
    EnergySequence2 = RE->GetRESEAt(1)->GetEnergy();
    PositionSequence2 = RE->GetRESEAt(1)->GetPosition();
  }
  
  if (HasTrack == false && m_UseComptelTypeEvents == false) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionComptelTypeEvent);
    mdebug<<"CSR-CS - Dual hit: Rejecting a ComptelType event"<<endl;
    return;
  }
  
  
  // Now link the two RESEs
  RE->GetRESEAt(HitSequence1)->AddLink(RE->GetRESEAt(HitSequence2));
  RE->GetRESEAt(HitSequence2)->AddLink(RE->GetRESEAt(HitSequence1));
  
  // (A)
  // If we do not assume that the first hit came from D1, then
  // create two Compton events one from D1 -> D2, the other D2 -> D1
  // If *only one* event is a compton event, then take it:
  // (B) ... or ... 
  // If we assume start D1 and both are from D1 or both are from D5 or none is in D1 
  if (m_GuaranteeStartD1 == false || 
      (RE->GetRESEAt(0)->GetDetector() == 1 && RE->GetRESEAt(1)->GetDetector() == 1) || 
      (RE->GetRESEAt(0)->GetDetector() == 5 && RE->GetRESEAt(1)->GetDetector() == 5)) {
    
    MComptonEvent Sequence1;
    bool Sequence1Good = Sequence1.Assimilate(PositionSequence1, PositionSequence2, ElectronDirection, EnergySequence1, EnergySequence2);
    
    MComptonEvent Sequence2;
    bool Sequence2Good = Sequence2.Assimilate(PositionSequence2, PositionSequence1, ElectronDirection, EnergySequence2, EnergySequence1);
    
    
    // Case 1: S1 good, S2 bad
    if ((Sequence1Good == true) && (Sequence2Good == false)) {
      mdebug<<"CSR-CS - Dual hit: Unique sequence"<<endl;
      IsGood = true;
      EstimatedFirst = HitSequence1;
    } 
    // Case 2: S2 good, S1 bad
    else if ((Sequence1Good == false) && (Sequence2Good == true)) {
      mdebug<<"CSR-CS - Dual hit: Unique sequence"<<endl;
      IsGood = true;
      EstimatedFirst = HitSequence2;
    } 
    // Case 3: both bad
    else if ((Sequence1Good == false) && (Sequence2Good == false)) {
      mdebug<<"CSR-CS - Dual hit: Event does not comply to a Compton event!"<<endl;
      IsGood = false;
      RE->SetRejectionReason(MRERawEvent::c_RejectionComptelTypeKinematicsBad);
    } 
    // Case 4: both sequences good
    else if ((Sequence1Good == true) && (Sequence2Good == true)) {
      // can't decide ...
      mdebug<<"CSR-CS - Dual hit: Cannot decide between Sequence1 first and Sequence2 first..."<<endl;
      
      // if we have no track, we can not decide now what hit was first
      // So for the meantime take it...
      if (HasTrack == false) {
        // Simply reject all undecided events
        if (m_UndecidedHandling == c_UndecidedIgnore) {
          mdebug<<"CSR-CS - Dual hit: Undecided event without track (automatic rejection)"<<endl;
          IsGood = false;
          RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartUndecided);
        } 
        // ... assume undecided start in D1...
        else if (m_UndecidedHandling == c_UndecidedAssumestartD1) {
          mdebug<<"CSR-CS - Dual hit: Assume undecided event without track starts in D1"<<endl;
          IsGood = true;
          EstimatedFirst = HitSequence1;
        } 
        // ... assume sequence with higher Klein-Nishina probability is correct:
        else if (m_UndecidedHandling == c_UndecidedLargerKleinNishina) {
          double KN1 = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergySequence1 + EnergySequence2, 
                                                                      MComptonEvent::ComputePhiViaEeEg(EnergySequence1, EnergySequence2));
          double KN2 = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergySequence1 + EnergySequence2, 
                                                                      MComptonEvent::ComputePhiViaEeEg(EnergySequence2, EnergySequence1));
          if (KN1 > KN2) {
            EstimatedFirst = HitSequence1;
            QualityFactor1 = KN1/(KN1+KN2);
            QualityFactor2 = KN2/(KN1+KN2);
          } else {
            EstimatedFirst = HitSequence2;
            QualityFactor1 = KN2/(KN1+KN2);
            QualityFactor2 = KN1/(KN1+KN2);
          }
          mdebug<<"CSR-CS - Dual hit: Undecided event without track: Using Klein-Nishina times photo absorption probability"<<endl;
          IsGood = true;
        } 
        // ... assume sequence with higher Klein-Nishina times photo absorption probability is correct:
        else if (m_UndecidedHandling == c_UndecidedLargerKleinNishinaTimesPhoto) {
          double KN1 = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergySequence1 + EnergySequence2, 
                                                                      MComptonEvent::ComputePhiViaEeEg(EnergySequence1, EnergySequence2));
          double KN2 = MComptonEvent::GetKleinNishinaNormalizedByArea(EnergySequence1 + EnergySequence2, 
                                                                      MComptonEvent::ComputePhiViaEeEg(EnergySequence2, EnergySequence1));
          double PA1 = m_Geometry->GetPhotoAbsorptionProbability(PositionSequence1, PositionSequence2, EnergySequence2); 
          double PA2 = m_Geometry->GetPhotoAbsorptionProbability(PositionSequence2, PositionSequence1, EnergySequence1); 
          
          if (KN1*PA1 > KN2*PA2) {
            EstimatedFirst = HitSequence1;
            QualityFactor1 = (KN1*PA1)/(KN1*PA1+KN2*PA2);
            QualityFactor2 = (KN2*PA2)/(KN1*PA1+KN2*PA2);
          } else {
            EstimatedFirst = HitSequence2;
            QualityFactor1 = (KN2*PA2)/(KN1*PA1+KN2*PA2);
            QualityFactor2 = (KN1*PA1)/(KN1*PA1+KN2*PA2);
          }
          mdebug<<"CSR-CS - Dual hit: Undecided event without track: Using Klein-Nishina times photon absorption probability"<<endl;
          IsGood = true;
        } 
        // ... assume sequence with higher Klein-Nishina probability is correct:
        else if (m_UndecidedHandling == c_UndecidedLargerEnergyDeposit) {
          if (EnergySequence1 > EnergySequence2) {
            EstimatedFirst = HitSequence1;
            QualityFactor1 = EnergySequence1/(EnergySequence1+EnergySequence2);
            QualityFactor2 = EnergySequence2/(EnergySequence1+EnergySequence2);
          } else {
            EstimatedFirst = HitSequence2;
            QualityFactor1 = EnergySequence2/(EnergySequence1+EnergySequence2);
            QualityFactor2 = EnergySequence1/(EnergySequence1+EnergySequence2);
          }
          mdebug<<"CSR-CS - Dual hit: Undecided event without track: Using sequence with larger first deposit"<<endl;
          IsGood = true;
        }
        // Unhandled case
        else {
          mdebug<<"CSR-CS - Dual hit: Undecided event without track"<<endl;
          IsGood = false;
          RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartUndecided);
        }
      } 
      // we do have an electron track
      else {
        // Now make a test on D2->D1 
        // If the angle between the IAs and and the electron-direction < 90° then
        // the event can't have happened this way 
        if (Theta < c_Pi/2.0) {
          // For sure D1->D2
          mdebug<<"CSR-CS - Dual hit: Case: alpha < c_Pi/2:"<<endl;
          
          // Now compare the alphas: the computed and the measured:
          // if they are comparable, then accept the event:
          
          if (fabs(Sequence1.Theta() - Theta) < LimitTheta) {
            mdebug<<"CSR-CS - Dual hit: Theta-difference within limits: "
            <<fabs(Sequence1.Theta() - Theta)*c_Deg<<" < "<<LimitTheta*c_Deg<<"(Limit)"<<endl;
            IsGood = true;
          } else { 
            mdebug<<"CSR-CS - Dual hit: Theta-difference not ok: "
            <<fabs(Sequence1.Theta() - Theta)*c_Deg<<" < "<<LimitTheta*c_Deg<<"(Limit)"<<endl;
            IsGood = false;
            RE->SetRejectionReason(MRERawEvent::c_RejectionTrackNotValid);
          }
        } else if (Theta > c_Pi/2.0 + Sequence1.Phi()) {
          // For sure: D2->D1
          mdebug<<"CSR-CS - Dual hit: Case: alpha > c_Pi/2.0 + phi1"<<endl;
          IsGood = false;
          RE->SetRejectionReason(MRERawEvent::c_RejectionTrackNotValid);
        } else {
          mdebug<<"CSR-CS - Dual hit: Case: c_Pi/2.0 < alpha < c_Pi/2.0 + phi1"<<endl;
          mdebug<<"Still undecided!"<<endl;
          
          // Now compare the alphas: the computed and the measured:
          // if they are comparable, then accept the event:
          
          if (fabs(Sequence1.Theta() - Theta) < LimitTheta) {
            // Accept it
            mdebug<<"CSR-CS - Dual hit: Theta-difference within limits: "
            <<fabs(Sequence1.Theta() - Theta)*c_Deg<<" < "<<LimitTheta*c_Deg<<"(Limit)"<<endl;
            IsGood = true;
          } else { 
            mdebug<<"CSR-CS - Dual hit: Theta-difference not ok: "
            <<fabs(Sequence1.Theta() - Theta)*c_Deg<<" < "<<LimitTheta*c_Deg<<"(Limit)"<<endl;
            IsGood = false;
            RE->SetRejectionReason(MRERawEvent::c_RejectionTrackNotValid);
          }
        }
      } // end decision: with/without track
    } 
    
  } else { // m_GuaranteeStartD1 == true or not both in D1 or D5
    
    // We should have an estimated first here
    if (EstimatedFirst < 0) {
      mdebug<<"CSR-CS - Dual hit: Event start in D1 required, but no hit is in D1 or D5"<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionNoHitsInTracker);
      return;
    }
    
    // PositionSequence1 is already selected as the first hit, check if it is Compton compatible
    MComptonEvent First;
    bool FirstGood = First.Assimilate(PositionSequence1, PositionSequence2, ElectronDirection, EnergySequence1, EnergySequence2);
    
    if (FirstGood == false) {
      // The event starts in the calorimeter
      mdebug<<"CSR-CS - Dual hit: Event start in D1 incompatible with Compton event!"<<endl;
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartNotD1);
      IsGood = false;
    } else {
      mdebug<<"CSR-CS - Dual hit: Assuming start of event in D1!"<<endl;
      IsGood = true;
    }
  }
  
  
  
  if (IsGood == true) {
    if (EstimatedFirst < 0) {
      merr<<"ERROR: We missed some condition during handling of two-site events... Bug!"<<endl;
      RE->SetEventType(MRERawEvent::c_UnknownEvent);
      RE->SetEventReconstructed(true);
      return;
    }
    
    if (m_GuaranteeStartD1 == true && (RE->GetRESEAt(EstimatedFirst)->GetDetector() != 1 && RE->GetRESEAt(EstimatedFirst)->GetDetector() != 5)) {
      RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartNotD1);
      mdebug<<"CSR-CS - Sequence: Event starts not in D1/D5 but in "<<RE->GetRESEAt(EstimatedFirst)->GetDetector()<<endl;
      IsGood = false;
    } else {
      
      // Start point...
      RE->SetStartPoint(RE->GetRESEAt(EstimatedFirst));
      
      if (HasTrack == true) {
        bool DirectionTest = false;
        if (EstimatedFirst == HitSequence1) {
          DirectionTest = RE->TestElectronDirection(EnergySequence1, EnergySequence2);
        } else {
          DirectionTest = RE->TestElectronDirection(EnergySequence2, EnergySequence1);
        }
        if (DirectionTest == false) {
          mdebug<<"CSR-CS - Dual hit: Electron direction test failed!"<<endl;
          RE->SetRejectionReason(MRERawEvent::c_RejectionTrackNotValid);
          IsGood = false;
        }
      }
      
      if (IsGood == true) {
        mdebug<<"CSR-CS - Dual hit: Good Compton event"<<endl;
        RE->SetEventType(MRERawEvent::c_ComptonEvent);
        if (m_TypeTestStatistics == c_TSSimple) {
          if (QualityFactor1 < 0) {
            RE->SetComptonQualityFactors(0, 1);
          } else {
            RE->SetComptonQualityFactors(QualityFactor1, QualityFactor2);
          }
        } else if (m_TypeTestStatistics == c_TSSimpleWithErrors) {
          if (QualityFactor1 < 0) {
            RE->SetComptonQualityFactors(0, 1);
          } else {
            mimp<<"The errors are missing..."<<endl;
            RE->SetComptonQualityFactors(QualityFactor1, QualityFactor2);
          }
        } else {
          if (QualityFactor1 < 0) {
            RE->SetComptonQualityFactors(1000.0, 0.0);
          } else {
            RE->SetComptonQualityFactors(QualityFactor1, QualityFactor2);
          }
        }     
        RE->SetGoodEvent(true);
      }
    }
  }
  
  if (IsGood == false) {
    mdebug<<"CSR-CS - Dual hit: Bad Compton event"<<endl;				
  }
  RE->SetEventReconstructed(true);
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRChiSquare::FindComptonSequence(MRERawEvent* RE)
{
  mdebug<<"CSR-CS - Sequence: Searching Compton sequence of event with "
  <<RE->GetNRESEs()<<" hits..."<<endl;
  
  RE->SetComptonQualityFactors(DBL_MAX, DBL_MAX);
  
  mdebug<<RE->ToString()<<endl;
  
  
  // Check if we have a 3+ Compton:
  if (RE->GetNRESEs() > m_MaxNInteractions) {
    mdebug<<"CSR-CS - Sequence: Too Many hits: "<<RE->GetNRESEs()<<" > "<<m_MaxNInteractions<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionTooManyHitsCSR);
    return;
  } else if (RE->GetNRESEs() == 2) {
    FindComptonSequenceDualHitEvent(RE);//Sets Event Reconstructed to true
    return;
  } else if (RE->GetNRESEs() == 1) {
    mdebug<<"CSR-CS - Sequence: Only single hit event!"<<endl;
    //RE->SetRejectionReason(MRERawEvent::c_RejectionSingleSiteEvent);
    if (RE->GetRESEAt(0)->GetType() == MRESE::c_Hit || RE->GetRESEAt(0)->GetType() == MRESE::c_Cluster) {
      RE->SetEventType(MRERawEvent::c_PhotoEvent);
      RE->SetGoodEvent(true);
      RE->SetEventReconstructed(true);
    } else {
      RE->SetRejectionReason(MRERawEvent::c_RejectionOneTrackOnly);
    }
    return; 
  } else if (RE->GetNRESEs() == 0) {
    mdebug<<"CSR-CS - Sequence: Event without hits"<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionNoHits);
    return; 
  }
  
  
  // The following function fill the m_QualityFactors map with
  // permutations and quality factors
  int NGoodSequences = ComputeAllQualityFactors(RE);
  
  // Dump all permutations:
  if (g_Verbosity > 0) {
    int NShownCombis = 120;
    mdebug<<"CSR-CS - Sequence: Top "<< NShownCombis<<" valid permutations ("<<m_QualityFactors.size()<<")"<<endl;
    for (m_QualityFactorsIterator = m_QualityFactors.begin(); (m_QualityFactorsIterator != m_QualityFactors.end() && NShownCombis > 0); m_QualityFactorsIterator++, NShownCombis--) {
      mdebug<<"   Combi "/*<<m_QualityFactorsIterator*/<<": ";
      for (unsigned int r = 0; r < (*m_QualityFactorsIterator).second.size(); ++r) {
        mdebug<<(*m_QualityFactorsIterator).second[r]->GetID();
        if ( r < (*m_QualityFactorsIterator).second.size()-1) {
          mdebug<<" - ";
        }
      }
      mdebug<<"  -->  "<<(*m_QualityFactorsIterator).first<<endl;
    }
  }
  
  if (NGoodSequences == 0) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRNoGoodCombination);
    mdebug<<"CSR-CS - Sequence: None of the sequences is valid!"<<endl;
    return;    
  }
  
  // Find best sequences:
  vector<MRESE*> BestSequence;
  double BestQualityFactor =  c_CSRFailed;
  double SecondBestQualityFactor = c_CSRFailed;
  
  BestSequence = (*m_QualityFactors.begin()).second;
  BestQualityFactor = (*m_QualityFactors.begin()).first;
  if (NGoodSequences > 1) {
    SecondBestQualityFactor = (*(++m_QualityFactors.begin())).first;
  }
  
  // Test if we have at least one good sequence:
  if (BestQualityFactor == c_CSRFailed) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRNoGoodCombination);
    mdebug<<"CSR-CS - Sequence: None of the sequences is valid!"<<endl;
    return;    
  }
  
  // Get additional energy -> only important for CSREnergyRecovery!
  double EscapedEnergy = GetEscapedEnergy(BestSequence);
  
  // Upgrade event to a sequence:
  RE->SetStartPoint(BestSequence[0]);
  RE->SetAdditionalEnergy(EscapedEnergy);
  for (unsigned int i = 0; i < BestSequence.size(); ++i) {
    if (i >= 1) {
      BestSequence[i]->AddLink(BestSequence[i-1]);
    }
    if (i+1 < BestSequence.size()) {
      BestSequence[i]->AddLink(BestSequence[i+1]);
    }
  }
  //   m_List->SetValidEvent(RE); // Problem wenn mehrere RE zu vergleichen sind...
  //   m_List->SetBestEvent(RE); // Problem wenn mehrere RE zu vergleichen sind...
  
  
  // Check if the threshold is ok:
  if (BestQualityFactor < m_QualityFactorMin || BestQualityFactor > m_QualityFactorMax) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRThreshold);
    mdebug<<"CSR-CS - Sequence: Teststatistics ("<<BestQualityFactor<<") out of threshold ("<<m_QualityFactorMin<<" - "<<m_QualityFactorMax<<")"<<endl;
    return;
  }
  
  // Check if start in tracker is necessary:
  if (BestSequence[0]->GetDetector() != 1 && BestSequence[0]->GetDetector() != 5 && m_GuaranteeStartD1 == true) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartNotD1);
    mdebug<<"CSR-CS - Sequence: Event starts in not in D1/D5 but in "<<BestSequence[0]->GetDetector()<<endl;
    mdebug<<"CSR-CS - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
    return;
  }
  
  // Check if track is ok:
  double E1, E2 = 0;
  E1 = BestSequence[0]->GetEnergy();
  for (unsigned int i = 1; i < BestSequence.size(); ++i) {
    E2 += BestSequence[i]->GetEnergy();
  }
  E2 += EscapedEnergy;
  if (BestSequence[0]->GetType() == MRESE::c_Track && RE->TestElectronDirection(E1, E2) == false) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionElectronDirectionBad);
    mdebug<<"CSR-CS - Sequence: Electron direction test failed!"<<endl;
    mdebug<<"CSR-CS - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
    return;
  }
  
  mdebug<<"CSR-CS - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
  
  RE->SetComptonQualityFactors(BestQualityFactor, SecondBestQualityFactor);
  //RE->SetEventType(MRERawEvent::c_ComptonEvent);
  RE->SetGoodEvent(true);
  RE->SetEventReconstructed(true);
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRChiSquare::ComputeQualityFactor(vector<MRESE*>& Interactions)
{
  // Calculate the Compton quality factor of this sequence
  
  if (m_GuaranteeStartD1 == true && Interactions.size() >= 1) {
    if (Interactions[0]->GetDetector() != 1 && Interactions[0]->GetDetector() != 5) {
      return c_CSRFailed;
    }
  }
  
  bool IsValid = true;
  
  static const double E0 = 511.044;
  static const double CosLimit = 100.5;
  
  double dTS = 0.0;
  double TS = 0.0;
  
  double CosPhiE = 0.0;  // cos(phi) computed by energies
  double dCosPhiE2 = 0.0;  // cos(phi) computed by energies
  double Ei = 0.0;         // energy incoming gamma
  double Eg = 0.0;         // energy scattered gamma
  double Ee = 0.0;         // energy recoil electron
  double dEg = 0.0;         // energy scattered gamma
  double dEe = 0.0;         // energy recoil electron
  
  double CosPhiA = 0.0;  // cos(phi) computed by angles
  double dCosPhiA2 = 0.0;  // cos(phi) computed by angles
  
  
  unsigned int NTSs = 0;
  for (unsigned int i = 1; i < Interactions.size() - 1; ++i) {
    
    // Calculate energies:
    Ee = Interactions[i]->GetEnergy(); // Das muss i heissen - definitiv!!!
    dEe = Interactions[i]->GetEnergyResolution();
    
    Eg = 0.0;
    dEg = 0.0;
    for (unsigned int j = i+1; j < Interactions.size(); ++j) {
      Eg += Interactions[j]->GetEnergy();
      dEg += Interactions[j]->GetEnergyResolution()*Interactions[j]->GetEnergyResolution();
    }
    Ei = Ee + Eg;
    
    if (dEg >= 0) {
      dEg = sqrt(dEg);
    } else {
      merr<<"Negative energy resolution!!!"<<endl;
      IsValid = false;
      break;
    }
    
    if (Eg <= 0) {
      merr<<"Eg is not positive!"<<endl;
      IsValid = false;
      break;
    }
    if (Eg+Ee <= 0) {
      merr<<"Eg+Ee is not positive!"<<endl;
      IsValid = false;
      break;
    }
    
    CosPhiE = 1 - E0/Eg + E0/(Ee+Eg);
    dCosPhiE2 = E0*E0/(Ei*Ei*Ei*Ei)*dEe*dEe+pow(E0/(Eg*Eg)-E0/(Ee+Eg)/(Ee+Eg),2)*dEg*dEg;
    
    if (CosPhiE < -1) {
      if (CosPhiE < -1 - CosLimit*sqrt(dCosPhiE2)) {
        mdebug<<"cos phi via energy out of bounds: "<<CosPhiE<<" +- "<<sqrt(dCosPhiE2)<<endl;
        IsValid = false;
        break;
      } // else {
      //         CosPhiE = -1;
      //       }
    }
    if (CosPhiE > 1) {
      if (CosPhiE > 1 + CosLimit*sqrt(dCosPhiE2)) {
        mdebug<<"cos phi via energy out of bounds: "<<CosPhiE<<" +- "<<sqrt(dCosPhiE2)<<endl;
        IsValid = false;
        break;
      } 
      // else {
      //   CosPhiE = 1;
      // }
    }
    //  if (CosPhiE <= -1 || CosPhiE >= 1) {
    //    mdebug<<"CosPhiE ("<<CosPhiE<<") out of bounds: event not Compton compatible: "<<endl;
    //     IsValid = false;
    //     break;
    //   }
    
    // The origin is only checked for the FIRST part of the sequence
    if (m_OriginObjects != 0 && i == 1) {
      MComptonEvent Compton;
      double CEe = Interactions[i-1]->GetEnergy(); // Das muss i-1 heissen - definitiv!!!
      double CEg = 0.0;
      for (unsigned int j = i; j < Interactions.size(); ++j) {
        CEg += Interactions[j]->GetEnergy();
      }
      //cout<<"CEe: "<<CEe<<"  CEg:"<<CEg<<endl;
      if (Compton.Assimilate(Interactions[i-1]->GetPosition(), Interactions[i]->GetPosition(), MVector(0,0,0), CEe, CEg) == false) {
        IsValid = false;
        break;
      }
      if (OriginatesFromObjects(Compton) == false) {
        IsValid = false;
        break;
      }
    }
    
    CosPhiA = 
    cos((Interactions[i]->GetPosition() - Interactions[i-1]->GetPosition()).
    Angle(Interactions[i+1]->GetPosition() - Interactions[i]->GetPosition()));
    //mout<<"phi v A: "<<acos(CosPhiA)*c_Deg<<endl;
    
    dCosPhiA2 = pow(ComputePositionError(Interactions[i-1], Interactions[i], Interactions[i+1]), 2);
    
    if (dCosPhiA2 <= 0 || dCosPhiE2 <= 0) {
      merr<<"Resolutions are not positive: dCosPhiA^2="<<dCosPhiA2<<" dCosPhiEd^2: "<<dCosPhiE2<<endl;
      dCosPhiA2 = 0.5;
      dCosPhiE2 = 0.5;
      IsValid = false;
      break;
    } 
    /*
     *      mout<<"Phi via E: "<<acos(CosPhiE)*c_Deg
     *      <<" error: "<<CosPhiE<<" +- "<<sqrt(dCosPhiE2)<<endl;
     *      mout<<"Phi via A: "<<acos(CosPhiA)*c_Deg
     *      <<" error: "<<CosPhiA<<" +- "<<sqrt(dCosPhiA2)<<endl;
     *      mout<<"TS: "<<(CosPhiE - CosPhiA)*(CosPhiE - CosPhiA)<<endl;
     */
    
    dTS += 2*fabs(CosPhiE - CosPhiA)*sqrt(dCosPhiE2 + dCosPhiA2);
    if (m_TypeTestStatistics == c_TSSimpleWithErrors || m_TypeTestStatistics == c_TSChiSquare) {
      TS += (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA)/(dCosPhiE2 + dCosPhiA2);
    } else {
      TS += (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA);
    }
    
    // TS += pow(acos(CosPhiE) - acos(CosPhiA), 2); //  
    //(pow(sin(acos(CosPhiE))*acos(sqrt(dCosPhiE2)), 2) + pow(sin(acos(CosPhiA))*acos(sqrt(dCosPhiA2)), 2));
    
    NTSs++;
  } 
  
  // Normalize the test statistics:
  if (IsValid == true && NTSs > 0) {
    TS /= NTSs;
    dTS /= NTSs;
  } else {
    TS = c_CSRFailed;
    dTS = c_CSRFailed;
  }
  
  if (m_TypeTestStatistics == c_TSChiSquare) {
    if (TS > 0 && dTS > 0) {
      // good
    } else {
      TS = c_CSRFailed;
    }
  }
  
  return TS;
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRChiSquare::OriginatesFromObjects(const MComptonEvent& Compton)
{
  // Test if the Compton events originates from one of the objects in the
  // ObjectsGeometry
  
  return m_OriginObjects->GetComptonIntersection(Compton);
}



////////////////////////////////////////////////////////////////////////////////


MString MERCSRChiSquare::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:
  
  ostringstream out;
  
  out<<"# CSR Chi-Square options:"<<endl;
  out<<"# "<<endl;
  out<<"# Test statistics:                 "<<m_TypeTestStatistics<<endl;
  out<<"# QualityFactorMin:                "<<m_QualityFactorMin<<endl;
  out<<"# QualityFactorMax:                "<<m_QualityFactorMax<<endl;
  out<<"# MaxNInteractions:                "<<m_MaxNInteractions<<endl;
  out<<"# UseComptelTypeEvents:            "<<m_UseComptelTypeEvents<<endl;
  out<<"# AssumeD1First:                   "<<m_AssumeD1First<<endl;
  out<<"# UndecidedHandling:               "<<m_UndecidedHandling<<endl;
  out<<"# GuaranteeStartD1:                "<<m_GuaranteeStartD1<<endl;
  out<<"# RejectOneDetectorTypeOnlyEvents: "<<m_RejectOneDetectorTypeOnlyEvents<<endl;
  out<<"# "<<endl;
  
  return out.str().c_str();
}


// MERCSRChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
