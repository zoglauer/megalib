/*
 * MCActivator.cxx
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
#include "MCActivator.hh"
#include "MCActivatorParticle.hh"
#include "MCSteppingAction.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"

// ROOT:
#include "TRandom.h"
#include "TMath.h"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4RadioactiveDecay.hh"
#include "G4ParticleDefinition.hh"
#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"
#include "G4ParticleTable.hh"
#include "G4RIsotopeTable.hh"
#include "G4IsotopeProperty.hh"
#include "G4NuclearLevelManager.hh"
#include "G4NuclearLevel.hh"
#include "G4NuclearLevelStore.hh"
#include "G4IonTable.hh"

// Standard lib:
#include <limits>

/******************************************************************************/

const unsigned int MCActivator::c_ConstantIrradiation = 0;
const unsigned int MCActivator::c_ConstantIrradiationWithCooldown = 1;
const unsigned int MCActivator::c_TimeProfile = 2;

/******************************************************************************
 * Not yet implemented
 */
MCActivator::MCActivator()
{
  m_OutputFileName = "Activation.act";  
  m_OutputModeActivities = true;
  m_ActivationMode = c_ConstantIrradiation;
  m_ActivationTime = 0.0;
  m_CooldownTime = 0.0;

  // If the half life is lower than this value, then the decay is assumed to be immediate
  m_HalfLifeCutOff = 1*ns;  
}


/******************************************************************************
 * Not yet implemented
 */
MCActivator::~MCActivator()
{
  // Intentionally left blank
}


/******************************************************************************
 * Set the activation mode ConstantIrradiation 
 */
bool MCActivator::SetConstantIrradiation(double ActivationTime)
{
  if (ActivationTime <= 0) {
    mout<<"Activator: ActivationTime must be positive"<<endl;   
    return false;
  }

  m_ActivationMode = c_ConstantIrradiation;  
  m_ActivationTime = ActivationTime;
  m_CooldownTime = 0.0;
  
  return true;
}


/******************************************************************************
 * Set the activation mode ConstantIrradiationWithCoolDown 
 */
bool MCActivator::SetConstantIrradiationWithCooldown(double ActivationTime, double CooldownTime)
{
  if (ActivationTime <= 0) {
    mout<<"Activator: ActivationTime must be positive"<<endl;   
    return false;
  }
  if (CooldownTime <= 0) {
    mout<<"Activator: CooldownTime must be positive"<<endl;   
    return false;
  }
  m_ActivationMode = c_ConstantIrradiationWithCooldown;
  m_ActivationTime = ActivationTime;
  m_CooldownTime = CooldownTime;

  return true;
}


/******************************************************************************
 * Set the activation mode TimeProfile
 */
bool MCActivator::SetTimeProfile(G4String TimeProfile, double ActivationTime)
{
  if (ActivationTime <= 0) {
    mout<<"Activator: ActivationTime must be positive"<<endl;   
    return false;
  }
  if (MFile::Exists(TimeProfile) == false) {
    mout<<"Activator: The time profile file name \""<<TimeProfile<<"\" must exist!"<<endl;   
    return false;
  }
  m_ActivationMode = c_TimeProfile;
  return false;
}

/******************************************************************************
 * Set one of the files containing the counts
 * You can add as many files as you wish
 */
bool MCActivator::AddCountsFile(const MString& File) 
{ 
  if (MFile::Exists(File) == true) {
    m_CountsFiles.push_back(File);
    return true;
  } else {
    merr<<"File does not exist: "<<File<<endl;
    return true;
  }
}

/******************************************************************************
 * Load the counts file
 */
bool MCActivator::LoadCountsFiles()
{
  double TotalTime = 0.0;

  m_Rates.Reset();

  for (unsigned int i = 0; i < m_CountsFiles.size(); ++i) {
    mout<<"Loading and merging isotopes file "<<m_CountsFiles[i]<<endl;
    MCIsotopeStore Counts;
    Counts.Load(m_CountsFiles[i]);

    if (Counts.GetTime() == 0) {
      merr<<"Isotope file does not contain a time for rates calculation: "<<m_CountsFiles[i]<<endl;
      return false;
    }

    cout<<"Counts (before stable removal):"<<endl<<Counts<<endl;
    
    Counts.RemoveStableElements();
    cout<<"Counts (after stable removal):"<<endl<<Counts<<endl;

    TotalTime += Counts.GetTime();

    // Merge with main file
    m_Rates.Add(Counts);
  }
  
  cout<<"*** NEW"<<endl;
  cout<<m_Rates<<endl;
  cout<<"*** NEW"<<endl;
  
  //m_Rates.Save("Merged.dat");

  cout<<"Total time: "<<m_Rates.GetTime()/s<<" sec"<<endl;
  m_Rates.Scale(1.0/TotalTime);
  
  
  return true;
}

  
/******************************************************************************
 * Calculate the activation rates after X time activation
 */
