/*
 * MERStripPairing.cxx
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
// MERStripPairing
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERStripPairing.h"

// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MRESE.h"
#include "MREStripHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERStripPairing)
#endif


////////////////////////////////////////////////////////////////////////////////


MERStripPairing::MERStripPairing()
{
  // Construct an instance of MERStripPairing

  m_Algorithm = c_ChiSquare;

  m_NFoundStripPairings = 0;
}


////////////////////////////////////////////////////////////////////////////////


MERStripPairing::~MERStripPairing()
{
  // Delete this instance of MERStripPairing
}


////////////////////////////////////////////////////////////////////////////////


vector<vector<vector<unsigned int>>> MERStripPairing::FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MREStripHit*> StripHits)
{
  vector<vector<vector<unsigned int>>> NewOnes; // <list> of <combinations> of <combined strips>
  
  for (unsigned int listspot = 0; listspot < OldOnes.size(); ++listspot) {
    // New single merges
    for (unsigned int combi1 = 0; combi1 < OldOnes[listspot].size(); ++combi1) {
      for (unsigned int combi2 = combi1+1; combi2 < OldOnes[listspot].size(); ++combi2) {
        vector<unsigned int> NewCombinedStrips;
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi1].begin(), OldOnes[listspot][combi1].end());
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi2].begin(), OldOnes[listspot][combi2].end());
        sort(NewCombinedStrips.begin(), NewCombinedStrips.end());
        
        vector<unsigned int> NewCombinedAsIDs;
        for (unsigned int s = 0; s < NewCombinedStrips.size(); ++s) {
          NewCombinedAsIDs.push_back(StripHits[NewCombinedStrips[s]]->GetStripID());
        }
        sort(NewCombinedAsIDs.begin(), NewCombinedAsIDs.end());
        
        bool AllAdjacent = true;
        for (unsigned int c = 1; c < NewCombinedAsIDs.size(); ++c) {
          if (NewCombinedAsIDs[c-1] + 1 != NewCombinedAsIDs[c]) {
            AllAdjacent = false;
            break;
          }
        }
        
        if (AllAdjacent == true) {
          vector<vector<unsigned int>> NewCombo;
          for (unsigned int news = 0; news < OldOnes[listspot].size(); ++news) {
            if (news != combi1 && news != combi2) { 
              NewCombo.push_back(OldOnes[listspot][news]);
            }
            if (news == combi1) {
              NewCombo.push_back(NewCombinedStrips);
            }
          }
          NewOnes.push_back(NewCombo);
        }
      }
    }
  }
  
  return NewOnes;
}


////////////////////////////////////////////////////////////////////////////////
  
  
bool MERStripPairing::Analyze(MRawEventIncarnationList* List)
{
  // Search for coincidences

  //cout<<"StripPairing started for "<<List->Get(0)->GetRawEventAt(0)->GetEventID()<<endl;

  unsigned int MaxCombinations = 5;
  
  MRERawEvent* RE = 0;

  if (m_Algorithm == c_None) {
    //cout<<"No strip pairing search"<<endl;
  } else if (m_Algorithm == c_ChiSquare) {
    //cout<<"StripPairing chi square"<<endl;
    unsigned int r_max = List->Size();
    for (unsigned int r = 0; r < r_max; ++r) {
      MRawEventIncarnations* REI = List->Get(r);
      if (REI->GetNRawEvents() != 1) {
        merr<<"Event clustering: Only one raw event incarnation can be present at this stage of the analysis"<<endl;
        //RE->SetRejectionReason(MRERawEvent::c_RejectionTooManyEventIncarnations);  // This automatically marks the event invalid for further analysis...
        continue; 
      }
      
      RE = REI->GetRawEventAt(0);
      
      //cout<<RE->ToString()<<endl;
      
      bool Rejected = false;
      
      // (1) Split hits by detector ID
      vector<long> DetectorIDs; 
      vector<vector<vector<MREStripHit*>>> StripHits; // list of detector ID, list of sides, list of hits
      
      for (int r = 0; r < RE->GetNRESEs(); ++r) {
        if (RE->GetRESEAt(r)->GetType() == MRESE::c_StripHit) { 
          MREStripHit* SH = dynamic_cast<MREStripHit*>(RE->GetRESEAt(r));
          unsigned int Side = (SH->IsXStrip() == true) ? 0 : 1;
          
          // Check if detector is on list
          bool DetectorFound = false;
          unsigned int DetectorPos = 0;
          for (unsigned int d = 0; d < DetectorIDs.size(); ++d) {
            if (DetectorIDs[d] == SH->GetDetectorID()) {
              DetectorFound = true; 
              DetectorPos = d;
            }
          }
          
          if (DetectorFound == true) {
            StripHits[DetectorPos][Side].push_back(SH);
          } else {
            vector<vector<MREStripHit*>> List;
            List.push_back(vector<MREStripHit*>()); // X
            List.push_back(vector<MREStripHit*>()); // Y
            List[Side].push_back(SH); 
            StripHits.push_back(List);
            DetectorIDs.push_back(SH->GetDetectorID());
          }
        }
      }
      
      // Limit the strip hits 
      const unsigned int MaxStripHits = 6;
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        for (unsigned int side = 0; side <=1; ++side) { // side loop
          if (StripHits[d][side].size() > MaxStripHits) {
            RE->SetRejectionReason(MRERawEvent::c_RejectionStripPairinTooManyStrips);
            Rejected = true;
            break;
          }
        }
      }
      
      
      /*
      cout<<"Strip hits: "<<endl;
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        cout<<"Detector: "<<d<<endl;
        for (unsigned int side = 0; side <=1; ++side) { // side loop
          cout<<"  Side: "<<side<<endl;
          if (StripHits[d][side].size() > 0) {
            cout<<"    Hits: "<<endl;;
            for (unsigned int sh = 0; sh < StripHits[d][side].size(); ++sh) { // side loop
              cout<<StripHits[d][side][sh]->ToString();
            }
          } else {
            cout<<"no hits"<<endl; 
          }
        }
      }
      */
           
      if (Rejected == true) {
        continue;
      }


      // Remove the strip hits
      RE->RemoveAllAndCompress();
      
      // (2) Check if we have enough strips and enough energy for each detector
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        
        if (StripHits[d][0].size() == 0 || StripHits[d][1].size() == 0) {
          RE->SetRejectionReason(MRERawEvent::c_RejectionStripPairingMissingStrips);
          Rejected = true;
          break;
        }
        
        double xEnergy = 0;
        double xEnergyRes = 0;
        for (unsigned int sh = 0; sh < StripHits[d][0].size(); ++sh) {
          xEnergy += StripHits[d][0][sh]->GetEnergy();
          xEnergyRes += StripHits[d][0][sh]->GetEnergyResolution()*StripHits[d][0][sh]->GetEnergyResolution();
        }
        double yEnergy = 0;
        double yEnergyRes = 0;
        for (unsigned int sh = 0; sh < StripHits[d][1].size(); ++sh) {
          yEnergy += StripHits[d][1][sh]->GetEnergy();
          yEnergyRes += StripHits[d][1][sh]->GetEnergyResolution()*StripHits[d][1][sh]->GetEnergyResolution();
        }
        
        //cout<<"Energies: "<<xEnergy<<":"<<xEnergyRes<<" -- "<<yEnergy<<":"<<yEnergyRes<<endl;
      }
       
      if (Rejected == true) {
        continue; 
      }
        
      
      // (3) Find all possible combinations
      vector<vector<vector<vector<vector<unsigned int>>>>> Combinations;  // list of detector IDs, list of sides, list of combinations; combination with list of combined strips
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
                
        Combinations.push_back(vector<vector<vector<vector<unsigned int>>>>());
        for (unsigned int s = 0; s <= 1 ; ++s) {
          Combinations[d].push_back(vector<vector<vector<unsigned int>>>()); // X
          Combinations[d].push_back(vector<vector<vector<unsigned int>>>()); // Y      
        }
        // Create the seed combinations
        for (unsigned int s = 0; s < StripHits[d].size(); ++s) {
          vector<vector<unsigned int>> Combination;
          for (unsigned int h = 0; h < StripHits[d][s].size(); ++h) {
            vector<unsigned int> CombinedStrips = { h };
            sort(CombinedStrips.begin(), CombinedStrips.end());
            Combination.push_back(CombinedStrips);
          }
          Combinations[d][s].push_back(Combination);
        }
      }
        
      // Starting from this seed, find more new combinations
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        for (unsigned int side = 0; side <=1; ++side) { // side loop
          
          vector<vector<vector<unsigned int>>> NewCombinations;
          
          bool CombinationsAdded = true;
          while (CombinationsAdded == true) {
            CombinationsAdded = false;

            NewCombinations = FindNewCombinations(Combinations[d][side], StripHits[d][side]);
            //cout<<"Size: "<<NewCombinations.size()<<endl;
            
            // Find equal combinations and eliminate them from the new list
            for (unsigned int c = 0; c < Combinations[d][side].size(); ++c) { 
              auto Iter = NewCombinations.begin();
              while (Iter != NewCombinations.end()) {
                if (Combinations[d][side][c] == (*Iter)) {
                  bool Equal = true;
                  for (unsigned int deep = 0; deep < Combinations[d][side][c].size(); ++deep) {
                    if (Combinations[d][side][c][deep] != (*Iter)[deep]) {
                      Equal = false;
                      break;
                    }
                  }
                  if (Equal == true) {
                    Iter = NewCombinations.erase(Iter);
                  } else {
                    Iter++; 
                  }
                } else {
                  Iter++; 
                }
              }
            }
            // If there are new combinations left, add them, and restart
            if (NewCombinations.size() > 0) {
              //cout<<NewCombinations.size()<<" new combinations found"<<endl;
              for (auto C: NewCombinations) {
                Combinations[d][side].push_back(C);
              }
              CombinationsAdded = true;
            }
          } // combination search
        } // side loop
        
        /*
        cout<<"All combinations:"<<endl;
        for (unsigned int xc = 0; xc < Combinations[d][0].size(); ++xc) {
          cout<<"X - "<<xc<<": ";
          for (unsigned h = 0; h < Combinations[d][0][xc].size(); ++h) {
            cout<<" (";
            for (unsigned int sh = 0; sh < Combinations[d][0][xc][h].size(); ++sh) {
              cout<<Combinations[d][0][xc][h][sh]<<" ";
            }
            cout<<")";
          }
          cout<<endl;
        }
        for (unsigned int yc = 0; yc < Combinations[d][1].size(); ++yc) {
          cout<<"Y - "<<yc<<": ";
          for (unsigned h = 0; h < Combinations[d][1][yc].size(); ++h) {
            cout<<" (";
            for (unsigned int sh = 0; sh < Combinations[d][1][yc][h].size(); ++sh) {
              cout<<Combinations[d][1][yc][h][sh]<<" ";
            }
            cout<<")";
          }
          cout<<endl;
        }
        */
        
        // (3) Evaluate all combinations
        // All strip combinations for one side have been found, now check for the best x-y combinations
        double BestChiSquare = numeric_limits<double>::max();
        vector<vector<unsigned int>> BestXSideCombo;
        vector<vector<unsigned int>> BestYSideCombo;
        
        for (unsigned int xc = 0; xc < Combinations[d][0].size(); ++xc) {
          for (unsigned int yc = 0; yc < Combinations[d][1].size(); ++yc) {
            
            if (abs(long(Combinations[d][0][xc].size()) - long(Combinations[d][1][yc].size())) > 1) {
              continue;
            }
            
            unsigned int MinSize = min(Combinations[d][0][xc].size(), Combinations[d][1][yc].size());
            
            if (max(Combinations[d][0][xc].size(), Combinations[d][1][yc].size()) > MaxCombinations) {
              continue;
            }
            
            bool MorePermutations = true;
            while (MorePermutations == true) {
              //cout<<"New permutation..."<<endl;
              //         if (Combinations[d][1][yc].size() > Combinations[d][0][xc].size()) {
              //           PrintCombi(Combinations[d][1][yc]);
              //         } else {
              //           PrintCombi(NCombi[p]);
              //         }
              double ChiSquare = 0;
              
              for (unsigned int en = 0; en < MinSize; ++en) {
                unsigned int ep = en;
                
                double xEnergy = 0;
                double xResolution = 0;
                for (unsigned int entry = 0; entry < Combinations[d][0][xc][en].size(); ++entry) {
                  xEnergy += StripHits[d][0][Combinations[d][0][xc][en][entry]]->GetEnergy();
                  xResolution += pow(StripHits[d][0][Combinations[d][0][xc][en][entry]]->GetEnergyResolution(), 2);
                } 
                
                double yEnergy = 0;
                double yResolution = 0;
                for (unsigned int entry = 0; entry < Combinations[d][1][yc][ep].size(); ++entry) {
                  yEnergy += StripHits[d][1][Combinations[d][1][yc][ep][entry]]->GetEnergy();
                  yResolution += pow(StripHits[d][1][Combinations[d][1][yc][ep][entry]]->GetEnergyResolution(), 2);
                } 
                //cout << "yEnergy: " << yEnergy << endl;
                //cout << "  Sub - Test en=" << en << " (" << xEnergy << ") with ep="
                //     << ep << " (" << yEnergy << "):" << endl;
                //cout<<xResolution<<":"<<yResolution<<endl;
                ChiSquare += (xEnergy - yEnergy)*(xEnergy - yEnergy) / (xResolution + yResolution);            
              }
              ChiSquare /= MinSize;
              //cout<<"Chi square: "<<ChiSquare<<endl;
              
              if (ChiSquare < BestChiSquare) {
                BestChiSquare = ChiSquare;
                BestXSideCombo = Combinations[d][0][xc];
                BestYSideCombo = Combinations[d][1][yc];
              }
              
              //cout<<"ChiSquare: "<<ChiSquare<<endl;
              
              if (Combinations[d][1][yc].size() > Combinations[d][0][xc].size()) {
                MorePermutations = next_permutation(Combinations[d][1][yc].begin(), Combinations[d][1][yc].end());
              } else {
                MorePermutations = next_permutation(Combinations[d][0][xc].begin(), Combinations[d][0][xc].end());
              } 
            }
          }
        }
        
        /*
        cout<<"Best combo:"<<endl;
        cout<<"X: ";
        for (unsigned h = 0; h < BestXSideCombo.size(); ++h) {
          cout<<" (";
          for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
            cout<<BestXSideCombo[h][sh]<<" ";
          }
          cout<<")";
        }
        cout<<endl;
        cout<<"Y: ";
        for (unsigned h = 0; h < BestYSideCombo.size(); ++h) {
          cout<<" (";
          for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
            cout<<BestYSideCombo[h][sh]<<" ";
          }
          cout<<")";
        }
        cout<<endl;
        */
        
        // Now create hits:
        if (BestChiSquare == numeric_limits<double>::max()) {
          RE->SetRejectionReason(MRERawEvent::c_RejectionStripPairingMissingStrips);
          Rejected = true;
          break;
        }
            
        // Create the hits
        double XPos = 0;
        double YPos = 0;
        double XEnergy = 0;
        double XEnergyRes = 0;
        double YEnergy = 0;
        double YEnergyRes = 0;
        double Energy = 0;
        double EnergyResolution = 0;
        double ZPos = 0;
        double MinTime = 0;
        
        double XEnergyTotal = 0;
        double YEnergyTotal = 0;
        double EnergyTotal = 0;
        
        for (unsigned int h = 0; h < min(BestXSideCombo.size(), BestYSideCombo.size()); ++h) {
          XPos = 0;
          YPos = 0;
          XEnergy = 0;
          YEnergy = 0;
          ZPos = 0;
          MinTime = numeric_limits<double>::max();
          
          for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
            //cout<<"x-pos: "<<StripHits[d][0][BestXSideCombo[h][sh]]->GetNonStripPosition()<<endl;
            XPos += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergy() * StripHits[d][0][BestXSideCombo[h][sh]]->GetNonStripPosition();
            ZPos += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergy() * StripHits[d][0][BestXSideCombo[h][sh]]->GetDepthPosition();
            XEnergy += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergy();
            XEnergyRes += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergyResolution()*StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergyResolution();
            if (StripHits[d][0][BestXSideCombo[h][sh]]->GetTime() < MinTime) {
              MinTime = StripHits[d][0][BestXSideCombo[h][sh]]->GetTime();
            }
          }
          XPos /= XEnergy;
          XEnergyRes = sqrt(XEnergyRes);
          XEnergyTotal += XEnergy;
          
          for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
            YPos += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergy() * StripHits[d][1][BestYSideCombo[h][sh]]->GetNonStripPosition();
            ZPos += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergy() * StripHits[d][1][BestYSideCombo[h][sh]]->GetDepthPosition();
            YEnergy += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergy();
            YEnergyRes += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergyResolution()*StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergyResolution();
            if (StripHits[d][1][BestYSideCombo[h][sh]]->GetTime() < MinTime) {
              MinTime = StripHits[d][1][BestYSideCombo[h][sh]]->GetTime();
            }
          }
          YPos /= YEnergy;
          YEnergyRes = sqrt(YEnergyRes);
          YEnergyTotal += YEnergy;
          
          ZPos /= (XEnergy + YEnergy);
          
          Energy = 0.0;
          if (XEnergy > YEnergy + 3*YEnergyRes) {
            Energy = XEnergy; 
            EnergyResolution = XEnergyRes;
          } else if (YEnergy > XEnergy + 3*XEnergyRes) {
            Energy = YEnergy; 
            EnergyResolution = YEnergyRes;
          } else {
            Energy = (XEnergy/(XEnergyRes*XEnergyRes) + YEnergy/(YEnergyRes*YEnergyRes)) / (1.0/(XEnergyRes*XEnergyRes) + 1.0/(YEnergyRes*YEnergyRes));
            EnergyResolution = sqrt( 1.0 / (1.0/(XEnergyRes*XEnergyRes) + 1.0/(YEnergyRes*YEnergyRes)) );
          }
          EnergyTotal += Energy;
          
          //cout<<"Energy: "<<Energy<<"  "<<XEnergy<<" vs. "<<YEnergy<<" ("<<XEnergyRes<<" vs. "<<YEnergyRes<<")"<<endl;
          
          MVector PosDet(XPos, YPos, ZPos);
          
          MVector PositionResolution = StripHits[d][1][BestYSideCombo[h][0]]->GetVolumeSequence()->GetDetector()->GetPositionResolution(PosDet, EnergyTotal);
          
          MVector PosWorld = StripHits[d][1][BestYSideCombo[h][0]]->GetVolumeSequence()->GetPositionInFirstVolume(PosDet, StripHits[d][1][BestYSideCombo[h][0]]->GetVolumeSequence()->GetDetectorVolume());
          
          double TimeResolution = StripHits[d][1][BestYSideCombo[h][0]]->GetVolumeSequence()->GetDetector()->GetTimeResolution(EnergyTotal);
          
          MREHit* Hit = new MREHit();
          Hit->SetEnergy(Energy);
          Hit->SetPosition(PosWorld);
          Hit->SetTime(MinTime);
          Hit->SetPositionResolution(PositionResolution);
          Hit->SetEnergyResolution(EnergyResolution);
          Hit->SetTimeResolution(TimeResolution);
          Hit->FixResolutions(true);
          Hit->SetVolumeSequence(new MDVolumeSequence(*StripHits[d][1][BestYSideCombo[h][0]]->GetVolumeSequence()));
          Hit->SetDetector(StripHits[d][1][BestYSideCombo[h][0]]->GetDetector());
          
          set<unsigned int> XOrigins;
          for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
            set<unsigned int> SHOrigins = StripHits[d][0][BestXSideCombo[h][sh]]->GetOriginIDs();
            XOrigins.insert(SHOrigins.begin(), SHOrigins.end());
          }
          
          set<unsigned int> YOrigins;
          for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
            set<unsigned int> SHOrigins = StripHits[d][1][BestYSideCombo[h][sh]]->GetOriginIDs();
            YOrigins.insert(SHOrigins.begin(), SHOrigins.end());
            //cout<<"Strip origin IDs: "; for (auto I: StripHits[d][1][BestYSideCombo[h][sh]]->GetOriginIDs()) cout<<I<<" "; cout<<endl;
          }
          
          // The origins can only be the origins which are common to both
          bool FoundCommon = false;
          for (auto IterX = XOrigins.begin(); IterX != XOrigins.end(); ++IterX) {
            for (auto IterY = XOrigins.begin(); IterY != XOrigins.end(); ++IterY) {
              if (*IterX == *IterY) {
                Hit->AddOriginID(*IterX);
                FoundCommon = true;
              }
              
            }
          }
          // If there are non common, take all, which
          if (FoundCommon == false) {
            Hit->AddOriginIDs(XOrigins);
            Hit->AddOriginIDs(YOrigins);
          }
          
          //cout<<"Origin IDs: "; for (auto I: Hit->GetOriginIDs()) cout<<I<<" "; cout<<endl;
          
          RE->AddRESE(Hit);
          
          
          //cout<<Hit->ToString()<<endl;
        }
        
        if (EnergyTotal > max(XEnergyTotal, YEnergyTotal) + 2.5*max(XEnergyRes, YEnergyRes) || EnergyTotal < min(XEnergyTotal, YEnergyTotal) - 2.5*max(XEnergyRes, YEnergyRes)) {
          //cout<<"Rejected"<<endl:;
          //cout<<"Strip-pairing: Rejected: not resolvable by energy: tot="<<EnergyTotal<<"  x:"<<XEnergyTotal<<" y:"<<YEnergyTotal<<"  max res: "<<2.5*max(XEnergyRes, YEnergyRes)<<endl;
          RE->SetRejectionReason(MRERawEvent::c_RejectionStripPairingNotResolvable);
          Rejected = true;
          break;
        }
        
        // 
        
      } // detector loop
      
      // Delete all strip hits:
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        for (unsigned int side = 0; side <=1; ++side) { // side loop
          for (unsigned int sh = 0; sh < StripHits[d][side].size(); ++sh) { // strip hit loop
            delete StripHits[d][side][sh];
          }
        }
      }
              
      //cout<<"After:"<<endl;
      //cout<<RE->ToString()<<endl;
      
      
    } // raw events loop
  } else {
    merr<<"Unknown strip pairing algorithm with ID="<<m_Algorithm<<endl;
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MERStripPairing::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# StripPairing options:"<<endl;
  out<<"# "<<endl;
  if (m_Algorithm == c_ChiSquare) {
    out<<"# StripPairing window: "<<m_Window.GetAsSeconds()<<endl;
  }
  out<<"# "<<endl;

  return out.str().c_str();
}

// MERStripPairing.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
