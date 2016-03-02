#include <iostream>
#include <fstream>
using namespace std;

#include "TRandom.h"
#include "TMath.h"
#include "TVector3.h"

class Particle
{
  public:
  long m_ID;
  bool m_ConcurrentWithPrevious;
  short m_ParticleType;
  double m_ParticleExcitation;
  double m_Time;
  double m_StartPosX;
  double m_StartPosY;
  double m_StartPosZ;
  double m_DirX;
  double m_DirY;
  double m_DirZ;
  double m_PolX;
  double m_PolY;
  double m_PolZ;
  double m_Energy; 
  
  bool Stream(ofstream& out) {
    out<<m_ID<<" "<<m_ConcurrentWithPrevious<<" "<<m_ParticleType<<" "<<m_ParticleExcitation<<" "<<m_Time<<" "<<m_StartPosX<<" "<<m_StartPosY<<" "<<m_StartPosZ<<" "<<m_DirX<<" "<<m_DirY<<" "<<m_DirZ<<" "<<m_PolX<<" "<<m_PolY<<" "<<m_PolZ<<" "<<m_Energy<<endl;
    return true;
  };
};

void EventListCreator(bool SamePolarization = true, bool Isotropic = true)
{
  ofstream out;
  if (SamePolarization == true) {
    out.open("EventList_356keV_SamePol.dat");
  } else {
    out.open("EventList_356keV_RandomPol.dat");
  }
  
  double Flux = 1.0;
  double Time = 0;
  for (unsigned int i = 0; i < 10000000; i += 2) {
    Time += gRandom->Exp(1.0/Flux);
    
    
    TVector3 Direction;
    
    if (Isotropic == true) {
      double Theta = acos(1-2*gRandom->Rndm());
      double Phi = 2*TMath::Pi()*gRandom->Rndm();
      Direction.SetMagThetaPhi(1.0, Theta, Phi);
    } else {
      Direction.SetXYZ(0.0, 0.0, -1.0);
    }
    
    TVector3 Polarization = Direction.Orthogonal();
    Polarization.Rotate(2*TMath::Pi()*gRandom->Rndm(), Direction);
    
    Particle D1;
    D1.m_ID = i+1;
    D1.m_ConcurrentWithPrevious = false;
    D1.m_ParticleType = 1;
    D1.m_ParticleExcitation = 0;
    D1.m_Time = Time;
    D1.m_StartPosX = 0.0;
    D1.m_StartPosY = 0.0;
    D1.m_StartPosZ = 0.0;    
    D1.m_DirX = Direction[0];
    D1.m_DirY = Direction[1];
    D1.m_DirZ = Direction[2];
    D1.m_PolX = Polarization[0];
    D1.m_PolY = Polarization[1];
    D1.m_PolZ = Polarization[2];
    D1.m_Energy = 511;
    D1.Stream(out);

    if (SamePolarization == false) {
      Polarization.Rotate(2*TMath::Pi()*gRandom->Rndm(), Direction);      
    }
    
    Particle D2;
    D2.m_ID = i+2;
    D2.m_ConcurrentWithPrevious = true;
    D2.m_ParticleType = 1;
    D2.m_ParticleExcitation = 0;
    D2.m_Time = Time;
    D2.m_StartPosX = 0.0;
    D2.m_StartPosY = 0.0;
    D2.m_StartPosZ = 0.0;    
    D2.m_DirX = -Direction[0];
    D2.m_DirY = -Direction[1];
    D2.m_DirZ = -Direction[2];
    D2.m_PolX = Polarization[0];
    D2.m_PolY = Polarization[1];
    D2.m_PolZ = Polarization[2];
    D2.m_Energy = 356;
    D2.Stream(out);
  }
  
  out.close();
}