bool MCActivator::CalculateEquilibriumRates()
{
  // Determine the activation after ActivationTime & Cool-down time

  /*
  // Some test
  G4IonTable* IonTable = G4IonTable::GetIonTable();
  int Z = 30;
  for (int A = 60; A <= 72; ++A) {
    G4Ions* Ion = dynamic_cast<G4Ions*>(IonTable->GetIon(Z, A, 0));
    if (Ion == 0) continue;
    //Ion->DumpTable();

    G4RadioactiveDecay* Decay = new G4RadioactiveDecay();
    G4DecayTable* DecayTable = Decay->LoadDecayTable(*Ion);
    if (DecayTable == 0) continue;
    if (DecayTable->entries() == 0) continue;
    DecayTable->DumpInfo();
  }
  return true;
  */
  

  // We utilize G4RadioactiveDecay to retrieve the decay tables
  G4RadioactiveDecay* Decay = new G4RadioactiveDecay();
  G4DecayTable* DecayTable;
  G4String ParticleName;
  G4String ParticleType;

  double HalfLife = 0.0;
  double ExcitationEnergy = 0.0;

  vector<vector<vector<MCActivatorParticle> > > CalculatedTrees;

  for (unsigned int v = 0; v < m_Rates.GetNVolumes(); ++v) {
    cout<<"Volume: "<<m_Rates.GetVolume(v)<<endl;
    for (unsigned int i = 0; i < m_Rates.GetNIDs(v); ++i) {
      cout<<"Start: ID: "<<m_Rates.GetID(v, i)<<"  #Ex: "<<m_Rates.GetNExcitations(v, i)<<endl;      

      if (m_Rates.GetID(v, i) == 72154 || m_Rates.GetID(v, i) == 71154 || m_Rates.GetID(v, i) == 70154 || m_Rates.GetID(v, i) == 73158) {
        cout<<"Warning: This element sends Geant4 in an infinite loop -- skipping!"<<endl;
        continue;
      }



      for (unsigned int e = 0; e < m_Rates.GetNExcitations(v, i); ++e) {
//         if (m_Rates.GetValue(v, i, e) < 0.001/s) {
//           mout<<"Skipped element: "<<m_Rates.GetID(v, i)<<"("<<m_Rates.GetExcitation(v, i, e)/keV<<" - "<<m_Rates.GetValue(v, i, e)*s<<" vs "<<0.001<<") "<<endl;
//           continue;
//         }

        vector<vector<MCActivatorParticle> > Tree;

        cout<<"Start: "<<m_Rates.GetID(v, i)<<"("<<m_Rates.GetNExcitations(v, i)<<"): Excitation: "<<m_Rates.GetExcitation(v, i, e)/keV<<" & LifeTime: "<<m_Rates.GetParticleDefinition(v, i, e)->GetPDGLifeTime()/s<<" sec"<<endl;      

        /* 
        if (m_Rates.GetID(v, i) == 72154) {
          for (unsigned int ii = 0; ii < 100; ++i) {
            cout<<"Warning: This element crashes Geant4.9.2.1 -- skipping!"<<endl; 
          }
          continue;
        }
        */

        // Check if the particle really exists...
        //G4ParticleTable* Table = G4ParticleTable::GetParticleTable();
        G4IonTable* Table = G4IonTable::GetIonTable();
        int ID = m_Rates.GetID(v, i);
        int AtomicNumber = int(ID/1000);
        int AtomicMass = ID - int(ID/1000)*1000;
        if (Table->GetIon(AtomicNumber, AtomicMass, ExcitationEnergy) == 0) {
          mout<<"Error: Unable to find isotope: "<<m_Rates.GetParticleDefinition(v, i, e)->GetParticleName()<<endl;
          continue;
        }

        // Seed the tree of activation chains by the current isotope:
        MCActivatorParticle Particle;
        if (DetermineHalfLife(m_Rates.GetParticleDefinition(v, i, e), HalfLife, ExcitationEnergy, false) == false) {
          mout<<"Error: Unable to determine half life: "<<m_Rates.GetParticleDefinition(v, i, e)->GetParticleName()<<endl;
          continue;
        }
        //cout<<"I: "<<HalfLife<<":"<<ExcitationEnergy<<endl;
        Particle.SetIDAndExcitation(m_Rates.GetID(v, i), ExcitationEnergy);
        Particle.SetProductionRate(m_Rates.GetValue(v, i, e));
        Particle.SetBranchingRatio(1.0);
        Particle.SetHalfLife(HalfLife);

        vector<MCActivatorParticle> NewBranch;
        NewBranch.push_back(Particle);
        Tree.push_back(NewBranch);



        if (Tree.size() == 0) {
          mout<<"Tree empty. Jumping to next element."<<endl;
          continue;        
        }

        DumpTree(Tree, "Tree after start: ");

        // Now we loop over all possible decays until all are found:
        bool TreeChanged = false;
        bool FirstRound = true;
        
        do {
          TreeChanged = false;
          
          DumpTree(Tree, "Tree at restart");
          
          // Step 1: Create new nuclei
          if (FirstRound == false) {
            unsigned int b_max = Tree.size(); // The tree will grow, thus do not go beyond the original max size --- we will visit the new branches later!
            for (unsigned int b = 0; b < b_max; ++b) {
              //if (Tree[b].back().GetHalfLife() == numeric_limits<double>::max()) continue;
              //cout<<Tree[b].back().GetName()<<"  EX: "<<Tree[b].back().GetExcitation()/keV<<"keV --- "<<b<<"/"<<Tree.size()<<endl;
              //DumpTree(Tree);
              if (Tree[b].back().GetHalfLife() == numeric_limits<double>::max()) continue;
              
              //cout<<<<endl;
              DecayTable = Decay->LoadDecayTable(*(Tree[b].back().GetDefinition()));
              if (DecayTable == 0) {
                mout<<"Error: Decay table not found for: "<<m_Rates.GetID(v, i)<<endl;
                continue;
              }
              
              // The trees which we intend to add to this on:
              vector<vector<MCActivatorParticle> > AdditionalTrees;
              
              // Loop over all decay channels, store decayable secondaries
              bool NewBranchAdded = false;
              for (int c = 0; c < DecayTable->entries(); ++c) {
                G4VDecayChannel* Channel = DecayTable->GetDecayChannel(c);
                //Channel->DumpInfo();
                
                //cout<<"  Channel "<<c<<": "<<Channel->GetKinematicsName()<<" with number of daughters: "<<Channel->GetNumberOfDaughters()<<"  BR: "<<Channel->GetBR()<<endl;
                for (int d = 0; d < Channel->GetNumberOfDaughters(); ++d) {
                  G4ParticleDefinition* ParticleDef = Channel->GetDaughter(d);
                  ParticleName = ParticleDef->GetParticleName();
                  ParticleType = ParticleDef->GetParticleType();
                  //cout<<"Daughter: "<<ParticleName<<endl;
                  
                  // If we are not excited and mother equal daughter then we skip it
                  if (Tree[b].back().GetName() == ParticleName && Tree[b].back().GetExcitation() == 0) {
                    cout<<"Error: We have a decay channel into the same particle when the original particle is not excited (Those are misqualified double beta decays)"<<endl;
                    continue; 
                  }
                  
                  if (ParticleType == "nucleus" && ParticleName != "alpha") {
                    if (DetermineHalfLife(ParticleDef, HalfLife, ExcitationEnergy, false) == true) {
                      vector<MCActivatorParticle> ABranch = Tree[b];
                      MCActivatorParticle NewParticle;
                      NewParticle.SetIDAndExcitation(MCSteppingAction::GetParticleType(ParticleDef->GetParticleName()), 
                                                  ExcitationEnergy);
                      if (Tree[b].back().GetBranchingRatio()* Channel->GetBR() > 1.01) {
                        mout<<"Error (DecayLoop): Branching ratio of "<<Tree[b].back().GetName()<<" larger than one: "<<Tree[b].back().GetBranchingRatio()* Channel->GetBR()<<endl;
                        continue;
                      }
                      
                      //cout<<"Adding to branch "<<b<<", after "<<ABranch.back().GetName()<<": "<<ParticleDef->GetParticleName()<<" -> BR="<<Channel->GetBR()<<endl;
                      NewParticle.SetBranchingRatio(Tree[b].back().GetBranchingRatio()* Channel->GetBR());
                      NewParticle.SetHalfLife(HalfLife);
                      
                      //cout<<"ID: "<<MCSteppingAction::GetParticleType(Nucleus->GetParticleName())<<":"<<Nucleus->GetExcitationEnergy()<<endl;
                      ABranch.push_back(NewParticle);
                      AdditionalTrees.push_back(ABranch);
                      NewBranchAdded = true;
                    } else {
                      mout<<"Error: Unable to determine half life: "<<ParticleDef->GetParticleName()<<endl;
                      continue;
                    }
                  }
                }
              
              }
              if (NewBranchAdded == true) {
                // Empty the current branch for later deletion:
                Tree[b].clear();
                CleanDecayChains(AdditionalTrees);
                for (unsigned int a = 0; a <  AdditionalTrees.size(); ++a) {
                  Tree.push_back(AdditionalTrees[a]); 
                }
                TreeChanged = true;
              }
            } // Master tree loop
            
          
            // Erase empty branches:
            vector<vector<MCActivatorParticle> >::iterator B = Tree.begin();
            while (B != Tree.end()) {
              if ((*B).size() == 0) {
                B = Tree.erase(B);
                continue;
              }
              ++B;
            }
          } else {
            FirstRound = false;
            TreeChanged = true;
          }
          
          // Clean the tree:
          CleanDecayChains(Tree);
          

          // Step two: Go through deexcitation cascade until only non-immidiate decaying nuclei (states) are present:
          // The initial rounds are 
          bool MoreDecays = true;
          bool ImmidiateDecayRound = true;
          int NChanges = 0;
          do {
            
            DumpTree(Tree, "Tree before excitation calculations: ");

            NChanges = 0;
            unsigned int b_max = Tree.size(); // The tree will grow, thus do not go beyond the original max size --- we will visit the new branches later!
            for (unsigned int b = 0; b < b_max; ++b) {
              if (Tree[b].size() == 0 || Tree[b].back().GetExcitation() < 1.0*keV) continue;
              
              // We have to make sure that in the first round only all immidiate decays are handled, which replace individial elements
              // in the chain, only in the second round we ONCE add to the chain and then test immidiate decays again
              if (Tree[b].back().GetHalfLife() > m_HalfLifeCutOff && ImmidiateDecayRound == true) {
                continue;
              }
              
              G4Ions* Nucleus = dynamic_cast<G4Ions*>(Tree[b].back().GetDefinition()); 
              
              bool LevelsOK = true;
              G4NuclearLevelManager* M = G4NuclearLevelStore::GetInstance()->GetManager(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass());
              if (M->IsValid() == true) {
                //M->PrintAll();
                const G4NuclearLevel* NuclearLevel = M->NearestLevel(Nucleus->GetExcitationEnergy());
                cout<<"Nearest level: "<<NuclearLevel->Energy()/keV<<" vs. "<<Nucleus->GetExcitationEnergy()/keV<<endl;
                if (NuclearLevel != 0) {
                  // Create new levels...
                  cout<<"Number of gammas: "<<NuclearLevel->NumberOfGammas()<<endl;
                  for (int h = 0; h < NuclearLevel->NumberOfGammas(); ++h) {
                    vector<MCActivatorParticle> ABranch = Tree[b];
                    cout<<"Gamma energy: "<<NuclearLevel->GammaEnergies()[h]/keV<<endl;
                    
                    double NewLevelEnergy = 0.0;
                    double NewHalfLife = 0.0;
                    if (fabs(NuclearLevel->Energy() - NuclearLevel->GammaEnergies()[h]) > 1*keV && /* NuclearLevel->GammaEnergies()[h] > 1*keV && */
                        NuclearLevel->Energy() != M->HighestLevel()->Energy()) { // table has some significant uncertainties...
                      // Make sure we know the exact energy of the new level:
                      const G4NuclearLevel* NewNuclearLevel = M->NearestLevel(NuclearLevel->Energy() - NuclearLevel->GammaEnergies()[h]);
                      if (NewNuclearLevel != 0) {
                        NewLevelEnergy = NewNuclearLevel->Energy();
                        if (NewNuclearLevel->HalfLife() > m_HalfLifeCutOff) {
                          NewHalfLife = NewNuclearLevel->HalfLife();
                        } else {
                          NewHalfLife = 0.0;
                        }
                      } else {
                        mout<<"Error: No nearest level found for: "<<Nucleus->GetParticleName()<<" Excitation: "<<NuclearLevel->Energy() - NuclearLevel->GammaEnergies()[h]<<endl;
                        mout<<"       This isotope is excluded from further analysis!"<<endl;
                        LevelsOK = false;                        
                      }
                    } else {
                      mout<<"Error: Identical levels! Decaying it to the ground state"<<endl;
                      NewLevelEnergy = 0.0;
                      NewHalfLife = 0.0;
                    }
                    
                    cout<<"NewLevel: "<<NewLevelEnergy/keV<<" with "<<NewHalfLife/s<<endl;

                    if (NewLevelEnergy != 0.0 && fabs(Tree[b].back().GetExcitation() - NewLevelEnergy) < 0.1*keV) {
                      mout<<"Error in data files: Missing level reference leads to de-excitation to the same state for "<<Nucleus->GetParticleName()<<endl;
                      mout<<"                     Forcing IMMEDIATE de-excitation to ground state!"<<endl;
                      NewLevelEnergy = 0.0;
                      NewHalfLife = 0.0;
                    }
                    
                    
                    MCActivatorParticle NewParticle;
                    NewParticle.SetIDAndExcitation(MCSteppingAction::GetParticleType(Nucleus->GetParticleName()), 
                                                NewLevelEnergy);
                    NewParticle.SetBranchingRatio(Tree[b].back().GetBranchingRatio()*NuclearLevel->GammaProbabilities()[h]);
                    NewParticle.SetProductionRate(Tree[b].back().GetProductionRate()*NuclearLevel->GammaProbabilities()[h]);
                    // The PDGLifeTime is not always ok for excited states, thus we have to get it this way:
                    if (NewLevelEnergy > 0.0) {
                      NewParticle.SetHalfLife(NewHalfLife);
                      cout<<NewParticle.GetName()<<":"<<NewHalfLife<<endl;
                    } else {
                      if (MCActivatorParticle::IsStable(NewParticle.GetDefinition()) == true) {
                        NewParticle.SetHalfLife(numeric_limits<double>::max());
                      } else {
                        NewParticle.SetHalfLife(NewParticle.GetDefinition()->GetPDGLifeTime()*log(2.0)); // ln == log
                        cout<<NewParticle.GetName()<<":"<<NewParticle.GetDefinition()->GetPDGLifeTime()*log(2)<<":"<<Nucleus->GetParticleName()<<endl;
                      }
                    }
                    if (ABranch.back().GetHalfLife() < m_HalfLifeCutOff) {
                      // REPLACE the last element, add it to the back
                      // Since we have immediate decays we do not want to increase the length of the branch, but just determine the final population of intermediate states
                      ABranch[ABranch.size()-1] = NewParticle;
                      Tree.push_back(ABranch);
                      TreeChanged = true;
                      NChanges++;
                      cout<<"Replacing highest entry"<<endl;
                    } else {
                      // We have a new branch
                      ABranch.push_back(NewParticle);
                      Tree.push_back(ABranch);
                      TreeChanged = true;
                      NChanges++;
                      cout<<"Adding entry with energy: "<<NewParticle.GetExcitation()/keV<<endl;
                    }
                  } // all possible gammas
                  if (NuclearLevel->NumberOfGammas() > 0 && LevelsOK == true) {
                    Tree[b].clear(); // mark for removal
                    TreeChanged = true;
                    cout<<"Original tree cleared"<<endl;
                  }
                } else { // level not ok
                  mout<<"Error: No nearest level found for: "<<Nucleus->GetParticleName()<<" Excitation: "<<Nucleus->GetExcitationEnergy()<<endl;
                  mout<<"       This isotope is excluded from further analysis!"<<endl;
                  LevelsOK = false;                        
                }
              } else { // level manager not ok
                mout<<"Error: No level manager found for: "<<Nucleus->GetParticleName()<<" Excitation: "<<Nucleus->GetExcitationEnergy()<<endl;
                mout<<"       This isotope is excluded from further analysis!"<<endl;
                LevelsOK = false;                                        
              }
              if (LevelsOK == false) {
                Tree[b].clear();
                TreeChanged = true;
                cout<<"Tree cleared - level not OK"<<endl;
              }
            } // all branches
            
            DumpTree(Tree, "Tree after generation of deexcited states: ");
            
            // Switch the round:
            // Make sure that a non immideate round is only called when no immidiate decays are left
            if (NChanges == 0 || ImmidiateDecayRound == false) {
              ImmidiateDecayRound = !ImmidiateDecayRound;
            }
            
            // Clean the tree:
            if (CleanDecayChains(Tree) == true) {
              TreeChanged = true;
            }

            
            // Check if we have anything which still can decay immediately
            MoreDecays = false;
            for (unsigned int b = 0; b < Tree.size(); ++b) {
              if (Tree[b].back().GetHalfLife() == 0 ||
                  Tree[b].back().GetExcitation() > 0.1*keV) {
                MoreDecays = true;
              }
            }

            DumpTree(Tree, "Tree after cleaning: ");
          
            // Sanity check to prevent endless loops:
            for (vector<vector<MCActivatorParticle> >::iterator I = Tree.begin(); I != Tree.end(); ) {
              if ((*I).size() >= 2) {
                if ((*I)[(*I).size()-1].GetID() == (*I)[(*I).size()-2].GetID() && 
                    fabs((*I)[(*I).size()-1].GetExcitation() - (*I)[(*I).size()-2].GetExcitation()) < 0.2*keV) {
                  mout<<"GEANT4 ERROR: mother and daughter in decay are identical! Erasing this branch!"<<endl;
                  I = Tree.erase(I);
                  continue;
                }
              }
              ++I;
            }

            /*
            for (unsigned int b = 0; b < Tree.size(); ++b) {
              if (Tree[b].size() >= 2) {
                if (Tree[b][Tree[b].size()-1].GetID() == Tree[b][Tree[b].size()-2].GetID() && 
                    fabs(Tree[b][Tree[b].size()-1].GetExcitation() - Tree[b][Tree[b].size()-2].GetExcitation()) < 0.2*keV) {
                  mout<<"Identical particles: "<<b<<endl;
                  massert(false);
                }
              }
            }
            */
            //break;
          } while (MoreDecays == true);
          
//           DumpTree(Tree, "Tree after generation of deexcited states: ");

          //           break;
        } while (TreeChanged == true);
        
        
        if (Tree.size() == 0) {
          mout<<"Warning: Tree is empty - ignoring it"<<endl;
          continue;
        }

        
        DumpTree(Tree, "Current tree - before relative branching ratios: ");
        
        // Up to now we have determined relative braching ratios.
        // For the following calculations absolute are prefered:
        double TotalBranchingRatio = 0.0;
        for (unsigned int br = 0; br < Tree.size(); ++br) {
          TotalBranchingRatio += Tree[br].back().GetBranchingRatio();
          for (unsigned int a = 1; a < Tree[br].size(); ++a) {
            if (Tree[br][a-1].GetBranchingRatio() != 0) {
              Tree[br][a].SetBranchingRatio(Tree[br][a].GetBranchingRatio()/Tree[br][a-1].GetBranchingRatio());
              if (Tree[br][a].GetBranchingRatio() > 1.01) {
                mout<<"Error (CalculateRelativeBranchingRatios): Branching ratio of "<<Tree[br][a].GetName()<<" larger than one: "<<Tree[br][a].GetBranchingRatio()<<endl;
              }
            } else {
              // It already is 0!
            }
          }
        }  
        if (TotalBranchingRatio > 1.01 || TotalBranchingRatio < 0.99) {
          mout<<"Error (CalculateRelativeBranchingRatios): Total branching ratio of "<<Tree[0][0].GetName()<<" not close to one: "<<TotalBranchingRatio<<endl;          
        }
        
        DumpTree(Tree, "Current tree - before determination of tree has already been calculated: ");
        
        // Since frequently the same isotopes are created in the same volumes,
        // we check if the tree has been already calculated
        unsigned int IdenticalTree = numeric_limits<unsigned int>::max();
        for (unsigned int a = 0; a < CalculatedTrees.size(); ++a) {
          bool Identical = true;
          if (CalculatedTrees[a].size() == Tree.size()) {
            for (unsigned int br = 0; br < CalculatedTrees[a].size(); ++br) {
              if (CalculatedTrees[a][br].size() == Tree[br].size()) {
                for (unsigned int l = 0; l < CalculatedTrees[a][br].size(); ++l) {
                  if (CalculatedTrees[a][br][l] == Tree[br][l]) {
                    // Nothing --- just keep the nice symmetry
                  } else {
                    Identical = false;
                    break;
                  }
                }
              } else {
                Identical = false;
                break;
              }
            }
          } else {
            Identical = false;
          }
          
          if (Identical == true) {
            IdenticalTree = a;
            break;
          }
        }
        
        if (IdenticalTree == numeric_limits<unsigned int>::max()) {
          // For each branch/chain calculate the activation:
          for (unsigned int br = 0; br < Tree.size(); ++br) {
            if (m_ActivationMode == c_ConstantIrradiation || m_ActivationMode == c_ConstantIrradiationWithCooldown) {
              if (ActivateByEquations(Tree[br], m_ActivationTime, m_CooldownTime) == false) {
                if (ActivateByPartialSimulation(Tree[br], m_ActivationTime, m_CooldownTime) == false) {
                  merr<<"Activation calculation failed utterly!"<<endl;
                  return false;
                }
              }
            } else {
              mout<<"Only constant irradiation implemented!!"<<endl;
            }
            for (unsigned int l = 0; l < Tree[br].size(); ++l) {
              cout<<Tree[br][l].GetName()<<" ("<<Tree[br][l].GetCounts()<<" cts, "<<Tree[br][l].GetActivation()*s<<" Bq) ";
            }
            cout<<endl;
          }
          
          // Store the tree so that we do not have to do this calculation twice...
          CalculatedTrees.push_back(Tree);
        } else {
          // We can now use the precalculated tree instead
          
          mout<<"Tree already exists -- reusing old one"<<endl;
          
          // First calculate a correction factor utilizing the production rates
          double Correction = Tree[0][0].GetProductionRate()/CalculatedTrees[IdenticalTree][0][0].GetProductionRate();
          
          // Copy the tree
          Tree = CalculatedTrees[IdenticalTree];
          
          // Correct the activation:
          for (unsigned int br = 0; br < Tree.size(); ++br) {
            for (unsigned int l = 0; l < Tree[br].size(); ++l) {
              Tree[br][l].SetActivation(Correction*Tree[br][l].GetActivation());
            }
          }
        }
        
        // Store the data, but make sure not to count any tree (trunk) twice!
        // Basically ignore everything which starts similarly: Count it only the first time
        
        for (unsigned int b1 = 0; b1 < Tree.size(); ++b1) {
          for (unsigned int b2 = b1+1; b2 < Tree.size(); ++b2) {
            // Compare:
            for (unsigned int a = 0; a < Tree[b1].size() && a < Tree[b2].size(); ++a) {
              if (Tree[b1][a].GetName() == Tree[b2][a].GetName()) {
                Tree[b2][a].SetStorageMarker(1); // Don't store
              } else {
                break;
              }
            }
          }
        }
        

        
        // Store
        for (unsigned int b1 = 0; b1 < Tree.size(); ++b1) {
          for (unsigned int l1 = 0; l1 < Tree[b1].size(); ++l1) {
            if (Tree[b1][l1].GetStorageMarker() == 0) {
              //cout<<"Storing: "<<Tree[b1][l1].GetID()<<":"<<Tree[b1][l1].GetExcitation()<<":"<<Tree[b1][l1].GetActivation()<<endl;
              if (m_OutputModeActivities == true) {
                m_Activation.Add(m_Rates.GetVolume(v), Tree[b1][l1].GetID(), Tree[b1][l1].GetExcitation(), Tree[b1][l1].GetActivation()*s);
              } else {
                double NElements = floor(0.5 + Tree[b1][l1].GetActivation()*Tree[b1][l1].GetHalfLife()/log(2));
                if (NElements > 0) {
                  m_Activation.Add(m_Rates.GetVolume(v), Tree[b1][l1].GetID(), Tree[b1][l1].GetExcitation(), NElements);
                }              
              }
            }
          }
        }
      } // excitations
    } // IDs
  } // volumes
  cout<<m_Activation<<endl;
  m_Activation.RemoveStableElements();
  cout<<m_Activation<<endl;

  return true;
}  

  

/******************************************************************************
 * Dump the tree
 */
