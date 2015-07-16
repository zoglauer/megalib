void Spherical()
{
  int ThetaBins = 45;
  int PhiBins = 90;
  int EnergyBins = 22;

  ofstream out;
  out.open("3DSpherical.dat");

  // Create axes:
  out<<"TA ";
  double ThetaDist = 180.0/ThetaBins;
  for (unsigned int b = 0; b <= ThetaBins; ++b) {
    out<<b*ThetaDist<<" ";
  }
  out<<endl;

  out<<"PA ";
  double PhiDist = 360.0/PhiBins;
  for (unsigned int b = 0; b <= PhiBins; ++b) {
    out<<b*PhiDist<<" ";
  }
  out<<endl;

  out<<"EA ";
  double EnergyMin = 500;
  double EnergyMax = 522;
  double EnergyDist = (EnergyMax-EnergyMin)/EnergyBins; 
  for (unsigned int b = 0; b <= EnergyBins; ++b) {
    out<<EnergyMin + b*EnergyDist<<" ";
  }
  out<<endl;

  double TotalFlux = 1.0; // ph/cm2/s 
  for (unsigned int t = 0; t <= ThetaBins; ++t) {
    double ThetaMin = (double(t)-0.5)*ThetaDist;
    if (ThetaMin < 0) ThetaMin = 0.0;
    double ThetaMax = (double(t)+0.5)*ThetaDist;
    if (ThetaMax > 180) ThetaMax = 180.0;
    for (unsigned int p = 0; p <= PhiBins; ++p) {
      double Area = PhiDist*TMath::DegToRad()*(cos(ThetaMin*TMath::DegToRad()) - cos(ThetaMax*TMath::DegToRad()));
      cout<<Area<<endl;
      for (unsigned int e = 0; e <= EnergyBins; ++e) {
        //double Flux = TotalFlux/Area/EnergyDist;
        double Flux = TotalFlux/4/TMath::Pi()/(EnergyMax-EnergyMin);
        out<<"AP "<<p<<" "<<t<<" "<<e<<" "<<Flux<<endl;
      }
    }
  }

  out.close();

}


