#include <fstream>
#include <iostream>
using namespace std;

int main()
{
  double xyMin = -4.0;
  double xyMax =  4.0;
  double zMin  = -1.0;
  double zMax  =  1.0;

  int xyBins = 20;
  int zBins = 20;

  double xyDist = (xyMax - xyMin)/(xyBins-1);
  double zDist = (zMax - zMin)/(zBins-1);

  ofstream fout;
  fout.open("EnergyResolutionTester.LossMap.dat");
  
  fout<<"XA ";
  for (int x = 0; x < xyBins; ++x) {
    fout<<x*xyDist+xyMin<<" ";
  }
  fout<<endl;
  
  fout<<"YA ";
  for (int x = 0; x < xyBins; ++x) {
    fout<<x*xyDist+xyMin<<" ";
  }
  fout<<endl;
  
  fout<<"ZA ";
  for (int x = 0; x < xyBins; ++x) {
    fout<<x*zDist+zMin<<" ";
  }
  fout<<endl;

  double Value;
  for (int x = 0; x < xyBins; ++x) {
    for (int y = 0; y < xyBins; ++y) {
      for (int z = 0; z < zBins; ++z) {
        Value = -1.0/(50*((z*zDist+zMin)+1.01))+0.95;
        if (Value < 0.0) Value = 0.01;
        fout<<"DP "<<x*xyDist+xyMin<<" "<<y*xyDist+xyMin<<" "<<z*zDist+zMin<<" "<<Value<<endl;;
      }
    }
  }

  fout<<"EN"<<endl;

  fout.close();

  return 0.0;
}
