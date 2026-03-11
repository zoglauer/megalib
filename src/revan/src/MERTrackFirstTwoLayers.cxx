#include "MERTrackFirstTwoLayers.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
using namespace std;


// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MREHit.h"

#ifdef ___CLING___
ClassImp(MERTrackFirstTwoLayers)
#endif

MERTrackFirstTwoLayers::MERTrackFirstTwoLayers() : MERTrack()
{
  // Construct an instance of MERTrackFirstTwoLayers
}

MERTrackFirstTwoLayers::~MERTrackFirstTwoLayers()
{
  // Delete this instance of MERTrackFirstTwoLayers
}   