#include "MERTrackFirstTwoLayers.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
using namespace std;


// MEGAlib libs:
#include "MRESE.h"
#include "MRETrack.h"
#include "MREHit.h"

#ifdef ___CLING___
ClassImp(MERTrackFirstTwoLayers)
#endif

MERTrackFirstTwoLayers::MERTrackFirstTwoLayers() : MERTrack()
{
  // Construct an instance of MERTrackFirstTwoLayers
  // printing a statement to make sure it runs (it does!)
  cout << "Running new tracker" << endl;
}

MERTrackFirstTwoLayers::~MERTrackFirstTwoLayers()
{
  // Delete this instance of MERTrackFirstTwoLayers
}   

// after creating the instance of MERTrackFirstTwoLayers, this method tracks the pairs (using existing function)
// just here to make sure that I can access the event ID and print it

// calls the TrackPairs method which is used in the MERTrack class -> MUST CHANGE FOR MY ACTUAL RECONSTRUCTION
void MERTrackFirstTwoLayers::TrackPairs(MRERawEvent* RE)
{
  cout << "Event " << RE->GetEventID() << endl;

  MERTrack::TrackPairs(RE);
}

// THIS WORKS!