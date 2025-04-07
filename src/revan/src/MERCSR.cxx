/*
 * MERCSR.cxx
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
// MERCSR
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSR.h"

// Standard libs:
#include <cmath>
#include <limits>
using namespace std;

// ROOT libs:
#include "TObjArray.h"
#include "TMath.h"

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
ClassImp(MERCSR)
#endif


////////////////////////////////////////////////////////////////////////////////


const double MERCSR::c_CSRFailed = numeric_limits<double>::max()/2; // Has to be a very large number! 


////////////////////////////////////////////////////////////////////////////////


MERCSR::MERCSR() : m_MaxNInteractions(5)
{
  // Construct an instance of MERCSR
}


////////////////////////////////////////////////////////////////////////////////


MERCSR::~MERCSR()
{
  // Delete this instance of MERCSR
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSR::SetParameters(MGeometryRevan* Geometry, 
                           double QualityFactorMin, 
                           double QualityFactorMax, 
                           int MaxNInteractions,
                           bool GuaranteeStartD1,
                           bool CreateOnlyPermutations)
{
  m_Geometry = Geometry;
  m_QualityFactorMin = QualityFactorMin;
  m_QualityFactorMax = QualityFactorMax;
  m_MaxNInteractions = MaxNInteractions;
  m_GuaranteeStartD1 = GuaranteeStartD1;
  m_CreateOnlyPermutations = CreateOnlyPermutations;

  mdebug<<"CSR called with the following options:"<<endl;
  mdebug<<"QualityFactorMin: "<<m_QualityFactorMin<<endl;
  mdebug<<"QualityFactorMax: "<<m_QualityFactorMax<<endl;
  mdebug<<"MaxNInteractions: "<<m_MaxNInteractions<<endl;
  mdebug<<"GuaranteeStartD1: "
        <<((m_GuaranteeStartD1 == true) ? "true" : "false")<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSR::Analyze(MRawEventIncarnations* List)
{
  // Analyze the raw event...

  MERConstruction::Analyze(List);

  if (m_CreateOnlyPermutations == true) {
    m_List->SetOptimumEvent(nullptr);

    int e_max = m_List->GetNRawEvents();
    for (int e = 0; e < e_max; ++e) {
      MRERawEvent* RE = m_List->GetRawEventAt(e);
      MRawEventIncarnations* NewList = CreateOnlyPermutations(RE);
      if (NewList != 0) {
        m_List->DeleteRawEvent(RE);
        e--;
        e_max--;
        for (int i = 0; i < NewList->GetNRawEvents(); ++i) {
          m_List->AddRawEvent(NewList->GetRawEventAt(i));
        }
        delete NewList;
      }        
    }
  } else {
    // Loop over all raw events and calculate all sequences:
    int e_max = m_List->GetNRawEvents();
    for (int e = 0; e < e_max; ++e) {
      FindComptonSequence(m_List->GetRawEventAt(e));
    }
    
    // Select the raw event with the best quality factor as the good one:
    int NGoodEvents = 0;
    double BestQualityFactor = c_CSRFailed;
    for (int e = m_List->GetNRawEvents()-1; e >= 0; e--) {
      if (m_List->GetRawEventAt(e)->IsGoodEvent() == true) {
        if (m_List->GetRawEventAt(e)->GetEventType() == MRERawEvent::c_ComptonEvent) {
          if (m_List->GetRawEventAt(e)->GetComptonQualityFactor() < BestQualityFactor) {
            BestQualityFactor = m_List->GetRawEventAt(e)->GetComptonQualityFactor();
            m_List->SetOptimumEvent(m_List->GetRawEventAt(e));
            m_List->SetBestTryEvent(m_List->GetRawEventAt(e));
          }
        } else if (m_List->GetRawEventAt(e)->GetEventType() == MRERawEvent::c_PhotoEvent) {
          m_List->SetOptimumEvent(m_List->GetRawEventAt(e));
          m_List->SetBestTryEvent(m_List->GetRawEventAt(e));
        }
        NGoodEvents++;
      }
    }

    if (NGoodEvents == 0) {
      mdebug<<"CSR - No good events"<<endl;
      m_List->SetOptimumEvent(0);
      // If there is a previous best try, then keep this one...
      // m_List->SetBestTryEvent(0); 
    }
  }

  return true;
}



////////////////////////////////////////////////////////////////////////////////


MRawEventIncarnations* MERCSR::CreateOnlyPermutations(MRERawEvent* RE)
{
  // Create and link all possible permutations:
  // Now we are in some programming trouble:
  // We have to evaluate all possible permutations if first degree 
  // sub elements in raw event:
  // For 4 hits there are 24 permutations which can be evaluated via this
  // shifting algorithm:
  // 4 times do: 432->32->2->32->2->32->2->
  // ("->" means: evaluate; 4 left shift the first 4 elements, 
  // 3 left shift 3... and so on.)

  if (RE->GetNRESEs() > m_MaxNInteractions) {
    mout<<"BCT - Sequence: Too Many hits: "<<RE->GetNRESEs()<<" > "<<m_MaxNInteractions<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionTooManyHitsCSR);
    return 0;
  } else if (RE->GetNRESEs() == 0) {
    mout<<"BCT - Sequence: Event without hits"<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionNoHits);
    return 0; 
  }

  vector<MRESE*> RESEs;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    RESEs.push_back(RE->GetRESEAt(i));
  }

  // First find all different permutations:
  vector<vector<MRESE*> > Permutations;
  Permutations.reserve(int(TMath::Factorial(RE->GetNRESEs())));
  FindPermutations(RESEs, RESEs.size(), Permutations);

  MRawEventIncarnations* List = new MRawEventIncarnations();
  for (unsigned int c = 0; c < Permutations.size(); ++c) {
    MRERawEvent* Dup = RE->Duplicate();

    // Create sequence:
    Dup->SetStartPoint(Dup->GetRESE(Permutations[c][0]->GetID()));
    for (unsigned int i = 0; i < Permutations[c].size(); ++i) {
      if (i >= 1) {
        Dup->GetRESE(Permutations[c][i]->GetID())->
          AddLink(Dup->GetRESE(Permutations[c][i-1]->GetID()));
      }
      if (i+1 < Permutations[c].size()) {
        Dup->GetRESE(Permutations[c][i]->GetID())->
          AddLink(Dup->GetRESE(Permutations[c][i+1]->GetID()));
      }
    }

    List->AddRawEvent(Dup);
  }

  return List;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSR::FindComptonSequence(MRERawEvent* RE)
{
  mout<<"CSR - Sequence: Searching Compton sequence of event with "
      <<RE->GetNRESEs()<<" hits ("<<RE->GetEventID()<<")..."<<endl;

  RE->SetComptonQualityFactors(DBL_MAX, DBL_MAX);

  //mout<<RE->ToString()<<endl;
  
  // Check if we have a 3+ Compton:
  if (RE->GetNRESEs() > m_MaxNInteractions) {
    mout<<"CSR - Sequence: Too Many hits: "<<RE->GetNRESEs()<<" > "<<m_MaxNInteractions<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionTooManyHitsCSR);
    return;
  } else if (RE->GetNRESEs() == 1) {
    mout<<"CSR - Sequence: Only single hit event!"<<endl;
    //RE->SetRejectionReason(MRERawEvent::c_RejectionSingleSiteEvent);
    if (RE->GetRESEAt(0)->GetType() == MRESE::c_Hit || 
        RE->GetRESEAt(0)->GetType() == MRESE::c_Cluster) {
      RE->SetEventType(MRERawEvent::c_PhotoEvent);
      RE->SetGoodEvent(true);
    } else {
      RE->SetRejectionReason(MRERawEvent::c_RejectionOneTrackOnly);
    }
    return; 
  } else if (RE->GetNRESEs() == 0) {
    mout<<"CSR - Sequence: Event without hits"<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionNoHits);
    return; 
  }

  // The following function fill the m_QualityFactors map with
  // permutations and quality factors
  int NGoodSequences = ComputeAllQualityFactors(RE);

  if (g_Verbosity >= 1) {
    // Dump all permutations:
    int NShownCombis = 120;
    mout<<"CSR - Sequence: Top "<< NShownCombis<<" valid permutations ("<<m_QualityFactors.size()<<")"<<endl;
    for (m_QualityFactorsIterator = m_QualityFactors.begin(); 
         (m_QualityFactorsIterator != m_QualityFactors.end() && NShownCombis > 0); 
         m_QualityFactorsIterator++, NShownCombis--) {
      mout<<"   Combi "/*<<m_QualityFactorsIterator*/<<": ";
      for (unsigned int r = 0; r < (*m_QualityFactorsIterator).second.size(); ++r) {
        mout<<(*m_QualityFactorsIterator).second[r]->GetID();
        if ( r < (*m_QualityFactorsIterator).second.size()-1) {
          mout<<" - ";
        }
      }
      mout<<"  -->  "<<(*m_QualityFactorsIterator).first<<endl;
    }
  }

  if (NGoodSequences == 0) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRNoGoodCombination);
    mout<<"CSR - Sequence: None of the sequences is valid!"<<endl;
    return;    
  }

  // Find best sequences:
  vector<MRESE*> BestSequence;
  double BestQualityFactor =  c_CSRFailed;
  double SecondBestQualityFactor = c_CSRFailed;

  // The good ones are expected to be the small ones...
  // The design is that we have always at least to permutations here...
  
  BestSequence = (*m_QualityFactors.begin()).second;
  BestQualityFactor = (*m_QualityFactors.begin()).first;
  if (NGoodSequences > 1) {
    SecondBestQualityFactor = (*(++m_QualityFactors.begin())).first;
  }

  // Test if we have at least one good sequence:
  if (BestQualityFactor == c_CSRFailed) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRNoGoodCombination);
    mout<<"CSR - Sequence: None of the sequences is valid!"<<endl;
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
  // m_List->SetBestEvent(RE); // Problem wenn mehrere RE zu vergleichen sind...


  // Check if the threshold is ok:
  if (BestQualityFactor < m_QualityFactorMin || 
      BestQualityFactor > m_QualityFactorMax) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionCSRThreshold);
    mout<<"CSR - Sequence: Teststatistics ("<<BestQualityFactor
        <<") out of threshold ("<<m_QualityFactorMin<<" - "<<m_QualityFactorMax<<")"<<endl;
    return;
  }

  // Check if start in tracker is necessary:
  if (BestSequence[0]->GetDetector() != 1 && 
      BestSequence[0]->GetDetector() != 5 && 
      m_GuaranteeStartD1 == true) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionEventStartNotD1);
    mout<<"CSR - Sequence: Event starts in not in D1/D5 but in "<<BestSequence[0]->GetDetector()<<endl;
    mout<<"CSR - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
    return;
  }
 
  // Check if track is ok:
  double E1, E2 = 0;
  E1 = BestSequence[0]->GetEnergy();
  for (unsigned int i = 1; i < BestSequence.size(); ++i) {
    E2 += BestSequence[i]->GetEnergy();
  }
  E2 += EscapedEnergy;
  if (BestSequence[0]->GetType() == MRESE::c_Track && 
      RE->TestElectronDirection(E1, E2) == false) {
    RE->SetRejectionReason(MRERawEvent::c_RejectionElectronDirectionBad);
    mout<<"CSR - Sequence: Electron direction test failed!"<<endl;
    mout<<"CSR - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
    return;
  }

  mout<<"CSR - Sequence: Good event with TS: "<<BestQualityFactor<<endl;
    
  RE->SetComptonQualityFactors(BestQualityFactor, SecondBestQualityFactor);
  RE->SetEventType(MRERawEvent::c_ComptonEvent);
  RE->SetGoodEvent(true);      
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::GetEscapedEnergy(vector<MRESE*>& RESEs)
{
  // The class is not searching for escaped energy!
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSR::FindPermutations(vector<MRESE*> RESEs, int Level, 
                              vector<vector<MRESE*> >& Permutations)
{
  // The basic formula to get all permutations is:
  // n = 2: 2> 2>
  // n = 3: 3(2> 2>) x 3
  // n = 4: 4(3(2> 2>) x 3) x 4, i.e.
  //        432> 2> 32> 2> 32> 2> 432> 2> 32> 2> 32> 2> 
  //        432> 2> 32> 2> 32> 2> 432> 2> 32> 2> 32> 2>  
  // while i = 2,3,4 means leftshift by i
  //
  // This function should be replace by the standard library function one day...

  if (Level < 2) {
    Permutations.push_back(RESEs);   
    return; 
  } else if (Level == 2) {

    LeftShift(RESEs, Level);
    Permutations.push_back(RESEs);
    LeftShift(RESEs, Level);
    Permutations.push_back(RESEs);

  } else {

    for (int i = 0; i < Level; i++) { // Level times repeat ...
      LeftShift(RESEs, Level);
      FindPermutations(RESEs, Level-1, Permutations);
    }  
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSR::LeftShift(vector<MRESE*>& RESEs, int Level)
{
  // Leftshift the first Level elements of RESEs
  //
  // Example: Leftshift("1,2,3,4", 3) --> "2,3,1,4"

  MRESE* RESE = RESEs[0];
  for (int i = 0; i < Level-1; i++) {
    RESEs[i] = RESEs[i+1];
  }
  RESEs[Level-1] = RESE;
  
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


int MERCSR::ComputeAllQualityFactors(MRERawEvent* RE)
{
  // This function computes all quality factors
  // At the end of this function call, the m_QualityFactors map has to be 
  // filled with permutations and their quality!

  vector<MRESE*> RESEs(RE->GetNRESEs());
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    RESEs[i] = RE->GetRESEAt(i);
  }

  // Now we are in some programming trouble:
  // We have to evaluate all possible permutations of first degree 
  // sub elements in raw event:
  // For 4 hits there are 24 permutations which can be evaluated via this
  // shifting algorithm:
  // 4 times do: 432->32->2->32->2->32->2->
  // ("->" means: evaluate; 4 left shift the first 4 elements, 
  // 3 left shift 3... and so on.)

  // First find all different permutations:
  vector<vector<MRESE*> > Permutations;
  FindPermutations(RESEs, RESEs.size(), Permutations);

  // Calculate quality factor:
  int NGoodPermutations = 0;
  double QualityFactor = c_CSRFailed;
  m_QualityFactors.clear();
  for (unsigned int c = 0; c < Permutations.size(); ++c) {
    QualityFactor = ComputeQualityFactor(Permutations[c]);
    if (QualityFactor != c_CSRFailed) {
      m_QualityFactors.insert(map<double, vector<MRESE*>, less_equal<double> >::value_type(QualityFactor, Permutations[c]));
      NGoodPermutations++;
    }
  }

  return NGoodPermutations;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::ComputeQualityFactor(vector<MRESE*>& Interactions)
{
  merr<<"This function needs to be derived!"<<endl;

  return c_CSRFailed;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::ComputePositionError(MRESE* First, MRESE* Second, MRESE* Third)
{
  // Elena Novikova's version:

  // This is very time critical!

  double Ax = First->GetPositionX();
  double Ay = First->GetPositionY();
  double Az = First->GetPositionZ();
  double Bx = Second->GetPositionX();
  double By = Second->GetPositionY();
  double Bz = Second->GetPositionZ();
  double Cx = Third->GetPositionX();
  double Cy = Third->GetPositionY();
  double Cz = Third->GetPositionZ();

  double dAx = First->GetPositionResolutionX();
  double dAy = First->GetPositionResolutionY();
  double dAz = First->GetPositionResolutionZ();
  double dBx = Second->GetPositionResolutionX();
  double dBy = Second->GetPositionResolutionY();
  double dBz = Second->GetPositionResolutionZ();
  double dCx = Third->GetPositionResolutionX();
  double dCy = Third->GetPositionResolutionY();
  double dCz = Third->GetPositionResolutionZ();
  
  double Vx = Ax - Bx;
  double Vy = Ay - By;
  double Vz = Az - Bz;
  double Ux = Bx - Cx;
  double Uy = By - Cy;
  double Uz = Bz - Cz;
  double UdotV = Ux*Vx + Uy*Vy + Uz*Vz;
  double lengthV2 = Vx*Vx + Vy*Vy + Vz*Vz;
  double lengthU2 = Ux*Ux + Uy*Uy + Uz*Uz;  
  double lengthV = sqrt(lengthV2);
  double lengthU = sqrt(lengthU2);
  
  double lengthVlengthU = lengthV * lengthU;  
  double lengthV3lengthU = lengthV2 * lengthVlengthU;
  double lengthVlengthU3 = lengthVlengthU * lengthU2;
  
  double DCosThetaDx1 = (Vx-Ux)/lengthVlengthU - Ux*UdotV/lengthVlengthU3+
    Vx*UdotV/lengthV3lengthU;
  double DCosThetaDx2 = Ux*UdotV/lengthVlengthU3-Vx/lengthVlengthU;
  double DCosThetaDx  =-Vx*UdotV/lengthV3lengthU+Ux/lengthVlengthU;
  double DCosThetaDy1 = (Vy-Uy)/lengthVlengthU - Uy*UdotV/lengthVlengthU3+
    Vy*UdotV/lengthV3lengthU;
  double DCosThetaDy2 = Uy*UdotV/lengthVlengthU3-Vy/lengthVlengthU;
  double DCosThetaDy  =-Vy*UdotV/lengthV3lengthU+Uy/lengthVlengthU;
  double DCosThetaDz1 = (Vz-Uz)/lengthVlengthU - Uz*UdotV/lengthVlengthU3+
    Vz*UdotV/lengthV3lengthU;
  double DCosThetaDz2 = Uz*UdotV/lengthVlengthU3-Vz/lengthVlengthU;
  double DCosThetaDz  =-Vz*UdotV/lengthV3lengthU+Uz/lengthVlengthU;
    
  double deltaF = sqrt(DCosThetaDx1*DCosThetaDx1 * dBx*dBx + DCosThetaDy1*DCosThetaDy1 * dBy*dBy + 
                       DCosThetaDz1*DCosThetaDz1 * dBz*dBz +
                       DCosThetaDx2*DCosThetaDx2 * dCx*dCx + DCosThetaDy2*DCosThetaDy2 * dCy*dCy + 
                       DCosThetaDz2*DCosThetaDz2 * dCz*dCz +
                       DCosThetaDx *DCosThetaDx  * dAx*dAx + DCosThetaDy *DCosThetaDy  * dAy*dAy + 
                       DCosThetaDz *DCosThetaDz  * dAz*dAz);                                

  if (deltaF == 0) {
    // In case they are on a straight line the above fails (no problem since this is anyway no good Compton event):
    if (fabs(UdotV/lengthU/lengthV) - 1 < 1E-10) {
      // Let's do an approximation
      double AvgResA = sqrt(dAx*dAx + dAy*dAy + dAz*dAz);
      double AvgResB = sqrt(dBx*dBx + dBy*dBy + dBz*dBz);
      double AvgResC = sqrt(dCx*dCx + dCy*dCy + dCz*dCz);

      deltaF = fabs(cos(atan((AvgResA+AvgResB)/lengthV) + atan((AvgResB+AvgResC)/lengthU)));

      //mout<<"Using crude position error approximation for hits on a straight line: "<<deltaF<<endl;
    }
  }

  return deltaF;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::CalculatePhotoDistance(const MVector& Start, 
                                              const MVector& Stop, double Etot)
{
  double Distance = m_Geometry->GetPhotoAbsorptionProbability(Start, Stop, Etot); 
  
  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::CalculateComptonDistance(const MVector& Start,
                                                const MVector& Stop, double Etot)
{
  double Distance = m_Geometry->GetComptonAbsorptionProbability(Start, Stop, Etot);

  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::CalculateReach(const MVector& Start, const MVector& Stop, double Etot)
{
  double ReachProbability = 1.0;
  ReachProbability *= (1 - m_Geometry->GetPairAbsorptionProbability(Start, Stop, Etot));
  ReachProbability *= (1 - m_Geometry->GetComptonAbsorptionProbability(Start, Stop, Etot));
  ReachProbability *= (1 - m_Geometry->GetPhotoAbsorptionProbability(Start, Stop, Etot));

  return ReachProbability;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSR::CalculateTotalDistance(const MVector& Start, 
                                              const MVector& Stop, double Etot)
{
  double Distance = m_Geometry->GetAbsorptionProbability(Start, Stop, Etot); 
  
  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSR::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# QualityFactorMin:                   "<<m_QualityFactorMin<<endl;
  out<<"# QualityFactorMax:                    "<<m_QualityFactorMax<<endl;
  out<<"# MaxNInteractions:                    "<<m_MaxNInteractions<<endl;
  out<<"# "<<endl;
  
  return out.str().c_str();
}


// MERCSR.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
