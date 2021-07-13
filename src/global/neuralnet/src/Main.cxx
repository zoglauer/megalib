
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "MString.h"
#include "TRandom.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TApplication.h"
#include "TMath.h"

#include "MStreams.h"
#include "MNeuron.h"
#include "MInputNeuron.h"
#include "MBiasNeuron.h"
#include "MFeedForwardNeuron.h"
#include "MAdalineNeuron.h"
#include "MBackpropagationNeuron.h"
#include "MBackpropagationMiddleNeuron.h"
#include "MBackpropagationOutputNeuron.h"
#include "MSynapse.h"
#include "MAdalineSynapse.h"
#include "MBackpropagationSynapse.h"
#include "MNeuralNetworkBackpropagation.h"
#include "MComptonEvent.h"


void TestFunctionMap(double X, double Y, double& Value) {
  //cout<<X<<":"<<Y<<endl;
  
  Value = TMath::Exp(-((X-0.5)*(X-0.5)-(Y-0.5)*(Y-0.5)))-0.5;
  // 	if (sqrt((X-0.5)*(X-0.5)+(Y-0.5)*(Y-0.5)) > 0.25) {
  //         //if (sqrt(X*X+Y*Y) > 1.0) {
  //     //if (sqrt(X*X+Y*Y) > 1.0 || sqrt(X*X+Y*Y) < 0.6) {
  //     //if (sqrt(X*X+Y*Y) < 0.8) {
  // // 	if (Y > X+0.5 || Y < X-0.5) {
  // 		Value = 0.75;
  // 	} else {
  // 		Value = 0.25;
  // 	}
}


void NeuralNet()
{
  cout<<"Launching Neural-Net classes..."<<endl;
  
  cout<<"Show test histogram"<<endl;
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  double Value;
  int NBins = 50;
  TH2D* Map = new TH2D("Original Map", "OriginalMap", NBins, 0, 1, NBins, 0, 1);
  for (int bx = 1; bx <= Map->GetNbinsX(); ++bx) {
    for (int by = 1; by <= Map->GetNbinsY(); ++by) {
      TestFunctionMap(Map->GetXaxis()->GetBinCenter(bx), Map->GetYaxis()->GetBinCenter(by), Value);
      Map->SetBinContent(bx, by, Value);
    }
  }
  TCanvas* MapCanvas = new TCanvas();
  MapCanvas->cd();
  Map->SetContour(50);
  Map->Draw("CONT4Z");
  MapCanvas->Update();
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(2);
  NN.SetNMiddleNodes(100);
  NN.SetNOutputNodes(2);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  int TrainingRuns = 0;
  int MaxGood = 200;
  int Good = 0;
  double Tolerance = 0.03;
  int TotalGood = 0;
  int TotalFalse = 0;
  
  double X, Y;
  
  TH2D* RMap = new TH2D("Reconstructed Map", "ReconstructedMap", NBins, 0, 1, NBins, 0, 1);
  TCanvas* RMapCanvas = new TCanvas("RMCanvas", "RMCanvas");
  
  while (Good < MaxGood) {
    TrainingRuns++;
    
    X = gRandom->Rndm();
    Y = gRandom->Rndm();
    TestFunctionMap(X, Y, Value);
    
    NN.SetInput(0, X);
    NN.SetInput(1, Y);
    NN.Run();
    
    if (fabs(NN.GetOutput(1) - Value) >= Tolerance) {
      Good = 0;
      TotalFalse++;
      cout<<NN.GetOutput(1)<<":"<<Value<<": bad: "<<TotalFalse<<"-"<<TotalGood<<endl;
    } else {
      Good++;
      TotalGood++;
      cout<<NN.GetOutput(1)<<":"<<Value<<": good"<<endl;
    }
    
    // Shall I believe this?????????
    NN.SetOutputError(1, Value);
    NN.Learn();
    
    if (TrainingRuns % 1000 == 0) {
      cout<<Good<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalFalse)<<" ("<<TotalGood<<", "<<TotalFalse<<")"<<endl;
      gSystem->ProcessEvents();
    }
    if ((Good == MaxGood) || (TrainingRuns % 1000 == 0 && TotalGood+TotalFalse > 0)) {
      for (int bx = 1; bx <= RMap->GetNbinsX(); ++bx) {
        for (int by = 1; by <= RMap->GetNbinsY(); ++by) {
          NN.SetInput(0, RMap->GetXaxis()->GetBinCenter(bx));
          NN.SetInput(1, RMap->GetYaxis()->GetBinCenter(by));
          NN.Run();
          
          //cout<<bx<<":"<<by<<":"<<NN.GetOutput(1)<<endl;
          RMap->SetBinContent(bx, by, NN.GetOutput(1));
          //RMap->SetBinContent(bx, by, ((NN.GetOutput(0) > 0.5) ? 1.0 : 0.0));
        }
      }
      RMapCanvas->cd();
      RMap->SetContour(50);
      RMap->Draw("CONT4Z");
      RMapCanvas->Update();		
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 10000001) break;
  }
}

//////////////////////////////////////////////////////////////////////

