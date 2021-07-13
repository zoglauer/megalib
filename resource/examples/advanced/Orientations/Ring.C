{
	ofstream out;
	out.open("Ring.ori");
	
	out<<"Type OrientationsLocal"<<endl;
	
	for (unsigned int i = 0; i <= 360; i += 1) {
    double t = 2.0*TMath::Pi() * i / 360.0;
    
		TVector3 Pos;
		Pos.SetXYZ(10.0*cos(t), 10.0*sin(t), 15.0);
	
    TVector3 DirX;
		DirX = -Pos;
		DirX[2] = 0.0;

    TVector3 DirZ;
		DirZ.SetXYZ(0.0, 0.0, 1.0);
	
	  out<<"OL "<<i / 360.0<<" "<<Pos.X()<<" "<<Pos.Y()<<" "<<Pos.Z()<<"   "<<DirX.Theta()*TMath::RadToDeg()<<" "<<DirX.Phi()*TMath::RadToDeg()<<"   "<<DirZ.Theta()*TMath::RadToDeg()<<" "<<DirZ.Phi()*TMath::RadToDeg()<<endl;
	}
	out<<"EN"<<endl;
	
	out.close();
}