void MCActivator::DumpTree(const vector<vector<MCActivatorParticle> >& Tree, const MString& Intro) const
{
  if (Intro != "") cout<<Intro<<endl;
    
  for (unsigned int br = 0; br < Tree.size(); ++br) {
    cout<<"  "<<br<<": ";
    for (unsigned int l = 0; l < Tree[br].size(); ++l) {
      cout<<Tree[br][l].GetName()<<" (PR: "<<Tree[br][l].GetProductionRate()*s<<"  BR:"<<Tree[br][l].GetBranchingRatio()<<"; E="<<Tree[br][l].GetExcitation()/keV<<" keV;  HL:"<<Tree[br][l].GetHalfLife()/s<<" sec) ";
      if (l < Tree[br].size()-1) cout<<"-> ";
    }
    cout<<endl;
  }
}

/******************************************************************************
 * Clean the trees calculated during decay chain determination
 * Return true if the tree has changed
 */
bool MCActivator::CleanDecayChains(vector<vector<MCActivatorParticle> >& Tree)
{
  bool TreeChanged = false;

//   // Dump the tree:
//   cout<<"Tree before cleaning: "<<endl;
//   for (unsigned int br = 0; br < Tree.size(); ++br) {
//     cout<<"  "<<br<<": ";
//     for (unsigned int l = 0; l < Tree[br].size(); ++l) {
//       cout<<Tree[br][l].GetName()<<" (PR:"<<Tree[br][l].GetProductionRate()*s<<"  BR:"<<Tree[br][l].GetBranchingRatio()<<" HL:"<<Tree[br][l].GetHalfLife()/s<<") - ";
//     }
//     cout<<endl;
//   }

  // Search for identical paths and merge them:
  for (unsigned int b1 = 0; b1 < Tree.size(); ++b1) {
    for (unsigned int b2 = b1+1; b2 < Tree.size(); ++b2) {
      if (Tree[b1].size() > 0 && Tree[b1].size() == Tree[b2].size()) {
        bool AreEqual = true;
        for (unsigned int a = 0; a < Tree[b1].size(); ++a) {
          //cout<<Tree[b1][a].GetID()<<":"<<Tree[b2][a].GetID()<<":"<<Tree[b1][a].GetExcitation()<<":"<<Tree[b2][a].GetExcitation()<<endl;
          if (Tree[b1][a].GetID() != Tree[b2][a].GetID() ||
              fabs(Tree[b1][a].GetExcitation() - Tree[b2][a].GetExcitation()) > 1*keV) { // bad data protection...
            AreEqual = false;
            break;
          }
        }
        if (AreEqual == true) {
          // Add the branching ratios of the LAST ISOTOPE were we have the deexcitations (the previous isotopes need to be fixed)
          for (unsigned int a = 0; a < Tree[b1].size(); ++a) {
            // Only the last isotope (but with different excitation!):
            if (Tree[b1][a].GetID() == Tree[b1].back().GetID()) {
              if (a == Tree[b1].size() - 1) { 
                Tree[b1][a].SetBranchingRatio(Tree[b1][a].GetBranchingRatio() + Tree[b2][a].GetBranchingRatio());
                if (Tree[b1][a].GetBranchingRatio() > 1.01) {
                  mout<<"Error (in CleanDecayChains): Branching ratio of "<<Tree[b1][a].GetName()<<" larger than one: "<<Tree[b1][a].GetBranchingRatio()<<endl;
                }
              }
              Tree[b1][a].SetProductionRate(Tree[b1][a].GetProductionRate() + Tree[b2][a].GetProductionRate());
            }
          }
          Tree[b2].clear();
          TreeChanged = true;
        }
      }
    }
  }

  // Remove empty branches:
  vector<vector<MCActivatorParticle> >::iterator B = Tree.begin();
  while (B != Tree.end()) {
    if ((*B).size() == 0) {
      B = Tree.erase(B);
      TreeChanged = true;
      continue;
    }
    ++B;
  }
  
//   // Dump the tree:
//   cout<<"Tree after cleaning: "<<endl;
//   for (unsigned int br = 0; br < Tree.size(); ++br) {
//     cout<<"  "<<br<<": ";
//     for (unsigned int l = 0; l < Tree[br].size(); ++l) {
//       cout<<Tree[br][l].GetName()<<" (PR:"<<Tree[br][l].GetProductionRate()*s<<"  BR:"<<Tree[br][l].GetBranchingRatio()<<" HL:"<<Tree[br][l].GetHalfLife()/s<<") - ";
//     }
//     cout<<endl;
//   }

  return TreeChanged;
}


/******************************************************************************
 * Calculate the activation using random numbers
 */
bool MCActivator::ActivateBySimulation(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime)
{
  // Section 1: Calculate activation:

  // Choose a 1% time window around ActivationTime for integration
  double MinTime = 0.995*ActivationTime;
  double MaxTime = 1.005*ActivationTime;

  if (P.size() == 1 && P[0].GetHalfLife() > 1E+99) {
    P[0].SetActivation(0.0);
    return true;
  }

  vector<int> Decays(P.size(), 0); 

  double T = 0.0;

  // Simulate some particles:
  unsigned int NDecays = 0;
  unsigned int NMaxDecays = 50000*P.size();
  unsigned int NTrials = 0;
  unsigned int NMaxTrials = 500*NMaxDecays;
  while (NDecays < NMaxDecays && NTrials < NMaxTrials) {
    NTrials++;
    // Start with a random generation time of the first particle:
    T = gRandom->Rndm()*MaxTime;
    // Now simulate all decays until the total time is larger than MaxTime
    for (unsigned int p = 0; p < P.size(); ++p) {
      //cout<<p<<":   "<<P[p].GetDecayConstant()<<":"<<P[p].GetHalfLife()<<":"<<ActivationTime<<":"<<P[p].GetBranchingRatio()<<endl;
      if (P[p].GetDecayConstant() == 0) break;
      if (P[p].GetBranchingRatio() < gRandom->Rndm()) break;
      T += gRandom->Exp(1.0/P[p].GetDecayConstant());
      if (T > MinTime && T <= MaxTime) {
        Decays[p]++;
        NDecays++;
      }
      if (T > MaxTime) break;
    }
  }

  if (NDecays == 0) {
    mout<<"No decays found!!!"<<endl;
  }
  mout<<"Decays: "<<NDecays<<"   Trials: "<<NTrials<<endl;

  // For the activation calculation we have to scale to the correct initial
  // generation rate and the time window
  for (unsigned int p = 0; p < P.size(); ++p) {
    if (MaxTime > 0 && (NTrials/MaxTime) > 0) {
      P[p].SetActivation(Decays[p] * P[0].GetProductionRate()/(NTrials/MaxTime) * 1.0/(MaxTime-MinTime));
    } else {
      P[p].SetActivation(0.0);
    }
  }


  cout<<"Activities after build-up: ";
  for (unsigned int p = 0; p < P.size(); ++p) {
    cout<<P[p].GetName()<<": "<<P[p].GetActivation()*s<<"Bq   ";
  }
  cout<<endl;


  // Section 2: Calculate cooldown


  if (CooldownTime > 0.0) {

    // Choose a 1% time window around CooldownTime for integration
    MinTime = 0.995*CooldownTime;
    MaxTime = 1.005*CooldownTime;
      
    vector<double> Activity(P.size(), 0); 
      
    // Perform the simulation for all elements in the chain individually
    for (unsigned int q = 0; q < P.size(); ++q) {
      
      // if the sum of all half-lifes < 1/100 Cool down time then nothing is left
      double HalfLifes = 0.0;
      for (unsigned int p = q; p < P.size(); ++p) {
        if (P[p].GetHalfLife() < 1E+99) { // defined as stable for this application
          HalfLifes += P[p].GetHalfLife();
        }
      }
      mout<<"Total halflife ("<<HalfLifes/s<<") vs ("<<CooldownTime/s<<")"<<endl;
      if (HalfLifes > 0.01*CooldownTime) {

        for (unsigned int r = 0; r < P.size(); ++r) {
          Decays[r] = 0;
        }
      
        NDecays = 0;
        NMaxDecays = 50000*(P.size()-q);
        NTrials = 0;
        NMaxTrials = 500*NMaxDecays;
        while (NDecays < NMaxDecays && NTrials < NMaxTrials) {
          NTrials++;
          // Start at time 0:
          T = 0.0;
          // Now simulate all decays until the total time is larger than MaxTime
          for (unsigned int p = q; p < P.size(); ++p) {
            //cout<<p<<":   "<<P[p].GetDecayConstant()<<":"<<P[p].GetHalfLife()<<":"<<ActivationTime<<":"<<P[p].GetBranchingRatio()<<endl;
            if (P[p].GetDecayConstant() == 0) break;
            if (P[p].GetBranchingRatio() < gRandom->Rndm()) break;
            T += gRandom->Exp(1.0/P[p].GetDecayConstant());
            if (T > MinTime && T <= MaxTime) {
              Decays[p]++;
              NDecays++;
            }
            if (T > MaxTime) break;
          }
        }
        mout<<"Decays: "<<NDecays<<"   Trials: "<<NTrials<<endl;
        for (unsigned int p = 0; p < P.size(); ++p) {
          if (P[q].GetDecayConstant() > 0.0 && NTrials > 0.0) {
            Activity[p] += P[q].GetActivation()* (Decays[p]/(MaxTime-MinTime)) / P[q].GetDecayConstant() / NTrials;
          }      
        }
      }
    }
    
    // For the activation calculation we have to scale to the correct initial
    // generation rate and the time window
    for (unsigned int p = 0; p < P.size(); ++p) {
      P[p].SetActivation(Activity[p]);
    }

    cout<<"Activities after cooldown: ";
    for (unsigned int p = 0; p < P.size(); ++p) {
      cout<<P[p].GetName()<<": "<<P[p].GetActivation()*s<<"Bq   ";
    }
    cout<<endl;
  }

  return true;
}


/******************************************************************************
 * Calculate the activation using random numbers
 */