void ClinicalComptonSimulatorLevel1(vector<MVector>& Positions, vector<double>& Energies)
{
  // Assumptions:
  
  // Photoeffect is only process below 100 keV, above only Compton
  double PhotoThreshold = 250;
  
  // Uniform Compton-absorption probability: 50% after ~1 cm
  double Mu_C = 0.4;
  
  // Uniform photo-absorption probability: 50% after ~0.5 cm
  double Mu_P = 1.4;
  
  // Start position:
  MVector Position(0, 0, 0);
  
  // We are only starting from top to bottom at position 
  MVector Direction;
  Direction.SetMagThetaPhi(1.0, gRandom->Rndm()*c_Pi, gRandom->Rndm()*2*c_Pi);
  Direction.SetXYZ(0.0, 0.0, 1.0);
  
  // Fixed Start energy:
  double Energy = 400.0 + 100.0*gRandom->Rndm();
  
  // Step length: 1 mm
  double StepLength = 0.2;
  
  Positions.clear();
  Energies.clear();
  
  double Phi = 0.0;
  double Theta = 0.0;
  double Probability = 0.0;
  
  TMatrix RotY;
  RotY.ResizeTo(3,3);
  RotY(1,1) = 1;
  TMatrix RotZ;
  RotZ.ResizeTo(3,3);
  RotZ(2,2) = 1;
  
  while (Energy > 0.0) {
    // Calculate new position
    Position += Direction*StepLength;
    
    
    // Calculate probabilty for interactions during this step
    if (Energy > PhotoThreshold) {
      // Compton
      Probability = 1-TMath::Exp(-Mu_C*StepLength);
      
      if (gRandom->Rndm() <= Probability) {
        // Calculate a random KN angle for the given energy...
        
        // Pick a phi
        do {
          Phi = gRandom->Rndm()*c_Pi;
        } while (gRandom->Rndm() > MComptonEvent::GetKleinNishinaNormalized(Energy, Phi));
        
        // Pick a theta:
        Theta = 2*gRandom->Rndm()*c_Pi;
        
        // Store...
        Positions.push_back(Position);
        double Ee = MComptonEvent::ComputeEeViaPhiEi(Phi, Energy);
        //if (Ee < 440) Ee += 50;
        Energies.push_back(Ee);
        
        //if (Energies.size() == 2) cout<<"PhiE (2) ="<<Phi<<":"<<acos(1-c_E0/(Energy - Ee) + c_E0/(Energy))<<endl;
        //if (Positions.size() == 3) cout<<"PhiA (2) = "<<(Positions[1]-Positions[0]).Angle(Positions[2]-Positions[1])<<endl;
        
        
        // Some faking...
        
        //Phi = c_Pi/2;
        //Theta = 0.0;
        //Direction = MVector(0.5, 0.5, 0.707107); //(0, 0, 1.0);
        
        MVector NewDirection(sin(Phi)*cos(Theta), sin(Phi)*sin(Theta), cos(Phi));
        //cout<<NewDirection<<endl;
        NewDirection.RotateReferenceFrame(Direction);
        //cout<<NewDirection<<":"<<Direction.Angle(NewDirection)<<":"<<Phi<<endl;
        
        Direction = NewDirection;
        
        // ... and energy
        Energy -= Energies.back();
      }
    } else {
      // Photo
      Probability = 1-TMath::Exp(-Mu_P*StepLength);
      
      if (gRandom->Rndm() <= Probability) {
        Positions.push_back(Position);
        Energies.push_back(Energy);
        Energy = 0.0;
      }
      
      //if (Positions.size() == 3) cout<<"PhiA (2) = "<<(Positions[1]-Positions[0]).Angle(Positions[2]-Positions[1])<<endl;
    }
  }
  
  return;
}

///////////////////////////////////////////////////////////////////////////

