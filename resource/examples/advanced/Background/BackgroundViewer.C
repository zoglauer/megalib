#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
using namespace std;

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"

TGraph* Load(TString FileName)
{
  vector<double> Energy;
  vector<double> Flux;
  
  ifstream in;
  in.open(FileName);
  if (in.is_open() == false) {
    cout<<"Unable to load file "<<FileName<<endl;
    return 0;
  }
  
  string line;
  while (getline(in, line)) {
    
    if (line.size() < 2) continue;
    if (line[0] != 'D' || line[1] != 'P') continue; 
    line = line.substr(2);
    
    istringstream iss(line);

    double E = 0;
    double F = 0;
    
    iss >> E;
    iss >> F;
  
    if (iss.bad()) break;
    
    Energy.push_back(E);
    Flux.push_back(1000*10000*F);
  }

  TGraph* G = new TGraph(Energy.size()-2, &Energy[0], &Flux[0]);
  return G;
};

void BackgroundViewer()
{
  vector<TString> FileNames;
  vector<TString> Names;
  vector<Color_t> Colors;
  FileNames.push_back("CosmicPhotonsGruber.spectrum.dat"); Names.push_back("Cosmic photons"); Colors.push_back(kRed);
  FileNames.push_back("CosmicProtonsSpenvis.spectrum.dat"); Names.push_back("Cosmic protons"); Colors.push_back(kGreen);
  FileNames.push_back("CosmicAlphasSpenvis.spectrum.dat"); Names.push_back("Cosmic alphas"); Colors.push_back(kOrange-7);
  FileNames.push_back("CosmicElectronsMizuno.spectrum.dat"); Names.push_back("Cosmic electrons"); Colors.push_back(kYellow);
  FileNames.push_back("CosmicPositronsMizuno.spectrum.dat"); Names.push_back("Cosmic positrons"); Colors.push_back(kPink);
  FileNames.push_back("AlbedoPhotonsTuerlerMizunoAbdo.spectrum.dat"); Names.push_back("Albedo photons"); Colors.push_back(kBlue);
  FileNames.push_back("AlbedoProtonMizuno.spectrum.dat"); Names.push_back("Albedo protons"); Colors.push_back(kOrange);
  FileNames.push_back("AlbedoNeutronsKole.spectrum.dat"); Names.push_back("Albedo neutrons"); Colors.push_back(kMagenta);
  FileNames.push_back("AlbedoElectronsAlcarazMizuno.spectrum.dat"); Names.push_back("Albedo electrons"); Colors.push_back(kCyan);
  FileNames.push_back("AlbedoPositronsAlcarazMizuno.spectrum.dat"); Names.push_back("Albedo positrons"); Colors.push_back(kAzure-4);
  FileNames.push_back("TrappedProtonsSpenvis.spectrum.dat"); Names.push_back("Trapped protons (orbit averaged)"); Colors.push_back(kGray);
  FileNames.push_back("TrappedElectronsSpenvis.spectrum.dat"); Names.push_back("Trapped electrons (orbit averaged)"); Colors.push_back(kGray+2);

  vector<TGraph*> AllGraphs;
  for (unsigned int i = 0; i < FileNames.size(); ++i) {
    AllGraphs.push_back(Load(FileNames[i]));
    AllGraphs.back()->SetLineColor(Colors[i]);
    AllGraphs.back()->SetMarkerColor(Colors[i]);
    AllGraphs.back()->SetLineWidth(3);
  }
  
  TGraph* Dimensions = new TGraph(2);
  Dimensions->SetPoint(0, 10, 10000000);
  Dimensions->SetPoint(1, 1000000000, 0.00000001); 
 
  TCanvas* C = new TCanvas();
  C->cd();
  C->SetLogx();
  C->SetLogy();
  C->SetGridx();
  C->SetGridy();
  Dimensions->Draw("A*");
  Dimensions->GetHistogram()->SetXTitle("Energy [keV]");
  Dimensions->GetHistogram()->SetYTitle("Flux [ph/(m^2 s MeV sr)");
  
  for (unsigned int i = 0; i < AllGraphs.size(); ++i) {
    AllGraphs[i]->Draw("SAME L");
  }
  
  TLegend* L = new TLegend(0.65,0.65,0.88,0.88);
  L->SetHeader("Background components");
  for (unsigned int i = 0; i < AllGraphs.size(); ++i) {
    L->AddEntry(AllGraphs[i], Names[i], "LP");
  }
  L->Draw("SAME");
  
  
  C->Update();
 

}
