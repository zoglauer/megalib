void LightCurve()
{
  double TimeMax = 201.0;
  TH1D* LightCurve = new TH1D("LC", "LC", TimeMax, 0.0, TimeMax);

  unsigned int Sources = 5+15*gRandom->Rndm();
  for (unsigned int s = 0; s < Sources; ++s) {
    double Sigma = 1+10*gRandom->Rndm();
    double Counts = 500+5000*gRandom->Rndm();
    double Mean = TimeMax*gRandom->Rndm();
    
    for (unsigned int c = 0; c < Counts; ++c) {
      double Value = gRandom->Gaus(Mean, Sigma);
      while (Value < 0) Value += TimeMax;
      while (Value > TimeMax) Value -= TimeMax;
      LightCurve->Fill(Value);
    } 
  }
  
  ofstream off;
  off.open("Lightcurve.dat");
  off<<"IP LinLin"<<endl;
  off<<endl;
  for (unsigned int l = 1; l <= LightCurve->GetXaxis()->GetNbins(); ++l) {
    off<<"DP "<<LightCurve->GetBinCenter(l)-LightCurve->GetBinCenter(1)<<"  "<<LightCurve->GetBinContent(l)<<endl;
  }
  off<<"EN"<<endl;
  off.close();

  TCanvas* LightCurveCanvas = new TCanvas();
  LightCurveCanvas->cd();
  LightCurve->Draw();
  LightCurveCanvas->Update();
  
  double SimTime = 2.5*TimeMax;
  TH1D* SimulatedLightCurve = new TH1D("Simulated LC", "Simulated LC", SimTime, 0.0, SimTime);  
  
  double NCounts = 50000;
  
  double Integral = LightCurve->Integral();
  double LightCurveScaler = NCounts/Integral; 
  unsigned int Cycles = 0; 
  double Time = 1E-14;
  double LastTimeBoundary = 0;
  int TimeBin = 0;
  double iIntegral = 0.0;

  do {
    double dIntegral = gRandom->Exp(1.0);
    while (LightCurveScaler*LightCurve->Integral(1, TimeBin+1) + Cycles*LightCurveScaler*LightCurve->Integral() < iIntegral + dIntegral) {
      TimeBin++;
      LastTimeBoundary += LightCurve->GetBinWidth(TimeBin);
      if (TimeBin == LightCurve->GetXaxis()->GetNbins()) {
        TimeBin = 0;
        Cycles++;
      }
    }
    //cout<<"TimeBin: "<<TimeBin<<" of "<<LightCurve->GetXaxis()->GetNbins()<<endl;
    
    double LightCurveIntegral = LightCurveScaler*LightCurve->Integral(1, TimeBin) + Cycles*LightCurveScaler*LightCurve->Integral();
    if (TimeBin == 0) LightCurveIntegral = 0 + Cycles*LightCurveScaler*LightCurve->Integral();
    // The partial dT
    //cout<<"Partial calculation: ("<<iIntegral<<"+"<<dIntegral<<"-"<<LightCurveIntegral<<")/"<<LightCurve->GetBinContent(TimeBin+1)<<endl;
    double Partial = (iIntegral + dIntegral - LightCurveIntegral)/(LightCurveScaler*LightCurve->GetBinContent(TimeBin+1));
    Time = Partial*LightCurve->GetBinWidth(TimeBin+1) + LastTimeBoundary;
    
    iIntegral += dIntegral;

    SimulatedLightCurve->Fill(Time);
  } while (Time < SimTime);
  
  
  TCanvas* SimulatedLightCurveCanvas = new TCanvas();
  SimulatedLightCurveCanvas->cd();
  SimulatedLightCurve->Draw();
  SimulatedLightCurveCanvas->Update();
}