void ComptonNetLevel3()
{
  cout<<"Launching Tripple Comption Scattering Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  // The normalizations:
  double xMin = -10.0;
  double xInt =  20.0;
  
  double yMin = -10.0;
  double yInt =  20.0;
  
  double zMin = -10.0;
  double zInt =  20.0;
  
  //double mMin =   0.0;
  //double mInt =  sqrt(3*100);
  
  double EMin = 0.0;
  double EInt = 500.0;
  
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(12);
  NN.SetNMiddleNodes(36);
  NN.SetNOutputNodes(3);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  
  vector<MVector> Positions;
  vector<double> Energies;
  vector<double> Ordering;
  Ordering.push_back(0.25);
  Ordering.push_back(0.50);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  
  vector<double> RandomDoubles;
  vector<int> Randoms;
  vector<MVector> RandomPositions;
  vector<double> RandomEnergies;
  vector<double> RandomOrdering;
  
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 1000;
  int LastGood = 0;
  
  double BestRatio = 0;
  
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    bool Done = true;
    do {
      ClinicalComptonSimulatorLevel1(Positions, Energies);
      
      Done = true;
      if (Energies.size() != 3) {
        Done = false;
        //cout<<"Not 3 - "<<Energies.size()<<endl;
      } else {
        for (unsigned int i = 0; i < Energies.size(); ++i) {
          if (Positions[i].X() < xMin || Positions[i].X() - xMin > xInt) Done = false;
          if (Positions[i].Y() < yMin || Positions[i].Y() - yMin > yInt) Done = false;
          if (Positions[i].Z() < zMin || Positions[i].Z() - zMin > zInt) Done = false;
          if (Done == false) {
            //cout<<"Out of range"<<endl;
          }
        }
      }
    } while (Done == false);
    
    // Add error:
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      Positions[i][0] += gRandom->Gaus(0.0, 0.1);
      Positions[i][1] += gRandom->Gaus(0.0, 0.1);
      Positions[i][2] += gRandom->Gaus(0.0, 0.1);
      Energies[i] += gRandom->Gaus(0.0, 3);
    }
    
    //cout<<"Acceptable"<<endl;
    
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      //cout<<"Positions ("<<i<<"): "<<Positions[i].X()<<", "<<Positions[i].Y()<<", "<<Positions[i].Z()<<" - "<<Energies[i]<<endl;
    }
    
    // Randomize:
    RandomDoubles.clear();
    RandomDoubles.resize(Energies.size());
    Randoms.clear();
    Randoms.resize(Energies.size());
    RandomPositions.clear();
    RandomEnergies.clear();
    RandomOrdering.clear();
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      RandomDoubles[i] = gRandom->Rndm();
      //cout<<RandomDoubles[i]<<endl;
    }
    // Stupid sort
    for (unsigned int i = Energies.size()-1; i < Energies.size(); --i) {
      int iLargest = -1;
      double Largest = -1.0;
      for (unsigned int j = 0; j < Energies.size(); ++j) {
        if (RandomDoubles[j] >= Largest) {
          Largest = RandomDoubles[j];
          iLargest = j;
        }
      }
      if (iLargest != -1) {
        Randoms[iLargest] = i;
        RandomDoubles[iLargest] = -2.0;
      }
    }
    
    for (unsigned int i = 0; i < Randoms.size(); ++i) {
      RandomPositions.push_back(Positions[Randoms[i]]);
      RandomEnergies.push_back(Energies[Randoms[i]]);
      RandomOrdering.push_back(Ordering[Randoms[i]]);
    }
    
    bool DoesKnow = true;
    bool InputValid = true;
    
    // Learn until the result is known 
    int Try = 0;
    //do {
    Try++;
    
    DoesKnow = true;
    InputValid = true;
    
    // Set the input:
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      //if (NN.SetInput(0 + 4*i, (RandomPositions[i].Mag() - mMin)/mInt) == false) InputValid = false;
      if (NN.SetInput(0 + 4*i, (RandomPositions[i].X() - xMin)/xInt) == false) InputValid = false;
      if (NN.SetInput(1 + 4*i, (RandomPositions[i].Y() - yMin)/yInt) == false) InputValid = false;
      if (NN.SetInput(2 + 4*i, (RandomPositions[i].Z() - zMin)/zInt) == false) InputValid = false;
      if (NN.SetInput(3 + 4*i, (RandomEnergies[i] - EMin)/EInt) == false) InputValid = false;
    }
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      //cout<<NN.GetOutput(i)<<":"<<RandomOrdering[i]<<endl;
      if (RandomOrdering[i] == 0.25) {
        if (NN.GetOutput(i) >= 0.75/2.0) DoesKnow = false;
      } else if (RandomOrdering[i] == 0.5) {
        if (NN.GetOutput(i) < 0.75/2.0 || NN.GetOutput(i) > 1.25/2.0) DoesKnow = false;
      } else if (RandomOrdering[i] == 0.75) {
        if (NN.GetOutput(i) < 1.25/2.0) DoesKnow = false;
      }
    }
    
    if (Try == 1) {
      if (DoesKnow == true) {
        //cout<<"GOOOOOOOD!"<<endl;
        TotalGood++;
        LastGood++;
      } else {
        //cout<<"BAD..."<<endl;
        TotalBad++;
        LastGood = 0;
      }
    }
    
    // Can this be correct????
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      NN.SetOutputError(i, RandomOrdering[i]);
    }
    NN.Learn();
    //} while (InputValid == true && DoesKnow == false);
    
    
    
    
    if (TrainingRuns % 100 == 0) {
      if (double(TotalGood)/(TotalGood+TotalBad) > BestRatio) {
        BestRatio = double(TotalGood)/(TotalGood+TotalBad);
      }
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<") --- best: "<<BestRatio<<endl;
      //cout<<NN.GetOutput(0)<<":"<<RandomOrdering[0]<<" - "<<NN.GetOutput(1)<<":"<<RandomOrdering[1]<<endl;
      //cout<<"First larger: "<<double(FirstLarger)/(FirstSmaller + FirstLarger)<<endl;
      //cout<<"First closer: "<<double(FirstCloserToZero)/(FirstCloserToZero + FirstFarerToZero)<<endl;
      //cout<<RandomPositions[0].Mag()<<":"<<RandomPositions[1].Mag()<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 1000000001) break;
  }
}

/////////////////////////////////////////////////////////////////////////

