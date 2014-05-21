#include <fstream>
#include <cstdint>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
  //! File name
  string Name = "Dummy.bsim";
  if (argc == 2) Name = argv[1];
  
  struct Interaction {
    uint64_t ID;
    float X;
    float Y;
    float Z;
    float E;
  } I;
  
  ifstream In;
  In.open(Name.c_str(), ios::binary);
  
  uint64_t LastID = 0;
  while (In.good() == true) {
    In.read(reinterpret_cast<char *>(&I), sizeof(I));
    if (In.good() == true) {
      if (I.ID != LastID) {
        cout<<endl;
      }
      cout<<I.ID<<": ("<<I.X<<", "<<I.Y<<", "<<I.Z<<")";
      if (I.E > 0.0) {
        cout<<" with "<<I.E<<" keV of continuing photon"<<endl;
      } else {
        cout<<" stopped"<<endl; 
      }
      LastID = I.ID;
    }
  }
  
  return 0;
}