bool MCActivator::ActivateByPartialSimulation(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime)
{
  // Section 1: Calculate activation:

  if (P.size() == 1 && P[0].GetHalfLife() > 1E+99) {
    P[0].SetActivation(0.0);
    return true;
  }

  double HalfLifeCutOff = 20; // Don't make it smaller than 15!

  vector<double> Decays(P.size(), 0); 
  vector<double> DecayProbs(P.size(), 0); 

  double Time = 0.0;
  double TimeTillActivation = 0.0;
  double TotalBR;

  double SimulationWindowCorrectionFactor = 1.0;
  double LowerSimulationWindow = 1.0; 
  double UpperSimulationWindow = 1.0; 

  // Simulate some particles:
  unsigned int NTrials = 0;
  unsigned int NMaxTrials = 250000; // --> should give an accuracy of 0.002
  while (NTrials < NMaxTrials ) {
    NTrials++;

    TotalBR = 1.0;
    SimulationWindowCorrectionFactor = 1.0;

    TimeTillActivation = ActivationTime;

    // Now simulate all decays until the total time is larger than MaxTime
    for (unsigned int p = 0; p < P.size(); ++p) {
      //cout<<p<<":   "<<P[p].GetDecayConstant()<<":"<<P[p].GetHalfLife()<<":"<<ActivationTime<<":"<<P[p].GetBranchingRatio()<<endl;
      if (P[p].GetDecayConstant() == 0) break;

      TotalBR *= P[p].GetBranchingRatio();

      if (p == 0) {
        // Start with a random generation time of the first particle:
        if (TimeTillActivation > HalfLifeCutOff*P[p].GetHalfLife()) {
          // Special mode:
          // For this element simulations farther away than "HalfLifeCutOff" half lifes do not make sense
          double InternalCorrection = TimeTillActivation/HalfLifeCutOff/P[p].GetHalfLife();
          double InternalTime = ActivationTime - CLHEP::RandFlat::shoot(1)*HalfLifeCutOff*P[p].GetHalfLife();

          DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*(ActivationTime-InternalTime))*TotalBR;
          Decays[p] += InternalCorrection; // We assume we had more unidentified decays

          // ... but we assume the decay happened somewhere within the window for all succeeding decays...
          Time = gRandom->Rndm()*ActivationTime;
          TimeTillActivation = ActivationTime - Time;
        } else {
          // Standard mode: 
          Time = CLHEP::RandFlat::shoot(1)*ActivationTime;
          TimeTillActivation = ActivationTime - Time;

          DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*TimeTillActivation)*TotalBR*SimulationWindowCorrectionFactor;          
          Decays[p]++;
        }
      } else {
        // A random decay without time window selection would be simulated in the follwoing way
        // T += gRandom->Exp(1.0/P[p].GetDecayConstant());
        // which is actually:
        // T += -1.0/P[p].GetDecayConstant()*log(gRandom->Uniform());
        // However we want to make sure that the simulation results in a decay 
        // occurring before the activation time to ensure enough statistics.
        // To modify the random generatior we do the following:
        // The lower bound of gRandom->Uniform() gives the upper time limit,
        // The upper bound the lower time limit
        // We correct the total number of occuring decays with the SimulationWindowCorrectionFactor,
        // since we ignored all events which might have happened ouside.
        
        // Now if (ActivationTime - T) > "HalfLifeCutOff" HalfLifes, its useless to consider this decay
        // because its probability to happen at ActivationTime is < 10-9
        // However, we still need statistics for those events, thus ignore the above time, for this decay
        // and force it to --- only for this element in the chain to happen within 30 half lifes
        // of the ActivationTime --- including the correct overall correction
        // However we cannot change the global time since this would bias all following decays

        if (ActivationTime > HalfLifeCutOff*P[p].GetHalfLife()) {
//           LowerSimulationWindow = 1.0/pow(2, TimeTillActivation/P[p].GetHalfLife());
//           UpperSimulationWindow = 1.0/pow(2, (TimeTillActivation-HalfLifeCutOff*P[p].GetHalfLife())/P[p].GetHalfLife());

//           double InternalCorrectionFactor = exp(-P[p].GetDecayConstant()*(TimeTillActivation-HalfLifeCutOff*P[p].GetHalfLife()))-exp(-P[p].GetDecayConstant()*TimeTillActivation);

//           // Make the decay happen somewhere within [T, ActivationTime]:
//           double InternalTime = Time + -1.0/P[p].GetDecayConstant()*log(LowerSimulationWindow+(UpperSimulationWindow-LowerSimulationWindow)*gRandom->Uniform());

//           DecayProbs[p] += exp(-P[p].GetDecayConstant()*(ActivationTime - InternalTime))*TotalBR;          
//           Decays[p] += InternalCorrectionFactor*SimulationWindowCorrectionFactor;        

          // We don't simulate anything, we assume the decay simply happened instantly
          DecayProbs[p] = DecayProbs[p-1]*P[p].GetBranchingRatio();   // <-- improve... but get's overwritten each time, so no error...
          Decays[p] = Decays[p-1];
        }

        // Standard mode
        // In addition, even if we were in the above mode, we need a good time for the next decay:
        //LowerSimulationWindow = 0.0;
        LowerSimulationWindow = 1.0/pow(2, TimeTillActivation/P[p-1].GetHalfLife());
        UpperSimulationWindow = 1.0;
        SimulationWindowCorrectionFactor *= 1-exp(-P[p-1].GetDecayConstant()*TimeTillActivation);

        // Make the decay --- of the previous isotope --- happen somewhere within [T, ActivationTime]:
        Time += -1.0/P[p-1].GetDecayConstant()*log(LowerSimulationWindow+(UpperSimulationWindow-LowerSimulationWindow)*gRandom->Uniform());
        TimeTillActivation = ActivationTime - Time;
        // if (Time >= ActivationTime) break;

        // We only modify the decay probabilities if they have not yet been calculated for the special case above:
        if (TimeTillActivation <= HalfLifeCutOff*P[p].GetHalfLife()) {
          DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*TimeTillActivation) * TotalBR * SimulationWindowCorrectionFactor;          
          Decays[p]++;
        }

//         // Old and working :
//         Time += gRandom->Exp(1.0/P[p-1].GetDecayConstant());
//         if (Time > ActivationTime) break;
//         TimeTillActivation = ActivationTime - Time;
//         DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*TimeTillActivation) * TotalBR;          
//         Decays[p] += SimulationWindowCorrectionFactor;
        
      }

    }
  }


  // For the activation calculation we have to scale to the correct initial
  // generation rate and the time window
  for (unsigned int p = 0; p < P.size(); ++p) {
    if (Decays[p] > 0 && DecayProbs[p] > 0) {
      P[p].SetActivation((P[0].GetProductionRate()*ActivationTime/Decays[p]) * DecayProbs[p]);
    } else {
      P[p].SetActivation(0.);
    }
  }

  cout<<"Activities after build-up: ";
  for (unsigned int p = 0; p < P.size(); ++p) {
    cout<<P[p].GetName()<<": "<<P[p].GetActivation()*s<<"Bq   ";
  }
  cout<<endl;


  // Section 2: Calculate cooldown
  // This should not be done together with the previous step to improve the accuracy of the result

  if (CooldownTime > 0.0) {

    double TimeTillCooldown = 0.0;
    vector<double> Activity(P.size(), 0); 

    for (unsigned int p = 0; p < P.size(); ++p) {
      DecayProbs[p] = 0;
      Decays[p] = 0;
    }
      
    // Perform the simulation for all elements in the chain individually
    for (unsigned int q = 0; q < P.size(); ++q) {
      
      // if the sum of all half-lifes < 1/100 Cool down time then nothing is left
      double HalfLifes = 0.0;
      for (unsigned int p = q; p < P.size(); ++p) {
        if (P[p].GetHalfLife() < 1E+99) { // defined as stable for this application
          HalfLifes += P[p].GetHalfLife();
        }
      }
      //mout<<"Total halflife ("<<HalfLifes/s<<") vs ("<<CooldownTime/s<<")"<<endl;

      for (unsigned int p = 0; p < P.size(); ++p) {
        DecayProbs[p] = 0;
        Decays[p] = 0;
      }
      
      NTrials = 0;
      while (NTrials < NMaxTrials ) {
        NTrials++;
        
        Time = 0.0;
        TimeTillCooldown = CooldownTime;
        TotalBR = 1.0;
        SimulationWindowCorrectionFactor = 1.0;

        // Now simulate all decays until the total time is larger than MaxTime
        for (unsigned int p = q; p < P.size(); ++p) {
          
          TotalBR *= P[p].GetBranchingRatio();

          if (p == q) {
            // The generation time can be assumed before T = 0 thus the time we use is TimeTillCooldown
            
            // We do not need a special mode here --- when they are gone they are gone...
            //if (TimeTillCooldown > HalfLifeCutOff*P[p].GetHalfLife()) {
            //  // Special mode:
            //  // For this element simulations farther away than "HalfLifeCutOff" half lifes do not make sense
            //  double InternalCorrection = TimeTillCooldown/HalfLifeCutOff/P[p].GetHalfLife();
            //  double InternalTime = CooldownTime - CLHEP::RandFlat::shoot(1)*HalfLifeCutOff*P[p].GetHalfLife();
              
            //  DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*(CooldownTime-InternalTime))*TotalBR;
            //  Decays[p] += InternalCorrection; // We assume we had more unidentified decays
            //  //cout<<p<<": simple: "<<DecayProbs[p]<<endl;

            //} else {
            
            // Standard mode: 
            DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*TimeTillCooldown)*TotalBR*SimulationWindowCorrectionFactor;          
            Decays[p]++;
            
              //cout<<p<<": standard: "<<DecayProbs[p]<<endl;
            //}
          } else {
            //cout<<p<<":   "<<P[p].GetDecayConstant()<<":"<<P[p].GetHalfLife()<<":"<<CooldownTime<<":"<<P[p].GetBranchingRatio()<<endl;
            if (CooldownTime > HalfLifeCutOff*P[p].GetHalfLife()) {
            
              // We don't simulate anything, we assume the decay simply happened instantly
              DecayProbs[p] = DecayProbs[p-1]*P[p].GetBranchingRatio();  // <-- improve... but get's overwritten each time, so no error
              Decays[p] = Decays[p-1];
            }
            
            // Standard mode
            // In addition, even if we were in the above mode, we need a good time for the next decay:
            //LowerSimulationWindow = 0.0;
            LowerSimulationWindow = 1.0/pow(2, TimeTillCooldown/P[p-1].GetHalfLife());
            UpperSimulationWindow = 1.0;
            SimulationWindowCorrectionFactor *= 1-exp(-P[p-1].GetDecayConstant()*TimeTillCooldown);
            
            // Make the decay --- of the previous isotope --- happen somewhere within [T, CooldownTime]:
            Time += -1.0/P[p-1].GetDecayConstant()*log(LowerSimulationWindow+(UpperSimulationWindow-LowerSimulationWindow)*gRandom->Uniform());
            TimeTillCooldown = CooldownTime - Time;
            // if (Time >= CooldownTime) break;
            
            // We only modify the decay probabilities if they have not yet been calculated for the special case above:
            if (TimeTillCooldown <= HalfLifeCutOff*P[p].GetHalfLife()) {
              DecayProbs[p] += P[p].GetDecayConstant() * exp(-P[p].GetDecayConstant()*TimeTillCooldown) * TotalBR * SimulationWindowCorrectionFactor;          
              Decays[p]++;
              //cout<<p<<":"<<DecayProbs[p]<<endl;
            }
          }
        }
      }
      for (unsigned int p = 0; p < P.size(); ++p) {
        if (P[q].GetDecayConstant() > 0.0 && NTrials > 0.0) {
          //cout<<p<<":"<<q<<":"<<P[q].GetActivation() / P[q].GetDecayConstant() / NTrials * DecayProbs[p]*s<<endl;
          Activity[p] += P[q].GetActivation() / P[q].GetDecayConstant() / NTrials * DecayProbs[p];
        }      
      }
    }
    
    // For the activation calculation we have to scale to the correct initial
    // generation rate and the time window
    for (unsigned int p = 0; p < P.size(); ++p) {
      P[p].SetActivation(Activity[p]);
    }

    cout<<"Activities after cooldown: ";
    for (unsigned int p = 0; p < P.size(); ++p) {
      cout<<P[p].GetName()<<": "<<P[p].GetActivation()*s<<"Bq   ";
    }
    cout<<endl;
  }

  return true;
}

  

/******************************************************************************
 * Determine the half life of the particle
 */
bool MCActivator::DetermineHalfLife(G4ParticleDefinition* ParticleDef, double& HalfLife, double& ExcitationEnergy, bool IgnoreCutOff)
{
  bool OK = true;
  HalfLife = 0.0;
  ExcitationEnergy = 0.0;
  
  //cout<<"Determine HL: Type="<<ParticleDef->GetParticleType()<<" - Name="<<ParticleDef->GetParticleName()<<endl;
  if (ParticleDef->GetParticleType() == "nucleus" && 
      ParticleDef->GetParticleName() != "alpha") {

    G4Ions* Nucleus = dynamic_cast<G4Ions*>(ParticleDef); 

    //cout<<"Nucleus: E="<<Nucleus->GetExcitationEnergy()<<" keV, A="<<Nucleus->GetAtomicNumber()<<", N="<<Nucleus->GetAtomicMass()<<endl;

    if (Nucleus->GetExcitationEnergy() == 0.0) {
      if (MCActivatorParticle::IsStable(Nucleus) == true) {
        //cout<<"Stable ("<<(Nucleus->GetPDGStable() == true ? "true" : "false")<<") with HL="<<Nucleus->GetPDGLifeTime()<<"="<<Nucleus->GetPDGLifeTime()*log(2.0)/s<<" sec"<<endl;
        HalfLife = numeric_limits<double>::max();
      } else {
        //cout<<"Not stable ("<<(Nucleus->GetPDGStable() == true ? "true" : "false")<<") with HL="<<Nucleus->GetPDGLifeTime()<<"="<<Nucleus->GetPDGLifeTime()*log(2.0)/s<<" sec"<<endl;
        HalfLife = Nucleus->GetPDGLifeTime()*log(2.0); // ln == log
        if (Nucleus->GetPDGLifeTime() < 0) {
          merr<<"BAD ERROR: Geant4 thinks that "<<ParticleDef->GetParticleName()<<" has a negative life time... Don't know what to do... Declaring this element as stable..."<<endl;
          HalfLife = numeric_limits<double>::max();
          OK = false;
        }
      }
    } else {
      G4NuclearLevelManager* M = G4NuclearLevelStore::GetInstance()->GetManager(Nucleus->GetAtomicNumber(), Nucleus->GetAtomicMass());
      //M->PrintAll();
      if (M->IsValid() == true) {
        const G4NuclearLevel* Level = M->NearestLevel(Nucleus->GetExcitationEnergy());
        //Level->PrintAll();
        if (Level != 0) {
          if (Level->HalfLife() > m_HalfLifeCutOff || IgnoreCutOff == true) {
            //cout<<"Half life: "<<Level->HalfLife()<<endl;
            HalfLife = Level->HalfLife();
          } else {
            //cout<<"Half life: "<<Level->HalfLife()<<" ---> Declared as immdiate decay!"<<endl;
            HalfLife = 0.0;
          }
          // This excitation energy is more exact:
          ExcitationEnergy = Level->Energy();
        } else {
          mout<<"Error: No nearest level found for: "<<Nucleus->GetParticleName()<<endl;
          mout<<"       This isotope is excluded from further analysis!"<<endl;
          OK = false;
        } 
      } else {
        mout<<"Error: No nuclear level manager found for: "<<Nucleus->GetParticleName()<<endl;
        mout<<"       This isotope is excluded from further analysis!"<<endl;
        OK = false;
      }
    }
  } else {
    if (MCActivatorParticle::IsStable(ParticleDef) == true) {
      //cout<<"Stable"<<endl;
      HalfLife = numeric_limits<double>::max();
    } else {
      //cout<<"Not stable: "<<ParticleDef->GetPDGLifeTime()*log(2.0)<<endl;
      HalfLife = ParticleDef->GetPDGLifeTime()*log(2.0); // ln == log
    }
  }

  //cout<<ParticleDef->GetParticleName()<<" has a half life of "<<HalfLife/s<<" sec and an excitation energy of "<<ExcitationEnergy/keV<<" keV"<<endl;
            
  return OK;
}


/******************************************************************************
 * Save the rates file
 * Make sure the rates have been calculated 
 */
bool MCActivator::SaveOutputFile()
{
  return m_Activation.Save(m_OutputFileName);
}


/******************************************************************************
 * Calculate the activation using equations (works up to 5 elements)
 */
bool MCActivator::ActivateByEquations(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime)
{
  cout<<"Activate by equations started"<<endl;

  cout<<"Activation time: "<<ActivationTime/s<<endl;
  cout<<"Cooldown time:   "<<CooldownTime/s<<endl;
  cout<<"Production rate: "<<P[0].GetProductionRate()*s<<endl;

  // This method only works if we have 5 or less elements
  if (P.size() > 3 && CooldownTime > 0) {
    return false;
  }
  if (P.size() > 5) {
    return false;
  }
  // If two of the decay constants are too identical it also fails
  double DecayConstantTolerance = 0.01; // this value is just a guess..
  for (unsigned int p1 = 0; p1 < P.size(); ++p1) {
    for (unsigned int p2 = p1+1; p2 < P.size(); ++p2) {
      if (P[p1].GetDecayConstant() > 0 && P[p2].GetDecayConstant() > 0) {
        if (P[p1].GetDecayConstant()/P[p2].GetDecayConstant() < 1+DecayConstantTolerance &&
            P[p1].GetDecayConstant()/P[p2].GetDecayConstant() > 1-DecayConstantTolerance) {
          mout<<"Found a rare case, where two decay constants are almost identical:"<<endl;
          mout<<P[p1].GetName()<<": "<<P[p1].GetDecayConstant()<<" vs. "<<P[p2].GetName()<<": "<<P[p2].GetDecayConstant()<<endl;
          mout<<"Switching to alternate calculation to avoid division by zero..."<<endl;
          return false;
        }
      }
    }
  }


  if (P.size() >= 1) {
    P[0].SetActivation(ActivationO1(P[0].GetProductionRate(), 
                                    P[0].GetDecayConstant(), 
                                    ActivationTime));
  }
  if (P.size() >= 2) {
    P[1].SetActivation(ActivationO2(P[0].GetProductionRate(), 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    P[1].GetDecayConstant(), 
                                    ActivationTime)); 
  }
  if (P.size() >= 3) {
    P[2].SetActivation(ActivationO3(P[0].GetProductionRate(), 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    P[1].GetDecayConstant(), 
                                    P[2].GetBranchingRatio(), 
                                    P[2].GetDecayConstant(), 
                                    ActivationTime)); 
  }
  if (P.size() >= 4) {
    P[3].SetActivation(ActivationO4(P[0].GetProductionRate(), 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    P[1].GetDecayConstant(), 
                                    P[2].GetBranchingRatio(), 
                                    P[2].GetDecayConstant(), 
                                    P[3].GetBranchingRatio(), 
                                    P[3].GetDecayConstant(), 
                                    ActivationTime)); 
  }
  if (P.size() >= 5) {
      // Since we allow only 5 elements, the last one has to be stable...
      P[4].SetActivation(0.0);
  }

  cout<<"Activities after build-up: ";
  for (unsigned int p = 0; p < P.size(); ++p) {
    cout<<P[p].GetName()<<"(T="<<P[p].GetHalfLife()/s<<"s, B="<<P[p].GetBranchingRatio()<<"): "<<P[p].GetActivation()*s<<"Bq   ";
  }
  cout<<endl;

//   for (unsigned int p = 0; p < P.size(); ++p) {
//     cout<<"HACK!"<<endl;
//     if (p != 2) P[p].SetActivation(0.0);
//   }

  if (CooldownTime > 0.0) {
    vector<double> Activities;
    for (unsigned int p = 0; p < P.size(); ++p) {
      Activities.push_back(P[p].GetActivation());
    }

    if (P.size() >= 1) {
      P[0].SetActivation(CooldownO1(Activities[0], 
                                    P[0].GetDecayConstant(), 
                                    CooldownTime)); 
    }
    if (P.size() >= 2) {
      P[1].SetActivation(CooldownO2(Activities[0], 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    Activities[1], 
                                    P[1].GetDecayConstant(), 
                                    CooldownTime)); 
    }
    if (P.size() >= 3) {
      P[2].SetActivation(CooldownO3(Activities[0], 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    Activities[1], 
                                    P[1].GetDecayConstant(), 
                                    P[2].GetBranchingRatio(), 
                                    Activities[2], 
                                    P[2].GetDecayConstant(), 
                                    CooldownTime)); 
    }
    if (P.size() >= 4) {
      P[3].SetActivation(CooldownO4(Activities[0], 
                                    P[0].GetDecayConstant(), 
                                    P[1].GetBranchingRatio(), 
                                    Activities[1], 
                                    P[1].GetDecayConstant(), 
                                    P[2].GetBranchingRatio(), 
                                    Activities[2], 
                                    P[2].GetDecayConstant(), 
                                    P[3].GetBranchingRatio(), 
                                    Activities[3], 
                                    P[3].GetDecayConstant(), 
                                    CooldownTime)); 
    }
    if (P.size() >= 5) {
      // Since we allow only 5 elements, the last one has to be stable...
      P[4].SetActivation(0.0);
    }

    cout<<"Activities after cooldown: ";
    for (unsigned int p = 0; p < P.size(); ++p) {
      cout<<P[p].GetName()<<"(T="<<P[p].GetHalfLife()/s<<"s, B="<<P[p].GetBranchingRatio()<<"): "<<P[p].GetActivation()*s<<"Bq   ";
    }
    cout<<endl;
  }

  return true;
}