void ComptonNetLevel3Extended()
{
  cout<<"Launching Tripple Comption Scattering Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  // The normalizations:
  double xMin = -10.0;
  double xInt =  20.0;
  
  double yMin = -10.0;
  double yInt =  20.0;
  
  double zMin = -10.0;
  double zInt =  20.0;
  
  double cMin = -0.2;
  double cInt =  1.4;
  
  //double mMin =   0.0;
  //double mInt =  sqrt(3*100);
  
  double EMin = 0.0;
  double EInt = 500.0;
  
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(18);
  NN.SetNMiddleNodes(36);
  NN.SetNOutputNodes(3);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  
  vector<MVector> Positions;
  vector<double> Energies;
  vector<double> Ordering;
  Ordering.push_back(0.25);
  Ordering.push_back(0.50);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  Ordering.push_back(0.75);
  
  vector<double> RandomDoubles;
  vector<int> Randoms;
  vector<MVector> RandomPositions;
  vector<double> RandomEnergies;
  vector<double> RandomOrdering;
  vector<double> RandomCosini;
  
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 100;
  int LastGood = 0;
  
  int CanBeFound = 0;
  
  double BestRatio = 0;
  
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    bool Done = true;
    do {
      ClinicalComptonSimulatorLevel1(Positions, Energies);
      
      Done = true;
      if (Energies.size() != 3) {
        Done = false;
        //cout<<"Not 3 - "<<Energies.size()<<endl;
      } else {
        for (unsigned int i = 0; i < Energies.size(); ++i) {
          if (Positions[i].X() < xMin || Positions[i].X() - xMin > xInt) Done = false;
          if (Positions[i].Y() < yMin || Positions[i].Y() - yMin > yInt) Done = false;
          if (Positions[i].Z() < zMin || Positions[i].Z() - zMin > zInt) Done = false;
          if (Done == false) {
            //cout<<"Out of range"<<endl;
          }
        }
      }
    } while (Done == false);
    
    // Add error:
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      Positions[i][0] += gRandom->Gaus(0.0, 0.1);
      Positions[i][1] += gRandom->Gaus(0.0, 0.1);
      Positions[i][2] += gRandom->Gaus(0.0, 0.1);
      Energies[i] += gRandom->Gaus(0.0, 3);
    }
    
    //cout<<"Acceptable"<<endl;
    
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      //cout<<"Positions ("<<i<<"): "<<Positions[i].X()<<", "<<Positions[i].Y()<<", "<<Positions[i].Z()<<" - "<<Energies[i]<<endl;
    }
    
    // Randomize:
    RandomDoubles.clear();
    RandomDoubles.resize(Energies.size());
    Randoms.clear();
    Randoms.resize(Energies.size());
    RandomPositions.clear();
    RandomEnergies.clear();
    RandomOrdering.clear();
    RandomCosini.clear();
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      RandomDoubles[i] = gRandom->Rndm();
      //cout<<RandomDoubles[i]<<endl;
    }
    // Stupid sort
    for (unsigned int i = Energies.size()-1; i < Energies.size(); --i) {
      int iLargest = -1;
      double Largest = -1.0;
      for (unsigned int j = 0; j < Energies.size(); ++j) {
        if (RandomDoubles[j] >= Largest) {
          Largest = RandomDoubles[j];
          iLargest = j;
        }
      }
      if (iLargest != -1) {
        Randoms[iLargest] = i;
        RandomDoubles[iLargest] = -2.0;
      }
    }
    
    //Randoms[0] = 0;
    //Randoms[1] = 1;
    //Randoms[2] = 2;
    
    for (unsigned int i = 0; i < Randoms.size(); ++i) {
      RandomPositions.push_back(Positions[Randoms[i]]);
      RandomEnergies.push_back(Energies[Randoms[i]]);
      RandomOrdering.push_back(Ordering[Randoms[i]]);
    }
    
    // Calculate the test statistics... works only for 3-site events...
    int a, b, c;
    double cosphiA, cosphiE;
    for (unsigned int i = 0; i < Randoms.size(); ++i) {
      a = i;
      b = (i+1) % 3;
      c = (i+2) % 3;
      cosphiA = TMath::Cos((RandomPositions[b]-RandomPositions[a]).Angle(RandomPositions[c]-RandomPositions[b]));
      cosphiE = 1-c_E0/(RandomEnergies[c]) + c_E0/(RandomEnergies[b]+RandomEnergies[c]);
      //cout<<acos(cosphiA)<<":"<<acos(cosphiE)<<endl;
      if ((cosphiA-cosphiE)*(cosphiA-cosphiE) > 1) {
        RandomCosini.push_back(1);
      } else {
        RandomCosini.push_back((cosphiA-cosphiE)*(cosphiA-cosphiE));
      }
    }
    for (unsigned int i = 0; i < Randoms.size(); ++i) {
      a = i;
      b = (i+2) % 3;
      c = (i+1) % 3;
      cosphiA = TMath::Cos((RandomPositions[b]-RandomPositions[a]).Angle(RandomPositions[c]-RandomPositions[b]));
      cosphiE = 1-c_E0/(RandomEnergies[c]) + c_E0/(RandomEnergies[b]+RandomEnergies[c]);
      //cout<<acos(cosphiA)<<":"<<acos(cosphiE)<<endl;
      if ((cosphiA-cosphiE)*(cosphiA-cosphiE) > 1) {
        RandomCosini.push_back(1);
      } else {
        RandomCosini.push_back((cosphiA-cosphiE)*(cosphiA-cosphiE));
      }
    }
    
    /*
     *		for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
     *			cout<<RandomCosini[i]<<" - ";
  }
  cout<<endl;
  for (unsigned int i = 0; i < Randoms.size(); ++i) {
    cout<<RandomOrdering[i]<<" - ";
  }
  cout<<endl;
  */
    
    bool DoesKnow = true;
    bool InputValid = true;
    
    // Learn until the result is known 
    int Try = 0;
    //do {
    Try++;
    
    DoesKnow = true;
    InputValid = true;
    
    // Set the input:
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      //if (NN.SetInput(0 + 4*i, (RandomPositions[i].Mag() - mMin)/mInt) == false) InputValid = false;
      if (NN.SetInput(0 + 4*i, (RandomPositions[i].X() - xMin)/xInt) == false) InputValid = false;
      if (NN.SetInput(1 + 4*i, (RandomPositions[i].Y() - yMin)/yInt) == false) InputValid = false;
      if (NN.SetInput(2 + 4*i, (RandomPositions[i].Z() - zMin)/zInt) == false) InputValid = false;
      if (NN.SetInput(3 + 4*i, (RandomEnergies[i] - EMin)/EInt) == false) InputValid = false;
    }
    for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
      if (NN.SetInput(4*RandomEnergies.size() +i, (RandomCosini[i] - cMin)/cInt) == false) InputValid = false;
      //if (NN.SetInput(i, (RandomCosini[i] - cMin)/cInt) == false) InputValid = false;
    }
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      //cout<<NN.GetOutput(i)<<":"<<RandomOrdering[i]<<endl;
      if (RandomOrdering[i] == 0.25) {
        if (NN.GetOutput(i) >= 0.75/2.0) DoesKnow = false;
      } else if (RandomOrdering[i] == 0.5) {
        if (NN.GetOutput(i) < 0.75/2.0 || NN.GetOutput(i) > 1.25/2.0) DoesKnow = false;
      } else if (RandomOrdering[i] == 0.75) {
        if (NN.GetOutput(i) < 1.25/2.0) DoesKnow = false;
      }
    }
    
    if (Try == 1) {
      if (DoesKnow == true) {
        //cout<<"GOOOOOOOD!"<<endl;
        TotalGood++;
        LastGood++;
      } else {
        //cout<<"BAD..."<<endl;
        TotalBad++;
        LastGood = 0;
      }
      
      // Now determine how many we can identify via smalles cosini
      double SmallestAbsCos = 10000;
      int iSmallestAbsCos = -1;
      for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
        if (fabs(RandomCosini[i]) < SmallestAbsCos) {
          SmallestAbsCos = fabs(RandomCosini[i]);
          iSmallestAbsCos = i;
        }
      }
      //cout<<iSmallestAbsCos<<endl;
      if ((iSmallestAbsCos == 0 && RandomOrdering[0] == 0.25 && RandomOrdering[1] == 0.5 && RandomOrdering[2] == 0.75) ||
        (iSmallestAbsCos == 1 && RandomOrdering[0] == 0.75 && RandomOrdering[1] == 0.25 && RandomOrdering[2] == 0.5) ||
        (iSmallestAbsCos == 2 && RandomOrdering[0] == 0.5 && RandomOrdering[1] == 0.75 && RandomOrdering[2] == 0.25) ||
        (iSmallestAbsCos == 3 && RandomOrdering[0] == 0.25 && RandomOrdering[1] == 0.75 && RandomOrdering[2] == 0.5) ||
        (iSmallestAbsCos == 4 && RandomOrdering[0] == 0.5 && RandomOrdering[1] == 0.25 && RandomOrdering[2] == 0.75) ||
        (iSmallestAbsCos == 5 && RandomOrdering[0] == 0.75 && RandomOrdering[1] == 0.5 && RandomOrdering[2] == 0.25)) {
        CanBeFound++;
        }
    }
    
    // Can this be correct????
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      NN.SetOutputError(i, RandomOrdering[i]);
    }
    NN.Learn();
    //} while (InputValid == true && DoesKnow == false);
    
    
    
    
    if (TrainingRuns % 100 == 0) {
      if (double(TotalGood)/(TotalGood+TotalBad) > BestRatio) {
        BestRatio = double(TotalGood)/(TotalGood+TotalBad);
      }
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<") --- best: "<<BestRatio<<" can be found: "<<100.0*double(CanBeFound)/TrainingRuns<<endl;
      //cout<<NN.GetOutput(0)<<":"<<RandomOrdering[0]<<" - "<<NN.GetOutput(1)<<":"<<RandomOrdering[1]<<endl;
      //cout<<"First larger: "<<double(FirstLarger)/(FirstSmaller + FirstLarger)<<endl;
      //cout<<"First closer: "<<double(FirstCloserToZero)/(FirstCloserToZero + FirstFarerToZero)<<endl;
      //cout<<RandomPositions[0].Mag()<<":"<<RandomPositions[1].Mag()<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 1000000001) break;
  }
}

