// This file creates a cosima source file with a certain number of 
// sources in a hexagonal pattern
// It can be used for source resolution tests...

// XYSource must be an uneven number or it will be reduced by one
// The minimum for XYSource is 3

void SourceResolution(double Distance = 1.8, int XYSource = 11, TString GeometryFileName = "$MEGALIB/resource/examples/geomega/mpesatellitebaseline/SatelliteWithACS.geo.setup", double Time = 1000.0) 
{
  if (XYSource % 2 == 0) XYSource -= 1;
  if (XYSource < 3) XYSource = 3;

  int XYSourceHalf = (XYSource-1)/2;

  double DistanceRad = Distance*TMath::DegToRad();
  double Epsilon = DistanceRad/10000;
  double LayerDistance = DistanceRad * sin(TMath::Pi()/3); 

  double XValue;
  double YValue;

  vector<double> XStore;
  vector<double> YStore;

  for (int y = -XYSourceHalf; y <= XYSourceHalf; ++y) {
    XValue = -XYSourceHalf*DistanceRad;
    if (y % 2 != 0) {
      XValue += 0.5*DistanceRad;
    }
    do {
      XStore.push_back(XValue);
      YStore.push_back(y*LayerDistance);
      XValue += DistanceRad;
    } while (XValue <= XYSourceHalf*DistanceRad + Epsilon);
  }

  /*
  TH2D* Test = new TH2D("Test", "Test", 100, -1.1*XYSourceHalf*DistanceRad, +1.1*XYSourceHalf*DistanceRad, 100, -1.1*XYSourceHalf*DistanceRad, +1.1*XYSourceHalf*DistanceRad);
  for (unsigned int i = 0; i < XStore.size(); ++i) {
    Test->Fill(XStore[i], YStore[i]);
  }
  TCanvas* TestCanvas = new TCanvas("TestCanvas", "TestCanvas", 600, 600);
  TestCanvas->cd();
  Test->Draw("colz");
  TestCanvas->Update();
  */

  // Now convert to theta and phi 
  vector<double> Theta;
  vector<double> Phi;

  for (unsigned int i = 0; i < XStore.size(); ++i) {
    Theta.push_back(sqrt(XStore[i]*XStore[i] + YStore[i]*YStore[i]));
    Phi.push_back(atan2(YStore[i], XStore[i]));      
  }

  TString DistanceTag = "_";
  DistanceTag += int(100*Distance);

  // Now create a dummy Source file
  ofstream out;
  out.open(TString("SourceResolution") + DistanceTag + TString(".source"));
  
  out<<"Version         1"<<endl;
  out<<"Geometry        "<<GeometryFileName<<endl;
  out<<endl;
  out<<"PhysicsListEM                        Livermore"<<endl;
  out<<"PhysicsListEMActivateFluorescence    false"<<endl;
  out<<endl;
  out<<"StoreCalibrated                      true"<<endl;
  out<<"StoreSimulationInfo                  true"<<endl;
  out<<"StoreSimulationInfoIonization        false"<<endl;
  out<<"DiscretizeHits                       true"<<endl;
  out<<endl;
  out<<"Run SourceResolution"<<endl;
  out<<"SourceResolution.FileName         SourceResolution"<<DistanceTag<<endl;
  out<<"SourceResolution.Time             "<<Time<<endl;
  out<<endl;
  for (unsigned int i = 0; i < Theta.size(); ++i) {
    out<<"SourceResolution.Source Source_"<<i+1<<endl;
    out<<"Source_"<<i+1<<".ParticleType           1"<<endl;
    out<<"Source_"<<i+1<<".Beam                   FarFieldPointSource "<<Theta[i]*TMath::RadToDeg()<<" "<<Phi[i]*TMath::RadToDeg()<<endl;
    out<<"Source_"<<i+1<<".Spectrum               Mono 1000"<<endl;
    out<<"Source_"<<i+1<<".Flux                   1"<<endl;
    out<<endl;
  }

  out.close();

}