/******************************************************************************
 * Calculate the activation using numerical integration
 */
//bool MCActivator::ActivateByNumericalIntegration(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime)
bool MCActivator::ActivateByNumericalIntegration(vector<MCActivatorParticle>&, double, double)
{
  mimp<<"Not yet implemented"<<show;
  return false;

//   // If two of the decay constants are too identical it also fails
//   double DecayConstantTolerance = 0.01; // this value is just a guess..
//   for (unsigned int p1 = 0; p1 < P.size(); ++p1) {
//     for (unsigned int p2 = p1+1; p2 < P.size(); ++p2) {
//       if (P[p1].GetDecayConstant() > 0 && P[p2].GetDecayConstant() > 0) {
//         if (P[p1].GetDecayConstant()/P[p2].GetDecayConstant() < 1+DecayConstantTolerance &&
//             P[p1].GetDecayConstant()/P[p2].GetDecayConstant() > 1-DecayConstantTolerance) {
//           mout<<"Found a rare case, where two decay constants are almost identical:"<<endl;
//           mout<<P[p1].GetName()<<": "<<P[p1].GetDecayConstant()<<" vs. "<<P[p2].GetName()<<": "<<P[p2].GetDecayConstant()<<endl;
//           mout<<"Switching to alternate calculation to avoid division by zero..."<<endl;
//           return false;
//         }
//       }
//     }
//   }

//   if (ActivationTime == 0.0) {
//     for (unsigned int p = 0; p < P.size(); ++p) {
//       P[p].SetActivation(0.0);
//     }
//     return true;
//   }

//   vector<int> InitalNumberOfNuclei(P.size(), 0); // n0
//   vector<int> NumberOfNuclei(P.size(), 0); // n
//   vector<vector<double> > Solver(P.size()); // k
//   vector<double> Exponent(P.size(), 0);

//   for (unsigned int p = 0; p < P.size(); ++p) {
//     cout<<"p: "<<p<<endl;
//     Exponent[p] = exp(-P[p].GetDecayConstant()*ActivationTime);
//     if (p == 0) {
//       InitalNumberOfNuclei[p] = P[p].GetProductionRate()*ActivationTime;
//     } else {
//       InitalNumberOfNuclei[p] = 0.0;
//     }
//     vector<double> SubSolver(p+1);
//     Solver[p] = SubSolver;
//     Solver[p][p] = InitalNumberOfNuclei[p];
//     cout<<"1"<<endl;
//     for (unsigned int q = 0; q < p; ++q) {
//       Solver[p][q] = Solver[p-1][q]*P[p].GetBranchingRatio()*P[p-1].GetDecayConstant()/(P[p].GetDecayConstant() - P[q].GetDecayConstant());
//       Solver[p][q] -= Solver[p][q];
//     }
//   }

//   cout<<"2"<<endl;
//   for (unsigned int p = 0; p < P.size(); ++p) {
//     NumberOfNuclei[p] = 0.0;
//     for (unsigned int q = 0; q <= p; ++q) {
//       NumberOfNuclei[p] += Solver[p][q]*Exponent[p];
//     }
//   }

//   cout<<"3"<<endl;
//   for (unsigned int p = 0; p < P.size(); ++p) {
//     P[p].SetActivation(NumberOfNuclei[p]*P[p].GetDecayConstant());
//   }

//   if (CooldownTime > 0.0) {

//   }

  return true;
}


/******************************************************************************
 * Determine the counts of the first produced element after Time t 
 */
double MCActivator::CountsO1(double R, double D1, double t) 
{
  if (D1 == 0) {
    return R*t;
  }

  return R*(1-exp(-D1*t))/D1;
}


/******************************************************************************
 * Determine the activation of the first produced element after Time t 
 */
double MCActivator::ActivationO1(double R, double D1, double t) 
{
  if (D1 == 0) {
    return 0;
  }

  return R*(1-exp(-D1*t));
}


/******************************************************************************
 * Determine the activation of the first produced element after Time t 
 */
double MCActivator::CooldownO1(double A1, double D1, double t) 
{
  if (D1 == 0) {
    return 0;
  }

  //cout<<A1*s<<":"<<D1*s<<":"<<t/s<<endl;
  return A1*exp(-D1*t);
}


/******************************************************************************
 * Determine the counts of the first produced element after Time t 
 */
double MCActivator::CountsO2(double R, double D1, double Branching12, double D2, double t) 
{
  if (D1 == 0) {
    return 0;
  }
  if (D2 == 0) {
    return Branching12*R/D1*(t*D1 + exp(-D1*t) - 1);
  }

  return Branching12*R/D2*(1 + 1/(D1-D2)*(D2*exp(-D1*t) - D1*exp(-D2*t)));
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::ActivationO2(double R, double D1, double Branching12, double D2, double t) 
{
  if (D1 == 0 || D2 == 0) {
    return 0;
  }
  //return Branching12*(R*((1-exp(-D1*t))-R/D2/(-D2+D1)*(-D2*D1*exp(-D1*t)+D1*D2*exp(-D2*t))));
  return Branching12*R*( (1-exp(-D1*t)) - D1/(D1-D2) * (exp(-D2*t)-exp(-D1*t)) );
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::CooldownO2(double A1, double D1, double Branching12, double A2, double D2, double t) 
{
  if (D1 == 0 || D2 == 0) {
    return 0;
  }

  //cout<<"CO2 data: "<<A1*s<<":"<<D1*s<<":"<<Branching12<<":"<<A2*s<<":"<<D2*s<<":"<<t/s<<endl;
  return Branching12*(-exp(-D2*t)*A2*D1+A1*exp(-D1*t)*D2+exp(-D2*t)*D2*A2-exp(-D2*t)*A1*D2)/(D2-D1);
  //return Branching12*(exp(-D2*t)*A2*D1+A1*exp(-D1*t)*D2-exp(-D2*t)*D2*A2-exp(-D2*t)*A1*D2)/(D2-D1);
}


/******************************************************************************
 * Determine the counts of the first produced element after Time t 
 */
double MCActivator::CountsO3(double R, double D1, double Branching12, double D2, double Branching23, double D3, double t) 
{
  if (D1 == 0 || D2 == 0) {
    return 0;
  }
  if (D3 == 0) {
    return Branching12*Branching23*(-R*D2*D2*t*D1+R*t*D1*D1*D2-R*exp(-D1*t)*D2*D2+R*D1*D1*exp(-D2*t)+R*(D1+D2)*D2-R*(D1+D2)*D1)/(-D2+D1)/D1/D2;
  }

  return Branching12*Branching23*(R*D2*D3*D3-R*D2*D2*D3+R*D2*D2*D1-R*D1*D3*D3+R*D1*D1*D3-R*D1*D1*D2-R*exp(-D1*t)*D2*D3*D3+R*exp(-D1*t)*D2*D2*D3+R*exp(-D2*t)*D1*D3*D3-R*exp(-D2*t)*D1*D1*D3-exp(-D3*t)*D2*D2*R*D1/(D3*D3-D3*D2-D1*D3+D1*D2)*D3*D3+exp(-D3*t)*D2*D2*D2*R*D1/(D3*D3-D3*D2-D1*D3+D1*D2)*D3-exp(-D3*t)*D2*D2*D2*R*D1*D1/(D3*D3-D3*D2-D1*D3+D1*D2)+exp(-D3*t)*D2*R*D1*D1/(D3*D3-D3*D2-D1*D3+D1*D2)*D3*D3-exp(-D3*t)*D2*R*D1*D1*D1/(D3*D3-D3*D2-D1*D3+D1*D2)*D3+exp(-D3*t)*D2*D2*R*D1*D1*D1/(D3*D3-D3*D2-D1*D3+D1*D2))/D3/(D3*D3*D2-D3*D2*D2+D1*D2*D2-D1*D3*D3+D1*D1*D3-D1*D1*D2);
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::ActivationO3(double R, double D1, double Branching12, double D2, double Branching23, double D3, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0) {
    return 0;
  }
  
  return Branching12*Branching23*(R*(1-exp(-D1*t))-R/D2/(-D2+D1)*(-D2*D1*exp(-D1*t)+D1*D2*exp(-D2*t))-(D1*exp(-D1*t)*R*D2*D3*D3-D1*exp(-D1*t)*R*D2*D2*D3-R*D2*exp(-D2*t)*D1*D3*D3+R*D2*exp(-D2*t)*D1*D1*D3+D3*D3*D3*exp(-D3*t)*D1*R*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*D3*exp(-D3*t)*D1*R*D2*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)+D3*exp(-D3*t)*D1*D1*R*D2*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*D3*D3*exp(-D3*t)*D1*D1*R*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)+D3*D3*exp(-D3*t)*D1*D1*D1*R*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*exp(-D3*t)*D1*D1*D1*R*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3))/D3/(D2*D3*D3-D2*D2*D3+D2*D2*D1-D1*D3*D3+D1*D1*D3-D1*D1*D2));
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::CooldownO3(double A1, double D1, double Branching12, double A2, double D2, double Branching23, double A3, double D3, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0) {
    return 0;
  }

  return Branching12*Branching23*(D3*D1*D1*exp(-D3*t)*A2-D1*D1*exp(-D2*t)*A2*D3-D3*D1*D1*exp(-D3*t)*A3+D1*D1*exp(-D3*t)*A3*D2-D1*exp(-D2*t)*A1*D2*D3-D3*D3*D1*exp(-D3*t)*A2+D3*D1*exp(-D3*t)*A1*D2-exp(-D3*t)*D2*D2*A3*D1-D3*exp(-D3*t)*D2*A2*D1+D3*D3*D1*exp(-D3*t)*A3+exp(-D2*t)*A2*D1*D3*D3+exp(-D2*t)*D2*A2*D1*D3-D3*D3*exp(-D3*t)*D2*A3+exp(-D2*t)*A1*D2*D3*D3-D3*exp(-D3*t)*D2*D2*A1-exp(-D2*t)*D2*A2*D3*D3+D3*D3*exp(-D3*t)*D2*A2-A1*exp(-D1*t)*D2*D3*D3+D3*exp(-D3*t)*D2*D2*A3+A1*exp(-D1*t)*D2*D2*D3)/(-D2*D3*D3+D2*D2*D3-D2*D2*D1+D1*D3*D3-D1*D1*D3+D2*D1*D1);


  //return Branching12*Branching23*(-(-D1*D1*exp(-D3*t)*A3*D2+D1*D1*exp(-D3*t)*D3*A3+D1*D1*exp(-D3*t)*A2*D3-exp(-D2*t)*D1*D1*A2*D3+D1*exp(-D3*t)*D2*D2*A3+exp(-D2*t)*D2*A2*D1*D3-D1*exp(-D3*t)*D2*A2*D3+exp(-D2*t)*D1*A1*D2*D3-D1*D2*exp(-D3*t)*A1*D3-D1*exp(-D3*t)*D3*D3*A2+exp(-D2*t)*A2*D1*D3*D3-D1*exp(-D3*t)*D3*D3*A3+exp(-D3*t)*D2*D2*A1*D3-exp(-D3*t)*A3*D2*D2*D3-A1*exp(-D1*t)*D2*D2*D3+exp(-D3*t)*D3*D3*A3*D2-exp(-D2*t)*A1*D2*D3*D3+exp(-D3*t)*D2*A2*D3*D3-exp(-D2*t)*D2*A2*D3*D3+A1*exp(-D1*t)*D2*D3*D3)/(-D2*D3*D3+D2*D2*D3-D2*D2*D1+D1*D3*D3-D1*D1*D3+D1*D1*D2));

  //return Branching12*Branching23*(-D1*D1*exp(-D3*t)*A3*D2+D1*D1*exp(-D3*t)*D3*A3-D1*D1*exp(-D3*t)*A2*D3+exp(-D2*t)*D1*D1*A2*D3+D1*exp(-D3*t)*D2*D2*A3-exp(-D2*t)*D2*A2*D1*D3+D1*D2*exp(-D3*t)*A1*D3+D1*exp(-D3*t)*D2*A2*D3-exp(-D2*t)*D1*A1*D2*D3+D1*exp(-D3*t)*D3*D3*A2-D1*exp(-D3*t)*D3*D3*A3-exp(-D2*t)*A2*D1*D3*D3+A1*exp(-D1*t)*D2*D2*D3-exp(-D3*t)*A3*D2*D2*D3-exp(-D3*t)*D2*D2*A1*D3+exp(-D2*t)*D2*A2*D3*D3-A1*exp(-D1*t)*D2*D3*D3+exp(-D2*t)*A1*D2*D3*D3-exp(-D3*t)*D2*A2*D3*D3+exp(-D3*t)*D3*D3*A3*D2)/(-D2*D3*D3+D2*D2*D3-D2*D2*D1+D1*D3*D3-D1*D1*D3+D1*D1*D2);

  // return Branching12*Branching23*(-D1*D1*exp(-D3*t)*A3*D2+D1*D1*exp(-D3*t)*D3*A3-D1*D1*exp(-D3*t)*A2*D3+exp(-D2*t)*D1*D1*A2*D3+D1*D2*D2*exp(-D3*t)*A3+D1*D2*exp(-D3*t)*A1*D3+D1*D2*exp(-D3*t)*A2*D3-exp(-D2*t)*D1*A1*D2*D3-exp(-D2*t)*D2*A2*D1*D3-exp(-D2*t)*A2*D1*D3*D3+D1*exp(-D3*t)*D3*D3*A2-D1*exp(-D3*t)*D3*D3*A3+A1*exp(-D1*t)*D2*D2*D3-exp(-D3*t)*A3*D2*D2*D3-D2*D2*exp(-D3*t)*A1*D3+exp(-D2*t)*A1*D2*D3*D3+D3*D3*exp(-D3*t)*A3*D2-D2*exp(-D3*t)*A2*D3*D3-A1*exp(-D1*t)*D2*D3*D3+exp(-D2*t)*D2*A2*D3*D3)/(-D2*D3*D3+D2*D2*D3-D2*D2*D1+D1*D3*D3-D1*D1*D3+D1*D1*D2);

  // return Branching12*Branching23*(-D2*D2*exp(-D3*t)*A3*D1+D2*D2*exp(-D3*t)*D3*A3-A1*exp(-D1*t)*D2*D2*D3+D2*D2*exp(-D3*t)*A1*D3+D2*D1*D1*exp(-D3*t)*A3-D2*D1*exp(-D3*t)*A1*D3+D2*exp(-D2*t)*A2*D1*D3+D2*exp(-D2*t)*A1*D1*D3-D2*D1*exp(-D3*t)*A2*D3-D2*exp(-D3*t)*D3*D3*A3-D2*exp(-D2*t)*A2*D3*D3+A1*exp(-D1*t)*D2*D3*D3+D2*exp(-D3*t)*D3*D3*A2-D2*exp(-D2*t)*A1*D3*D3-exp(-D3*t)*A3*D1*D1*D3+D1*D1*exp(-D3*t)*A2*D3-exp(-D2*t)*A2*D1*D1*D3+D3*D3*exp(-D3*t)*A3*D1-exp(-D3*t)*D3*D3*A2*D1+exp(-D2*t)*A2*D1*D3*D3)/(D2*D3*D3-D2*D2*D3+D2*D2*D1-D1*D3*D3+D1*D1*D3-D1*D1*D2);

  //  return Branching12*Branching23*(D3*D3*exp(-D3*t)*D2*A3-exp(-D2*t)*A2*D1*D3*D3+D2*exp(-D2*t)*A1*D3*D3-A1*exp(-D1*t)*D2*D3*D3-D3*D3*exp(-D3*t)*D2*A2+D3*D3*exp(-D3*t)*D1*A2+D2*exp(-D2*t)*A2*D3*D3-D3*D3*exp(-D3*t)*D1*A3-D3*exp(-D3*t)*D2*D2*A1+D3*exp(-D3*t)*D2*A2*D1-exp(-D2*t)*D1*A1*D2*D3-D3*exp(-D3*t)*D2*D2*A3-D3*exp(-D3*t)*D1*D1*A2+D3*exp(-D3*t)*D1*A1*D2-exp(-D2*t)*D2*A2*D1*D3+D3*exp(-D3*t)*D1*D1*A3+A1*exp(-D1*t)*D2*D2*D3+exp(-D2*t)*D1*D1*A2*D3+exp(-D3*t)*D2*D2*A3*D1-exp(-D3*t)*D1*D1*A3*D2)/(D2*D3*D3-D2*D2*D3+D2*D2*D1-D1*D3*D3+D1*D1*D3-D1*D1*D2)/D3
}