/////////////////////////////////////////////////////////////////////////

int GetOutputID(int x1, int x2, int x3) {
  if (x1 == 0 && x2 == 1 && x3 == 2) return 0;
  else if (x1 == 0 && x2 == 2 && x3 == 1) return 1;
  else if (x1 == 1 && x2 == 0 && x3 == 2) return 2;
  else if (x1 == 1 && x2 == 2 && x3 == 0) return 3;
  else if (x1 == 2 && x2 == 0 && x3 == 1) return 4;
  else if (x1 == 2 && x2 == 1 && x3 == 0) return 5;
  else cout<<"Wrong Output ID"<<endl;
  return -1;
}

int GetIDOriginalOut(int x1, int x2, int x3) {
  if (x1 == 0 && x2 == 1 && x3 == 2) return 0;
  else if (x1 == 0 && x2 == 2 && x3 == 1) return 1;
  else if (x1 == 1 && x2 == 0 && x3 == 2) return 2;
  else if (x1 == 1 && x2 == 2 && x3 == 0) return 4;
  else if (x1 == 2 && x2 == 0 && x3 == 1) return 3;
  else if (x1 == 2 && x2 == 1 && x3 == 0) return 5;
  else cout<<"Wrong Output ID"<<endl;
  return -1;
}

int GetOutputInX1(int ID) {
  if (ID == 0) return 0;
  else if (ID == 1) return 0;
  else if (ID == 2) return 1;
  else if (ID == 3) return 1;
  else if (ID == 4) return 2;
  else if (ID == 5) return 2;
  else cout<<"Wrong Output ID X1"<<endl;
  return -1;
}
int GetOutputInX2(int ID) {
  if (ID == 0) return 1;
  else if (ID == 1) return 2;
  else if (ID == 2) return 0;
  else if (ID == 3) return 2;
  else if (ID == 4) return 0;
  else if (ID == 5) return 1;
  else cout<<"Wrong Output ID X2"<<endl;
  return -1;
}
int GetOutputInX3(int ID) {
  if (ID == 0) return 2;
  else if (ID == 1) return 1;
  else if (ID == 2) return 2;
  else if (ID == 3) return 0;
  else if (ID == 4) return 1;
  else if (ID == 5) return 0;
  else cout<<"Wrong Output ID X3"<<endl;
  return -1;
}


