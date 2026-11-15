/*
 * UTDGrid.cxx
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


// Standard libs:
#include <cstring>
#include <new>
#include <sstream>

// POSIX libs:
// Process control has no standard C++ equivalent
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib libs:
#include "MDGrid.h"
#include "MDGridPoint.h"
#include "MString.h"
#include "MUnitTest.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the three-dimensional value grid MDGrid
class UTDGrid : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDGrid() : UTGeomegaBase("UTDGrid") {}
  //! Default destructor
  virtual ~UTDGrid() {}

  //! Run all tests
  virtual bool Run();

private:

  //! Test the type constants
  bool TestConstants();
  //! Test the construction and the initial content
  bool TestConstruction();
  //! Test that the constructors and Set handle the grid type
  bool TestType();
  //! Test the use of a default constructed grid
  bool TestDefaultGridAccess();
  //! Test the bin number validation of Set
  bool TestSetValidation();
  //! Test reusing a grid with a second Set
  bool TestSetReuse();
  //! Test the index mapping of set and get
  bool TestIndexMapping();
  //! Test the access just inside and just outside of the grid
  bool TestBoundaries();
  //! Test the access via grid points
  bool TestGridPointAccess();
  //! Test the copy constructor
  bool TestCopy();
  //! Test the stream output
  bool TestStream();
  //! Test the assignment into a populated grid and to itself
  bool TestAssignment();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDGrid::Run()
{
  bool Passed = true;

  Passed = TestConstants() && Passed;
  Passed = TestConstruction() && Passed;
  Passed = TestType() && Passed;
  Passed = TestDefaultGridAccess() && Passed;
  Passed = TestSetValidation() && Passed;
  Passed = TestSetReuse() && Passed;
  Passed = TestIndexMapping() && Passed;
  Passed = TestBoundaries() && Passed;
  Passed = TestGridPointAccess() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestAssignment() && Passed;
  Passed = TestStream() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the type constants
bool UTDGrid::TestConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_Unknown", "constant", "The unknown type is zero", MDGrid::c_Unknown, 0U) && Passed;
  Passed = Evaluate("c_Voxel", "constant", "The voxel type is one", MDGrid::c_Voxel, 1U) && Passed;
  Passed = Evaluate("c_Strip", "constant", "The strip type is two", MDGrid::c_Strip, 2U) && Passed;
  Passed = Evaluate("c_MinType", "constant", "The first valid type is the voxel", MDGrid::c_MinType, MDGrid::c_Voxel) && Passed;
  Passed = Evaluate("c_MaxType", "constant", "The last valid type is the strip", MDGrid::c_MaxType, MDGrid::c_Strip) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the construction and the initial content
bool UTDGrid::TestConstruction()
{
  bool Passed = true;

  MDGrid Single(MDGrid::c_Voxel);
  Passed = EvaluateNear("MDGrid(Type)", "default bins", "A grid with the default bin numbers has one voxel with value 0", Single.GetVoxelValue(0), 0.0, 0.0) && Passed;
  Passed = EvaluateTrue("SetVoxelValue()", "default bins, voxel 0", "The single voxel can be set", Single.SetVoxelValue(2.5, 0)) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "default bins, voxel 0", "The single voxel returns the value", Single.GetVoxelValue(0), 2.5, 0.0) && Passed;

  // All voxels start with zero:
  MDGrid Grid(MDGrid::c_Strip, 2, 3, 4);
  bool AllZero = true;
  for (unsigned int z = 0; z < 4; ++z) {
    for (unsigned int y = 0; y < 3; ++y) {
      for (unsigned int x = 0; x < 2; ++x) {
        if (Grid.GetVoxelValue(x, y, z) != 0.0) {
          AllZero = false;
        }
      }
    }
  }
  Passed = EvaluateTrue("MDGrid(Type, x, y, z)", "2 x 3 x 4", "All voxels of a new grid are zero", AllZero) && Passed;

  // Zero bins are handled as one bin:
  DisableDefaultStreams();
  MDGrid ZeroBins(MDGrid::c_Voxel, 0, 0, 0);
  bool ZeroBinsSet = ZeroBins.SetVoxelValue(4.0, 0, 0, 0);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MDGrid(Type, x, y, z)", "0 x 0 x 0", "Zero bins in the constructor result in one usable voxel", ZeroBinsSet) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "0 x 0 x 0", "The single voxel of the corrected grid returns the value", ZeroBins.GetVoxelValue(0, 0, 0), 4.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructors and Set handle the grid type
bool UTDGrid::TestType()
{
  bool Passed = true;

  // The type is private without a getter, so its traces are searched in the memory of the object
  const unsigned int Pattern = 0xABABABABU;
  const unsigned int NWords = sizeof(MDGrid)/sizeof(unsigned int);

  alignas(MDGrid) unsigned char Block[sizeof(MDGrid)];

  // The default constructor sets the type, thus only the padding keeps the pattern:
  FillMemory(Block, sizeof(MDGrid), 0xAB);
  MDGrid* Default = new (Block) MDGrid();
  unsigned int PatternWordsDefault = 0;
  for (unsigned int w = 0; w < NWords; ++w) {
    unsigned int Word = 0;
    memcpy(&Word, Block + w*sizeof(unsigned int), sizeof(unsigned int));
    if (Word == Pattern) {
      ++PatternWordsDefault;
    }
  }
  Default->~MDGrid();

  FillMemory(Block, sizeof(MDGrid), 0xAB);
  MDGrid* WithType = new (Block) MDGrid(MDGrid::c_Voxel, 7, 7, 7);
  unsigned int PatternWordsWithType = 0;
  for (unsigned int w = 0; w < NWords; ++w) {
    unsigned int Word = 0;
    memcpy(&Word, Block + w*sizeof(unsigned int), sizeof(unsigned int));
    if (Word == Pattern) {
      ++PatternWordsWithType;
    }
  }
  WithType->~MDGrid();

  // Defect GEO-E-1: The constructor with a type never initializes m_Type
  Passed = ExpectedToFail(Evaluate("MDGrid(Type, x, y, z)", "memory pattern", "The constructor with type initializes the same members as the default constructor", PatternWordsWithType, PatternWordsDefault)) && Passed;

  // Set must store the type - no pointer word or bin number can have the value of c_Strip:
  FillMemory(Block, sizeof(MDGrid), 0);
  MDGrid* Typed = new (Block) MDGrid();
  Typed->Set(MDGrid::c_Strip, 7, 7, 7);
  bool TypeFound = false;
  for (unsigned int w = 0; w < NWords; ++w) {
    unsigned int Word = 0;
    memcpy(&Word, Block + w*sizeof(unsigned int), sizeof(unsigned int));
    if (Word == MDGrid::c_Strip) {
      TypeFound = true;
    }
  }
  Typed->~MDGrid();

  // Defect GEO-E-2: Set ignores its type argument, the type is never stored
  Passed = ExpectedToFail(EvaluateTrue("Set(Type, x, y, z)", "c_Strip", "The given type is stored in the grid", TypeFound)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the use of a default constructed grid
bool UTDGrid::TestDefaultGridAccess()
{
  bool Passed = true;

  // Defect GEO-E-3: A default grid claims 1 x 1 x 1 bins but has no storage, access to voxel 0 is out of bounds
  MDGrid Read;
  Passed = ExpectedToCrash(EvaluateNear("GetVoxelValue()", "default grid, voxel 0", "Reading the only voxel of a default grid does not crash and returns 0", Read.GetVoxelValue(0, 0, 0), 0.0, 0.0)) && Passed;

  // Defect GEO-E-3: A default grid claims 1 x 1 x 1 bins but has no storage, access to voxel 0 is out of bounds
  MDGrid Written;
  Passed = ExpectedToCrash(EvaluateTrue("SetVoxelValue()", "default grid, voxel 0", "Writing the only voxel of a default grid is accepted and does not crash", Written.SetVoxelValue(1.0, 0, 0, 0))) && Passed;

  // Defect GEO-E-3: A default grid claims 1 x 1 x 1 bins but has no storage, access to voxel 0 is out of bounds
  MDGrid Streamed;
  ostringstream StreamedText;
  Passed = ExpectedToCrash(EvaluateTrue("operator<<", "default grid", "Streaming a default grid does not crash", (StreamedText<<Streamed, true))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the bin number validation of Set
bool UTDGrid::TestSetValidation()
{
  bool Passed = true;

  MDGrid Grid(MDGrid::c_Voxel);
  Passed = EvaluateTrue("Set()", "1 x 1 x 1", "The smallest valid grid is accepted", Grid.Set(MDGrid::c_Voxel, 1, 1, 1)) && Passed;
  Passed = EvaluateTrue("Set()", "3 x 2 x 2", "A representative grid is accepted", Grid.Set(MDGrid::c_Strip, 3, 2, 2)) && Passed;

  // Each single invalid dimension is rejected and replaced by one bin:
  DisableDefaultStreams();
  bool XResult = Grid.Set(MDGrid::c_Voxel, 0, 2, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Set()", "x bins 0", "Zero x bins are rejected", XResult) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "x bins 0", "The other dimensions are still set", Grid.GetVoxelValue(0, 1, 1), 0.0, 0.0) && Passed;
  DisableDefaultStreams();
  bool XOutside = Grid.SetVoxelValue(1.0, 1, 0, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetVoxelValue()", "x bins 0, x = 1", "The x dimension has been replaced by one bin", XOutside) && Passed;

  DisableDefaultStreams();
  bool YResult = Grid.Set(MDGrid::c_Voxel, 2, 0, 2);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Set()", "y bins 0", "Zero y bins are rejected", YResult) && Passed;
  DisableDefaultStreams();
  bool YOutside = Grid.SetVoxelValue(1.0, 0, 1, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetVoxelValue()", "y bins 0, y = 1", "The y dimension has been replaced by one bin", YOutside) && Passed;

  DisableDefaultStreams();
  bool ZResult = Grid.Set(MDGrid::c_Voxel, 2, 2, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Set()", "z bins 0", "Zero z bins are rejected", ZResult) && Passed;
  DisableDefaultStreams();
  bool ZOutside = Grid.SetVoxelValue(1.0, 0, 0, 1);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetVoxelValue()", "z bins 0, z = 1", "The z dimension has been replaced by one bin", ZOutside) && Passed;

  // 2^16 x 2^16 voxels do not fit into the unsigned int used for the size - only Set is called, no access:
  MDGrid Large(MDGrid::c_Voxel);
  bool LargeResult = Large.Set(MDGrid::c_Voxel, 65536, 65536, 1);
  // Defect GEO-E-4: The voxel count overflows, Set reports success for a grid without storage
  Passed = ExpectedToFail(EvaluateFalse("Set()", "65536 x 65536 x 1", "A grid whose voxel count does not fit into an unsigned int is rejected", LargeResult)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reusing a grid with a second Set
bool UTDGrid::TestSetReuse()
{
  bool Passed = true;

  // Growing in z keeps the index of existing voxels, the new voxels are zero:
  MDGrid Grow(MDGrid::c_Voxel, 2, 1, 1);
  Grow.SetVoxelValue(3.0, 1, 0, 0);
  Grow.Set(MDGrid::c_Voxel, 2, 2, 1);
  Passed = EvaluateNear("Set()", "grow, new voxel", "A voxel added by growing is zero", Grow.GetVoxelValue(1, 1, 0), 0.0, 0.0) && Passed;

  // After a new shape the old values cannot belong to the same voxels any more
  // (1, 1) of a 2 x 2 grid has the storage index 3, which is (3, 0) of a 4 x 1 grid
  MDGrid Reshape(MDGrid::c_Voxel, 2, 2, 1);
  Reshape.SetVoxelValue(5.0, 1, 1, 0);
  Reshape.Set(MDGrid::c_Voxel, 4, 1, 1);
  // Defect GEO-E-5: Set with a different shape keeps the old values and maps them to unrelated voxels
  Passed = ExpectedToFail(EvaluateNear("Set()", "reshape 2 x 2 to 4 x 1", "A value set before the reshape does not appear in the unrelated voxel (3, 0)", Reshape.GetVoxelValue(3, 0, 0), 0.0, 0.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the index mapping of set and get
bool UTDGrid::TestIndexMapping()
{
  bool Passed = true;

  // Three different prime sized dimensions: a swapped dimension or a wrong stride overwrites other voxels
  const unsigned int NX = 3;
  const unsigned int NY = 5;
  const unsigned int NZ = 7;
  MDGrid Grid(MDGrid::c_Voxel, NX, NY, NZ);

  bool AllSet = true;
  for (unsigned int z = 0; z < NZ; ++z) {
    for (unsigned int y = 0; y < NY; ++y) {
      for (unsigned int x = 0; x < NX; ++x) {
        if (Grid.SetVoxelValue(100.0*x + 10.0*y + z, x, y, z) == false) {
          AllSet = false;
        }
      }
    }
  }
  Passed = EvaluateTrue("SetVoxelValue()", "3 x 5 x 7", "All voxels inside the grid can be set", AllSet) && Passed;

  bool AllCorrect = true;
  for (unsigned int z = 0; z < NZ; ++z) {
    for (unsigned int y = 0; y < NY; ++y) {
      for (unsigned int x = 0; x < NX; ++x) {
        if (Grid.GetVoxelValue(x, y, z) != 100.0*x + 10.0*y + z) {
          AllCorrect = false;
        }
      }
    }
  }
  Passed = EvaluateTrue("GetVoxelValue()", "3 x 5 x 7", "Every voxel returns its own value", AllCorrect) && Passed;

  // The default arguments select y = 0 and z = 0:
  Passed = EvaluateNear("GetVoxelValue()", "default y and z", "The default arguments select y = 0 and z = 0", Grid.GetVoxelValue(2), 200.0, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "default z", "The default argument selects z = 0", Grid.GetVoxelValue(2, 3), 230.0, 0.0) && Passed;
  Grid.SetVoxelValue(-1.5, 1);
  Passed = EvaluateNear("SetVoxelValue()", "default y and z, negative value", "The value goes to (1, 0, 0) and may be negative", Grid.GetVoxelValue(1, 0, 0), -1.5, 0.0) && Passed;

  // Overwriting replaces and does not add:
  Grid.SetVoxelValue(9.0, 2, 4, 6);
  Grid.SetVoxelValue(11.0, 2, 4, 6);
  Passed = EvaluateNear("SetVoxelValue()", "overwrite", "A second value replaces the first one", Grid.GetVoxelValue(2, 4, 6), 11.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the access just inside and just outside of the grid
bool UTDGrid::TestBoundaries()
{
  bool Passed = true;

  MDGrid Grid(MDGrid::c_Voxel, 2, 3, 4);
  Passed = EvaluateTrue("SetVoxelValue()", "last voxel", "The voxel (n - 1) in all dimensions is valid", Grid.SetVoxelValue(6.0, 1, 2, 3)) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "last voxel", "The voxel (n - 1) in all dimensions returns the value", Grid.GetVoxelValue(1, 2, 3), 6.0, 0.0) && Passed;

  // The error path contains an massert, which ends the program when NDEBUG is not defined - then it cannot be checked in the test program:
#ifdef NDEBUG
  DisableDefaultStreams();
  bool XSet = Grid.SetVoxelValue(1.0, 2, 0, 0);
  bool YSet = Grid.SetVoxelValue(1.0, 0, 3, 0);
  bool ZSet = Grid.SetVoxelValue(1.0, 0, 0, 4);
  double XGet = Grid.GetVoxelValue(2, 2, 3);
  double YGet = Grid.GetVoxelValue(1, 3, 3);
  double ZGet = Grid.GetVoxelValue(1, 2, 4);
  double PointGet = Grid.GetVoxelValue(MDGridPoint(1, 2, 4));
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetVoxelValue()", "x = n", "The first x index outside is rejected", XSet) && Passed;
  Passed = EvaluateFalse("SetVoxelValue()", "y = n", "The first y index outside is rejected", YSet) && Passed;
  Passed = EvaluateFalse("SetVoxelValue()", "z = n", "The first z index outside is rejected", ZSet) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "x = n", "The first x index outside returns 0 and not a neighbor", XGet, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "y = n", "The first y index outside returns 0 and not a neighbor", YGet, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "z = n", "The first z index outside returns 0 and not a neighbor", ZGet, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue(MDGridPoint)", "z = n", "A grid point outside returns 0", PointGet, 0.0, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue()", "last voxel", "The rejected writes did not change the last voxel", Grid.GetVoxelValue(1, 2, 3), 6.0, 0.0) && Passed;
#endif

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the access via grid points
bool UTDGrid::TestGridPointAccess()
{
  bool Passed = true;

  MDGrid Grid(MDGrid::c_Voxel, 2, 3, 4);
  Grid.SetVoxelValue(1.25, 1, 2, 3);
  Grid.SetVoxelValue(2.25, 0, 1, 2);

  Passed = EvaluateNear("GetVoxelValue(MDGridPoint)", "(1, 2, 3)", "The grid point selects the voxel", Grid.GetVoxelValue(MDGridPoint(1, 2, 3)), 1.25, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue(MDGridPoint)", "(0, 1, 2)", "The grid point selects the voxel", Grid.GetVoxelValue(MDGridPoint(0, 1, 2)), 2.25, 0.0) && Passed;
  Passed = EvaluateNear("GetVoxelValue(MDGridPoint)", "(1, 1, 2)", "An empty voxel returns 0", Grid.GetVoxelValue(MDGridPoint(1, 1, 2)), 0.0, 0.0) && Passed;

  // The type of the point does not matter, as the guard ring and unknown points of the trigger threshold check show:
  Passed = EvaluateNear("GetVoxelValue(MDGridPoint)", "guard ring type", "Only the indices of the point are used", Grid.GetVoxelValue(MDGridPoint(1, 2, 3, MDGridPoint::c_GuardRing)), 1.25, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the copy constructor
bool UTDGrid::TestCopy()
{
  bool Passed = true;

  MDGrid Grid(MDGrid::c_Voxel, 2, 2, 2);
  Grid.SetVoxelValue(3.5, 1, 0, 1);

  MDGrid Copy(Grid);
  Passed = EvaluateNear("MDGrid(const MDGrid&)", "value", "The copy has the value", Copy.GetVoxelValue(1, 0, 1), 3.5, 0.0) && Passed;
  Passed = EvaluateTrue("MDGrid(const MDGrid&)", "last voxel", "The copy has the same dimensions", Copy.SetVoxelValue(4.5, 1, 1, 1)) && Passed;

  Passed = EvaluateNear("MDGrid(const MDGrid&)", "independence", "A change of the copy does not change the original", Grid.GetVoxelValue(1, 1, 1), 0.0, 0.0) && Passed;
  Grid.SetVoxelValue(5.5, 1, 0, 1);
  Passed = EvaluateNear("MDGrid(const MDGrid&)", "independence", "A change of the original does not change the copy", Copy.GetVoxelValue(1, 0, 1), 3.5, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the assignment into a populated grid and to itself
bool UTDGrid::TestAssignment()
{
  bool Passed = true;

  MDGrid Source(MDGrid::c_Voxel, 2, 2, 2);
  Source.SetVoxelValue(3.5, 1, 0, 1);
  MDGrid Target(MDGrid::c_Voxel, 4, 1, 1);
  Target.SetVoxelValue(9.5, 3, 0, 0);

  Target = Source;
  Passed = EvaluateNear("operator=", "value", "The assigned grid has the value", Target.GetVoxelValue(1, 0, 1), 3.5, 0.0) && Passed;
  Passed = EvaluateNear("operator=", "old value", "The old values of the grid are gone", Target.GetVoxelValue(1, 1, 1), 0.0, 0.0) && Passed;
  Passed = EvaluateTrue("operator=", "dimensions", "The assigned grid has the new dimensions", Target.SetVoxelValue(4.5, 1, 1, 1)) && Passed;
  Passed = EvaluateNear("operator=", "independence", "A change of the assigned grid does not change the source", Source.GetVoxelValue(1, 1, 1), 0.0, 0.0) && Passed;

  MDGrid& Same = Target;
  Target = Same;
  Passed = EvaluateNear("operator=", "itself", "An assignment to itself keeps the values", Target.GetVoxelValue(1, 0, 1), 3.5, 0.0) && Passed;
  Passed = EvaluateNear("operator=", "itself", "An assignment to itself keeps the other values", Target.GetVoxelValue(1, 1, 1), 4.5, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the stream output
bool UTDGrid::TestStream()
{
  bool Passed = true;

  MDGrid Grid(MDGrid::c_Voxel, 2, 1, 2);
  Grid.SetVoxelValue(1.5, 0, 0, 0);
  Grid.SetVoxelValue(2.5, 1, 0, 0);
  Grid.SetVoxelValue(3.5, 0, 0, 1);
  Grid.SetVoxelValue(4.5, 1, 0, 1);

  ostringstream Out;
  Out<<Grid;
  MString Expected = "Grid (x:2 y:1 z:2)\nz = 0\n1.5\t2.5\t\n\nz = 1\n3.5\t4.5\t\n\n";
  Passed = EvaluateTrue("operator<<", "2 x 1 x 2", "The output has the dimensions and one block per z layer in x order", MString(Out.str()) == Expected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDGrid Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDGrid.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