/******************************************************************************
 * Determine the counts of the first produced element after Time t 
 */
double MCActivator::CountsO4(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0) {
    return 0;
  }
  if (D4 == 0) {
    return Branching12*Branching23*Branching34*(R*t*D1*D1*D1*D2*D2*D3+exp(-D3*t)*D1*D1*D1*R*D2*D2-R*exp(-D2*t)*D1*D1*D1*D3*D3-R*D1*D1*D1*D3*D3*t*D2-R*D2*D2*D2*t*D1*D1*D3-exp(-D3*t)*D1*D1*R*D2*D2*D2+R*D1*D1*D3*D3*D3*t*D2+R*exp(-D2*t)*D1*D1*D3*D3*D3+R*exp(-D1*t)*D2*D2*D2*D3*D3+R*D3*D3*D2*D2*D2*t*D1-R*exp(-D1*t)*D2*D2*D3*D3*D3-R*D3*D3*D3*D2*D2*t*D1+R*(D1*D2+D1*D3+D3*D2)*D3*D3*D2+R*(D1*D2+D1*D3+D3*D2)*D1*D2*D2-R*(D1*D2+D1*D3+D3*D2)*D3*D2*D2+R*(D1*D2+D1*D3+D3*D2)*D1*D1*D3-R*(D1*D2+D1*D3+D3*D2)*D1*D3*D3-R*(D1*D2+D1*D3+D3*D2)*D1*D1*D2)/D1/D3/D2/(-D3*D3*D2+D3*D2*D2-D2*D2*D1+D1*D3*D3-D1*D1*D3+D1*D1*D2);
  }
  
  return Branching12*Branching23*Branching34*(R*D1*D1*D1*D3*D3*D4-R*D1*D1*D1*D3*D3*D2+R*D1*D1*D1*D2*D4*D4-R*D1*D1*D1*D2*D2*D4+R*D2*D2*D3*D3*D3*D4-R*D2*D2*D3*D3*D3*D1-R*D2*D3*D3*D3*D4*D4+R*D2*D3*D3*D4*D4*D4+R*D1*D1*D1*D2*D2*D3-R*D1*D1*D2*D4*D4*D4+R*D2*D2*D2*D3*D3*D1-R*D2*D2*D2*D3*D3*D4-R*D2*D2*D3*D4*D4*D4+R*D2*D2*D2*D3*D4*D4-R*D2*D2*D2*D1*D1*D3+R*D2*D2*D2*D1*D1*D4+exp(-D4*t)*D2*D2*D2*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4-exp(-D4*t)*D2*D2*D1*D1*D1*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)-exp(-D4*t)*D2*D1*D1*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4+exp(-D4*t)*D2*D1*D1*D1*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4+R*D1*D1*D3*D4*D4*D4-R*D1*D1*D1*D3*D4*D4+R*D1*D1*D3*D3*D3*D2-R*D1*D1*D3*D3*D3*D4+R*D1*D3*D3*D3*D4*D4-R*D1*D3*D3*D4*D4*D4-exp(-D4*t)*D2*D1*D1*D1*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*
D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4+exp(-D4*t)*D2*D2*D1*D1*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)-exp(-D4*t)*D2*D2*D2*D1*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)+exp(-D4*t)*D2*D2*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4+exp(-D4*t)*D2*D2*D2*D2*D1*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)-exp(-D4*t)*D2*D2*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4+exp(-D4*t)*D2*D2*D2*D2*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4-exp(-D4*t)*D2*D2*D2*D2*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4-exp(-D4*t)*D2*D2*D2*D2*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4+exp(-D4*t)*D2*D2*D2*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*
D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4-exp(-D4*t)*D2*D2*D2*D2*D1*D1*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)+exp(-D4*t)*D2*D2*D2*D2*D1*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4-exp(-D4*t)*D2*D1*D1*R*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4+exp(-D4*t)*D2*D1*D1*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4+exp(-D4*t)*D2*D1*D1*R*D3*D3*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4-exp(-D4*t)*D2*D2*D2*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4-exp(-D4*t)*D2*D2*D1*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4*D4+exp(-D4*t)*D2*D2*D2*D1*D1*D1*D1*R*D3*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)+exp(-D4*t)*D2*D2*D1*D1*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-
D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4*D4-exp(-D4*t)*D2*D2*D2*D1*D1*D1*D1*R*D3/(D2*D1*D4-D2*D1*D3-D2*D4*D4+D2*D3*D4-D1*D4*D4+D1*D3*D4+D4*D4*D4-D3*D4*D4)*D4-R*exp(-D2*t)*D1*D3*D3*D3*D4*D4+R*exp(-D2*t)*D1*D1*D3*D3*D3*D4+R*exp(-D3*t)*D2*D2*D2*D1*D4*D4-R*exp(-D3*t)*D2*D2*D1*D4*D4*D4-R*exp(-D3*t)*D2*D2*D2*D1*D1*D4-R*exp(-D1*t)*D2*D2*D2*D3*D4*D4+R*exp(-D1*t)*D2*D2*D3*D4*D4*D4+R*exp(-D3*t)*D2*D1*D1*D4*D4*D4+R*exp(-D1*t)*D2*D2*D2*D3*D3*D4+R*exp(-D3*t)*D2*D2*D1*D1*D1*D4-R*exp(-D3*t)*D2*D1*D1*D1*D4*D4-R*exp(-D1*t)*D2*D2*D3*D3*D3*D4-R*exp(-D1*t)*D2*D3*D3*D4*D4*D4+R*exp(-D2*t)*D1*D1*D1*D3*D4*D4+R*exp(-D1*t)*D2*D3*D3*D3*D4*D4-R*exp(-D2*t)*D1*D1*D3*D4*D4*D4-R*exp(-D2*t)*D1*D1*D1*D3*D3*D4+R*exp(-D2*t)*D1*D3*D3*D4*D4*D4+R*D2*D2*D1*D4*D4*D4-R*D2*D2*D2*D1*D4*D4)/D4/(D1*D2*D2*D4*D4*D4+D3*D3*D2*D2*D2*D1-D3*D3*D2*D2*D2*D4+D3*D2*D2*D2*D4*D4-D3*D2*D2*D4*D4*D4-D3*D3*D3*D2*D2*D1+D3*D3*D3*D2*D2*D4-D3*D3*D3*D2*D4*D4+D3*D3*D2*D4*D4*D4-D1*D1*D1*D2*D2*D4+D1*D1*D1*D2*D4*D4-D1*D1*D2*D4*D4*D4-D1*D1*D1*D3*D4*D4+D1*D1*D3*D4*D4*D4+D1*D3*D3*D3*
D4*D4-D1*D3*D3*D4*D4*D4-D1*D1*D2*D2*D2*D3+D1*D1*D2*D2*D2*D4-D1*D2*D2*D2*D4*D4-D1*D1*D1*D3*D3*D2+D1*D1*D1*D3*D3*D4+D1*D1*D3*D3*D3*D2-D1*D1*D3*D3*D3*D4+D1*D1*D1*D2*D2*D3);
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::ActivationO4(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0 || D4 == 0) {
    return 0;
  }
  
  return Branching12*Branching23*Branching34*(R*(1-exp(-D1*t))-R/D2/(-D2+D1)*(-D2*D1*exp(-D1*t)+D1*D2*exp(-D2*t))-(D1*exp(-D1*t)*R*D2*D3*D3-D1*exp(-D1*t)*R*D2*D2*D3-R*D2*exp(-D2*t)*D1*D3*D3+R*D2*exp(-D2*t)*D1*D1*D3+D3*D3*D3*exp(-D3*t)*D1*R*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*D3*exp(-D3*t)*D1*R*D2*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)+D3*exp(-D3*t)*D1*D1*R*D2*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*D3*D3*exp(-D3*t)*D1*D1*R*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)+D3*D3*exp(-D3*t)*D1*D1*D1*R*D2/(-D1*D3+D2*D1+D3*D3-D2*D3)-D3*exp(-D3*t)*D1*D1*D1*R*D2*D2/(-D1*D3+D2*D1+D3*D3-D2*D3))/D3/(D2*D3*D3-D2*D2*D3+D2*D2*D1-D1*D3*D3+D1*D1*D3-D1*D1*D2)-(-D4*D4*exp(-D4*t)*D2*D2*D2*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*D4*D4*exp(-D4*t)*D2*D1*D1*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-R*D2*exp(-D2*t)*D1*D1*D1*D3*D4*D4+R*D3*exp(-D3*t)*D1*D1*D1*D2*D4*D4-R*D3*exp(-D3*t)*D1*D1*D1*D2*D2*D4-D4*D4*exp(-D4*t)*D2*D1*D1*D1*D1*R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-
D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*D4*exp(-D4*t)*D2*D2*D1*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*D4*exp(-D4*t)*D2*D2*D2*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*D4*D4*exp(-D4*t)*D2*D2*D2*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*exp(-D4*t)*D2*D2*D2*D2*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*exp(-D4*t)*D2*D2*D2*D2*D1*D1*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*exp(-D4*t)*D2*D2*D2*D2*D1*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*exp(-D4*t)*D2*D2*D2*D1*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*D4*D4*exp(-D4*t)*D2*D2*D1*R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*exp(-D4*t)*D2*D2*D2*D2*D1*D1*
R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*D4*exp(-D4*t)*D2*D2*D2*D2*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*exp(-D4*t)*D2*D2*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*exp(-D4*t)*D2*D2*D2*D2*D1*R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*exp(-D4*t)*D2*D2*D1*D1*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*exp(-D4*t)*D2*D1*D1*D1*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*exp(-D4*t)*D2*D2*D1*D1*D1*D1*R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*exp(-D4*t)*D2*D2*D2*D1*D1*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*D4*D4*exp(-D4*t)*D2*D1*D1*R*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*
D4*D4*exp(-D4*t)*D2*D2*D1*D1*D1*D1*R*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*exp(-D4*t)*D2*D2*D2*D1*D1*D1*D1*R*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)-D4*D4*D4*exp(-D4*t)*D2*D1*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+D4*D4*exp(-D4*t)*D2*D1*D1*D1*R*D3*D3*D3*D3/(D3*D1*D4-D3*D2*D1-D3*D4*D4+D3*D2*D4-D1*D4*D4+D1*D2*D4+D4*D4*D4-D2*D4*D4)+R*D2*exp(-D2*t)*D1*D1*D3*D4*D4*D4-R*D2*exp(-D2*t)*D1*D3*D3*D4*D4*D4+D1*exp(-D1*t)*R*D2*D2*D3*D3*D3*D4+D1*exp(-D1*t)*R*D2*D3*D3*D4*D4*D4-R*D2*exp(-D2*t)*D1*D1*D3*D3*D3*D4+R*D2*exp(-D2*t)*D1*D3*D3*D3*D4*D4-D1*exp(-D1*t)*R*D2*D3*D3*D3*D4*D4-R*D3*exp(-D3*t)*D1*D1*D2*D4*D4*D4-D1*exp(-D1*t)*R*D2*D2*D2*D3*D3*D4+D1*exp(-D1*t)*R*D2*D2*D2*D3*D4*D4-D1*exp(-D1*t)*R*D2*D2*D3*D4*D4*D4+R*D2*exp(-D2*t)*D1*D1*D1*D3*D3*D4+R*D3*exp(-D3*t)*D1*D1*D2*D2*D2*D4-R*D3*exp(-D3*t)*D1*D2*D2*D2*D4*D4+R*D3*exp(-D3*t)*D1*D2*D2*D4*D4*D4)/D4/(D2*D3*D3*D4*D4*D4-D2*D3*D3*D3*D4*D4+D2*D2*
D3*D3*D3*D4-D2*D2*D3*D4*D4*D4+D2*D2*D2*D3*D4*D4-D2*D2*D2*D3*D3*D4+D2*D2*D1*D4*D4*D4-D2*D2*D2*D1*D4*D4-D1*D3*D3*D4*D4*D4+D1*D3*D3*D3*D4*D4-D1*D1*D3*D3*D3*D4+D1*D1*D3*D4*D4*D4-D1*D1*D1*D3*D4*D4+D1*D1*D1*D3*D3*D4-D1*D1*D2*D4*D4*D4+D1*D1*D1*D2*D4*D4-D1*D1*D1*D2*D2*D4-D2*D2*D3*D3*D3*D1+D2*D2*D2*D1*D1*D4+D2*D2*D1*D1*D1*D3-D2*D1*D1*D1*D3*D3+D2*D1*D1*D3*D3*D3-D2*D2*D2*D1*D1*D3+D2*D2*D2*D3*D3*D1));
}