void ComptonNetLevel3ExtendedB()
{
  cout<<"Launching Tripple Comption Scattering (ExtB) Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  // The normalizations:
  double xMin = -10.0;
  double xInt =  20.0;
  
  double yMin = -10.0;
  double yInt =  20.0;
  
  double zMin = -10.0;
  double zInt =  20.0;
  
  double cMin = -0.2;
  double cInt =  1.4;
  
  //double mMin =   0.0;
  //double mInt =  sqrt(3*100);
  
  double EMin = 0.0;
  double EInt = 500.0;
  
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(18);
  NN.SetNMiddleNodes(36);
  NN.SetNOutputNodes(6);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  
  vector<MVector> Positions;
  vector<double> Energies;
  vector<double> Ordering;
  Ordering.push_back(0.25);
  Ordering.push_back(0.751);
  Ordering.push_back(0.752);
  Ordering.push_back(0.753);
  Ordering.push_back(0.754);
  Ordering.push_back(0.755);
  
  vector<double> RandomDoubles;
  vector<int> Randoms;
  vector<MVector> RandomPositions;
  vector<double> RandomEnergies;
  vector<double> RandomOrdering;
  vector<double> RandomCosini;
  
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 100;
  int LastGood = 0;
  
  int CanBeFound = 0;
  
  double BestRatio = 0;
  
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    bool Done = true;
    do {
      ClinicalComptonSimulatorLevel1(Positions, Energies);
      
      Done = true;
      if (Energies.size() != 3) {
        Done = false;
        //cout<<"Not 3 - "<<Energies.size()<<endl;
      } else {
        for (unsigned int i = 0; i < Energies.size(); ++i) {
          if (Positions[i].X() < xMin || Positions[i].X() - xMin > xInt) Done = false;
          if (Positions[i].Y() < yMin || Positions[i].Y() - yMin > yInt) Done = false;
          if (Positions[i].Z() < zMin || Positions[i].Z() - zMin > zInt) Done = false;
          if (Done == false) {
            //cout<<"Out of range"<<endl;
          }
        }
      }
    } while (Done == false);
    
    // Add error:
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      Positions[i][0] += gRandom->Gaus(0.0, 0.1);
      Positions[i][1] += gRandom->Gaus(0.0, 0.1);
      Positions[i][2] += gRandom->Gaus(0.0, 0.1);
      Energies[i] += gRandom->Gaus(0.0, 3);
    }
    
    //cout<<"Acceptable"<<endl;
    
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      //cout<<"Positions ("<<i<<"): "<<Positions[i].X()<<", "<<Positions[i].Y()<<", "<<Positions[i].Z()<<" - "<<Energies[i]<<endl;
    }
    
    // Randomize:
    RandomDoubles.clear();
    RandomDoubles.resize(Energies.size());
    Randoms.clear();
    Randoms.resize(Energies.size());
    RandomPositions.clear();
    RandomEnergies.clear();
    RandomOrdering.clear();
    RandomCosini.clear();
    for (unsigned int i = 0; i < Energies.size(); ++i) {
      RandomDoubles[i] = gRandom->Rndm();
      //cout<<RandomDoubles[i]<<endl;
    }
    // Stupid sort
    for (unsigned int i = Energies.size()-1; i < Energies.size(); --i) {
      int iLargest = -1;
      double Largest = -1.0;
      for (unsigned int j = 0; j < Energies.size(); ++j) {
        if (RandomDoubles[j] >= Largest) {
          Largest = RandomDoubles[j];
          iLargest = j;
        }
      }
      if (iLargest != -1) {
        Randoms[iLargest] = i;
        RandomDoubles[iLargest] = -2.0;
      }
    }
    
    //Randoms[0] = 0;
    //Randoms[1] = 1;
    //Randoms[2] = 2;
    
    for (unsigned int i = 0; i < Randoms.size(); ++i) {
      RandomPositions.push_back(Positions[Randoms[i]]);
      RandomEnergies.push_back(Energies[Randoms[i]]);
      RandomOrdering.push_back(Ordering[Randoms[i]]);
    }
    
    // Calculate the test statistics... works only for 3-site events...
    int a, b, c;
    double cosphiA, cosphiE;
    for (unsigned int i = 0; i < 6; ++i) {
      a = GetOutputInX1(i);
      b = GetOutputInX2(i);
      c = GetOutputInX3(i);
      cosphiA = TMath::Cos((RandomPositions[b]-RandomPositions[a]).Angle(RandomPositions[c]-RandomPositions[b]));
      cosphiE = 1-c_E0/(RandomEnergies[c]) + c_E0/(RandomEnergies[b]+RandomEnergies[c]);
      //cout<<acos(cosphiA)<<":"<<acos(cosphiE)<<endl;
      if ((cosphiA-cosphiE)*(cosphiA-cosphiE) > 1) {
        RandomCosini.push_back(1);
      } else {
        RandomCosini.push_back((cosphiA-cosphiE)*(cosphiA-cosphiE));
      }
    }
    /*
     *		for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
     *			cout<<RandomCosini[i]<<" - ";
  }
  cout<<endl;
  for (unsigned int i = 0; i < Randoms.size(); ++i) {
    cout<<RandomOrdering[i]<<" - ";
  }
  cout<<endl;
  */
    
    bool DoesKnow = true;
    bool InputValid = true;
    
    // Learn until the result is known 
    int Try = 0;
    //do {
    Try++;
    
    DoesKnow = true;
    InputValid = true;
    
    // Set the input:
    for (unsigned int i = 0; i < RandomEnergies.size(); ++i) {
      //if (NN.SetInput(0 + 4*i, (RandomPositions[i].Mag() - mMin)/mInt) == false) InputValid = false;
      if (NN.SetInput(0 + 4*i, (RandomPositions[i].X() - xMin)/xInt) == false) InputValid = false;
      if (NN.SetInput(1 + 4*i, (RandomPositions[i].Y() - yMin)/yInt) == false) InputValid = false;
      if (NN.SetInput(2 + 4*i, (RandomPositions[i].Z() - zMin)/zInt) == false) InputValid = false;
      if (NN.SetInput(3 + 4*i, (RandomEnergies[i] - EMin)/EInt) == false) InputValid = false;
    }
    for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
      if (NN.SetInput(4*RandomEnergies.size() +i, (RandomCosini[i] - cMin)/cInt) == false) InputValid = false;
      //if (NN.SetInput(i, (RandomCosini[i] - cMin)/cInt) == false) InputValid = false;
    }
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    
    int rGood = GetIDOriginalOut(Randoms[0], Randoms[1], Randoms[2]);
    //cout<<rGood<<":"<<Randoms[0]<<":"<<Randoms[1]<<":"<<Randoms[2]<<endl;
    
    //cout<<"RandomCosini: "<<(RandomCosini[rGood] - cMin)/cInt<<endl;
    
    // Check if sequence rGood is also the smallest NN-Output:
    int nGood = 0;
    double nSmallest = 1;
    for (unsigned int i = 0; i < 6; ++i) {
      if (NN.GetOutput(i) < nSmallest) {
        nSmallest = NN.GetOutput(i);
        nGood = i;
      }
    }
    
    if (rGood != nGood) {
      DoesKnow = false;
    }
    
    
    if (Try == 1) {
      if (DoesKnow == true) {
        //cout<<"GOOOOOOOD!"<<endl;
        TotalGood++;
        LastGood++;
      } else {
        //cout<<"BAD..."<<endl;
        TotalBad++;
        LastGood = 0;
      }
      
      // Now determine how many we can identify via smallest cosini
      double SmallestAbsCos = 10000;
      int iSmallestAbsCos = -1;
      for (unsigned int i = 0; i < RandomCosini.size(); ++i) {
        if (fabs(RandomCosini[i]) < SmallestAbsCos) {
          SmallestAbsCos = fabs(RandomCosini[i]);
          iSmallestAbsCos = i;
        }
      }
      //cout<<RandomCosini[iSmallestAbsCos]<<"vs."<<RandomCosini[rGood]<<endl;
      if (iSmallestAbsCos == rGood) {
        CanBeFound++;
      }
    }
    
    // Can this be correct????
    for (unsigned int i = 0; i < 6; ++i) {
      NN.SetOutputError(i, 0.75);
    }
    NN.SetOutputError(rGood, 0.25);
    NN.Learn();
    //} while (InputValid == true && DoesKnow == false);
    
    
    
    
    if (TrainingRuns % 100 == 0) {
      if (double(TotalGood)/(TotalGood+TotalBad) > BestRatio) {
        BestRatio = double(TotalGood)/(TotalGood+TotalBad);
      }
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<") --- best: "<<BestRatio<<" can be found: "<<100.0*double(CanBeFound)/TrainingRuns<<endl;
      //cout<<NN.GetOutput(0)<<":"<<RandomOrdering[0]<<" - "<<NN.GetOutput(1)<<":"<<RandomOrdering[1]<<endl;
      //cout<<"First larger: "<<double(FirstLarger)/(FirstSmaller + FirstLarger)<<endl;
      //cout<<"First closer: "<<double(FirstCloserToZero)/(FirstCloserToZero + FirstFarerToZero)<<endl;
      //cout<<RandomPositions[0].Mag()<<":"<<RandomPositions[1].Mag()<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 1000000001) break;
  }
}

