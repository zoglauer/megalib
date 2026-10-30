/*
 * Ring.C
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


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