/******************************************************************************
 * Determine the activation of the second produced element after Time t 
 */
double MCActivator::CooldownO4(double A1, double D1, double Branching12, double A2, double D2, double Branching23, double A3, double D3, double Branching34, double A4, double D4, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0 || D4 == 0) {
    return 0;
  }

  return Branching12*Branching23*Branching34*(D4*exp(-D4*t)*D1*D1*D3*D3*D3*A4-D4*exp(-D4*t)*D1*D1*D3*D3*D3*A2+D4*D4*exp(-D4*t)*D2*D2*D3*D3*A2-D4*D4*D2*D1*D1*D1*exp(-D4*t)*A4+D4*D4*D2*D1*D1*D1*exp(-D4*t)*A3+D4*D4*D1*D1*D1*D3*exp(-D4*t)*A4+D4*D4*D2*D2*D2*D3*exp(-D4*t)*A3-D4*D4*D2*D3*D3*D3*exp(-D4*t)*A2+D4*D4*D2*D3*D3*D3*exp(-D4*t)*A4-D4*D4*D2*D2*D3*D3*exp(-D4*t)*A3-D4*D4*exp(-D4*t)*D2*D2*D2*D1*A3+D4*D4*exp(-D4*t)*D2*D2*D2*D1*A4-D4*D4*D2*D2*D2*D3*exp(-D4*t)*A4-D4*D4*D4*exp(-D4*t)*D1*D3*D3*A3+D4*D2*D2*D1*D1*D1*exp(-D4*t)*A4-D4*D2*D2*D1*D1*D1*exp(-D4*t)*A3-D4*exp(-D4*t)*D2*D2*D2*D3*D3*A1+D4*D4*exp(-D4*t)*D1*D3*D3*D3*A2-D4*D4*D1*D1*D1*D3*exp(-D4*t)*A3+D4*D4*exp(-D4*t)*D1*D1*D3*D3*A3-D4*D4*D1*D1*D3*D3*exp(-D4*t)*A2-D4*D4*exp(-D4*t)*D1*D3*D3*D3*A4-D4*D1*D1*D1*D3*D3*exp(-D4*t)*A4-D4*D2*D2*D3*D3*D3*exp(-D4*t)*A4+D4*D2*D2*D3*D3*D3*exp(-D4*t)*A1-D4*exp(-D4*t)*D2*D2*D2*D1*D1*A4+D4*D4*exp(-D4*t)*D2*D2*D3*D1*A3+D4*D4*exp(-D4*t)*D2*D1*D1*D3*A2-D4*D4*D1*D1*D3*exp(-D4*t)*D2*A3-D4*D4*D2*D2*D1*exp(-D4*t)*D3*A2+D4*D4*D4*D2*D3*D3*
exp(-D4*t)*A3-D4*D4*D4*D2*D2*D1*exp(-D4*t)*A4-D4*D4*D4*D2*D3*D3*exp(-D4*t)*A4-D4*D4*D4*D1*D1*D3*exp(-D4*t)*A4+D4*D4*D4*exp(-D4*t)*D2*D2*D3*A4+D4*D4*D4*exp(-D4*t)*D2*D1*D1*A4+exp(-D2*t)*D1*D1*A1*D2*D3*D4*D4+D4*D4*D4*D1*D1*D3*exp(-D4*t)*A3-D4*D4*D4*exp(-D4*t)*D2*D1*D1*A3+exp(-D3*t)*D1*A1*D2*D3*D4*D4*D4-D4*D4*D4*exp(-D4*t)*D2*D2*D3*A3+D4*D4*D4*D2*D2*D1*exp(-D4*t)*A3+D4*D4*D4*exp(-D4*t)*D1*D3*D3*A4+D4*D1*D1*D1*D3*D3*exp(-D4*t)*A2+D4*exp(-D4*t)*D2*D2*D2*D1*D1*A3+D4*D2*D2*D2*D3*D3*exp(-D4*t)*A4+exp(-D3*t)*D3*D2*D2*D2*A1*D4*D4+exp(-D2*t)*D2*A2*D3*D3*D3*D4*D4+exp(-D3*t)*D3*D3*D1*A3*D4*D4*D4+exp(-D3*t)*D2*D2*D2*A3*D1*D4*D4+exp(-D3*t)*D2*D2*A2*D3*D3*D1*D4+D1*D1*D1*exp(-D2*t)*A2*D3*D4*D4+exp(-D3*t)*D1*D1*A1*D2*D2*D3*D4+D3*D1*D1*exp(-D3*t)*A2*D4*D4*D4+D3*D1*D1*D1*exp(-D3*t)*A2*D2*D4+exp(-D3*t)*D3*D3*D1*D1*A2*D4*D4+exp(-D3*t)*D3*D3*D2*D2*A3*D4*D4+exp(-D1*t)*D3*D2*D2*A1*D4*D4*D4+exp(-D1*t)*D3*D3*D2*D2*D2*A1*D4+exp(-D3*t)*D3*D2*D2*D2*A3*D1*D4+exp(-D1*t)*A1*D2*D3*D3*D3*D4*D4+exp(-D3*t)*D3*D2*D2*A3*D4*D4*D4+exp(-D3*t)*D2*A2*
D3*D3*D4*D4*D4-exp(-D3*t)*D1*D1*A1*D2*D3*D4*D4-exp(-D3*t)*D3*D3*D1*D1*A2*D2*D4-D3*D1*D1*D1*exp(-D3*t)*A2*D4*D4-D1*D1*exp(-D2*t)*A2*D3*D4*D4*D4-D3*D1*D1*D1*exp(-D3*t)*A3*D2*D4-D1*D1*D1*exp(-D2*t)*A2*D3*D3*D4+D3*D1*D1*exp(-D3*t)*A3*D2*D4*D4+D1*D1*exp(-D3*t)*A3*D2*D4*D4*D4+exp(-D2*t)*A1*D2*D3*D3*D3*D1*D4+D3*D1*D1*D1*exp(-D3*t)*A3*D4*D4+exp(-D2*t)*A1*D2*D3*D3*D4*D4*D4+exp(-D2*t)*D3*D3*D2*A2*D1*D1*D4-exp(-D2*t)*D3*D2*A2*D1*D1*D4*D4-D3*D1*D1*exp(-D3*t)*A3*D4*D4*D4+exp(-D2*t)*D3*D3*D1*A2*D4*D4*D4+exp(-D2*t)*D3*D2*A2*D1*D4*D4*D4-exp(-D2*t)*A1*D2*D3*D3*D3*D4*D4-D1*D1*D1*exp(-D3*t)*A3*D2*D4*D4-exp(-D2*t)*D1*D1*A1*D2*D3*D3*D4-exp(-D3*t)*D3*D3*D1*A2*D4*D4*D4-exp(-D2*t)*D1*A1*D2*D3*D4*D4*D4-exp(-D3*t)*D3*D2*A2*D1*D4*D4*D4-exp(-D3*t)*D3*D2*D2*A2*D1*D1*D4-exp(-D3*t)*D2*D2*D2*A3*D1*D1*D4-exp(-D2*t)*D3*D3*D3*D1*A2*D4*D4-exp(-D3*t)*D3*D3*D1*D1*A3*D4*D4+exp(-D2*t)*D3*D3*D3*D1*D1*A2*D4-exp(-D3*t)*D2*D2*A2*D3*D3*D4*D4-exp(-D3*t)*D3*D2*D2*D2*A3*D4*D4-exp(-D3*t)*D2*D2*A3*D1*D4*D4*D4-exp(-D3*t)*D3*D2*D2*A1*D4*D4*D4-exp(-D1*t)*A1*D2*
D2*D3*D3*D3*D4-exp(-D3*t)*D3*D2*D2*D2*A1*D1*D4-exp(-D2*t)*D2*A2*D3*D3*D3*D1*D4-exp(-D1*t)*A1*D2*D3*D3*D4*D4*D4-exp(-D3*t)*D3*D3*D2*D2*A3*D1*D4-exp(-D3*t)*D3*D3*D2*A3*D4*D4*D4-exp(-D3*t)*D3*D2*D2*A3*D4*D4*D1+D1*D1*D1*exp(-D3*t)*A3*D2*D2*D4+exp(-D3*t)*D3*D3*D1*D1*A3*D2*D4-D4*exp(-D4*t)*D1*D1*D3*D3*D2*A3-D2*D2*D2*D3*D3*exp(-D4*t)*A4*D1+D1*D1*D1*D3*D3*exp(-D4*t)*A4*D2+exp(-D4*t)*D2*D2*D2*D1*D1*A4*D3+D2*D2*D3*D3*D3*exp(-D4*t)*A4*D1-exp(-D2*t)*D2*A2*D3*D3*D4*D4*D4+D4*D2*D2*D1*D1*exp(-D4*t)*A2*D3-D4*D2*D2*D2*D3*exp(-D4*t)*D1*A3-D4*D2*D1*D1*D1*exp(-D4*t)*D3*A2-D4*exp(-D4*t)*D2*D2*D3*D3*A2*D1-D4*exp(-D4*t)*D2*D2*D1*D1*D3*A1+D4*D2*D2*D2*D1*exp(-D4*t)*D3*A1+D4*D1*D1*D1*D3*exp(-D4*t)*D2*A3+D4*D1*D1*D3*D3*exp(-D4*t)*A1*D2+D4*D2*D3*D3*D3*exp(-D4*t)*A2*D1+D4*D2*D2*D3*D3*exp(-D4*t)*D1*A3-D4*exp(-D4*t)*D1*D3*D3*D3*A1*D2-D2*D2*D1*D1*D1*exp(-D4*t)*A4*D3-exp(-D1*t)*D3*D2*D2*D2*A1*D4*D4+exp(-D3*t)*D3*D2*D2*A2*D1*D4*D4-D1*D1*D3*D3*D3*D2*exp(-D4*t)*A4)/(D2*D2*D3*D4*D4*D4+D2*D3*D3*D3*D4*D4-D2*D3*D3*D4*D4*D4+D2*D2*D1*D1*D1*D4-D2*D1*
D1*D1*D4*D4+D2*D1*D1*D4*D4*D4+D1*D1*D1*D3*D3*D2-D1*D1*D1*D3*D3*D4+D1*D1*D1*D3*D4*D4-D1*D1*D3*D4*D4*D4-D1*D1*D3*D3*D3*D2+D1*D1*D3*D3*D3*D4-D1*D3*D3*D3*D4*D4+D1*D3*D3*D4*D4*D4+D2*D2*D2*D1*D1*D3-D2*D2*D2*D1*D1*D4+D2*D2*D2*D1*D4*D4-D2*D2*D1*D4*D4*D4-D2*D2*D2*D3*D4*D4-D2*D2*D2*D3*D3*D1+D2*D2*D2*D3*D3*D4+D2*D2*D3*D3*D3*D1-D2*D2*D3*D3*D3*D4-D2*D2*D1*D1*D1*D3);
  
  /*
  return Branching12*Branching23*Branching34*(-(-D1*D1*D3*D3*D3*exp(-D4*t)*D4*A2+D1*D1*D3*D3*exp(-D4*t)*A3*D4*D4+exp(-D4*t)*D2*D2*D3*D3*A2*D4*D4+D3*D3*D2*exp(-D4*t)*A3*D4*D4*D4-D3*D3*D3*D2*exp(-D4*t)*A2*D4*D4-D3*D3*D2*exp(-D4*t)*A4*D4*D4*D4+exp(-D4*t)*D2*D2*D2*D3*D3*D4*A1-exp(-D4*t)*D2*D2*D3*A3*D4*D4*D4+exp(-D4*t)*D2*D2*D3*A4*D4*D4*D4-D3*D3*D3*D2*D2*exp(-D4*t)*D4*A1-D1*D1*D1*D2*D2*D4*exp(-D4*t)*A3+D1*D1*D2*D2*D2*exp(-D4*t)*D3*A4-D1*D1*D2*D2*D2*D4*exp(-D4*t)*A4+D1*D1*D2*D2*D2*D4*exp(-D4*t)*A3-D1*D1*exp(-D4*t)*D4*D4*D4*A4*D3+D1*D1*D1*exp(-D4*t)*D3*D3*D4*A2+D1*D3*D3*D3*D2*D2*exp(-D4*t)*A4-D1*exp(-D4*t)*D3*D3*D3*D4*D4*A4-D1*exp(-D4*t)*D3*D3*D4*D4*D4*A3+D1*exp(-D4*t)*D3*D3*D3*D4*D4*A2-D1*D2*D2*exp(-D4*t)*D4*D4*D4*A4+D1*D4*D4*D4*D3*D3*exp(-D4*t)*A4+D1*D2*D2*exp(-D4*t)*D4*D4*D4*A3-D1*D2*D2*D2*exp(-D4*t)*D4*D4*A3+D1*D2*D2*D2*exp(-D4*t)*D4*D4*A4-D1*exp(-D4*t)*D2*D2*D2*D3*D3*A4+D1*D1*D1*exp(-D4*t)*D3*A4*D4*D4-D1*D1*D1*exp(-D4*t)*D3*A3*D4*D4+D1*D1*D1*D2*exp(-D4*t)*D3*D3*A4-D1*D1*D1*D2*exp(-D4*t)*D4*D4*A4+D1*D1*D1*D2*
exp(-D4*t)*D4*D4*A3+D1*D1*D4*D4*D4*D3*exp(-D4*t)*A3+D1*D1*D3*D3*D3*exp(-D4*t)*A4*D4-D1*D1*D1*exp(-D4*t)*D3*D3*A4*D4-D1*D1*D2*D3*D3*D3*exp(-D4*t)*A4+D1*D1*D2*exp(-D4*t)*D4*D4*D4*A4-D1*D1*D2*exp(-D4*t)*D4*D4*D4*A3+D3*D3*exp(-D3*t)*A3*D2*D2*D4*D4-D3*D3*D2*D2*exp(-D4*t)*A3*D4*D4-D1*D1*D1*D3*exp(-D3*t)*A3*D2*D4+D1*D1*D2*exp(-D4*t)*D4*D4*A2*D3-D1*D1*D2*exp(-D4*t)*D3*A3*D4*D4+D1*D3*D3*D3*D2*exp(-D4*t)*D4*A2+D1*D3*D3*D2*D2*exp(-D4*t)*D4*A3-D1*D2*D2*exp(-D4*t)*D4*D4*A2*D3+D1*exp(-D4*t)*D2*D2*D3*A3*D4*D4-D1*exp(-D4*t)*D2*D2*D2*D3*D4*A3-D1*exp(-D4*t)*D2*D2*D3*D3*D4*A2-D1*D1*D2*D3*D3*exp(-D4*t)*D4*A3-D1*D1*D4*D4*exp(-D4*t)*A2*D3*D3+D1*D1*D1*D2*exp(-D4*t)*D3*D4*A3-D1*D1*D2*exp(-D4*t)*D3*D3*D4*A1+A1*exp(-D1*t)*D2*D3*D3*D4*D4*D4+A1*exp(-D1*t)*D2*D2*D3*D3*D3*D4+D1*D2*D2*D3*exp(-D3*t)*A2*D4*D4+D1*exp(-D2*t)*A2*D3*D3*D4*D4*D4+D1*D1*exp(-D2*t)*A2*D3*D3*D3*D4+D2*exp(-D2*t)*A2*D3*D3*D3*D4*D4+D1*D1*exp(-D3*t)*A3*D2*D4*D4*D4+D1*D1*D3*D3*exp(-D3*t)*A2*D4*D4+D3*exp(-D3*t)*A3*D2*D2*D4*D4*D4+D2*D2*D3*exp(-D3*t)*A1*D4*D4*D4+D1*D3*D3*
exp(-D3*t)*A3*D4*D4*D4+D1*D1*D3*exp(-D3*t)*A2*D4*D4*D4+D3*exp(-D3*t)*A3*D2*D2*D2*D1*D4+D1*D1*D3*D3*exp(-D3*t)*A3*D2*D4+D1*D1*D2*D3*exp(-D3*t)*A1*D4*D4+D2*D2*D2*D3*exp(-D3*t)*A1*D1*D4+D2*exp(-D2*t)*A1*D3*D3*D3*D4*D4+D1*D1*D3*exp(-D3*t)*A3*D2*D4*D4+D1*D1*D1*D3*exp(-D3*t)*A3*D4*D4+A1*exp(-D1*t)*D2*D2*D2*D3*D4*D4+D1*D1*D2*exp(-D2*t)*A1*D3*D3*D4+D1*D1*D1*exp(-D3*t)*A3*D2*D2*D4+D2*exp(-D2*t)*A2*D1*D3*D4*D4*D4-D2*exp(-D2*t)*A2*D3*D3*D4*D4*D4-D1*D2*D3*exp(-D3*t)*A2*D4*D4*D4-D2*exp(-D2*t)*A2*D1*D1*D3*D4*D4-D2*exp(-D2*t)*A2*D3*D3*D3*D1*D4+D1*D2*D2*D2*exp(-D3*t)*A3*D4*D4+D1*D1*D1*exp(-D2*t)*A2*D3*D4*D4-D1*exp(-D2*t)*A2*D3*D3*D3*D4*D4+D1*D1*D2*D2*D4*exp(-D4*t)*D3*A1+D1*D1*D1*D3*exp(-D3*t)*A2*D2*D4-D1*D1*D3*D3*exp(-D3*t)*A2*D2*D4-D1*D2*D2*D2*D4*exp(-D4*t)*D3*A1+D1*D1*D2*D2*D4*exp(-D4*t)*A2*D3+D1*D2*D3*D3*D3*exp(-D4*t)*D4*A1-D1*D1*D2*exp(-D2*t)*A1*D3*D4*D4-D2*exp(-D2*t)*A1*D3*D3*D3*D1*D4-A1*exp(-D1*t)*D2*D2*D2*D3*D3*D4-D1*D2*D2*exp(-D3*t)*A3*D4*D4*D4-D1*D1*D2*D2*D3*exp(-D3*t)*A1*D4-D1*D2*D3*exp(-D3*t)*A1*D4*D4*D4-D1*D1*D2*
D2*D2*exp(-D3*t)*A3*D4-D3*D3*exp(-D3*t)*A3*D2*D2*D1*D4-A1*exp(-D1*t)*D2*D3*D3*D3*D4*D4-D2*D2*D3*D3*exp(-D3*t)*A2*D4*D4-D1*D3*D3*exp(-D3*t)*A2*D4*D4*D4-D3*D3*exp(-D3*t)*A3*D2*D4*D4*D4-D1*D1*D3*D3*exp(-D3*t)*A3*D4*D4-D2*D2*D2*D3*exp(-D3*t)*A1*D4*D4-D3*exp(-D3*t)*A3*D2*D2*D2*D4*D4-D2*exp(-D2*t)*A1*D3*D3*D4*D4*D4-D1*D1*D1*exp(-D3*t)*A3*D2*D4*D4-D1*D1*D1*D2*D2*exp(-D4*t)*D3*A4-D1*D1*D3*exp(-D3*t)*A3*D4*D4*D4-D1*D1*exp(-D2*t)*A2*D3*D4*D4*D4+D1*D1*D1*D2*D2*D4*exp(-D4*t)*A4-D1*D1*D1*D2*D4*exp(-D4*t)*A2*D3-D2*D2*exp(-D4*t)*D3*D3*D3*A4*D4+D2*exp(-D4*t)*D3*D3*D3*A4*D4*D4+D2*D2*D2*exp(-D4*t)*D3*D3*A4*D4+D2*D2*D2*D4*D4*exp(-D4*t)*A3*D3-D2*D2*D2*D4*D4*exp(-D4*t)*A4*D3+D2*D3*D3*exp(-D3*t)*A2*D4*D4*D4+D2*D2*D3*D3*exp(-D3*t)*A2*D1*D4+D1*D2*exp(-D2*t)*A1*D3*D4*D4*D4-D1*D1*D1*exp(-D2*t)*A2*D3*D3*D4-D1*D1*D1*D3*exp(-D3*t)*A2*D4*D4-D3*exp(-D3*t)*A3*D2*D2*D4*D4*D1-A1*exp(-D1*t)*D2*D2*D3*D4*D4*D4+D2*exp(-D2*t)*A2*D1*D1*D3*D3*D4-D1*D1*D2*D2*D3*exp(-D3*t)*A2*D4)/(-D2*D3*D3*D4*D4*D4+D2*D3*D3*D3*D4*D4+D2*D2*D3*D4*D4*D4-D2*D2*D2*D3*D4*
D4-D2*D2*D1*D4*D4*D4+D2*D2*D2*D1*D4*D4-D2*D2*D2*D1*D1*D4+D2*D2*D2*D1*D1*D3+D1*D3*D3*D4*D4*D4-D1*D3*D3*D3*D4*D4+D1*D1*D3*D3*D3*D4-D1*D1*D3*D3*D3*D2-D1*D1*D3*D4*D4*D4+D1*D1*D1*D3*D4*D4-D1*D1*D1*D3*D3*D4+D1*D1*D1*D3*D3*D2+D1*D1*D2*D4*D4*D4-D1*D1*D1*D2*D4*D4+D1*D1*D1*D2*D2*D4-D1*D1*D1*D2*D2*D3-D2*D2*D3*D3*D3*D4+D2*D2*D3*D3*D3*D1+D2*D2*D2*D3*D3*D4-D2*D2*D2*D3*D3*D1));
  */
}