//////////////////////////////////////////////////////////////////////////

void LargerSmallerNet()
{
  cout<<"Launching Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(2);
  NN.SetNMiddleNodes(2);
  NN.SetNOutputNodes(1);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 1000;
  int LastGood = 0;
  
  //int FirstLarger = 0;
  //int FirstSmaller = 0;
  
  
  double X, Y;
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    X = gRandom->Rndm();
    Y = gRandom->Rndm();
    
    // Set the input:
    bool InputValid = true;
    
    if (NN.SetInput(0, X) == false) InputValid = false;
    if (NN.SetInput(1, Y) == false) InputValid = false;
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    bool DoesKnow = false;
    if ((X > Y && NN.GetOutput(0) > 0.5) || (X < Y && NN.GetOutput(0) < 0.5)) DoesKnow = true;
    
    if (DoesKnow == true) {
      //cout<<"GOOOOOOOD!"<<endl;
      TotalGood++;
      LastGood++;
    } else {
      //cout<<"BAD..."<<endl;
      TotalBad++;
      LastGood = 0;
    }
    
    // Can this be correct????
    if (X > Y) {
      NN.SetOutputError(0, 0.75);
    } else {
      NN.SetOutputError(0, 0.25);
    }
    NN.Learn();
    
    if (TrainingRuns % 100 == 0) {
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<")"<<endl;
      cout<<NN.GetOutput(0)<<":"<<X<<" - "<<Y<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 10000001) break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void HexNet()
{
  cout<<"Launching Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  int Size = 6;
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(Size);
  NN.SetNMiddleNodes(2*Size);
  NN.SetNOutputNodes(Size);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 1000;
  int LastGood = 0;
  
  //int FirstLarger = 0;
  //int FirstSmaller = 0;
  
  
  vector<double> In(Size);
  vector<double> Out(Size);
  
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    for (unsigned int i = 0; i < In.size(); ++i) {
      In[i] = 0.2 + 0.6*gRandom->Rndm();
    }
    
    // Find the smallest:
    double Smallest = 10000;
    int iSmallest = -1;
    for (unsigned int i = 0; i < In.size(); ++i) {
      if (In[i] < Smallest) {
        Smallest = In[i];
        iSmallest = i;
      }
    }
    
    for (unsigned int i = 0; i < Out.size(); ++i) {
      Out[i] = 0.75;
    }
    Out[iSmallest] = 0.25;
    
    
    // Set the input:
    bool InputValid = true;
    
    for (unsigned int i = 0; i < In.size(); ++i) {
      if (NN.SetInput(i, In[i]) == false) InputValid = false;
    }
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    bool DoesKnow = true;
    
    int lSmallest = -1;
    Smallest = 10000;
    for (unsigned int i = 0; i < Out.size(); ++i) {
      if (NN.GetOutput(i) < Smallest) {
        lSmallest = i;
        Smallest = NN.GetOutput(i);
      }
    }
    
    if (lSmallest != iSmallest) {
      DoesKnow = false;
    }
    //cout<<NN.GetOutput(0)<<":"<<NN.GetOutput(1)<<":"<<NN.GetOutput(2)<<endl;
    
    if (DoesKnow == true) {
      //cout<<"GOOOOOOOD!"<<endl;
      TotalGood++;
      LastGood++;
    } else {
      //cout<<"BAD..."<<endl;
      TotalBad++;
      LastGood = 0;
    }
    
    
    for (unsigned int i = 0; i < Out.size(); ++i) {
      NN.SetOutputError(i, Out[i]);
    }
    
    NN.Learn();
    
    if (TrainingRuns % 100 == 0) {
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<")"<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 10000001) break;
  }
}

