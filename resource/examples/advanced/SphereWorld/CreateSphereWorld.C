#include "Parameters.h"

void CreateSphereWorld()
{
  cell_LineOnly = false;
  
  double m_GapHeight = (cell_MaxHeight - cell_MinHeight) / (cell_NBinsHeight - 1);
  double m_GapDistance = (cell_MaxDistance - cell_MinDistance) / (cell_NBinsDistance - 1);
  
  ofstream out;
  out.open("SphereWorld.geo.setup");

  out<<"SurroundingSphere 1 0 0 0 1"<<endl;
  
  out<<"Material Air"<<endl;
  out<<"Air.Density                     1.2041E-03"<<endl;
  out<<"Air.ComponentByMass             N  0.7556"<<endl;
  out<<"Air.ComponentByMass             O  0.2315"<<endl;
  out<<"Air.ComponentByMass             Ar 0.0129 "<<endl;

  out<<"Volume World"<<endl;
  out<<"World.Material Air"<<endl;
  out<<"World.Shape BOX "<<2*cell_MaxDistance<<" "<<2*cell_MaxDistance<<" "<<2*cell_MaxDistance<<endl;
  out<<"World.Position 0 0 0"<<endl;
  out<<"World.Mother 0"<<endl;
  
  out<<"Volume Sphere"<<endl;
  out<<"Sphere.Material Air"<<endl;
  out<<"Sphere.Shape Sphere 0.0 "<<cell_Radius<<endl;
  out<<"Sphere.Visibility 1"<<endl;
  out<<"Sphere.Color 2"<<endl;
  
  out<<"Volume Plane"<<endl; 
  out<<"Plane.Material Air"<<endl;
  out<<"Plane.Shape TUBS 0.0 { "<<cell_MaxDistance<<" + "<<cell_Radius<<" } "<<cell_Radius<<endl;
  out<<"Plane.Visibility 0"<<endl;
  out<<"Plane.Color 3"<<endl;
  
  for (unsigned int d = 0; d < cell_NBinsDistance; ++d) {
    double TorusRadius = d*m_GapDistance;
    out<<"Volume AlmostTorus_"<<d<<endl; 
    out<<"AlmostTorus_"<<d<<".Material Air"<<endl;
    if (d == 0) {
      out<<"AlmostTorus_"<<d<<".Shape TUBS "<<0<<"  "<<cell_Radius<<" "<<cell_Radius<<endl;      
    } else {
      out<<"AlmostTorus_"<<d<<".Shape TUBS "<<TorusRadius - cell_Radius<<"  "<<TorusRadius + cell_Radius<<" "<<cell_Radius<<endl;
    }
    out<<"AlmostTorus_"<<d<<".Visibility 0"<<endl;
    out<<"AlmostTorus_"<<d<<".Color 4"<<endl;
    out<<"AlmostTorus_"<<d<<".Position 0 0 0"<<endl;
    out<<"AlmostTorus_"<<d<<".Mother Plane"<<endl;
    
    double Radius = 2* TorusRadius * TMath::Pi();
    int Spheres = Radius / (2*cell_Radius);
    Spheres -= 1; // Safety to avoid overlaps
    if (Spheres < 1) Spheres = 1;
    cout<<"Spheres at d="<<d<<": "<<Spheres<<endl;
    // Now the test spheres
    if (d == 0) {
      out<<"Sphere.Copy Sphere_0_0"<<endl;
      out<<"Sphere_0_0.Position 0 0 0"<<endl;
      out<<"Sphere_0_0.Mother AlmostTorus_0"<<endl;
    } else {
      for (unsigned int s = 0; s < Spheres; ++s) {
        //if (s != 0) continue;
        if (cell_LineOnly == true && s > 0) continue;
        double Angle = s * 2.0 * TMath::Pi()/Spheres;
        out<<"Sphere.Copy Sphere_"<<d<<"_"<<s<<endl;
        out<<"Sphere_"<<d<<"_"<<s<<".Position "<<TorusRadius * cos(Angle)<<" "<<TorusRadius * sin(Angle)<<" 0"<<endl;
        out<<"Sphere_"<<d<<"_"<<s<<".Mother AlmostTorus_"<<d<<endl;
      }
    } 
  }
  
  for (unsigned int h = 0; h < cell_NBinsHeight; ++h) {
    out<<"Plane.Copy Plane_"<<h<<endl;
    out<<"Plane_"<<h<<".Position 0 0 "<<cell_MinHeight + h*m_GapHeight<<endl;
    out<<"Plane_"<<h<<".Mother World"<<endl;
  }
  
  out.close();
}
  
  