/******************************************************************************
 * Determine the counts of the first produced element after Time t 
 */
double MCActivator::CountsO5(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double Branching45, double D5, double t) 
{
  if (D1 == 0 || D2 == 0 || D3 == 0 || D4 == 0) {
    return 0;
  }
  if (D5 == 0) {
    return Branching12*Branching23*Branching34*Branching45*(-R*D2*D2*D2*D2*D3*D3*D4*D4*D4*t*D1+R*D2*D2*D2*D3*D3*D4*D4*D4*D4*t*D1+R*D2*D2*D2*D2*t*D1*D1*D1*D3*D3*D4-R*t*D1*D1*D1*D1*D2*D2*D2*D3*D3*D4+R*D2*D2*D3*D3*D3*D3*D4*D4*D4*t*D1+R*D1*D1*D1*D2*D2*D4*D4*D4*D4*t*D3-R*D1*D1*D3*D3*D3*D3*D4*D4*D4*t*D2-R*D2*D2*D3*D3*D3*D4*D4*D4*D4*t*D1-R*D1*D1*D1*D3*D3*D3*D3*t*D2*D2*D4-R*D2*D2*D2*D1*D1*D4*D4*D4*D4*t*D3-R*D3*D3*D3*D2*D2*D2*D2*t*D1*D1*D4-R*D2*D2*D2*D2*D1*D1*D1*D4*D4*t*D3+R*D2*D2*D2*D2*D1*D1*D4*D4*D4*t*D3+R*D1*D1*D3*D3*D3*D4*D4*D4*D4*t*D2+R*D1*D1*D1*D1*D3*D3*D4*D4*D4*t*D2+R*D2*D2*D2*D2*D3*D3*D3*D4*D4*t*D1+R*D3*D3*D3*D3*D2*D2*D2*t*D1*D1*D4-R*D2*D2*D2*D3*D3*D3*D3*D4*D4*t*D1-R*D1*D1*D1*D1*D3*D3*D3*D4*D4*t*D2-R*D1*D1*D1*D3*D3*D4*D4*D4*D4*t*D2+R*D1*D1*D1*D1*D3*D3*D3*t*D2*D2*D4+R*D1*D1*D1*D1*D2*D2*D2*D4*D4*t*D3-R*D1*D1*D1*D1*D2*D2*D4*D4*D4*t*D3+R*D1*D1*D1*D3*D3*D3*D3*D4*D4*t*D2+R*exp(-D1*t)*D2*D2*D3*D3*D3*D3*D4*D4*D4+R*exp(-D3*t)*D1*D1*D1*D1*D2*D2*D2*D4*D4+R*exp(-D4*t)*D1*D1*D1*D2*D2*D2*D2*D3*D3+R*exp(-D4*t)*D1*D1*D2*D2*
D2*D3*D3*D3*D3+R*exp(-D2*t)*D1*D1*D3*D3*D3*D4*D4*D4*D4+R*exp(-D1*t)*D2*D2*D2*D4*D4*D4*D4*D3*D3+R*exp(-D4*t)*D1*D1*D1*D1*D2*D2*D3*D3*D3+R*exp(-D1*t)*D2*D2*D2*D2*D3*D3*D3*D4*D4+R*exp(-D2*t)*D1*D1*D1*D1*D3*D3*D4*D4*D4-R*exp(-D1*t)*D2*D2*D2*D3*D3*D3*D3*D4*D4-R*exp(-D1*t)*D2*D2*D3*D3*D3*D4*D4*D4*D4+R*exp(-D3*t)*D1*D1*D2*D2*D2*D2*D4*D4*D4+R*exp(-D3*t)*D1*D1*D1*D2*D2*D4*D4*D4*D4-R*exp(-D3*t)*D1*D1*D1*D2*D2*D2*D2*D4*D4-R*exp(-D1*t)*D2*D2*D2*D2*D4*D4*D4*D3*D3-R*exp(-D3*t)*D1*D1*D2*D2*D2*D4*D4*D4*D4-R*exp(-D2*t)*D1*D1*D1*D1*D3*D3*D3*D4*D4-R*exp(-D2*t)*D1*D1*D3*D3*D3*D3*D4*D4*D4-R*exp(-D3*t)*D1*D1*D1*D1*D2*D2*D4*D4*D4-R*exp(-D2*t)*D1*D1*D1*D4*D4*D4*D4*D3*D3-R*exp(-D4*t)*D1*D1*D1*D3*D3*D3*D3*D2*D2-R*exp(-D4*t)*D1*D1*D2*D2*D2*D2*D3*D3*D3-R*exp(-D4*t)*D1*D1*D1*D1*D2*D2*D2*D3*D3+R*exp(-D2*t)*D1*D1*D1*D3*D3*D3*D3*D4*D4+D1*D1*D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4+D1*D1*D1*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3-D1*D1*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4*D4*D4-D1*D1*D1*D2*R*(D1*D2*D4+D1*D3*
D2+D1*D3*D4+D2*D3*D4)*D3*D3+D1*D1*D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D4+D1*D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D4*D4*D4-D1*D1*D1*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4-D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D4+D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D4*D4-D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D4*D4*D4-D1*D1*D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D4*D4-D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3*D4*D4+D1*D1*D1*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4*D4+D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D4*D4*D4-D1*D1*D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3+D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3*D4*D4+D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3*D4-D1*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3-D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D4*D4*D4+D1*D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3-D1*D2*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4*D4+D1*D1*D2*
R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3+D1*D2*D2*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D4*D4*D4-D1*D1*R*(D1*D2*D4+D1*D3*D2+D1*D3*D4+D2*D3*D4)*D3*D3*D3*D4)/D1/D2/D3/D4/(-D2*D2*D2*D1*D1*D4-D1*D1*D1*D2*D2*D3+D1*D1*D2*D4*D4*D4-D1*D1*D1*D2*D4*D4+D1*D1*D1*D2*D2*D4+D1*D1*D1*D3*D3*D2-D1*D1*D1*D3*D3*D4-D1*D1*D3*D4*D4*D4+D1*D1*D1*D3*D4*D4+D2*D2*D2*D1*D1*D3-D1*D3*D3*D3*D4*D4-D2*D3*D3*D4*D4*D4+D2*D3*D3*D3*D4*D4+D2*D2*D3*D4*D4*D4-D2*D2*D2*D3*D4*D4+D2*D2*D2*D3*D3*D4-D2*D2*D2*D3*D3*D1+D2*D2*D3*D3*D3*D1-D2*D2*D3*D3*D3*D4+D1*D1*D3*D3*D3*D4-D1*D1*D3*D3*D3*D2-D2*D2*D1*D4*D4*D4+D2*D2*D2*D1*D4*D4+D1*D3*D3*D4*D4*D4);
  }

  cout<<"More than 4 unstable elements in queue not allowed!"<<endl;
  return 0.0;
}

/*
 * MCActivator.cc: the end...
 ******************************************************************************/