///////////////////////////////////////////////////////////////////////////

void DistanceNet()
{
  cout<<"Launching Neural-Net classes..."<<endl;
  
  gStyle->SetPalette(1, 0);
  gRandom->SetSeed(0);
  
  
  MNeuralNetworkBackpropagation NN;
  NN.SetNInputNodes(6);
  NN.SetNMiddleNodes(6);
  NN.SetNOutputNodes(2);
  NN.SetLearningRate(0.4);
  NN.SetMomentum(0.4);
  NN.Create();
  
  
  // Train the network:
  int TrainingRuns = 0;
  int TotalGood = 0;
  int TotalBad = 0;
  
  int MaxGood = 100;
  int LastGood = 0;
  
  //int FirstLarger = 0;
  //int FirstSmaller = 0;
  
  
  double X1, X2, X3, Y1, Y2, Y3;
  while (LastGood < MaxGood) {
    TrainingRuns++;
    
    X1 = gRandom->Rndm();
    Y1 = gRandom->Rndm();
    X2 = gRandom->Rndm();
    Y2 = gRandom->Rndm();
    X3 = gRandom->Rndm();
    Y3 = gRandom->Rndm();
    
    // Set the input:
    bool InputValid = true;
    
    if (NN.SetInput(0, X1) == false) InputValid = false;
    if (NN.SetInput(1, Y1) == false) InputValid = false;
    if (NN.SetInput(2, X2) == false) InputValid = false;
    if (NN.SetInput(3, Y2) == false) InputValid = false;
    if (NN.SetInput(4, X3) == false) InputValid = false;
    if (NN.SetInput(5, Y3) == false) InputValid = false;
    
    // We only continue if the input was valid!
    if (InputValid == false) {
      continue;
    }
    
    NN.Run();
    
    // Check if we have learned something up to now...
    bool DoesKnow = false;
    if ((sqrt(X1*X1+X2*X2+X3*X3) > sqrt(Y1*Y1+Y2*Y2+Y3*Y3) && NN.GetOutput(0) > 0.5) || 
      (sqrt(X1*X1+X2*X2+X3*X3) < sqrt(Y1*Y1+Y2*Y2+Y3*Y3) && NN.GetOutput(0) < 0.5) ||
      (sqrt(X1*X1+X2*X2+X3*X3) > sqrt(Y1*Y1+Y2*Y2+Y3*Y3) && NN.GetOutput(1) < 0.5) || 
      (sqrt(X1*X1+X2*X2+X3*X3) < sqrt(Y1*Y1+Y2*Y2+Y3*Y3) && NN.GetOutput(1) > 0.5)) DoesKnow = true;
    
    if (DoesKnow == true) {
      //cout<<"GOOOOOOOD!"<<endl;
      TotalGood++;
      LastGood++;
    } else {
      //cout<<"BAD..."<<endl;
      TotalBad++;
      LastGood = 0;
    }
    
    // Can this be correct????
    if (sqrt(X1*X1+X2*X2+X3*X3) > sqrt(Y1*Y1+Y2*Y2+Y3*Y3)) {
      NN.SetOutputError(0, 0.75);
      NN.SetOutputError(1, 0.25);
    } else {
      NN.SetOutputError(0, 0.25);
      NN.SetOutputError(1, 0.75);
    }
    
    NN.Learn();
    
    if (TrainingRuns % 100 == 0) {
      cout<<LastGood<<" good after "<<TrainingRuns<<" test runs. Ratio: "<<double(TotalGood)/(TotalGood+TotalBad)<<" ("<<TotalGood<<", "<<TotalBad<<")"<<endl;
      gSystem->ProcessEvents();
    }
    
    if (TrainingRuns > 10000001) break;
  }
}

//////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  //   void (*handler)(int);
  // 	handler = CatchSignal;
  //   (void) signal(SIGINT, CatchSignal);
  
  //   // Initialize global MEGALIB variables, especially mgui, etc.
  //   MGlobal::Initialize();
  
  TApplication BackgroundMixerApp("BackgroundMixerApp", 0, 0);
  
  //NeuralNet();
  ComptonNetLevel3ExtendedB();
  //LargerSmallerNet();
  //HexNet();
  //DistanceNet();
  
  BackgroundMixerApp.Run();
  
  cout<<"Program exited normally!"<<endl;
  
  return 0;
}


// MResponseCreatorMain: the end...
//////////////////////////////////////////////////////////////////////////////////
